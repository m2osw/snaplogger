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
 * \brief Format a message.
 *
 * This file declares the format class used to transform a message with
 * the administrator defined format.
 */


// self
//
#include    <snaplogger/variable.h>



namespace snaplogger
{




class format
{
public:
    typedef std::shared_ptr<format>     pointer_t;

                        format(std::string const & f);

    std::string         get_format() const;
    std::string         process_message(message const & msg, bool ignore_on_no_repeat = false);

private:
    std::string const   f_format;
    variable::vector_t  f_variables = variable::vector_t();
};





} // snaplogger namespace
// vim: ts=4 sw=4 et
