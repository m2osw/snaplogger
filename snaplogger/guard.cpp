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

/** \file
 * \brief Create a guard in all functions that need to run on their own.
 *
 * This library is thread safe. It uses one mutex to guard any access
 * which requires safe access:
 *
 * \code
 *     {
 *         guard        lock;
 *
 *         ...do multi-thread safe work...
 *     }
 * \endcode
 */

// self
//
#include    "snaplogger/guard.h"


// cppthread
//
#include    <cppthread/guard.h>
#include    <cppthread/mutex.h>
#include    <cppthread/thread.h>


// C++
//
#include    <iomanip>
#include    <iostream>
#include    <mutex>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{



std::once_flag          g_init_once = std::once_flag();
cppthread::mutex *      g_mutex = new cppthread::mutex;
bool                    g_mutex_done = false;



}
// no name namespace





guard::guard()
{
    std::call_once(g_init_once, []{
        g_mutex = new cppthread::mutex;
    });

    g_mutex->lock();
}


guard::~guard()
{
    g_mutex->unlock();
}


void delete_guard()
{
    g_mutex_done = true;
    delete g_mutex;
    g_mutex = nullptr;
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
