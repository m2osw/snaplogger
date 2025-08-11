// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/snaplogger
// contact@m2osw.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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


// snapdev
//
#include    <snapdev/not_used.h>


// C++
//
#include    <map>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



component::pointer_t        g_as2js_component(get_component(COMPONENT_AS2JS));
component::pointer_t        g_cppthread_component(get_component(COMPONENT_CPPTHREAD));
component::pointer_t        g_clog_component(get_component(COMPONENT_CLOG));
component::pointer_t        g_debug_component(get_component(COMPONENT_DEBUG));
component::pointer_t        g_normal_component(get_component(COMPONENT_NORMAL));
component::pointer_t        g_secure_component(get_component(COMPONENT_SECURE, { g_normal_component }));
component::pointer_t        g_self_component(get_component(COMPONENT_SELF));
component::pointer_t        g_banner_component(get_component(COMPONENT_BANNER));
component::pointer_t        g_not_implemented_component(get_component(COMPONENT_NOT_IMPLEMENTED));


/** \brief Create a new component.
 *
 * Please use the get_component() function whenever you want to create
 * a new component. If it already exists, then the existing one will
 * be returned.
 *
 * It is not possible to register the same component more than once.
 * The get_component() functions make sure that won't happen.
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


void component::add_mutually_exclusive_components(set_t components)
{
    f_mutually_exclusive_components.insert(components.begin(), components.end());
}


bool component::is_mutually_exclusive(pointer_t other_component) const
{
    return f_mutually_exclusive_components.find(other_component) != f_mutually_exclusive_components.end();
}


bool component::is_mutually_exclusive(set_t const & other_components) const
{
    // IMPORTNAT NOTE: the sets are sorted by pointers, not names (it
    //                 makes them a lot faster!) so this code compares
    //                 the pointers
    //
    auto mc(f_mutually_exclusive_components.begin());
    auto me(f_mutually_exclusive_components.end());
    auto oc(other_components.begin());
    auto oe(other_components.end());
    while(mc != me && oc != oe)
    {
        std::intptr_t const c(mc->get() - oc->get());
        if(c < 0)
        {
            ++mc;
        }
        else if(c > 0)
        {
            ++oc;
        }
        else
        {
            return true;
        }
    }

    return false;
}











component::pointer_t get_component(std::string const & name)
{
    return get_private_logger()->get_component(name);
}


component::pointer_t get_component(std::string const & name, component::set_t mutually_exclusive)
{
    component::pointer_t component(get_private_logger()->get_component(name));

    component->add_mutually_exclusive_components(mutually_exclusive);

    component::set_t new_component = { component };
    for(auto & me : mutually_exclusive)
    {
        me->add_mutually_exclusive_components(new_component);
    }

    return component;
}


component::pointer_t get_component(message const & msg, std::string const & name)
{
    return get_private_logger(msg)->get_component(name);
}


} // snaplogger namespace
// vim: ts=4 sw=4 et
