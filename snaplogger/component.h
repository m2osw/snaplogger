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
#pragma once

/** \file
 * \brief Define the component class used as a way to group messages.
 *
 * Whenever you send a log, you can assign a component to it.
 *
 * By default, when a log component's list is empty, it is view as if
 * the list was only composed of the NORMAL component.
 */


// C++ lib
//
#include    <set>
#include    <string>
#include    <vector>



namespace snaplogger
{


class component
{
public:
    typedef component *         pointer_t;
    typedef std::set<pointer_t> set_t;

    static pointer_t            get_component(std::string const & name);

    std::string const &         get_name() const;

private:
                                component(std::string const & name);

    std::string const           f_name;
};


extern component::pointer_t     debug_component;
extern component::pointer_t     normal_component;
extern component::pointer_t     secure_component;


struct section_ptr
{
    component::pointer_t    f_component;
};

inline section_ptr section(component::pointer_t comp)
{
    return { comp };
}





} // snaplogger namespace
// vim: ts=4 sw=4 et
