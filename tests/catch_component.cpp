// Copyright (c) 2006-2021  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "catch_main.h"


// snaplogger lib
//
#include    <snaplogger/component.h>






CATCH_TEST_CASE("component", "[component]")
{
    CATCH_START_SECTION("component: Make sure creating component generates unique entries")
    {
        // this worked from the start since the private logger instance
        // uses a map to store the components
        //
        struct name_ptr
        {
            typedef std::vector<name_ptr>   vector_t;

            std::string     f_name = std::string();
            snaplogger::component::pointer_t
                            f_component = snaplogger::component::pointer_t();
        };
        name_ptr::vector_t names =
        {
            { "component1", },
            { "component2", },
            { "component3", },
            { "component4", },
            { "component5", },
            { "component6", },
            { "component7", },
            { "component8", },
            { "component9", },
            { "component10", },
        };

        // create the components
        //
        for(auto & p : names)
        {
            p.f_component = snaplogger::get_component(p.f_name);
        }

        // verify the component pointers
        //
        for(auto & p : names)
        {
            CATCH_REQUIRE(p.f_component == snaplogger::get_component(p.f_name));
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
