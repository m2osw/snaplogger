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
 * \brief Implementation of the date, time, locale variables.
 *
 * This file implements a set of variables offering ways to display the
 * message timestamp date and time.
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


constexpr char const * g_day_name[] =
{
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

constexpr char const * g_month_name[] =
{
    "January",
    "February",
    "Marsh",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

timespec const g_start_date = []() { timespec tp; clock_gettime(CLOCK_REALTIME_COARSE, &tp); return tp; }();





DEFINE_LOGGER_VARIABLE(date)
{
    tm t;
    timespec const & timestamp(msg.get_timestamp());
    gmtime_r(&timestamp.tv_sec, &t);

    std::string date_format("%Y/%m/%d");
    auto params(get_params());
    if(!params.empty())
    {
        auto const p(params[0]->get_name());
        if(p == "day_of_week_name")
        {
            value += g_day_name[t.tm_wday];
            date_format.clear();
        }
        else if(p == "day_of_week")
        {
            date_format = "%w"; // TBD: people may want %u though...
        }
        else if(p == "year_week")
        {
            date_format = "%U"; // TBD: people may want %V or %W though...
        }
        else if(p == "year_day")
        {
            date_format = "%j";
        }
        else if(p == "month_name")
        {
            value += g_month_name[t.tm_mon];
            date_format.clear();
        }
        else if(p == "month")
        {
            date_format = "%m";
        }
        else if(p == "year")
        {
            date_format = "%Y";
        }
        else
        {
            throw invalid_variable("the ${hostbyname:...} variable first parameter must be its name parameter.");
        }
    }

    if(!date_format.empty())
    {
        char buf[256];
        strftime(buf, sizeof(buf), date_format.c_str(), &t);
        buf[sizeof(buf) - 1] = '\0';
        value += buf;
    }

    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(time)
{
    auto nanosec = [](timespec const & tp)
    {
        std::string sec(std::to_string(tp.tv_sec));
        std::string nsec(std::to_string(tp.tv_nsec));
        if(nsec.length() < 9)
        {
            nsec = std::string("000000000").substr(0, 9 - nsec.length()) + nsec;
        }
        return sec + nsec;
    };

    tm t;
    timespec timestamp(msg.get_timestamp());
    gmtime_r(&timestamp.tv_sec, &t);

    std::string time_format("%H:%M:%S");
    auto params(get_params());
    if(!params.empty())
    {
        auto const p(params[0]->get_name());
        if(p == "hour")
        {
            if(params[0]->get_value() == "12")
            {
                time_format = "%I";
            }
            else
            {
                time_format = "%H";
            }
        }
        else if(p == "minute")
        {
            time_format = "%M"; // TBD: people may want %u though...
        }
        else if(p == "second")
        {
            time_format = "%S"; // TBD: people may want %V or %W though...
        }
        else if(p == "nanosecond")
        {
            value += std::to_string(timestamp.tv_nsec);
            time_format.clear();
        }
        else if(p == "unix")
        {
            value += std::to_string(timestamp.tv_sec);
            time_format.clear();
        }
        else if(p == "meridiem")
        {
            // TODO: we should force English here (AM or PM)
            //
            time_format = "%p";
        }
        else if(p == "offset")
        {
            timespec offset;
            offset.tv_nsec = timestamp.tv_nsec - g_start_date.tv_nsec;
            offset.tv_sec = timestamp.tv_sec - g_start_date.tv_sec;
            if(offset.tv_nsec < 0)
            {
                --offset.tv_sec;
                offset.tv_nsec += 1000000000;
            }
            value += nanosec(offset);
            time_format.clear();
        }
        else if(p == "process")
        {
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timestamp);
            value += nanosec(timestamp);
            time_format.clear();
        }
        else if(p == "thread")
        {
            clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timestamp);
            value += nanosec(timestamp);
            time_format.clear();
        }
        else if(p == "process_ms")
        {
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timestamp);
            gmtime_r(&timestamp.tv_sec, &t);
            std::string ms(std::to_string(timestamp.tv_nsec / 1000000UL));
            if(ms.length() < 3)
            {
                ms = std::string("000").substr(0, 3 - ms.length());
            }
            time_format += ".";
            time_format += ms;
        }
        else if(p == "thread_ms")
        {
            clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timestamp);
            gmtime_r(&timestamp.tv_sec, &t);
            std::string ms(std::to_string(timestamp.tv_nsec / 1000000UL));
            if(ms.length() < 3)
            {
                ms = std::string("000").substr(0, 3 - ms.length());
            }
            time_format += ".";
            time_format += ms;
        }
        else
        {
            throw invalid_variable("the ${hostbyname:...} variable first parameter must be its name parameter.");
        }
    }

    if(!time_format.empty())
    {
        char buf[256];
        strftime(buf, sizeof(buf), time_format.c_str(), &t);
        buf[sizeof(buf) - 1] = '\0';
        value += buf;
    }


    variable::process_value(msg, value);
}


DEFINE_LOGGER_VARIABLE(locale)
{
    tm t;
    timespec const & timestamp(msg.get_timestamp());
    localtime_r(&timestamp.tv_sec, &t);

    std::string locale_format("%c");
    auto params(get_params());
    if(!params.empty())
    {
        auto const p(params[0]->get_name());
        if(p == "day_of_week_name")
        {
            locale_format = "%A";   // TBD: support %a as well
        }
        else if(p == "month_name")
        {
            locale_format = "%B";     // TBD: support %b as well
        }
        else if(p == "date")
        {
            locale_format = "%x";
        }
        else if(p == "time")
        {
            locale_format = "%X";
        }
        else if(p == "meridiem")
        {
            locale_format = "%p";
        }
        else if(p == "timezone")
        {
            locale_format = "%Z";
        }
        else if(p == "timezone_offset")
        {
            locale_format = "%z";
        }
        // else -- assume params are system parameters
    }

    if(!locale_format.empty())
    {
        char buf[256];
        strftime(buf, sizeof(buf), locale_format.c_str(), &t);
        buf[sizeof(buf) - 1] = '\0';
        value += buf;
    }

    variable::process_value(msg, value);
}


}
// no name namespace


} // snaplogger namespace
// vim: ts=4 sw=4 et
