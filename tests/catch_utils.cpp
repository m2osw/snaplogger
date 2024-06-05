// Copyright (c) 2006-2024  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/utils.h>



CATCH_TEST_CASE("is_rotational", "[utils]")
{
    CATCH_START_SECTION("is_rotational: verify cache returns proper result")
    {
        // this really just verifies that the cache works
        //
        bool const here1(snaplogger::is_rotational("."));
        CATCH_REQUIRE(errno == 0);
        bool const here2(snaplogger::is_rotational("."));
        CATCH_REQUIRE(errno == 0);
        CATCH_REQUIRE(here1 == here2);

        bool const root1(snaplogger::is_rotational("/"));
        CATCH_REQUIRE(errno == 0);
        bool const root2(snaplogger::is_rotational("/"));
        CATCH_REQUIRE(errno == 0);
        CATCH_REQUIRE(root1 == root2);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("is_rotational: verify non-existant file")
    {
        bool const bad(snaplogger::is_rotational("this-file-does-not-exist"));
        CATCH_REQUIRE(errno == ENOENT);
        CATCH_REQUIRE_FALSE(bad);
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
