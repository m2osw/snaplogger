/*
 * Copyright (c) 2013-2021  Made to Order Software Corp.  All Rights Reserved
 *
 * https://snapwebsites.org/project/snaplogger
 * contact@m2osw.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#pragma once

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
//
#include    "snaplogger/appender.h"
#include    "snaplogger/message.h"



namespace snaplogger
{

typedef std::vector<std::size_t>        severity_stats_t;


class logger
{
public:
    typedef std::shared_ptr<logger>     pointer_t;
    typedef std::weak_ptr<logger>       weak_pointer_t;

    virtual                     ~logger();

    static logger::pointer_t    get_instance();

    void                        reset();
    virtual void                shutdown();
    bool                        is_configured() const;
    bool                        has_appender(std::string const & type) const;
    appender::pointer_t         get_appender(std::string const & name) const;
    void                        set_config(advgetopt::getopt const & params);
    void                        reopen();

    void                        add_appender(appender::pointer_t a);
    void                        add_config(std::string const & config_filename);
    appender::pointer_t         add_console_appender();
    appender::pointer_t         add_syslog_appender(std::string const & identity);
    appender::pointer_t         add_file_appender(std::string const & filename);

    severity_t                  get_lowest_severity() const;
    void                        set_severity(severity_t severity_level);
    void                        set_fatal_severity(severity_t severity_level);
    void                        reduce_severity(severity_t severity_level);
    void                        severity_changed(severity_t severity_level);
    severity_t                  get_default_severity() const;
    bool                        set_default_severity(severity_t severity_level);
    void                        add_component_to_include(component::pointer_t comp);
    void                        add_component_to_ignore(component::pointer_t comp);

    bool                        is_asynchronous() const;
    void                        set_asynchronous(bool status);
    void                        log_message(message const & msg);
    void                        process_message(message const & msg);
    void                        set_fatal_error_callback(std::function<void(void)> & f);
    void                        call_fatal_error_callback();
    severity_stats_t            get_severity_stats() const;

protected:
                                logger();

    component::pointer_t        f_normal_component = component::pointer_t();

private:
                                logger(logger const & rhs) = delete;

    logger &                    operator = (logger const & rhs) = delete;

    appender::vector_t          f_appenders = appender::vector_t();
    component::set_t            f_components_to_include = component::set_t();
    component::set_t            f_components_to_ignore = component::set_t();
    severity_t                  f_lowest_severity = severity_t::SEVERITY_OFF;
    severity_t                  f_fatal_severity = severity_t::SEVERITY_OFF;
    std::function<void(void)>   f_fatal_error_callback = nullptr;
    bool                        f_asynchronous = false;
    severity_stats_t            f_severity_stats = severity_stats_t(static_cast<std::size_t>(severity_t::SEVERITY_MAX) - static_cast<std::size_t>(severity_t::SEVERITY_MIN) + 1);
};


bool                is_configured();
bool                has_appender(std::string const & type);
void                reopen();

bool                configure_console(bool force = false);
bool                configure_syslog(std::string const & identity);
bool                configure_file(std::string const & filename);
bool                configure_config(std::string const & config_filename);


} // snaplogger namespace
// vim: ts=4 sw=4 et
