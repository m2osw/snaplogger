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
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
//
#include    "snaplogger/file_appender.h"

#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/map_diagnostic.h"
#include    "snaplogger/syslog_appender.h"


// advgetopt
//
#include    <advgetopt/validator_size.h>


// snapdev
//
#include    <snapdev/lockfile.h>


// C++
//
#include    <iostream>


// C
//
#include    <fcntl.h>
#include    <syslog.h>
#include    <sys/stat.h>
#include    <sys/types.h>
#include    <unistd.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{


APPENDER_FACTORY(file);


}
// no name namespace



file_appender::file_appender(std::string const & name)
    : appender(name, "file")
{
}


file_appender::~file_appender()
{
}


void file_appender::set_config(advgetopt::getopt const & opts)
{
    guard g;

    appender::set_config(opts);

    // PATH
    //
    std::string const path_field(get_name() + "::path");
    if(opts.is_defined(path_field))
    {
        f_path = opts.get_string(path_field);
    }
    else if(opts.is_defined("path"))
    {
        f_path = opts.get_string("path");
    }

    // FILENAME
    //
    std::string const filename_field(get_name() + "::filename");
    if(opts.is_defined(filename_field))
    {
        f_filename = opts.get_string(filename_field);
    }
    // else -- we'll try to dynamically determine a filename when we
    //         reach the process_message() function

    // MAXIMUM SIZE
    //
    std::string const maximum_size_field(get_name() + "::maximum_size");
    if(opts.is_defined(maximum_size_field))
    {
        std::string const size_str(opts.get_string(maximum_size_field));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        __int128 size(0);
        if(advgetopt::validator_size::convert_string(
                      size_str
                    , advgetopt::validator_size::VALIDATOR_SIZE_DEFAULT_FLAGS
                    , size))
        {
            f_maximum_size = std::max(size, static_cast<__int128>(INT64_MAX));
        }
#pragma GCC diagnostic pop
    }

    // ON OVERFLOW
    //
    std::string const on_overflow_field(get_name() + "::on_overflow");
    if(opts.is_defined(on_overflow_field))
    {
        f_on_overflow = opts.get_string(on_overflow_field);
    }

    // LOCK
    //
    std::string const lock_field(get_name() + "::lock");
    if(opts.is_defined(lock_field))
    {
        f_lock = advgetopt::is_true(opts.get_string(lock_field));
    }

    // FLUSH
    //
    std::string const flush_field(get_name() + "::flush");
    if(opts.is_defined(flush_field))
    {
        f_flush = advgetopt::is_true(opts.get_string(flush_field));
    }

    // SECURE
    //
    std::string const secure_field(get_name() + "::secure");
    if(opts.is_defined(secure_field))
    {
        f_secure = !advgetopt::is_false(opts.get_string(secure_field));
    }

    // FALLBACK TO CONSOLE
    //
    std::string const fallback_to_console_field(get_name() + "::fallback_to_console");
    if(opts.is_defined(fallback_to_console_field))
    {
        f_fallback_to_console = advgetopt::is_true(opts.get_string(fallback_to_console_field));
    }
    std::string const severity_considered_an_error_field(get_name() + "::severity_considered_an_error");
    if(opts.is_defined(severity_considered_an_error_field))
    {
        severity::pointer_t sev(snaplogger::get_severity(opts.get_string(severity_considered_an_error_field)));
        if(sev != nullptr)
        {
            f_severity_considered_an_error = sev->get_severity();
        }
    }

    // FALLBACK TO SYSLOG
    //
    std::string const fallback_to_syslog_field(get_name() + "::fallback_to_syslog");
    if(opts.is_defined(fallback_to_syslog_field))
    {
        f_fallback_to_syslog = advgetopt::is_true(opts.get_string(fallback_to_syslog_field));
    }
}


void file_appender::reopen()
{
    guard g;

    f_fd.reset();
    f_initialized = false;
}


void file_appender::set_filename(std::string const & filename)
{
    guard g;

    if(f_filename != filename)
    {
        f_filename = filename;
        f_initialized = false;
    }
}


bool file_appender::process_message(message const & msg, std::string const & formatted_message)
{
    guard g;

    // verify whether the output file is too large, if so rename it .log.1
    // and create a new file; the process will delete an existing .log.1 if
    // present; as a result we make sure that files never grow over a
    // user specified maximum; by default this feature uses a maximum size
    // of 10Mb
    //
    if(f_maximum_size > 0
    && !!f_fd)
    {
        struct stat s = {};
        int const r(fstat(f_fd.get(), &s));
        if(r != 0
        || s.st_size >= f_maximum_size)
        {
            if(!f_limit_reached)
            {
                f_limit_reached = true;

                // ignore the result of this call; if it fails, it's
                // less important than the user's message
                //
                // TODO: look at properly formatting this message (missing date/time, pid, file:line, etc.)
                //
                snapdev::NOT_USED(output_message(msg, "-- file size limit reached, this will be the last message --", false));
            }

            if(f_on_overflow == "skip")
            {
                return true;
            }

            if(f_on_overflow == "fatal")
            {
                throw fatal_error("logger's output file is full");
            }

            if(f_on_overflow == "rotate")
            {
                // poorman's log rotate replacing the .1 if it exists
                // this way we avoid the compression which is really
                // slow...
                //
                std::string one(f_filename + ".1");
                snapdev::NOT_USED(unlink(one.c_str()));
                if(rename(f_filename.c_str(), one.c_str()) != 0)
                {
                    snapdev::NOT_USED(unlink(f_filename.c_str()));
                }
            }
            else if(f_on_overflow == "logrotate")
            {
                // TODO: run logrotate properly
                //
                //       right now, we could only run it as ourselves
                //       also we do want to specify which configuration
                //       file to use otherwise it would attempt to
                //       rotate everything which is not what we want
                //
                if(system("/usr/sbin/logrotate /etc/logrotate.conf") != 0)
                {
                    // assume our rotation failed
                    //
                    return false;
                }
            }
            else
            {
                // act like "failed" to let fallback take over
                //
                return false;
            }

            // force a reopen as when we do a logrotate with an event
            // (we would not yet have received the event here)
            //
            reopen();
        }
        else
        {
            f_limit_reached = false;
        }
    }

    if(!f_initialized)
    {
        f_initialized = true;

        if(f_filename.empty())
        {
            // try to generate a filename
            //
            map_diagnostics_t map(get_map_diagnostics());
            auto const it(map.find("progname"));
            if(it == map.end())
            {
                return false;
            }
            if(it->second.empty())
            {
                return false;
            }

            f_filename = f_path + '/';
            if(f_secure)
            {
                f_filename += "secure/";
            }
            f_filename += it->second;
            f_filename += ".log";
        }
        else if(f_filename.find('/') == std::string::npos)
        {
            f_filename = f_path + '/' + f_filename;
        }
        std::string::size_type pos(f_filename.rfind('/'));
        if(pos == std::string::npos)
        {
            pos = 0;
        }
        if(f_filename.find('.', pos + 1) == std::string::npos)
        {
            f_filename += ".log";
        }

        if(access(f_filename.c_str(), R_OK | W_OK) != 0
        && errno != ENOENT)
        {
            return false;
        }

        int flags(O_CREAT | O_WRONLY | O_APPEND | O_CLOEXEC | O_LARGEFILE | O_NOCTTY);
        int mode(S_IRUSR | S_IWUSR);
        if(!f_secure)
        {
            mode |= S_IRGRP;
        }

        f_fd.reset(open(f_filename.c_str(), flags, mode));

        // verify that the file isn't already too big
        //
        // TODO: we may want to look at making the code above that verifies
        //       the size and allow rotation to run here too if the file
        //       is already too large?
        //
        struct stat s = {};
        int const r(fstat(f_fd.get(), &s));
        if(r != 0
        || s.st_size >= f_maximum_size)
        {
            return false;
        }
    }

    return output_message(msg, formatted_message, true);
}


bool file_appender::output_message(message const & msg, std::string const & formatted_message, bool allow_fallbacks)
{
    if(!f_fd)
    {
        return false;
    }

    std::unique_ptr<snapdev::lockfd> lock_file;
    if(f_lock)
    {
        lock_file = std::make_unique<snapdev::lockfd>(f_fd.get(), snapdev::operation_t::OPERATION_EXCLUSIVE);
    }

    ssize_t const l(write(f_fd.get(), formatted_message.c_str(), formatted_message.length()));
    if(static_cast<size_t>(l) == formatted_message.length())
    {
        // writing to file worked
        //
        return true;
    }

    if(!allow_fallbacks)
    {
        return false;
    }

    // caller allows fallbacks
    //
    if(f_fallback_to_console)
    {
        severity_t const sev(msg.get_severity());
        if(sev >= f_severity_considered_an_error)
        {
            if(isatty(fileno(stderr)))
            {
                std::cerr << formatted_message;
                return true;
            }
        }
        if(isatty(fileno(stdout)))
        {
            std::cout << formatted_message;
            return true;
        }
    }

    if(f_fallback_to_syslog)
    {
        // in this case we skip on the openlog() call...
        //
        int const priority(syslog_appender::message_severity_to_syslog_priority(msg.get_severity()));
        syslog(priority, "%s", formatted_message.c_str());
        return true;
    }

    return false;
}





} // snaplogger namespace
// vim: ts=4 sw=4 et
