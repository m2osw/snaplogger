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
#include    "snaplogger/format.h"


// C++ lib
//
#include    <vector>



namespace snaplogger
{


class appender
{
public:
    typedef std::shared_ptr<appender>       pointer_t;
    typedef std::vector<pointer_t>          vector_t;

                                appender(std::string const & name);
    virtual                     ~appender();

    std::string const &         get_name() const;
    bool                        is_enabled() const;
    virtual void                set_enabled(bool status);

    severity_t                  get_severity() const;
    virtual void                set_severity(severity_t severity_level);

    virtual void                set_config(advgetopt::getopt const & params);

    format::pointer_t           get_format() const;
    virtual format::pointer_t   set_format(format::pointer_t new_format);

    void                        send_message(message const & msg);

protected:
    virtual void                process_message(message const & msg, std::string const & formatted_message);

private:
    std::string const           f_name;
    bool                        f_enabled = true;
    format::pointer_t           f_format = format::pointer_t();
    severity_t                  f_severity = severity_t::SEVERITY_INFORMATION;
};


class appender_factory
{
public:
                                    appender_factory(std::string const & type);
    virtual                         ~appender_factory();

    virtual appender::pointer_t     create(std::string const & name) = 0;
};


#define APPENDER_FACTORY(name)                                          \
    class name##_appender_factory                                       \
        : public ::snaplogger::appender_factory                         \
    {                                                                   \
    public:                                                             \
        name##_appender_factory()                                       \
            : appender_factory(#name)                                   \
        {}                                                              \
        virtual ::snaplogger::appender::pointer_t create(std::string const & name) override   \
        {                                                               \
            return std::make_shared<name##_appender>(name);             \
        }                                                               \
    };                                                                  \
    name##_appender_factory      g_##file##_factory


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
