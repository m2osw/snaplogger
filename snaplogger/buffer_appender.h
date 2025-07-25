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
#pragma once

/** \file
 * \brief The buffer appender sends its output to a memory buffer.
 *
 * This file declares the buffer appender class.
 */

// self
//
#include    <snaplogger/appender.h>


// C++
//
#include    <sstream>



namespace snaplogger
{


class buffer_appender
    : public appender
    , public std::stringstream
{
public:
    typedef std::shared_ptr<buffer_appender>      pointer_t;

                            buffer_appender(std::string const name);

    bool                    empty() const;
    void                    clear(bool keep_buffer = false);
    std::string             str();
    void                    str(std::string const & buf);

protected:
    virtual void            process_message(message const & msg, std::string const & formatted_message) override;
};


} // snaplogger namespace
// vim: ts=4 sw=4 et
