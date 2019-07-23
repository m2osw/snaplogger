// Snap Websites Servers -- logger
// Copyright (c) 2013-2019  Made to Order Software Corp.  All Rights Reserved
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#pragma once


// advgetopt lib
//
#include "advgetopt/advgetopt.h"
#include "advgetopt/option_info.h"


// C++ lib
//
#include <memory>



namespace snap_logger
{


enum class log_level_t
{
    LOG_LEVEL_OFF,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,

    LOG_LEVEL_DEFAULT = LOG_LEVEL_INFO
};

enum class log_security_t
{
    LOG_SECURITY_NONE,
    LOG_SECURITY_SECURE
};

class logger
{
public:
                    logger(log_level_t const log_level, char const * file = nullptr, char const * func = nullptr, int const line = -1);
                    logger(logger const & l);
                    ~logger();

    // avoid assignments
    logger &        operator = (logger const &) = delete;

    logger &        operator () ();
    logger &        operator () (log_security_t const v);
    logger &        operator () (char const * s);
    logger &        operator () (wchar_t const * s);
    logger &        operator () (char16_t const * s);
    logger &        operator () (char32_t const * s);
    logger &        operator () (std::string const & s);
    logger &        operator () (std::wstring const & s);
    logger &        operator () (std::u16string const & s);
    logger &        operator () (std::u32string const & s);
    logger &        operator () (advgetopt::option_info_ref const & s);
    logger &        operator () (char const v);
    logger &        operator () (signed char const v);
    logger &        operator () (unsigned char const v);
    logger &        operator () (wchar_t const v);
    logger &        operator () (char16_t const v);
    logger &        operator () (char32_t const v);
    logger &        operator () (signed short const v);
    logger &        operator () (unsigned short const v);
    logger &        operator () (signed int const v);
    logger &        operator () (unsigned int const v);
    logger &        operator () (signed long const v);
    logger &        operator () (unsigned long const v);
    logger &        operator () (signed long long const v);
    logger &        operator () (unsigned long long const v);
    logger &        operator () (float const v);
    logger &        operator () (double const v);
    logger &        operator () (long double const v);
    logger &        operator () (bool const v);
    logger &        operator () (void const * p);

protected:
    mutable bool    f_ignore = false;

private:
    log_level_t     f_log_level = log_level_t::LOG_LEVEL_DEFAULT;
    char const *    f_file = nullptr;
    char const *    f_func = nullptr;
    int32_t         f_line = 1;
    log_security_t  f_security = log_security_t::LOG_SECURITY_SECURE;
    std::string     f_message = std::string();
};

void                add_logger_command_line_options(advgetopt::getopt & opt);

void                set_progname(std::string const & progname);
std::string         get_progname();

void                configure_null();
void                configure_console();
void                configure_logfile(std::string const & logfile);
void                configure_syslog();
void                configure_conffile(std::string const & filename);
void                unconfigure();
void                reconfigure();
bool                is_configured();

log_level_t         get_log_output_level();
void                set_log_output_level(log_level_t level);
void                reduce_log_output_level(log_level_t level);
bool                is_enabled_for(log_level_t const level);

class raii_log_level
{
public:
    raii_log_level(log_level_t new_level)
        : f_save_log_level(get_log_output_level())
    {
        set_log_output_level(new_level);
    }

    ~raii_log_level()
    {
        set_log_output_level(f_save_log_level);
    }

private:
    log_level_t     f_save_log_level = log_level_t::LOG_LEVEL_DEFAULT;
};

//logger & operator << (logger & l, std::basic_string<char> const &                msg);
//logger & operator << (logger & l, std::basic_string<wchar_t> const &             msg);
//logger & operator << (logger & l, snap_config::snap_config_parameter_ref const & msg);
//logger & operator << (logger & l, char const *                                   msg);
//logger & operator << (logger & l, wchar_t const *                                msg);

template <class T>
logger & operator << (logger & l, T const & msg)
{
    l(msg);
    return l;
}

logger fatal  (char const * file = nullptr, char const * func = nullptr, int line = -1);
logger error  (char const * file = nullptr, char const * func = nullptr, int line = -1);
logger warning(char const * file = nullptr, char const * func = nullptr, int line = -1);
logger info   (char const * file = nullptr, char const * func = nullptr, int line = -1);
logger debug  (char const * file = nullptr, char const * func = nullptr, int line = -1);
logger trace  (char const * file = nullptr, char const * func = nullptr, int line = -1);

#define    SNAP_LOG_FATAL       snap_logger::fatal  (__FILE__, __func__, __LINE__)
#define    SNAP_LOG_ERROR       snap_logger::error  (__FILE__, __func__, __LINE__)
#define    SNAP_LOG_WARNING     snap_logger::warning(__FILE__, __func__, __LINE__)
#define    SNAP_LOG_INFO        snap_logger::info   (__FILE__, __func__, __LINE__)
#define    SNAP_LOG_DEBUG       snap_logger::debug  (__FILE__, __func__, __LINE__)
#define    SNAP_LOG_TRACE       snap_logger::trace  (__FILE__, __func__, __LINE__)

} // namespace snap_logger
// vim: ts=4 sw=4 et
