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
 * \brief Implementation of the components.
 *
 * This file implements the component base classes.
 */

// self
//
#include    "snaplogger/component.h"
#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"


// snapdev lib
//
#include    <snapdev/not_used.h>


// C++ lib
//
#include    <map>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{


typedef std::map<std::string, component::pointer_t>        component_map_t;

component_map_t     g_components;


}
// no name namespace


component::pointer_t        debug_component(component::get_component("debug"));
component::pointer_t        normal_component(component::get_component("normal"));
component::pointer_t        secure_component(component::get_component("secure"));


component::component(std::string const & name)
    : f_name(name)
{
    if(g_components.find(name) != g_components.end())
    {
        throw duplicate_component(
                  "component \""
                + name
                + "\" already exists, it can only be registered once.");
    }

    g_components[name] = this;
}


component::pointer_t component::get_component(std::string const & name)
{
    guard g;

    auto it(g_components.find(name));
    if(it == g_components.end())
    {
        component::pointer_t c(new component(name));
        return c;
    }

    return it->second;
}


std::string const & component::get_name() const
{
    return f_name;
}


} // snaplogger namespace
// vim: ts=4 sw=4 et
