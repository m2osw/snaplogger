// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Implementation of the environment support.
 *
 * Whenever a log is emitted, an environment is attached to it. It is
 * very important since the environment of the appender (the one used
 * to format the message) may be different than the environment where
 * the message was emitted.
 *
 * Examples:
 *
 * 1. if you use asynchronous logging, then the tid changes, we use a
 * separate thread to work on the log so the tid is going to be different
 * from the tid of the emitter
 * 2. if you use the network feature, the appenders are going to be run
 * on a completely different computer through a log service which is
 * likely to have a different PID, UID, TID, domain name, hostname, etc.
 *
 * Environments are used through smart pointers, if one changes the
 * ones in existing messages do not change.
 */

// self
//
#include    "snaplogger/environment.h"

#include    "snaplogger/guard.h"
#include    "snaplogger/map_diagnostic.h"
#include    "snaplogger/private_logger.h"


// cppthread
//
#include    <cppthread/thread.h>


// C
//
#include    <grp.h>
#include    <netdb.h>
#include    <pwd.h>
#include    <sys/param.h>
#include    <sys/sysinfo.h>
#include    <sys/types.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



environment::environment(pid_t tid)
    : f_tid(tid)
{
    f_uid = getuid();
    f_pid = getpid();
    f_gid = getgid();

    char buf[1024];
    passwd pw;
    passwd * pw_ptr(nullptr);
    if(getpwuid_r(f_uid, &pw, buf, sizeof(buf), &pw_ptr) == 0
    && pw_ptr == &pw)
    {
        f_username = pw.pw_name;
    }

    group gr;
    group * gr_ptr(nullptr);
    if(getgrgid_r(f_gid, &gr, buf, sizeof(buf), &gr_ptr) == 0
    && gr_ptr == &gr)
    {
        f_groupname += gr.gr_name;
    }

    char host_buffer[HOST_NAME_MAX + 2];
    host_buffer[HOST_NAME_MAX + 1] = '\0'; // make sure it's null terminated

    if(gethostname(host_buffer, HOST_NAME_MAX + 1) == 0)
    {
        f_hostname = host_buffer;
    }

    if(getdomainname(host_buffer, HOST_NAME_MAX + 1) == 0)
    {
        f_domainname = host_buffer;
    }

    map_diagnostics_t const diag(get_map_diagnostics());

    auto const prog_it(diag.find("progname"));
    if(prog_it != diag.end())
    {
        f_progname = prog_it->second;
    }

    std::string const tid_str(std::to_string(tid));
    auto const thread_it(diag.find("threadname#" + tid_str));
    if(thread_it != diag.end())
    {
        f_threadname = thread_it->second;
    }

    f_boot_id = cppthread::get_boot_id();
}



uid_t environment::get_uid() const
{
    return f_uid;
}


pid_t environment::get_pid() const
{
    return f_pid;
}


gid_t environment::get_gid() const
{
    return f_gid;
}


pid_t environment::get_tid() const
{
    return f_tid;
}


std::string environment::get_username() const
{
    guard g;

    return f_username;
}


std::string environment::get_groupname() const
{
    guard g;

    return f_groupname;
}


std::string environment::get_hostname() const
{
    guard g;

    return f_hostname;
}


std::string environment::get_domainname() const
{
    guard g;

    return f_domainname;
}


std::string environment::get_progname() const
{
    guard g;

    return f_progname;
}


std::string environment::get_threadname() const
{
    guard g;

    return f_threadname;
}


std::string environment::get_boot_id() const
{
    guard g;

    return f_boot_id;
}








environment::pointer_t create_environment()
{
    return get_private_logger()->create_environment();
}





} // snaplogger namespace
// vim: ts=4 sw=4 et
