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
#include    "snaplogger/exception.h"
#include    "snaplogger/map_diagnostic.h"
#include    "snaplogger/variable.h"


// C lib
//
#include    <netdb.h>
#include    <sys/param.h>
#include    <sys/syscall.h>
#include    <sys/sysinfo.h>
#include    <unistd.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{


DEFINE_LOGGER_VARIABLE(hostname)

void hostname_variable::process_value(message const & msg, std::string & value) const
{
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        char host[HOST_NAME_MAX + 1];
        gethostname(host, sizeof(host));
        host[HOST_NAME_MAX] = '\0'; // make sure it's null terminated
        value += host;
    }
    else
    {
        value += msg.get_environment()->get_hostname();
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(hostbyname)

void hostbyname_variable::process_value(message const & msg, std::string & value) const
{
    auto params(get_params());
    if(params.empty())
    {
        throw invalid_variable("the ${hostbyname:...} variable must have a name parameter.");
    }
    if(params[0]->get_name() != "name")
    {
        throw invalid_variable("the ${hostbyname:...} variable first parameter must be its name parameter.");
    }
    auto hostname(params[0]->get_value());
    if(hostname.empty())
    {
        throw invalid_variable("the ${hostbyname:...} variable first parameter must be its non-empty name.");
    }
    hostent * h(gethostbyname(hostname.c_str()));
    if(h != nullptr)
    {
        value += h->h_name;
    }
    else
    {
        value += "<host " + std::string(h->h_name) + " not found>";
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(domainname)

void domainname_variable::process_value(message const & msg, std::string & value) const
{
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        char domain[HOST_NAME_MAX + 1];
        getdomainname(domain, sizeof(domain));
        domain[HOST_NAME_MAX] = '\0'; // make sure it's null terminated
        value += domain;
    }
    else
    {
        value += msg.get_environment()->get_domainname();
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(pid)

void pid_variable::process_value(message const & msg, std::string & value) const
{
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        value += std::to_string(getpid());
    }
    else
    {
        value += std::to_string(msg.get_environment()->get_pid());
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(tid)

void tid_variable::process_value(message const & msg, std::string & value) const
{
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        value += std::to_string(static_cast<pid_t>(syscall(SYS_gettid)));
    }
    else
    {
        value += std::to_string(msg.get_environment()->get_tid());
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(threadname)

void threadname_variable::process_value(message const & msg, std::string & value) const
{
    auto params(get_params());
    if(params.size() > 0
    && params[0]->get_name() == "running")
    {
        // we assume that the user has a map_diagnostic with the name
        // "threadname"; this is going to be automatic in our own snap_thread
        // implementation, any others would have to be done manually
        //
        map_diagnostics_t diag(get_map_diagnostics());
        std::string tid(std::to_string(static_cast<pid_t>(syscall(SYS_gettid))));
        auto it(diag.find("threadname#" + tid));
        if(it != diag.end())
        {
            value += it->second;
        }
    }
    else
    {
        value += msg.get_environment()->get_threadname();
    }

    variable::process_value(msg, value);
}


}
// no name namespace


} // snaplogger namespace
// vim: ts=4 sw=4 et
