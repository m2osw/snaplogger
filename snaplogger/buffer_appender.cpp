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
 * \brief Appender used to bufferize all the log data.
 *
 * This appender is a memory buffer. It is primarily used by the tests
 * to be able to verify all the other features avalaible in the library,
 * such as the line formatter.
 *
 * It could be useful to you in case you have to create a network connection
 * and do so in parallel. While the connection is being created, you may
 * want to bufferize the logs using this appender.
 *
 * Keep in mind that a buffer uses RAM which you may need for other things.
 */

// self
//
#include    "snaplogger/buffer_appender.h"

#include    "snaplogger/guard.h"


// snapdev lib
//
#include    <snapdev/lockfile.h>


// C++ lib
//
#include    <iostream>


// C lib
//
#include    <fcntl.h>
#include    <sys/stat.h>
#include    <sys/types.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{



APPENDER_FACTORY(buffer);



}
// no name namespace



buffer_appender::buffer_appender(std::string const name)
    : appender(name, "buffer")
{
}


void buffer_appender::process_message(message const & msg, std::string const & formatted_message)
{
    snap::NOTUSED(msg);

    guard g;

    *this << formatted_message << std::endl;
}





} // snaplogger namespace
// vim: ts=4 sw=4 et
