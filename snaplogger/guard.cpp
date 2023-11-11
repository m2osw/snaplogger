// Copyright (c) 2013-2023  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Create a guard in all functions that need to run on their own.
 *
 * This library is thread safe. It uses one mutex to guard any access
 * which requires safe access:
 *
 * \code
 *     {
 *         guard        g;
 *
 *         ...do multi-thread safe work...
 *     }
 * \endcode
 */

// self
//
#include    "snaplogger/guard.h"


// cppthread lib
//
#include    <cppthread/guard.h>
#include    <cppthread/mutex.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{



cppthread::mutex *      g_mutex = nullptr;



}
// no name namespace





guard::guard()
{
    {
        // we know for sure that g_system_mutex was already initialized
        // so we can use it here
        //
        cppthread::guard lock(*cppthread::g_system_mutex);

        if(g_mutex == nullptr)
        {
            g_mutex = new cppthread::mutex;
        }
    }

    g_mutex->lock();
}


guard::~guard()
{
    g_mutex->unlock();
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
