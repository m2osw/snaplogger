// Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Implementation of user related variables.
 *
 * This file implements support for data intrinsic to the current user.
 *
 * \li `uid`
 *
 * The user identifier as found in `/etc/passwd`.
 *
 * \li `username`
 *
 * The name of the current user as defined in the password database.
 * This function searches for a name for the current getuid().
 *
 * \li `gid`
 *
 * The group identifier of the current user as found in `/etc/group`.
 *
 * \li `groupname`
 *
 * The name of the current group as defined in the group database.
 * This function searches for a name for the current getgid().
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
{
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        value += std::to_string(getuid());
    }
    else
    {
        value += std::to_string(msg.get_environment()->get_uid());
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(username)
{
    uid_t uid;
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        uid = getuid();
    }
    else
    {
        uid = msg.get_environment()->get_uid();
    }

    char buf[1024];
    passwd pw;
    passwd * pw_ptr(nullptr);
    if(getpwuid_r(uid, &pw, buf, sizeof(buf), &pw_ptr) == 0
    && pw_ptr == &pw)
    {
        value += pw.pw_name;
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(gid)
{
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        value += std::to_string(getgid());
    }
    else
    {
        value += std::to_string(msg.get_environment()->get_gid());
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(groupname)
{
    gid_t gid;
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        gid = getgid();
    }
    else
    {
        gid = msg.get_environment()->get_gid();
    }

    char buf[1024];
    group gr;
    group * gr_ptr(nullptr);
    if(getgrgid_r(gid, &gr, buf, sizeof(buf), &gr_ptr) == 0
    && gr_ptr == &gr)
    {
        value += gr.gr_name;
    }

    variable::process_value(msg, value);
}


}
// no name namespace


} // snaplogger namespace
// vim: ts=4 sw=4 et
