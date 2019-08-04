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
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
//
#include    "snaplogger/private_logger.h"

#include    "snaplogger/console_appender.h"
#include    "snaplogger/exception.h"
#include    "snaplogger/file_appender.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/logger.h"
#include    "snaplogger/syslog_appender.h"


// cppthread lib
//
#include    <cppthread/runner.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace detail
{




class asynchronous_logger
    : public cppthread::runner
{
public:
    asynchronous_logger(message_fifo_t::pointer_t fifo)
        : runner("logger asynchronous thread")
        , f_logger(logger::get_instance())
        , f_fifo(fifo)
    {
    }

    virtual void run()
    {
        // loop until the FIFO is marked as being done
        //
        for(;;)
        {
            message::pointer_t msg;
            if(!f_fifo->pop_front(msg, -1))
            {
                break;
            }
            logger::pointer_t l(f_logger.lock());
            if(l != nullptr)
            {
                l->process_message(*msg);
            }
        }
    }

private:
    logger::weak_pointer_t      f_logger = logger::pointer_t();
    message_fifo_t::pointer_t   f_fifo = message_fifo_t::pointer_t();
};



}
// detail namespace



private_logger::private_logger()
{
    f_normal_component = get_component(COMPONENT_NORMAL);
}


private_logger::~private_logger()
{
    delete_thread();
}


void private_logger::shutdown()
{
    delete_thread();
}


void private_logger::register_appender_factory(appender_factory::pointer_t factory)
{
    if(factory == nullptr)
    {
        throw logger_logic_error(
                "register_appender_factory() called with a nullptr.");
    }

    guard g;

    if(f_appender_factories.find(factory->get_type()) != f_appender_factories.end())
    {
        throw duplicate_error(
                  "trying to register appender type \""
                + factory->get_type()
                + "\" twice won't work.");
    }

    f_appender_factories[factory->get_type()] = factory;
}


appender::pointer_t private_logger::create_appender(std::string const & type, std::string const & name)
{
    guard g;

    auto it(f_appender_factories.find(type));
    if(it != f_appender_factories.end())
    {
        return it->second->create(name);
    }

    return appender::pointer_t();
}


component::pointer_t private_logger::get_component(std::string const & name)
{
    guard g;

    auto it(f_components.find(name));
    if(it != f_components.end())
    {
        return it->second;
    }

    auto comp(std::make_shared<component>(name));
    f_components[name] = comp;

    return comp;
}


format::pointer_t private_logger::get_default_format()
{
    guard g;

    if(f_default_format == nullptr)
    {
        f_default_format = std::make_shared<format>(
            //"${env:name=HOME:padding='-':align=center:exact_width=6} "
            "${date} ${time} ${hostname}"
            " ${progname}[${pid}]: ${severity}:"
            " ${message:escape:max_width=1000}"
            " (in function \"${function}()\")"
            " (${basename}:${line})"
        );
    }

    return f_default_format;
}


environment::pointer_t private_logger::create_environment()
{
    pid_t const tid(cppthread::gettid());

    guard g;

    auto it(f_environment.find(tid));
    if(it == f_environment.end())
    {
        auto result(std::make_shared<environment>(tid));
        f_environment[tid] = result;
        return result;
    }

    return it->second;
}


bool private_logger::has_severities() const
{
    return !f_severity_by_severity.empty();
}


/** \brief Add a severity.
 *
 * This function adds a severity to the private logger object.
 *
 * Remember that a severity can be given aliases so this function may
 * add quite a few entries, not just one.
 *
 * \warning
 * You should not be calling this function directly. Please see the
 * direct snaplogger::add_severity() function instead.
 *
 * \exception duplicate_error
 * The function verifies that the new severity is not a duplicate of
 * an existing system severity.
 *
 * \param[in] sev  The severity object to be added.
 */
void private_logger::add_severity(severity::pointer_t sev)
{
    guard g;

    auto it(f_severity_by_severity.find(sev->get_severity()));
    if(it != f_severity_by_severity.end())
    {
        if(it->second->is_system())
        {
            throw duplicate_error("a system severity cannot be replaced.");
        }
    }

    for(auto n : sev->get_all_names())
    {
        auto s(f_severity_by_name.find(n));
        if(s != f_severity_by_name.end())
        {
            if(s->second->is_system())
            {
                throw duplicate_error("a system severity cannot be replaced.");
            }
        }
    }

    f_severity_by_severity[sev->get_severity()] = sev;

    for(auto n : sev->get_all_names())
    {
        f_severity_by_name[n] = sev;
    }
}


severity::pointer_t private_logger::get_severity(std::string const & name) const
{
    guard g;

    auto it(f_severity_by_name.find(name));
    if(it == f_severity_by_name.end())
    {
        return severity::pointer_t();
    }

    return it->second;
}


severity::pointer_t private_logger::get_severity(severity_t sev) const
{
    guard g;

    auto it(f_severity_by_severity.find(sev));
    if(it == f_severity_by_severity.end())
    {
        return severity::pointer_t();
    }

    return it->second;
}


void private_logger::set_diagnostic(std::string const & key, std::string const & diagnostic)
{
    guard g;

    f_map_diagnostics[key] = diagnostic;
}


void private_logger::unset_diagnostic(std::string const & key)
{
    guard g;

    auto it(f_map_diagnostics.find(key));
    if(it != f_map_diagnostics.end())
    {
        f_map_diagnostics.erase(it);
    }
}


map_diagnostics_t private_logger::get_map_diagnostics()
{
    guard g;

    return f_map_diagnostics;
}


void private_logger::push_nested_diagnostic(std::string const & diagnostic)
{
    guard g;

    f_nested_diagnostics.push_back(diagnostic);
}


void private_logger::pop_nested_diagnostic()
{
    guard g;

    f_nested_diagnostics.pop_back();
}


string_vector_t private_logger::get_nested_diagnostics() const
{
    guard g;

    return f_nested_diagnostics;
}


void private_logger::register_variable_factory(variable_factory::pointer_t factory)
{
    guard g;

    auto it(f_variable_factories.find(factory->get_type()));
    if(it != f_variable_factories.end())
    {
        throw duplicate_error(
                  "trying to add two variable factories of type \""
                + factory->get_type()
                + "\".");
    }

    f_variable_factories[factory->get_type()] = factory;
}


variable::pointer_t private_logger::get_variable(std::string const & type)
{
    guard g;

    if(f_variable_factories.empty())
    {
        throw invalid_variable("No variable factories were registered yet; you can't create a variable with type \""
                             + type
                             + "\" at this point.");
    }

    auto it(f_variable_factories.find(type));
    if(it == f_variable_factories.end())
    {
        // TBD: should we instead return a null var.?
        throw invalid_variable("You can't create variable with type \""
                             + type
                             + "\", no such variable type was registered.");
    }

    return it->second->create_variable();
}


bool private_logger::has_functions() const
{
    guard g;

    return !f_functions.empty();
}


void private_logger::register_function(function::pointer_t func)
{
    guard g;

    auto it(f_functions.find(func->get_name()));
    if(it != f_functions.end())
    {
        throw duplicate_error(
                  "trying to add two functions named \""
                + func->get_name()
                + "\".");
    }
    f_functions[func->get_name()] = func;
}


function::pointer_t private_logger::get_function(std::string const & name) const
{
    guard g;

    auto it(f_functions.find(name));
    if(it != f_functions.end())
    {
        return it->second;
    }

    return function::pointer_t();
}


void private_logger::create_thread()
{
    guard g;

    try
    {
        f_fifo = std::make_shared<message_fifo_t>();
        f_asynchronous_logger = std::make_shared<detail::asynchronous_logger>(f_fifo);
        f_thread = std::make_shared<cppthread::thread>("asynchronous logger thread", f_asynchronous_logger.get());
        f_thread->start();
    }
    catch(...)
    {
        if(f_fifo != nullptr)
        {
            f_fifo->done(false);
        }

        f_thread.reset();
        f_asynchronous_logger.reset();
        f_fifo.reset();
        throw;
    }
}


void private_logger::delete_thread()
{
    // WARNING: we can't call fifo::done() while our guard is locked
    //          we also have to make sure it's not a null pointer
    //
    message_fifo_t::pointer_t       fifo                = message_fifo_t::pointer_t();
    asynchronous_logger_pointer_t   asynchronous_logger = asynchronous_logger_pointer_t();
    cppthread::thread::pointer_t    thread              = cppthread::thread::pointer_t();

    {
        guard g;

        std::swap(thread,              f_thread);
        std::swap(asynchronous_logger, f_asynchronous_logger);
        std::swap(fifo,                f_fifo);
    }

    if(fifo != nullptr)
    {
        fifo->done(false);
    }

    try
    {
        thread.reset();
    }
    catch(std::exception const & e)
    {
        // in most cases this one happens when quitting when one of your
        // functions attempts to get an instance of the logger, which is
        // forbidden once you return from your main() function
        //
        std::cerr << "got exception \""
                  << e.what()
                  << "\" while deleting the asynchronous thread."
                  << std::endl;
    }
}


void private_logger::send_message_to_thread(message::pointer_t msg)
{
    {
        guard g;

        if(f_fifo == nullptr)
        {
            create_thread();
        }
    }

    f_fifo->push_back(msg);
}




private_logger::pointer_t get_private_logger()
{
    return std::dynamic_pointer_cast<private_logger>(logger::get_instance());
}


private_logger::pointer_t get_private_logger(message const & msg)
{
    return std::dynamic_pointer_cast<private_logger>(msg.get_logger());
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
