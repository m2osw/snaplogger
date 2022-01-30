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
#pragma once

/** \file
 * \brief The console appender sends its output to stdout.
 *
 * This file declares the console appender class.
 */

// self
//
#include    "snaplogger/appender.h"


// snapdev lib
//
#include    <snapdev/raii_generic_deleter.h>



namespace snaplogger
{


class console_appender
    : public appender
{
public:
    typedef std::shared_ptr<console_appender>      pointer_t;

                            console_appender(std::string const name);
    virtual                 ~console_appender() override;

    virtual bool            unique() const override;
    virtual void            set_config(advgetopt::getopt const & params) override;

protected:
    virtual void            process_message(message const & msg, std::string const & formatted_message) override;

private:
    std::string             f_filename = std::string();
    std::string             f_output = std::string("stderr");
    int                     f_fd = -1;
    snapdev::raii_fd_t      f_console = snapdev::raii_fd_t();
    bool                    f_initialized = false;
    bool                    f_force_style = false;
    bool                    f_lock = true;
    bool                    f_flush = true;
    bool                    f_is_a_tty = true;
};


} // snaplogger namespace
// vim: ts=4 sw=4 et
