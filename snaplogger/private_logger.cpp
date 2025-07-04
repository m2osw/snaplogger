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


// cppthread
//
#include    <cppthread/log.h>
#include    <cppthread/runner.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{



void getopt_logs(cppthread::log_level_t l, std::string const & m)
{
    severity_t sev(severity_t::SEVERITY_ERROR);
    switch(l)
    {
    case cppthread::log_level_t::debug:
        sev = severity_t::SEVERITY_DEBUG;
        break;

    case cppthread::log_level_t::info:
        sev = severity_t::SEVERITY_INFORMATION;
        break;

    case cppthread::log_level_t::warning:
        sev = severity_t::SEVERITY_WARNING;
        break;

    case cppthread::log_level_t::fatal:
        sev = severity_t::SEVERITY_FATAL;
        break;

    //case cppthread::log_level_t::error:
    default:
        // anything else, keep SEVERITY_ERROR
        break;

    }

    message msg(sev);

    // we do not use the g_... names in case they were not yet allocated
    //
    msg.add_component(get_component(COMPONENT_NORMAL));
    msg.add_component(get_component(COMPONENT_CPPTHREAD));

    msg << m;

    // this call cannot create a loop, if the creation of the logger
    // generates an cppthread log, then the second call will generate
    // an exception (see get_instance() in snaplogger/logger.cpp)
    //
    logger::pointer_t lg(logger::get_instance());

    lg->log_message(msg);
}



}
// no name namespace


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
    // if a call arrives really early, this is defined in the logger
    //
    f_normal_component = get_component(COMPONENT_NORMAL);

    cppthread::set_log_callback(getopt_logs);
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
        throw logger_logic_error(                                       // LCOV_EXCL_LINE
                "register_appender_factory() called with a nullptr.");  // LCOV_EXCL_LINE
    }

    guard g;

    if(f_appender_factories.find(factory->get_type()) != f_appender_factories.end())
    {
        throw duplicate_error(                                  // LCOV_EXCL_LINE
                  "trying to register appender type \""         // LCOV_EXCL_LINE
                + factory->get_type()                           // LCOV_EXCL_LINE
                + "\" twice won't work.");                      // LCOV_EXCL_LINE
    }

    f_appender_factories[factory->get_type()] = factory;
}


appender_factory_t private_logger::appender_factory_list() const
{
    return f_appender_factories;
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


/** \brief Get a component by name.
 *
 * All components are stored in the f_components set managed by the
 * private_logger object instance. This way each component is unique.
 *
 * This function searches the list of existing components. If one with
 * the same name already exists, then that one is picked and returned.
 * If it doesn't exist yet, then a new component is created and that
 * new component's pointer is saved in the f_components list and
 * returned.
 *
 * The name of the components must be composed of letters (a-z),
 * underscores (_), and digits (0-9). Any other character is considered
 * invalid. The function will force uppercase characters (A-Z) to lowercase
 * and dashes (-) to underscores (_). Finally, a component name can't start
 * with a digit (0-9).
 *
 * \exception invalid_parameter
 * This function raises an invalid_parameter exception when it find an
 * invalid character in the input name.
 *
 * \param[in] name  The name of the component to retrieve.
 *
 * \return The pointer to the component named \p name.
 */
component::pointer_t private_logger::get_component(std::string const & name)
{
    guard g;

    std::string n;
    n.reserve(name.length());
    for(char const * s(name.c_str()); *s != '\0'; ++s)
    {
        if(*s >= 'a' && *s <= 'z')
        {
            n += *s;
        }
        else if(*s >= 'A' && *s <= 'Z')
        {
            // force to lowercase
            //
            n += *s | 0x20;
        }
        else if(*s == '-' || *s == '_')
        {
            n += '_';
        }
        else if(*s >= '0' && *s <= '9')
        {
            if(n.empty())
            {
                throw invalid_parameter(
                          "a component name cannot start with a digits ("
                        + name
                        + ")");
            }
            n += *s;
        }
        else
        {
            throw invalid_parameter(
                      "a component name cannot include a '"
                    + *s
                    + ("' character ("
                    + name
                    + ")"));
        }
    }

    auto it(f_components.find(n));
    if(it != f_components.end())
    {
        return it->second;
    }

    // the component constructor is only accessible to the private_logger
    // so we can't do a make_shared<>()
    //
    //auto comp(std::make_shared<component>(n));

    f_components[n].reset(new component(n));

    return f_components[n];
}


component::map_t private_logger::get_component_list() const
{
    return f_components;
}


format::pointer_t private_logger::get_default_format()
{
    guard g;

    if(f_default_format == nullptr)
    {
        f_default_format = std::make_shared<format>(
            //"${env:name=HOME:padding='-':align=center:exact_width=6} "
            "${date} ${time} ${hostname}"
            " ${progname}[${pid}/${tid}]: ${severity}:"
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


/** \brief Add a severity.
 *
 * This function adds a severity to the private logger object.
 *
 * Remember that a severity can be given aliases so this function may
 * add quite a few entries, not just one.
 *
 * To add an alias after creation, make sure to use the add_alias() instead.
 * This makes sure you link the same severity to several names.
 *
 * \warning
 * You should not be calling this function directly. Please see the
 * direct snaplogger::add_severity() function instead.
 *
 * \exception duplicate_error
 * The function verifies that the new severity is not a duplicate of
 * an existing system severity. The verification process checks the
 * severity by severity level and by name. You can, however, have
 * _duplicates_ of user defined severity levels. However, the last
 * user defined severity of a given name & level sticks, the others
 * get deleted.
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
            throw duplicate_error("a system severity ("
                                + std::to_string(static_cast<long>(it->first))
                                + ") cannot be replaced (same severity level: "
                                + std::to_string(static_cast<long>(sev->get_severity()))
                                + ").");
        }
    }

    for(auto const & n : sev->get_all_names())
    {
        auto s(f_severity_by_name.find(n));
        if(s != f_severity_by_name.end())
        {
            if(s->second->is_system())
            {
                // note that any severity can be partially edited, just not
                // added more than once
                //
                throw duplicate_error("a system severity ("
                                    + n
                                    + ") cannot be replaced (same name).");
            }
        }
    }

    sev->mark_as_registered();

    f_severity_by_severity[sev->get_severity()] = sev;

    for(auto const & n : sev->get_all_names())
    {
        f_severity_by_name[n] = sev;
    }
}


/** \brief Add yet another alias.
 *
 * This function is used when the system aliases get assigned additional
 * aliases. The add_severity() was already called with system definitions,
 * so this is the only way to add additional aliases to them trhough the .ini
 * files.
 *
 * \param[in] sev  A pointer to the severity to be added.
 * \param[in] name  The name of the alias.
 */
void private_logger::add_alias(severity::pointer_t sev, std::string const & name)
{
    guard g;

    auto it(f_severity_by_severity.find(sev->get_severity()));
    if(it == f_severity_by_severity.end())
    {
        throw duplicate_error("to register an alias the corresponding main severity must already be registered. We could not find a severity with level "
                            + std::to_string(static_cast<long>(sev->get_severity()))
                            + ".");
    }

    auto s(f_severity_by_name.find(name));
    if(s != f_severity_by_name.end())
    {
        if(s->second->is_system())
        {
            // note that any severity can be partially edited, just not
            // added more than once
            //
            throw duplicate_error("a system severity ("
                                + name
                                + ") cannot be replaced (same name).");
        }
    }

    f_severity_by_name[name] = sev;
}


severity::pointer_t private_logger::get_severity(std::string const & name) const
{
    guard g;

    std::string n(name);
    std::transform(n.begin(), n.end(), n.begin(), std::towlower);
    auto it(f_severity_by_name.find(n));
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


severity::pointer_t private_logger::get_default_severity() const
{
    return f_default_severity;
}


void private_logger::set_default_severity(severity::pointer_t sev)
{
    f_default_severity = sev;
}


/** \brief Get the list of currently defined severities.
 *
 * This function gets the list of all the severities sorted by name.
 *
 * Note that some severities are given multiple names (i.e. "info" and
 * "information"). In that case, both names will appear in this list.
 * If you want to avoid such duplicates, use the get_severities_by_severity().
 *
 * \note
 * The function return the map by copy so that way it works in a
 * multithreaded environment. However, the pointer is a set of pointers
 * to severities. Those pointers are not duplicated.
 *
 * \return A copy of the current severity list.
 *
 * \sa get_severities_by_name()
 */
severity_by_name_t private_logger::get_severities_by_name() const
{
    // this call is required in case the severities were not yet defined
    //
    snapdev::NOT_USED(snaplogger::get_severity("error"));   // TODO: assuming that "error" won't change

    guard g;

    return f_severity_by_name;
}


severity_by_severity_t private_logger::get_severities_by_severity() const
{
    // this call is required in case the severities were not yet defined
    //
    snapdev::NOT_USED(snaplogger::get_severity("error"));   // TODO: assuming that "error" won't change

    guard g;

    return f_severity_by_severity;
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


void private_logger::set_maximum_trace_diagnostics(size_t max)
{
    f_maximum_trace_diagnostics = max;
}


size_t private_logger::get_maximum_trace_diagnostics() const
{
    return f_maximum_trace_diagnostics;
}


void private_logger::add_trace_diagnostic(std::string const & diagnostic)
{
    guard g;

    f_trace_diagnostics.push_back(diagnostic);
    if(f_trace_diagnostics.size() > f_maximum_trace_diagnostics)
    {
        f_trace_diagnostics.pop_front();
    }
}


void private_logger::clear_trace_diagnostics()
{
    guard g;

    f_trace_diagnostics.clear();
}


trace_diagnostics_t private_logger::get_trace_diagnostics()
{
    guard g;

    return f_trace_diagnostics;
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
        throw invalid_variable("No variable factories were registered yet; you can't create a variable with type \""    // LCOV_EXCL_LINE
                             + type                                     // LCOV_EXCL_LINE
                             + "\" at this point.");                    // LCOV_EXCL_LINE
    }

    auto it(f_variable_factories.find(type));
    if(it == f_variable_factories.end())
    {
        // if a message includes a variable like pattern, it may just be part
        // of the input message so leave it alone (just return nullptr) instead
        // of "crashing" the app. completely
        //
        return variable::pointer_t();
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
    catch(...)                              // LCOV_EXCL_LINE
    {
        if(f_fifo != nullptr)               // LCOV_EXCL_LINE
        {
            f_fifo->done(false);            // LCOV_EXCL_LINE
        }

        f_thread.reset();                   // LCOV_EXCL_LINE
        f_asynchronous_logger.reset();      // LCOV_EXCL_LINE
        f_fifo.reset();                     // LCOV_EXCL_LINE
        throw;                              // LCOV_EXCL_LINE
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

        swap(thread,              f_thread);
        swap(asynchronous_logger, f_asynchronous_logger);
        swap(fifo,                f_fifo);
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
