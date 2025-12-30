// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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

// Tell catch we want it to add the runner code in this file.
#define CATCH_CONFIG_RUNNER

// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/logger.h>
#include    <snaplogger/version.h>


// libexcept
//
#include    <libexcept/exception.h>


// C++
//
#include    <sstream>



void init_callback()
{
    libexcept::set_collect_stack(libexcept::collect_stack_t::COLLECT_STACK_NO);

    // the ready() function never gets called if we don't do it because
    // we do not currently initialize the logger using the opts
    //
    // see: snaplogger::process_logger_options()
    //
    snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
    l->ready();
}


int main(int argc, char * argv[])
{
    return SNAP_CATCH2_NAMESPACE::snap_catch2_main(
              "snaplogger"
            , SNAPLOGGER_VERSION_STRING
            , argc
            , argv
            , &init_callback
        );
}


// vim: ts=4 sw=4 et
