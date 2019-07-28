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
 * \brief Create a guard in all functions that need to run on their own.
 *
 * This librar y is thread safe. It uses on mutex which it locks and
 * unlocks using a class named guard:
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
#include    "guard.h"


// C lib
//
#include    <pthread.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{



bool                    g_initialized = false;
pthread_mutex_t         g_mutex = pthread_mutex_t();



}
// no name namespace





guard::guard()
{
    if(!g_initialized)
    {
        // TODO: this could happen after threads were created
        //       how do you resolve such?!
        //
        g_initialized = true;

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&g_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    pthread_mutex_lock(&g_mutex);
}


guard::~guard()
{
    pthread_mutex_unlock(&g_mutex);
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
