// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/snaplogger
// contact@m2osw.com
//

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


// snapdev
//
#include    <snapdev/lockfile.h>


// C++
//
#include    <iostream>


// C
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


bool buffer_appender::process_message(message const & msg, std::string const & formatted_message)
{
    snapdev::NOT_USED(msg);

    guard g;

    *this << formatted_message;
    return true;
}


bool buffer_appender::empty() const
{
    return rdbuf()->in_avail() == 0;
}


void buffer_appender::clear(bool keep_buffer)
{
    std::stringstream::clear();     // ios bits
    if(keep_buffer)
    {
        // just rewind (i.e. this means all the buffers are kept)
        //
        seekp(0);
    }
    else
    {
        std::stringstream::str(std::string());
    }
}


std::string buffer_appender::str()
{
    // tellp() may not be the end of the file if clear(true) was called
    //
    pos_type const pos(tellp());
    std::string result(pos, '\0');
    seekg(0);
    read(const_cast<char *>(result.data()), pos);
    return result;
}


void buffer_appender::str(std::string const & buf)
{
    // note: std::stringstream::str(buf) may reset all the buffers, here
    //       we make sure the buffers are kept and we just write buf in
    //       them after adjusting the output seek position. The STL may
    //       do it right for large `buf`, though, but we do not expect
    //       the `buf` parameter to ever be very large.
    //
    clear(true);
    *this << buf;
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
