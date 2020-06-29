/*
 * Copyright (c) 2013-2019  Made to Order Software Corp.  All Rights Reserved
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
 * \brief The declaration of the private logger.
 *
 * In order to properly manage all the objects the logger uses, we need to
 * have them all in one place. This declares the private logger which is
 * responsible for maintaining the object that manages all the others.
 *
 * The main point here is to have a cppthread::cppthread object at the
 * end of the list of variable members. It has to be at the end because
 * that way it gets destroyed first and that means the thread (if used)
 * gets stopped before any of the other variables get destroyed.
 *
 * Note that when you call the get_instance() you actually get a pointer
 * to the private logger.
 */

// self
//
#include    "snaplogger/logger.h"
#include    "snaplogger/map_diagnostic.h"
#include    "snaplogger/trace_diagnostic.h"


// cppthread lib
//
#include    <cppthread/fifo.h>
#include    <cppthread/thread.h>



namespace snaplogger
{



namespace detail
{
class asynchronous_logger;
}
// detail namespace



typedef cppthread::fifo<message::pointer_t>                 message_fifo_t;

typedef std::map<std::string, appender_factory::pointer_t>  appender_factory_t;
typedef std::map<std::string, component::pointer_t>         component_map_t;
typedef std::map<pid_t, environment::pointer_t>             environment_map_t;
typedef std::map<severity_t, severity::pointer_t>           severity_by_severity_t;
typedef std::map<std::string, severity::pointer_t>          severity_by_name_t;
typedef std::map<std::string, function::pointer_t>          function_map_t;
typedef std::map<std::string, variable_factory::pointer_t>  variable_factory_map_t;
typedef std::shared_ptr<detail::asynchronous_logger>        asynchronous_logger_pointer_t;







class private_logger
    : public logger
{
public:
    typedef std::shared_ptr<private_logger>     pointer_t;

                                private_logger();
    virtual                     ~private_logger() override;

    virtual void                shutdown() override;

    void                        register_appender_factory(appender_factory::pointer_t factory);
    appender::pointer_t         create_appender(std::string const & type, std::string const & name);

    component::pointer_t        get_component(std::string const & name);

    format::pointer_t           get_default_format();

    environment::pointer_t      create_environment();

    void                        add_severity(severity::pointer_t sev);
    void                        add_alias(severity::pointer_t sev, std::string const & name);
    severity::pointer_t         get_severity(std::string const & name) const;
    severity::pointer_t         get_severity(severity_t sev) const;

    void                        set_diagnostic(std::string const & key, std::string const & diagnostic);
    void                        unset_diagnostic(std::string const & key);
    map_diagnostics_t           get_map_diagnostics();

    void                        set_maximum_trace_diagnostics(size_t max);
    size_t                      get_maximum_trace_diagnostics() const;
    void                        add_trace_diagnostic(std::string const & diagnostic);
    void                        clear_trace_diagnostics();
    trace_diagnostics_t         get_trace_diagnostics();

    void                        push_nested_diagnostic(std::string const & diagnostic);
    void                        pop_nested_diagnostic();
    string_vector_t             get_nested_diagnostics() const;

    void                        register_variable_factory(variable_factory::pointer_t factory);
    variable::pointer_t         get_variable(std::string const & type);

    bool                        has_functions() const;
    void                        register_function(function::pointer_t func);
    function::pointer_t         get_function(std::string const & name) const;

    bool                        has_thread() const;
    void                        create_thread();
    void                        delete_thread();
    void                        send_message_to_thread(message::pointer_t msg);

private:
                                private_logger(private_logger const & rhs) = delete;

    private_logger &            operator = (private_logger const & rhs) = delete;

    appender_factory_t          f_appender_factories = appender_factory_t();
    component_map_t             f_components = component_map_t();
    format::pointer_t           f_default_format = format::pointer_t();
    environment_map_t           f_environment = environment_map_t();
    severity_by_severity_t      f_severity_by_severity = severity_by_severity_t();
    severity_by_name_t          f_severity_by_name = severity_by_name_t();
    map_diagnostics_t           f_map_diagnostics = map_diagnostics_t();
    trace_diagnostics_t         f_trace_diagnostics = trace_diagnostics_t();
    size_t                      f_maximum_trace_diagnostics = DIAG_TRACE_SIZE;
    string_vector_t             f_nested_diagnostics = string_vector_t();
    function_map_t              f_functions = function_map_t();
    variable_factory_map_t      f_variable_factories = variable_factory_map_t();

    // thread handling
    //
    message_fifo_t::pointer_t       f_fifo = message_fifo_t::pointer_t();
    asynchronous_logger_pointer_t   f_asynchronous_logger = asynchronous_logger_pointer_t();
    cppthread::thread::pointer_t    f_thread = cppthread::thread::pointer_t();   // <--- MUST REMAIN LAST VARIABLE MEMBER
};



private_logger::pointer_t       get_private_logger();
private_logger::pointer_t       get_private_logger(message const & msg);



} // snaplogger namespace
// vim: ts=4 sw=4 et
