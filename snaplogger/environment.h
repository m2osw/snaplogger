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
#pragma once

/** \file
 * \brief Environments from where a SNAP_LOG_... is used.
 *
 * Whenever you generate a log, the environment from that point in time
 * is registered in your message. This is very important since a log
 * can be handled by a different thread or even a different computer
 * and the appenders have to have the ability to show the PID, GID,
 * user name, group name, etc. from the original process and not
 * from the process that actually runs the appenders.
 */

// self
//


// C++
//
#include    <memory>
#include    <string>


// C
//
#include    <unistd.h>



namespace snaplogger
{


class environment
{
public:
    typedef std::shared_ptr<environment>     pointer_t;

                        environment(pid_t tid);

    uid_t               get_uid() const;
    pid_t               get_pid() const;
    gid_t               get_gid() const;
    pid_t               get_tid() const;
    std::string         get_username() const;
    std::string         get_groupname() const;
    std::string         get_hostname() const;
    std::string         get_domainname() const;
    std::string         get_progname() const;
    std::string         get_threadname() const;
    std::string         get_boot_id() const;

private:
    uid_t               f_uid = -1;
    pid_t               f_pid = -1;
    gid_t               f_gid = -1;
    pid_t               f_tid = -1;
    std::string         f_username = std::string();
    std::string         f_groupname = std::string();
    std::string         f_hostname = std::string();
    std::string         f_domainname = std::string();
    std::string         f_progname = std::string();
    std::string         f_threadname = std::string();
    std::string         f_boot_id = std::string();
};


environment::pointer_t  create_environment();


} // snaplogger namespace
// vim: ts=4 sw=4 et
