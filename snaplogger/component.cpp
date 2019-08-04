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
#include    "snaplogger/private_logger.h"


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



component::pointer_t        g_debug_component(get_component(COMPONENT_DEBUG));
component::pointer_t        g_normal_component(get_component(COMPONENT_NORMAL));
component::pointer_t        g_secure_component(get_component(COMPONENT_SECURE));


/** \brief Create a new component.
 *
 * \warning
 * DO NOT CREATE A COMPONENT DIRECTLY.
 *
 * Please use the get_component() function whenever you want to create
 * a new component. If it already exists, then the existing one will
 * be returned.
 *
 * It is not possible to register the same component more than once.
 * The get_component() function makes sure that won't happen.
 *
 * \param[in] name  The name of the new component.
 */
component::component(std::string const & name)
    : f_name(name)
{
}


std::string const & component::get_name() const
{
    return f_name;
}


component::pointer_t get_component(std::string const & name)
{
    return get_private_logger()->get_component(name);
}


component::pointer_t get_component(message const & msg, std::string const & name)
{
    return get_private_logger(msg)->get_component(name);
}


} // snaplogger namespace
// vim: ts=4 sw=4 et
