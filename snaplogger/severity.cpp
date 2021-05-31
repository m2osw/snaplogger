// Copyright (c) 2013-2021  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Severity levels for your log messages.
 *
 * The severity implementation loads the severity configuration file
 * and generates a set of severity levels that one can attach to
 * log messages.
 */


// self
//
#include    "snaplogger/severity.h"

#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/private_logger.h"


// advgetopt lib
//
#include    <advgetopt/advgetopt.h>
#include    <advgetopt/options.h>


// boost lib
//
#include    <boost/preprocessor/stringize.hpp>



// C++ lib
//
#include    <iostream>
#include    <map>


// C lib
//
#include    <sys/time.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{


bool        g_severity_auto_added = false;


struct system_severity
{
    severity_t          f_severity      = severity_t::SEVERITY_ALL;
    char const *        f_name          = nullptr;
    char const *        f_alias         = nullptr; // at most 1 alias for system severities
    char const *        f_description   = nullptr;
    char const *        f_styles        = nullptr;
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
constexpr system_severity g_system_severity[] =
{
    {
        .f_severity     = severity_t::SEVERITY_ALL,
        .f_name         = "all",
        .f_alias        = "everything",
        .f_description  = "all",
        .f_styles       = nullptr
    },
    {
        .f_severity     = severity_t::SEVERITY_TRACE,
        .f_name         = "trace",
        .f_alias        = nullptr,
        .f_description  = "trace",
        .f_styles       = nullptr
    },
    {
        .f_severity     = severity_t::SEVERITY_DEBUG,
        .f_name         = "debug",
        .f_alias        = "dbg",
        .f_description  = "debug",
        .f_styles       = nullptr
    },
    {
        .f_severity     = severity_t::SEVERITY_NOTICE,
        .f_name         = "notice",
        .f_alias        = "note",
        .f_description  = "notice",
        .f_styles       = nullptr
    },
    {
        .f_severity     = severity_t::SEVERITY_UNIMPORTANT,
        .f_name         = "unimportant",
        .f_alias        = nullptr,
        .f_description  = "unimportant",
        .f_styles       = nullptr
    },
    {
        .f_severity     = severity_t::SEVERITY_VERBOSE,
        .f_name         = "verbose",
        .f_alias        = "verb",
        .f_description  = "verbose",
        .f_styles       = nullptr
    },
    {
        .f_severity     = severity_t::SEVERITY_INFORMATION,
        .f_name         = "information",
        .f_alias        = "info",
        .f_description  = "info",
        .f_styles       = nullptr
    },
    {
        .f_severity     = severity_t::SEVERITY_IMPORTANT,
        .f_name         = "important",
        .f_alias        = "significant",
        .f_description  = "important",
        .f_styles       = "green"
    },
    {
        .f_severity     = severity_t::SEVERITY_MINOR,
        .f_name         = "minor",
        .f_alias        = nullptr,
        .f_description  = "minor",
        .f_styles       = "green"
    },
    {
        .f_severity     = severity_t::SEVERITY_DEPRECATED,
        .f_name         = "deprecated",
        .f_alias        = nullptr,
        .f_description  = "deprecated",
        .f_styles       = "orange"
    },
    {
        .f_severity     = severity_t::SEVERITY_WARNING,
        .f_name         = "warning",
        .f_alias        = "warn",
        .f_description  = "warning",
        .f_styles       = "orange"
    },
    {
        .f_severity     = severity_t::SEVERITY_MAJOR,
        .f_name         = "major",
        .f_alias        = "paramount",
        .f_description  = "major",
        .f_styles       = "orange"
    },
    {
        .f_severity     = severity_t::SEVERITY_RECOVERABLE_ERROR,
        .f_name         = "recoverable-error",
        .f_alias        = "recoverable",
        .f_description  = "recoverable error",
        .f_styles       = "red"
    },
    {
        .f_severity     = severity_t::SEVERITY_ERROR,
        .f_name         = "error",
        .f_alias        = "err",
        .f_description  = "error",
        .f_styles       = "red"
    },
    {
        .f_severity     = severity_t::SEVERITY_CRITICAL,
        .f_name         = "critical",
        .f_alias        = "crit",
        .f_description  = "critical",
        .f_styles       = "red"
    },
    {
        .f_severity     = severity_t::SEVERITY_ALERT,
        .f_name         = "alert",
        .f_alias        = nullptr,
        .f_description  = "alert",
        .f_styles       = "red"
    },
    {
        .f_severity     = severity_t::SEVERITY_EMERGENCY,
        .f_name         = "emergency",
        .f_alias        = "emerg",
        .f_description  = "emergency",
        .f_styles       = "red"
    },
    {
        .f_severity     = severity_t::SEVERITY_FATAL,
        .f_name         = "fatal",
        .f_alias        = "fatal-error",
        .f_description  = "fatal",
        .f_styles       = "red"
    },
    {
        .f_severity     = severity_t::SEVERITY_OFF,
        .f_name         = "off",
        .f_alias        = "nothing",
        .f_description  = "off",
        .f_styles       = nullptr
    }
};


constexpr char const * const g_configuration_directories[] = {
    "/usr/share/snaplogger",
    "/etc/snaplogger",
    nullptr
};


advgetopt::options_environment g_config_option =
{
    .f_project_name = "logger",
    .f_group_name = nullptr,
    .f_options = nullptr,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = nullptr,
    .f_configuration_files = nullptr,
    .f_configuration_filename = "severity.ini",
    .f_configuration_directories = g_configuration_directories,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS

    //.f_help_header = nullptr,
    //.f_help_footer = nullptr,
    //.f_version = nullptr,
    //.f_license = nullptr,
    //.f_copyright = nullptr,
    //.f_build_date = UTC_BUILD_DATE,
    //.f_build_time = UTC_BUILD_TIME,
    //.f_groups = nullptr
};
#pragma GCC diagnostic pop



/** \brief Add system severities.
 *
 * This function goes through the list of system securities and add them.
 * Then it loads the severity.ini files it files and updates the system
 * security levels and adds the user defined security levels.
 *
 * The severity.ini file format is as follow:
 *
 * * Section name, this is the name of the severity such as `[debug]`
 * * Section field: `severity`, the severity level of this severity
 * * Section field: `alias`, a list of other names for this severity (i.e.
 * the `information` severity is also named `info`)
 * * Section field: `description`, the description of the severity
 * * Section field: `styles`, the styles such as the color (red, orange...)
 *
 * For example:
 *
 * \code
 *     [information]
 *     level=50
 *     aliases=info
 *     description=info
 *     default=true
 *     styles=green
 * \endcode
 */
void auto_add_severities()
{
    guard g;

    if(g_severity_auto_added)
    {
        return;
    }
    g_severity_auto_added = true;

    private_logger::pointer_t l(get_private_logger());

    for(auto ss : g_system_severity)
    {
        severity::pointer_t sev(std::make_shared<severity>(ss.f_severity, ss.f_name, true));
        if(ss.f_alias != nullptr)
        {
            // at this time we have one at the most here
            //
            sev->add_alias(ss.f_alias);
        }
        sev->set_description(ss.f_description);
        if(ss.f_styles != nullptr)
        {
            sev->set_styles(ss.f_styles);
        }

        add_severity(sev);
    }

    // load user editable parameters
    //
    advgetopt::getopt::pointer_t config(std::make_shared<advgetopt::getopt>(g_config_option));
    config->parse_configuration_files();
    advgetopt::option_info::pointer_t const sections(config->get_option(advgetopt::CONFIGURATION_SECTIONS));
    if(sections != nullptr)
    {
        size_t const max(sections->size());
        for(size_t idx(0); idx < max; ++idx)
        {
            std::string const section_name(sections->get_value(idx));

            // we found a section name, this is the name of a severity,
            // gather the info and create the new severity
            //
            severity::pointer_t sev(get_severity(section_name));
            if(sev != nullptr)
            {
                // it already exists...
                //
                // we allow system severities to get updated in various ways
                // but user defined severities must be defined just once
                //
                if(!sev->is_system())
                {
                    throw duplicate_error("we found two severity levels named \""
                                        + section_name
                                        + "\" in your severity.ini file.");
                }

                // although we allow most parameters to be changed,
                // we do not want the severity level to be changed
                //
                std::string const severity_field(section_name + "::severity");
                if(config->is_defined(severity_field))
                {
                    long const level(config->get_long(severity_field));
                    if(level < 0 || level > 255)
                    {
                        throw invalid_severity("severity level must be between 0 and 255.");
                    }
                    if(static_cast<severity_t>(level) != sev->get_severity())
                    {
                        throw invalid_severity("severity level of a system entry cannot be changed. \""
                                             + section_name
                                             + "\" is trying to do so with "
                                             + std::to_string(level)
                                             + " instead of "
                                             + std::to_string(static_cast<long>(sev->get_severity()))
                                             + " (but it's best not to define the severity level at all for system severities).");
                    }
                }

                std::string const aliases_field(section_name + "::aliases");
                if(config->is_defined(aliases_field))
                {
                    std::string const aliases(config->get_string(aliases_field));
                    advgetopt::string_list_t names;
                    advgetopt::split_string(aliases, names, {","});
                    auto const & existing_names(sev->get_all_names());
                    for(auto n : names)
                    {
                        auto const existing_alias(std::find(existing_names.begin(), existing_names.end(), n));
                        if(existing_alias == existing_names.end())
                        {
                            sev->add_alias(n);
                        }
                    }
                }
            }
            else
            {
                std::string const severity_field(section_name + "::severity");
                if(!config->is_defined(severity_field))
                {
                    throw invalid_severity("severity level must be defined for non-system severity entries. \""
                                         + section_name
                                         + "\" was not found.");
                }
                long const level(config->get_long(severity_field));
                if(level < 0 || level > 255)
                {
                    throw invalid_severity("severity level must be between 0 and 255, "
                                         + std::to_string(level)
                                         + " is not valid.");
                }

                sev = get_severity(static_cast<severity_t>(level));
                if(sev != nullptr)
                {
                    throw duplicate_error("there is another severity with level "
                                         + std::to_string(level)
                                         + ", try using aliases=... instead.");
                }

                sev = std::make_shared<severity>(static_cast<severity_t>(level), section_name);

                std::string const aliases_field(section_name + "::aliases");
                if(config->is_defined(aliases_field))
                {
                    std::string const aliases(config->get_string(aliases_field));
                    advgetopt::string_list_t names;
                    advgetopt::split_string(aliases, names, {","});
                    for(auto n : names)
                    {
                        sev->add_alias(n);
                    }
                }

                add_severity(sev);
            }

            std::string const description_field(section_name + "::description");
            if(config->is_defined(description_field))
            {
                sev->set_description(config->get_string(description_field));
            }

            std::string const styles_field(section_name + "::styles");
            if(config->is_defined(styles_field))
            {
                sev->set_styles(config->get_string(styles_field));
            }

            std::string const default_field(section_name + "::default");
            if(config->is_defined(default_field))
            {
                if(advgetopt::is_true(config->get_string(default_field)))
                {
                    l->set_default_severity(sev);
                }
            }
        }
    }
}



}
// no name namespace




severity::severity(severity_t sev, std::string const & name, bool system)
    : f_severity(sev)
    , f_names(string_vector_t({name}))
    , f_system(system)
{
    if(sev < severity_t::SEVERITY_MIN || sev > severity_t::SEVERITY_MAX)
    {
        throw invalid_severity("the severity level can't be "
                            + std::to_string(static_cast<int>(sev))
                            + ". The possible range is ["
                              BOOST_PP_STRINGIZE(severity_t::SEVERITY_MIN)
                              ".."
                              BOOST_PP_STRINGIZE(severity_t::SEVERITY_MAX)
                              "]");
    }
}


severity_t severity::get_severity() const
{
    return f_severity;
}


bool severity::is_system() const
{
    return f_system;
}


/** \brief Mark the severity as registered with the logger.
 *
 * \internal
 *
 * This function is considered internal. It is called by the private logger
 * implementation at the time the severity is added to the system.
 *
 * You should never have to call this function directly. You can use the
 * is_registered() function to check whether the severity was already
 * added or not.
 *
 * \sa private_logger::add_severity()
 */
void severity::mark_as_registered()
{
    f_registered = true;
}


bool severity::is_registered() const
{
    return f_registered;
}


std::string severity::get_name() const
{
    return f_names[0];
}


/** \brief All the aliases must be added before registering a severity.
 *
 * This function adds another alias to the severity.
 *
 * \exception duplicate_error
 * The function raises the duplicate error whenever the alias being added
 * is one that already exists in the list of this severity aliases.
 * It will not warn about adding the same alias to another severity as
 * long as that other severity is not a system defined severity.
 */
void severity::add_alias(std::string const & name)
{
    auto it(std::find(f_names.begin(), f_names.end(), name));
    if(it != f_names.end())
    {
        throw duplicate_error("severity \""
                            + f_names[0]
                            + "\" already has an alias \""
                            + name
                            + "\".");
    }

    f_names.push_back(name);

    // TODO: we may be able to add a test here, but we allow
    //       adding new aliases to system severity

    if(is_registered())
    {
        private_logger::pointer_t l(get_private_logger());
        l->add_alias(l->get_severity(f_severity), name);
    }
}


string_vector_t severity::get_all_names() const
{
    return f_names;
}


void severity::set_description(std::string const & description)
{
    f_description = description;
}


std::string severity::get_description() const
{
    if(f_description.empty())
    {
        return get_name();
    }
    return f_description;
}


void severity::set_styles(std::string const & styles)
{
    f_styles = styles;
}


std::string severity::get_styles() const
{
    return f_styles;
}





void add_severity(severity::pointer_t sev)
{
    auto_add_severities();
    get_private_logger()->add_severity(sev);
}


severity::pointer_t get_severity(std::string const & name)
{
    auto_add_severities();
    return get_private_logger()->get_severity(name);
}


severity::pointer_t get_severity(message const & msg, std::string const & name)
{
    auto_add_severities();
    return get_private_logger(msg)->get_severity(name);
}


severity::pointer_t get_severity(severity_t sev)
{
    auto_add_severities();
    return get_private_logger()->get_severity(sev);
}


severity::pointer_t get_severity(message const & msg, severity_t sev)
{
    auto_add_severities();
    return get_private_logger(msg)->get_severity(sev);
}



} // snaplogger namespace


#if defined(__GNUC__) && __GNUC__ >= 7 && __GNUC_MINOR__ >= 5 && __GNUC_PATCHLEVEL__ >= 0
snaplogger::severity::pointer_t operator ""_sev (char const * name, unsigned long size)
{
    return snaplogger::get_severity(std::string(name, size));
}
#endif


// vim: ts=4 sw=4 et
