// Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Variables are used to dynamically add parameters to log messages.
 *
 * This file declares the base variable class.
 *
 * The format defines \em functions which are written as in
 * `${function-name}`.
 *
 * Parameters can be passed to these functions by adding `:<param>`
 * to those definitions. These are named parameters and their default
 * value is "present" or not. A specific value can be assignd using
 * the equal sign as in `:param=<value>`.
 *
 * For example, the date function can be called as follow:
 *
 * \code
 *     ${date:year:align=right:exact_width=2}
 * \endcode
 *
 * The `year` parameter is specific to the `date` function. The other
 * parameters are available whatever the function. This variable asks
 * to truncate the year to 2 character right aligned (i.e. "18" in
 * "2018".)
 *
 * In C, this would look something like:
 *
 * \code
 *     date(FLAG_YEAR, ALIGN_RIGHT, 2);
 * \endcode
 */


// self
//
#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/variable.h"


// libutf8 lib
//
#include    <libutf8/libutf8.h>


// snapdev lib
//
#include    <snapdev/not_used.h>


// C++ lib
//
#include    <algorithm>
#include    <iostream>
#include    <queue>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{



//
// Change Parameters
//


DECLARE_FUNCTION(padding)
{
    snapdev::NOT_USED(msg);

    std::u32string pad;
    if(p->get_type() == param::type_t::TYPE_STRING)
    {
        pad = libutf8::to_u32string(p->get_value());
    }
    else
    {
        std::int64_t const digit(p->get_integer());
        if(digit < 0 || digit > 9)
        {
            throw invalid_parameter(
                      "the ${...:padding=<value>} when set to a number must be one digit ('0' to '9'), not \""
                    + std::to_string(digit)
                    + "\".");
        }
        pad = libutf8::to_u32string(std::to_string(digit));
    }
    if(pad.length() == 1)
    {
        d.set_param(std::string("padding"), pad);
    }
    else
    {
        throw invalid_parameter(
                  "the ${...:padding=' '} must be exactly one character, not \""
                + p->get_value()
                + "\".");
    }
}


DECLARE_FUNCTION(align)
{
    snapdev::NOT_USED(msg);

    if(p->get_value() == "left")
    {
        d.set_param("align", "L");
    }
    else if(p->get_value() == "right")
    {
        d.set_param("align", "R");
    }
    else if(p->get_value() == "center")
    {
        d.set_param("align", "C");
    }
    else
    {
        throw invalid_parameter("the ${...:align=left|center|right} was expected, got \""
                              + p->get_value()
                              + "\".");
    }
}





//
// Apply Functions
//


DECLARE_FUNCTION(max_width)
{
    snapdev::NOT_USED(msg);

    std::int64_t const max_width(p->get_integer());
    std::int64_t const extra(static_cast<std::int64_t>(d.get_value().length()) - max_width);
    if(extra > 0)
    {
        char32_t const align(d.get_param("align", U"L")[0]);
        if(align == U'C')
        {
            d.set_value(d.get_value().substr(extra / 2, max_width));
        }
        else if(align == U'L')
        {
            d.get_value().resize(max_width);
        }
        else
        {
            d.set_value(d.get_value().substr(extra));
        }
    }
}


DECLARE_FUNCTION(min_width)
{
    snapdev::NOT_USED(msg);

    std::int64_t const min_width(p->get_integer());
    std::int64_t const pad(min_width - static_cast<std::int64_t>(d.get_value().length()));
    if(pad > 0)
    {
        char32_t const padding_char(d.get_param("padding", U" ")[0]);
        char32_t const align(d.get_param("align", U"L")[0]);
        if(align == U'C')
        {
            std::u32string const padding(pad / 2, padding_char);
            d.set_value(padding + d.get_value() + padding);
            if((pad & 1) != 0)
            {
                d.get_value() += padding_char;
            }
        }
        else
        {
            std::u32string const padding(pad, padding_char);
            if(align == U'L')
            {
                d.get_value() += padding;
            }
            else
            {
                d.set_value(padding + d.get_value());
            }
        }
    }
}


DECLARE_FUNCTION(exact_width)
{
    snapdev::NOT_USED(msg);

    std::int64_t const exact_width(p->get_integer());

    std::int64_t const pad(exact_width - static_cast<std::int64_t>(d.get_value().length()));
    char32_t const align(d.get_param("align", U"L")[0]);
    if(pad > 0)
    {
        char32_t const padding_char(d.get_param("padding", U" ")[0]);
        if(align == U'C')
        {
            std::u32string const padding(pad / 2, padding_char);
            d.set_value(padding + d.get_value() + padding);
            if((pad & 1) != 0)
            {
                d.get_value() += padding_char;
            }
        }
        else
        {
            std::u32string const padding(pad, padding_char);
            if(align == U'L')
            {
                d.get_value() += padding;
            }
            else
            {
                d.set_value(padding + d.get_value());
            }
        }
        return;
    }

    if(pad < 0)
    {
        if(align == U'C')
        {
            d.set_value(d.get_value().substr(-pad / 2, exact_width));
        }
        else if(align == U'L')
        {
            d.get_value().resize(exact_width);
        }
        else
        {
            d.set_value(d.get_value().substr(-pad));
        }
    }
}





DECLARE_FUNCTION(append)
{
    snapdev::NOT_USED(msg);

    std::string const append_utf8(p->get_value());
    std::u32string const str(libutf8::to_u32string(append_utf8));
    d.get_value() += str;
}

DECLARE_FUNCTION(prepend)
{
    snapdev::NOT_USED(msg);

    std::string const prepend_utf8(p->get_value());
    std::u32string const prepend(libutf8::to_u32string(prepend_utf8));
    d.set_value(prepend + d.get_value());
}





DECLARE_FUNCTION(escape)
{
    snapdev::NOT_USED(msg);

    std::string to_escape(p->get_value());
    if(to_escape.empty())
    {
        to_escape = "\\\n\r\t";
    }

    std::u32string e(libutf8::to_u32string(to_escape));
    std::u32string r;
    for(auto wc : d.get_value())
    {
        if(e.find(wc) != std::u32string::npos)
        {
            if(wc >= 0x80 && wc < 0xA0)
            {
                // "graphical controls"
                //
                r += '@';
                wc -= 0x40;
            }
            else if(wc < 0x20)
            {
                switch(wc)
                {
                case '\a':
                    r += '\\';
                    wc = U'a';
                    break;

                case '\b':
                    r += '\\';
                    wc = U'b';
                    break;

                case '\f':
                    r += '\\';
                    wc = U'f';
                    break;

                case '\n':
                    r += '\\';
                    wc = U'n';
                    break;

                case '\r':
                    r += '\\';
                    wc = U'r';
                    break;

                case '\t':
                    r += '\\';
                    wc = U't';
                    break;

                case '\v':
                    r += '\\';
                    wc = U'v';
                    break;

                default:
                    r += '^';
                    wc += 0x40;
                    break;

                }
            }
            else
            {
                r += '\\';
            }
        }
        r += wc;
    }
    d.set_value(r);
}





DECLARE_FUNCTION(caps)
{
    snapdev::NOT_USED(msg, p);

    std::u32string r;
    bool first(true);
    for(auto wc : d.get_value())
    {
        // we should look into having a libutf8::isalpha() function
        // instead of testing two special cases
        //
        if(std::isspace(wc)
        || wc == '-')
        {
            first = true;
            r += wc;
        }
        else
        {
            // TODO: use libutf8 to properly handle multi-byte characters
            //
            if(first)
            {
                r += std::towupper(wc);
                first = false;
            }
            else
            {
                r += std::towlower(wc);
            }
        }
    }
    d.set_value(r);
}

DECLARE_FUNCTION(lower)
{
    snapdev::NOT_USED(msg, p);

    // TODO: use libutf8 to properly handle multi-byte characters
    //
    std::transform(d.get_value().begin(), d.get_value().end(), d.get_value().begin(), std::towlower);
}

DECLARE_FUNCTION(upper)
{
    snapdev::NOT_USED(msg, p);

    // TODO: use libutf8 to properly handle multi-byte characters
    //
    std::transform(d.get_value().begin(), d.get_value().end(), d.get_value().begin(), std::towupper);
}




}
// no name namespace







} // snaplogger namespace
// vim: ts=4 sw=4 et
