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
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
//
#include    "appender.h"
#include    "message.h"



namespace snaplogger
{


class logger
{
public:
    typedef std::shared_ptr<logger>     pointer_t;

                                logger();

    static logger::pointer_t    get_instance();

    void                        reset();
    bool                        is_configured() const;
    void                        set_config(advgetopt::getopt const & params);

    void                        add_config(std::string const & config_filename);
    void                        add_console_appender();
    void                        add_syslog_appender(std::string const & identity);
    void                        add_file_appender(std::string const & filename);

    bool                        is_asynchronous() const;
    void                        set_asynchronous(bool status);
    void                        log_message(message const & msg);

private:
    appender::vector_t          f_appenders = appender::vector_t();
    bool                        f_asynchronous = false;
};


bool                is_configured();

bool                configure_console();
bool                configure_syslog(std::string const & identity);
bool                configure_file(std::string const & filename);
bool                configure_config(std::string const & config_filename);


} // snaplogger namespace
// vim: ts=4 sw=4 et
