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
#pragma once

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
//
#include    <snaplogger/format.h>


// advgetopt
//
#include    <advgetopt/advgetopt.h>
#include    <advgetopt/utils.h>


// C++
//
#include    <regex>
#include    <vector>



namespace snaplogger
{


typedef std::shared_ptr<std::regex>         regex_pointer_t;

constexpr long const                        NO_REPEAT_OFF     = 0;
constexpr long const                        NO_REPEAT_MAXIMUM = 100;
constexpr long const                        NO_REPEAT_DEFAULT = 10;

static_assert(NO_REPEAT_DEFAULT <= NO_REPEAT_MAXIMUM
            , "the default no-repeat must be lower or equal to the maximum.");


class appender
{
public:
    typedef std::shared_ptr<appender>       pointer_t;
    typedef std::vector<pointer_t>          vector_t;

                                appender(std::string const & name, std::string const & type = "null");
    virtual                     ~appender();

    std::string const &         get_type() const;
    void                        set_name(std::string const & name);
    std::string const &         get_name() const;
    bool                        is_enabled() const;
    virtual void                set_enabled(bool status);
    virtual bool                unique() const;

    severity_t                  get_severity() const;
    virtual void                set_severity(severity_t severity_level);
    virtual void                reduce_severity(severity_t severity_level);
    virtual void                increase_severity(severity_t severity_level);
    bool                        operator < (appender const & rhs) const;

    virtual void                set_config(advgetopt::getopt const & params);
    virtual void                reopen();
    void                        add_component(component::pointer_t comp);

    format::pointer_t           get_format() const;
    virtual format::pointer_t   set_format(format::pointer_t new_format);

    long                        get_bytes_per_minute() const;
    std::size_t                 get_bitrate_dropped_messages() const;

    void                        send_message(message const & msg);

protected:
    virtual void                process_message(message const & msg, std::string const & formatted_message);

private:
    std::string const           f_type;
    std::string                 f_name = std::string();
    bool                        f_enabled = true;
    format::pointer_t           f_format = format::pointer_t();
    severity_t                  f_severity = severity_t::SEVERITY_DEFAULT;
    component::pointer_t        f_normal_component = component::pointer_t();
    component::set_t            f_components = component::set_t();
    regex_pointer_t             f_filter = regex_pointer_t();
    std::size_t                 f_no_repeat_size = NO_REPEAT_OFF;
    std::deque<std::string>     f_last_messages = {};
    long                        f_bytes_per_minute = 0;
    long                        f_bytes_received = 0;
    time_t                      f_bytes_minute = 0;
    std::size_t                 f_bitrate_dropped_messages = 0;
};


class appender_factory
{
public:
    typedef std::shared_ptr<appender_factory>   pointer_t;

                                    appender_factory(std::string const & type);
    virtual                         ~appender_factory();

    std::string const &             get_type() const;
    virtual appender::pointer_t     create(std::string const & name) = 0;

private:
    std::string const               f_type;
};


#define APPENDER_FACTORY(name)                                          \
    class name##_appender_factory                                       \
        : public ::snaplogger::appender_factory                         \
    {                                                                   \
    public:                                                             \
        name##_appender_factory()                                       \
            : appender_factory(#name)                                   \
        {}                                                              \
        virtual ::snaplogger::appender::pointer_t                       \
                            create(std::string const & name) override   \
        {                                                               \
            return std::make_shared<name##_appender>(name);             \
        }                                                               \
    };                                                                  \
    int __attribute__((unused))                                         \
        g_##name##_register_appender_factory = []() {                   \
            register_appender_factory(                                  \
                    std::make_shared<name##_appender_factory>());       \
            return 0;                                                   \
        }()


void                register_appender_factory(appender_factory::pointer_t factory);
appender::pointer_t create_appender(std::string const & type, std::string const & name);




class safe_format
{
public:
                        safe_format(appender::pointer_t a, format::pointer_t new_format);
                        ~safe_format();

private:
    appender::pointer_t f_appender;
    format::pointer_t   f_old_format;
};




} // snaplogger namespace
// vim: ts=4 sw=4 et
