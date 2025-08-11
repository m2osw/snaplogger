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
 * \brief Implementation of the environment variable support.
 *
 * This file implements a variable which retrieves its value from the
 * process environment. For example, you could retrieve the path to
 * the HOME directory.
 *
 * This is often used to distinguish between runs.
 */

// self
//
#include    "snaplogger/exception.h"
#include    "snaplogger/variable.h"


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{


DEFINE_LOGGER_VARIABLE(env)
{
    auto params(get_params());
    if(params.empty())
    {
        throw invalid_variable("the ${env:...} variable must have a \"name\" parameter.");
    }
    if(params[0]->get_name() != "name")
    {
        throw invalid_variable("the ${env:...} variable first parameter must be its \"name\" parameter.");
    }
    std::string const env_name(params[0]->get_value());
    if(env_name.empty())
    {
        throw invalid_variable("the ${env:name=...} variable first parameter cannot be empty.");
    }
    char const * env(getenv(env_name.c_str()));
    if(env != nullptr)
    {
        value += env;
    }

    variable::process_value(msg, value);
}


}
// no name namespace


} // snaplogger namespace
// vim: ts=4 sw=4 et
