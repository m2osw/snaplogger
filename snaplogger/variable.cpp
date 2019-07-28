/*
 * License:
 *    Copyright (c) 2013-2019  Made to Order Software Corp.  All Rights Reserved
 *
 *    https://snapwebsites.org/
 *    contact@m2osw.com
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors:
 *    Alexis Wilke   alexis@m2osw.com
 */

/** \file
 * \brief Variables are used to dynamically add parameters to log messages.
 *
 * This file declares the base variable class.
 *
 * The format defines \em functions which are written as in
 * `${function-name}`.
 *
 * Parameters can be passed to these functions by adding `:<param>`
 * to those definitions. These are named parameters and their default
 * value is "present" or not. A specific value can be assignd using
 * the equal sign as in `:param=<value>`.
 *
 * For example, the date function can be called as follow:
 *
 * \code
 *     ${date:year:align=right:exact_width=2}
 * \endcode
 *
 * The `year` parameter is specific to the `date` function. The other
 * parameters are available whatever the function. This variable asks
 * to truncate the year to 2 character right aligned (i.e. "18" in
 * "2018".)
 *
 * In C, this would look something like:
 *
 * \code
 *     date(FLAG_YEAR, ALIGN_RIGHT, 2);
 * \endcode
 */


// self
//
#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/variable.h"


// libutf8 lib
//
#include    <libutf8/libutf8.h>


// C++ lib
//
#include    <iostream>
#include    <queue>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{




typedef std::map<std::string, function *>   function_map_t;

function_map_t *                            g_functions = nullptr;



typedef std::map<std::string, variable_factory *>       variable_factory_map_t;

variable_factory_map_t *         g_variable_factories = nullptr;

DEFINE_LOGGER_VARIABLE(direct)

void direct_variable::process_value(
                  message const & msg
                , std::string & value) const
{
    snap::NOTUSED(msg);

    // apply all our parameters as is
    //
    auto const & params(get_params());
    for(auto p : params)
    {
        value += p->get_value();
    }

    // do NOT apply parameters further, the user has no access to those
    // anyway; this is the direct text we find in between variables
    //
    //variable::process_value(msg, value);
}



}
// no name namespace



param::param(std::string const & name)
    : f_name(name)
{
}


std::string const & param::get_name() const
{
    return f_name;
}


param::type_t param::get_type() const
{
    return f_type;
}


std::string const & param::get_value() const
{
    if(f_type != type_t::TYPE_STRING)
    {
        // TBD: we may instead want to return the integer as a string
        //
        throw invalid_parameter(
                  "the ${...:"
                + f_name
                + "=<value>} parameter must be a valid string (not an integer).");
    }
    return f_value;
}


void param::set_value(std::string const & value)
{
    f_value = value;
    f_type = type_t::TYPE_STRING;
}


int64_t param::get_integer() const
{
    if(f_type != type_t::TYPE_INTEGER)
    {
        // TBD: we may want to check whether the string represents a valid
        //      integer first and return that if so
        //
        throw invalid_parameter(
                  "the ${...:"
                + f_name
                + "=<value>} parameter must be a valid integer.");
    }
    return f_integer;
}


void param::set_integer(std::int64_t integer)
{
    f_integer = integer;
    f_type = type_t::TYPE_INTEGER;
}





variable::~variable()
{
}


void variable::add_param(param::pointer_t p)
{
    f_params.push_back(p);
}


param::vector_t const & variable::get_params() const
{
    return f_params;
}


std::string variable::get_value(message const & msg) const
{
    std::string value;
    process_value(msg, value);
    return value;
}


void variable::process_value(message const & msg, std::string & value) const
{
    {
        guard g;

        if(g_functions == nullptr)
        {
std::cerr << "-------------- no funcs?!\n";
            // no functions available, we're done
            return;
        }
    }

    function_data d;
    d.set_value(value);

    for(auto p : f_params)
    {
        std::string const & name(p->get_name());
        if(name.empty())
        {
            continue;
        }

        auto it(g_functions->find(name));
        if(it != g_functions->end())
        {
            it->second->apply(msg, d, p);
        }
    }

    value = libutf8::to_u8string(d.get_value());
}







variable_factory::variable_factory(std::string const & type)
{
    if(g_variable_factories == nullptr)
    {
        g_variable_factories = new variable_factory_map_t;
    }

    (*g_variable_factories)[type] = this;
}


variable_factory::~variable_factory()
{
}








variable::pointer_t get_variable(std::string const & type)
{
    if(g_variable_factories == nullptr)
    {
        throw invalid_variable("No variable factories were registered yet; you can't create variable with type \""
                             + type
                             + "\".");
    }

    auto it(g_variable_factories->find(type));
    if(it == g_variable_factories->end())
    {
        // TBD: should we instead return a null var.?
        throw invalid_variable("You can't create variable with type \""
                             + type
                             + "\", no such variable type was registered.");
    }

    return it->second->create_variable();
}






function::function(std::string const & function_name)
{
    guard g;

    if(g_functions == nullptr)
    {
        g_functions = new function_map_t();
    }

    (*g_functions)[function_name] = this;
}


function::~function()
{
}




} // snaplogger namespace
// vim: ts=4 sw=4 et
