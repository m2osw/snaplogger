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
 * \brief Implemented the console appender.
 *
 * This file implements the console appender. This appender directly writes
 * to the console (std::cout).
 *
 * Since most console support color, this appender uses color to highlight
 * more important messages.
 */

// self
//
#include    "snaplogger/console_appender.h"

#include    "snaplogger/guard.h"


// snapdev
//
#include    <snapdev/lockfile.h>
#include    <snapdev/join_strings.h>


// C++
//
#include    <iostream>
#include    <set>


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



APPENDER_FACTORY(console);



struct name_to_style
{
    char const *        f_name = nullptr;
    char const *        f_style = nullptr;
    char const *        f_unstyle = nullptr;
};

// See: https://en.wikipedia.org/wiki/ANSI_escape_code
name_to_style const       g_name_to_style[] =
{
    { "bg-black",                   "\x1B[40m",  "\x1B[0m" },
    { "bg-blue",                    "\x1B[44m",  "\x1B[0m" },
    { "bg-bright-black",            "\x1B[100m", "\x1B[0m" },
    { "bg-bright-blue",             "\x1B[104m", "\x1B[0m" },
    { "bg-bright-cyan",             "\x1B[106m", "\x1B[0m" },
    { "bg-bright-green",            "\x1B[102m", "\x1B[0m" },
    { "bg-bright-magenta",          "\x1B[105m", "\x1B[0m" },
    { "bg-bright-red",              "\x1B[101m", "\x1B[0m" },
    { "bg-bright-white",            "\x1B[107m", "\x1B[0m" },
    { "bg-bright-yellow",           "\x1B[103m", "\x1B[0m" },
    { "bg-cyan",                    "\x1B[46m",  "\x1B[0m" },
    { "bg-green",                   "\x1B[42m",  "\x1B[0m" },
    { "bg-orange",                  "\x1B[43m",  "\x1B[0m" },
    { "bg-magenta",                 "\x1B[45m",  "\x1B[0m" },
    { "bg-red",                     "\x1B[41m",  "\x1B[0m" },
    { "bg-white",                   "\x1B[47m",  "\x1B[0m" },
    { "bg-yellow",                  "\x1B[43m",  "\x1B[0m" },
    { "black",                      "\x1B[30m",  "\x1B[0m" },
    { "blink-off",                  "\x1B[25m",  "\x1B[0m" },
    { "blue",                       "\x1B[34m",  "\x1B[0m" },
    { "bold",                       "\x1B[1m",   "\x1B[0m" },
    { "bright-black",               "\x1B[90m",  "\x1B[0m" },
    { "bright-blue",                "\x1B[94m",  "\x1B[0m" },
    { "bright-cyan",                "\x1B[96m",  "\x1B[0m" },
    { "bright-green",               "\x1B[92m",  "\x1B[0m" },
    { "bright-magenta",             "\x1B[95m",  "\x1B[0m" },
    { "bright-red",                 "\x1B[91m",  "\x1B[0m" },
    { "bright-white",               "\x1B[97m",  "\x1B[0m" },
    { "bright-yellow",              "\x1B[93m",  "\x1B[0m" },
    { "conceal",                    "\x1B[8m",   "\x1B[0m" },
    { "crossed-out",                "\x1B[9m",   "\x1B[0m" },
    { "cyan",                       "\x1B[36m",  "\x1B[0m" },
    { "default-font",               "\x1B[10m",  "\x1B[0m" },
    { "default-background-color",   "\x1B[49m",  "\x1B[0m" },
    { "default-foreground-color",   "\x1B[39m",  "\x1B[0m" },
    { "double-underline",           "\x1B[21m",  "\x1B[0m" },       // may remove bold instead
    { "encircled",                  "\x1B[52m",  "\x1B[0m" },
    { "faint",                      "\x1B[2m",   "\x1B[0m" },
    { "font0",                      "\x1B[10m",  "\x1B[0m" },
    { "font1",                      "\x1B[11m",  "\x1B[0m" },
    { "font2",                      "\x1B[12m",  "\x1B[0m" },
    { "font3",                      "\x1B[13m",  "\x1B[0m" },
    { "font4",                      "\x1B[14m",  "\x1B[0m" },
    { "font5",                      "\x1B[15m",  "\x1B[0m" },
    { "font6",                      "\x1B[16m",  "\x1B[0m" },
    { "font7",                      "\x1B[17m",  "\x1B[0m" },
    { "font8",                      "\x1B[18m",  "\x1B[0m" },
    { "font9",                      "\x1B[19m",  "\x1B[0m" },
    { "font10",                     "\x1B[20m",  "\x1B[0m" },
    { "fraktur",                    "\x1B[20m",  "\x1B[0m" },
    { "framed",                     "\x1B[51m",  "\x1B[0m" },
    { "green",                      "\x1B[32m",  "\x1B[0m" },
    { "inverse-off",                "\x1B[27m",  "\x1B[0m" },
    { "italic",                     "\x1B[3m",   "\x1B[0m" },
    { "magenta",                    "\x1B[35m",  "\x1B[0m" },
    { "normal",                     "\x1B[22m",  "\x1B[0m" },       // remove bold & faint
    { "not-crossed-out",            "\x1B[29m",  "\x1B[0m" },
    { "not-framed",                 "\x1B[54m",  "\x1B[0m" },       // remove italic & fraktur (font10)
    { "not-italic",                 "\x1B[23m",  "\x1B[0m" },       // remove italic & fraktur (font10)
    { "not-overline",               "\x1B[55m",  "\x1B[0m" },
    { "orange",                     "\x1B[33m",  "\x1B[0m" },
    { "overline",                   "\x1B[53m",  "\x1B[0m" },
    { "rapid-blink",                "\x1B[6m",   "\x1B[0m" },
    { "red",                        "\x1B[31m",  "\x1B[0m" },
    { "reveal",                     "\x1B[28m",  "\x1B[0m" },
    { "reverse-video",              "\x1B[7m",   "\x1B[0m" },
    { "slow-blink",                 "\x1B[5m",   "\x1B[0m" },
    { "standout",                   "\x1B[7m",   "\x1B[0m" },
    { "underline",                  "\x1B[4m",   "\x1B[0m" },
    { "underline-off",              "\x1B[24m",  "\x1B[0m" },
    { "white",                      "\x1B[37m",  "\x1B[0m" },
    { "yellow",                     "\x1B[33m",  "\x1B[0m" }
};




}
// no name namespace



console_appender::console_appender(std::string const name)
    : appender(name, "console")
{
}


console_appender::~console_appender()
{
}


bool console_appender::unique() const
{
    return true;
}


void console_appender::set_config(advgetopt::getopt const & opts)
{
    appender::set_config(opts);

    // FORCE STYLE
    //
    std::string const force_style_field(get_name() + "::force_style");
    if(opts.is_defined(force_style_field))
    {
        f_force_style = advgetopt::is_true(opts.get_string(force_style_field));
    }

    // LOCK
    //
    std::string const lock_field(get_name() + "::lock");
    if(opts.is_defined(lock_field))
    {
        f_lock = advgetopt::is_true(opts.get_string(lock_field));
    }

    // FLUSH
    //
    std::string const flush_field(get_name() + "::flush");
    if(opts.is_defined(flush_field))
    {
        f_flush = advgetopt::is_true(opts.get_string(flush_field));
    }

    // TTY
    //
    std::string const tty_field(get_name() + "::tty");
    if(opts.is_defined(tty_field))
    {
        f_tty = advgetopt::is_true(opts.get_string(tty_field));
    }

    // OUTPUT
    //
    std::string const output_field(get_name() + "::output");
    if(opts.is_defined(output_field))
    {
        f_output = opts.get_string(output_field);
    }
}


bool console_appender::get_force_style() const
{
    return f_force_style;
}


void console_appender::set_force_style(bool force_style)
{
    f_force_style = force_style;
}


void console_appender::process_message(message const & msg, std::string const & formatted_message)
{
    guard g;

    if(!f_initialized)
    {
        f_initialized = true;

        if(f_output == "stderr")
        {
            f_fd = fileno(stderr);
        }
        else if(f_output == "stdout")
        {
            f_fd = fileno(stdout);
        }
        else if(f_output == "console")
        {
            f_console.reset(open("/dev/console", O_WRONLY | O_APPEND | O_CLOEXEC | O_NOCTTY));
            f_fd = f_console.get();
        }
        // else -- we cannot really emit an error in this case...

        f_is_a_tty = isatty(f_fd);

        if(!f_is_a_tty && f_tty)
        {
            // the user require to use the console only if it is a TTY
            // void the fd if that's not the case
            //
            f_console.reset();  // in case it was a console
            f_fd = -1;
        }
    }

    if(f_fd == -1)
    {
        return;
    }

    std::unique_ptr<snapdev::lockfd> lock_file;
    if(f_lock)
    {
        lock_file = std::make_unique<snapdev::lockfd>(f_fd, snapdev::operation_t::OPERATION_EXCLUSIVE);
    }

    std::string style;
    std::string unstyle;
    if(f_is_a_tty || f_force_style)
    {
        severity::pointer_t const sev(snaplogger::get_severity(msg, msg.get_severity()));
        if(sev != nullptr)
        {
            advgetopt::string_list_t styles;
            advgetopt::split_string(sev->get_styles(), styles, {","});
            if(!styles.empty())
            {
                std::set<std::string> style_set;
                std::set<std::string> unstyle_set;
                for(auto s : styles)
                {
                    int i(0);
                    int j(sizeof(g_name_to_style) / sizeof(g_name_to_style[0]));
                    while(i < j)
                    {
                        int const p(i + (j - i) / 2);
                        int const r(s.compare(g_name_to_style[p].f_name));
                        if(r == 0)
                        {
                            style_set.insert(g_name_to_style[p].f_style);
                            unstyle_set.insert(g_name_to_style[p].f_unstyle);
                            break;
                        }
                        if(r < 0)
                        {
                            j = p;
                        }
                        else // if(r > 0)
                        {
                            i = p + 1;
                        }
                    }
                }
                if(!style_set.empty())
                {
                    style = snapdev::join_strings(style_set, std::string());
                }
                if(!unstyle_set.empty())
                {
                    unstyle = snapdev::join_strings(unstyle_set, std::string());
                }
            }
        }
    }

    ssize_t const l1(write(f_fd, style.c_str(), style.length()));
    if(static_cast<size_t>(l1) == style.length())
    {
        ssize_t const l2(write(f_fd, formatted_message.c_str(), formatted_message.length()));
        if(static_cast<size_t>(l2) == formatted_message.length())
        {
            ssize_t const l3(write(f_fd, unstyle.c_str(), unstyle.length()));
            if(static_cast<size_t>(l3) == unstyle.length())
            {
                return;
            }
        }
    }

    // an error occurred, what can we do about it?!
}





} // snaplogger namespace
// vim: ts=4 sw=4 et
