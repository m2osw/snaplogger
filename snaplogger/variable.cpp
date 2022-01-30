// Copyright (c) 2013-2022  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/snaplogger
// contact@m2osw.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

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
#include    "snaplogger/variable.h"

#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/private_logger.h"


// libutf8 lib
//
#include    <libutf8/exception.h>
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





DEFINE_LOGGER_VARIABLE(direct)
{
    snapdev::NOT_USED(msg);

    // insert all our parameters as is
    //
    auto const & params(get_params());
    for(auto p : params)
    {
        // TODO: should we add a space too? or can we have spaces in the params?
        value += p->get_value();
    }

    // do NOT apply parameters further, the user has no access to those
    // anyway; this is the direct text we find in between variables
    //
    //variable::process_value(msg, value);
}



}
// no name namespace



//////////////////////////////
// PARAM
//

param::param(std::string const & name)
    : f_name(name)
{
    if(f_name.empty())
    {
        throw invalid_parameter("a parameter must have a non-empty name.");
    }
}


std::string const & param::get_name() const
{
    return f_name;
}


param::type_t param::get_type() const
{
    return f_type;
}


std::string param::get_value() const
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





//////////////////////////////
// VARIABLE
//

variable::~variable()
{
}


bool variable::ignore_on_no_repeat() const
{
    return false;
}


void variable::add_param(param::pointer_t p)
{
    guard g;

    f_params.push_back(p);
}


param::vector_t variable::get_params() const
{
    guard g;

    return f_params;
}


std::string variable::get_value(message const & msg) const
{
    guard g;

    std::string value;
    process_value(msg, value);
    return value;
}


void variable::process_value(message const & msg, std::string & value) const
{
    auto l(get_private_logger(msg));

    {
        guard g;

        if(!l->has_functions())
        {
            // no functions available, we're done
            return;     // LCOV_EXCL_LINE
        }
    }

    function_data d;
    try
    {
        d.set_value(value);
    }
    catch(libutf8::libutf8_exception_decoding const & e)
    {
        // ignore this exception because an invalid byte in a message is
        // something somewhat common and here it just means the functions
        // won't be applied, not that this specific log fails 100%
        //
        value += " {WARNING: your value has invalid UTF-8 characters; do you use std::int8_t or std::uint8_t? those are often inserted as characters instead of numbers; exception message: \"";
        value += e.what();
        value += "\"} ";
        return;
    }

    for(auto p : f_params)
    {
        std::string const & name(p->get_name());
        auto func(l->get_function(name));
        if(func != nullptr)
        {
            func->apply(msg, d, p);
        }
        // else -- ignore missing functions
    }

    value = libutf8::to_u8string(d.get_value());
}











//////////////////////////////
// VARIABLE FACTORY
//

variable_factory::variable_factory(std::string const & type)
    : f_type(type)
{
}


variable_factory::~variable_factory()
{
}


std::string const & variable_factory::get_type() const
{
    return f_type;
}




void register_variable_factory(variable_factory::pointer_t factory)
{
    get_private_logger()->register_variable_factory(factory);
}



variable::pointer_t get_variable(std::string const & type)
{
    return get_private_logger()->get_variable(type);
}







//////////////////////////////
// FUNCTION DATA
//

void function_data::set_value(std::string value)
{
    f_value = libutf8::to_u32string(value);
}

void function_data::set_value(std::u32string value)
{
    f_value = value;
}

std::u32string & function_data::get_value()
{
    return f_value;
}

void function_data::set_param(std::string const & name, std::string const & value)
{
    f_params[name] = libutf8::to_u32string(value);
}

void function_data::set_param(std::string const & name, std::u32string const & value)
{
    f_params[name] = value;
}

std::u32string function_data::get_param(std::string const & name, std::u32string const & default_value)
{
    auto it(f_params.find(name));
    if(it == f_params.end())
    {
        return default_value;
    }
    return it->second;
}





//////////////////////////////
// FUNCTION
//

function::function(std::string const & function_name)
    : f_name(function_name)
{
}


function::~function()
{
}


std::string const & function::get_name() const
{
    return f_name;
}



void register_function(function::pointer_t func)
{
    guard g;

    get_private_logger()->register_function(func);
}




} // snaplogger namespace
// vim: ts=4 sw=4 et
