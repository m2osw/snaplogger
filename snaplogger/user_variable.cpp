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
 * \brief Implementation of the environment variable support.
 *
 * This file implements a variable which retrieves its value from the
 * process environment. For example, you could retrieve the path to
 * the HOME directory.
 *
 * This is often used to distinguish between runs.
 */

// self
//
#include    "snaplogger/variable.h"


// C lib
//
#include    <grp.h>
#include    <pwd.h>
#include    <sys/types.h>
#include    <unistd.h>

// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{


DEFINE_LOGGER_VARIABLE(uid)

void uid_variable::process_value(message const & msg, std::string & value) const
{
    value += std::to_string(getuid());

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(username)

void username_variable::process_value(message const & msg, std::string & value) const
{
    passwd * pw(getpwuid(getuid()));
    if(pw != nullptr)
    {
        value += pw->pw_name;
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(gid)

void gid_variable::process_value(message const & msg, std::string & value) const
{
    value += std::to_string(getgid());

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(groupname)

void groupname_variable::process_value(message const & msg, std::string & value) const
{
    group * gr(getgrgid(getgid()));
    if(gr != nullptr)
    {
        value += gr->gr_name;
    }

    variable::process_value(msg, value);
}


}
// no name namespace


} // snaplogger namespace
// vim: ts=4 sw=4 et
