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


// C++ Lib
//
//#include    <string>


// C lib
//
#include    <grp.h>
#include    <netdb.h>
#include    <pwd.h>
#include    <sys/param.h>
#include    <sys/syscall.h>
#include    <sys/sysinfo.h>
#include    <sys/types.h>
#include    <unistd.h>

// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{



typedef std::map<pid_t, environment::pointer_t>     environment_map_t;

environment_map_t                                   g_environment;



}
// no name namespace





environment::environment(pid_t tid)
    : f_tid(tid)
{
    f_uid = getuid();
    f_pid = getpid();
    f_gid = getgid();

    passwd * pw(getpwuid(f_uid));
    if(pw != nullptr)
    {
        f_username = pw->pw_name;
    }

    group * gr(getgrgid(f_gid));
    if(gr != nullptr)
    {
        f_groupname += gr->gr_name;
    }

    char host_buffer[HOST_NAME_MAX + 1];
    host_buffer[HOST_NAME_MAX] = '\0'; // make sure it's null terminated

    gethostname(host_buffer, sizeof(host_buffer));
    f_hostname = host_buffer;

    getdomainname(host_buffer, sizeof(host_buffer));
    f_domainname = host_buffer;

    map_diagnostics_t diag(get_map_diagnostics());

    auto prog_it(diag.find("progname"));
    if(prog_it != diag.end())
    {
        f_progname = prog_it->second;
    }

    std::string tid_str(std::to_string(tid));
    auto thread_it(diag.find("threadname#" + tid_str));
    if(thread_it != diag.end())
    {
        f_threadname = thread_it->second;
    }
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


std::string const & environment::get_username() const
{
    return f_username;
}


std::string const & environment::get_groupname() const
{
    return f_groupname;
}


std::string const & environment::get_hostname() const
{
    return f_hostname;
}


std::string const & environment::get_domainname() const
{
    return f_domainname;
}


std::string const & environment::get_progname() const
{
    return f_progname;
}


std::string const & environment::get_threadname() const
{
    return f_threadname;
}






environment::pointer_t  create_environment()
{
    pid_t const tid(static_cast<pid_t>(syscall(SYS_gettid)));

    guard g;

    auto it(g_environment.find(tid));
    if(it == g_environment.end())
    {
        auto result(std::make_shared<environment>(tid));
        g_environment[tid] = result;
        return result;
    }

    return it->second;
}





} // snaplogger namespace
// vim: ts=4 sw=4 et
