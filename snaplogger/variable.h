// Copyright (c) 2013-2021  Made to Order Software Corp.  All Rights Reserved
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
#pragma once

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
 * In C++, this would look something like:
 *
 * \code
 *     date(FLAG_YEAR, ALIGN_RIGHT, 2);
 * \endcode
 */


// self
//
#include    "snaplogger/message.h"


// libutf8 lib
//
#include    <libutf8/libutf8.h>


// C++
//
#include    <vector>



namespace snaplogger
{

// variables
//
#define SNAPLOGGER_VAR_BASENAME     "basename"
#define SNAPLOGGER_VAR_DATE         "date"
#define SNAPLOGGER_VAR_DIAGNOSTIC   "diagnostic"
#define SNAPLOGGER_VAR_DOMAINNAME   "domainname"
#define SNAPLOGGER_VAR_ENV          "env"
#define SNAPLOGGER_VAR_FILENAME     "filename"
#define SNAPLOGGER_VAR_FUNCTION     "function"
#define SNAPLOGGER_VAR_GID          "gid"
#define SNAPLOGGER_VAR_GROUPNAME    "groupname"
#define SNAPLOGGER_VAR_HOSTBYNAME   "hostbyname"
#define SNAPLOGGER_VAR_HOSTNAME     "hostname"
#define SNAPLOGGER_VAR_LINE         "line"
#define SNAPLOGGER_VAR_LOCALE       "locale"
#define SNAPLOGGER_VAR_MESSAGE      "message"
#define SNAPLOGGER_VAR_PATH         "path"
#define SNAPLOGGER_VAR_PID          "pid"
#define SNAPLOGGER_VAR_PROGNAME     "progname"
#define SNAPLOGGER_VAR_SEVERITY     "severity"
#define SNAPLOGGER_VAR_TID          "tid"
#define SNAPLOGGER_VAR_TIME         "time"
#define SNAPLOGGER_VAR_THREADNAME   "threadname"
#define SNAPLOGGER_VAR_UID          "uid"
#define SNAPLOGGER_VAR_USERNAME     "username"
#define SNAPLOGGER_VAR_VERSION      "version"

// functions
//
#define SNAPLOGGER_VAR_ALIGN        "align"
#define SNAPLOGGER_VAR_APPEND       "append"
#define SNAPLOGGER_VAR_CAPS         "caps"
#define SNAPLOGGER_VAR_ESCAPE       "escape"
#define SNAPLOGGER_VAR_EXACT_WIDTH  "exact_width"
#define SNAPLOGGER_VAR_LOWER        "lower"
#define SNAPLOGGER_VAR_MAX_WIDTH    "max_width"
#define SNAPLOGGER_VAR_MIN_WIDTH    "min_width"
#define SNAPLOGGER_VAR_PADDING      "padding"
#define SNAPLOGGER_VAR_PREPEND      "prepend"
#define SNAPLOGGER_VAR_UPPER        "upper"

// parameters
//
#define SNAPLOGGER_VAR_ALIGN_LEFT   "left"
#define SNAPLOGGER_VAR_ALIGN_RIGHT  "right"



class param
{
public:
    typedef std::shared_ptr<param>          pointer_t;
    typedef std::vector<pointer_t>          vector_t;

    enum class type_t
    {
        TYPE_STRING,
        TYPE_INTEGER
    };

                        param(std::string const & name);

    std::string const & get_name() const;

    type_t              get_type() const;

    std::string         get_value() const;
    void                set_value(std::string const & value);

    int64_t             get_integer() const;
    void                set_integer(std::int64_t integer);

private:
    std::string const   f_name;
    type_t              f_type = type_t::TYPE_STRING;
    std::string         f_value = std::string();
    std::int64_t        f_integer = 0;
};


template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &
operator << (std::basic_ostream<CharT, Traits> & os, param::pointer_t p)
{
    if(p != nullptr)
    {
        os << p->get_name();
    }
    else
    {
        os << "(nullptr)";
    }
    return os;
}


template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &
operator << (std::basic_ostream<CharT, Traits> & os, param::type_t t)
{
    switch(t)
    {
    case param::type_t::TYPE_STRING:
        os << "string";
        break;

    case param::type_t::TYPE_INTEGER:
        os << "integer";
        break;

    default:
        os << "(unknown)";
        break;

    }
    return os;
}





class variable
{
public:
    typedef std::shared_ptr<variable>       pointer_t;
    typedef std::vector<pointer_t>          vector_t;

    virtual             ~variable();

    virtual bool        ignore_on_no_repeat() const = 0;
    void                add_param(param::pointer_t p);
    param::vector_t     get_params() const;
    std::string         get_value(message const & msg) const;

protected:
    virtual void        process_value(message const & msg, std::string & value) const;

private:
    param::vector_t     f_params = param::vector_t();
};


class variable_factory
{
public:
    typedef std::shared_ptr<variable_factory>       pointer_t;

                                    variable_factory(std::string const & type);
    virtual                         ~variable_factory();

    std::string const &             get_type() const;
    virtual variable::pointer_t     create_variable() = 0;

private:
    std::string const               f_type;
}; 


void                    register_variable_factory(variable_factory::pointer_t factory);
variable::pointer_t     get_variable(std::string const & type);


#define DEFINE_LOGGER_VARIABLE_IMPL_(type, do_ignore_on_no_repeat)  \
    class type##_variable                                           \
        : public ::snaplogger::variable                             \
    {                                                               \
    protected:                                                      \
        virtual bool ignore_on_no_repeat() const override           \
        { return do_ignore_on_no_repeat; }                          \
        virtual void process_value(                                 \
                  ::snaplogger::message const & msg                 \
                , std::string & value) const override;              \
    };                                                              \
    class type##_variable_factory final                             \
        : public ::snaplogger::variable_factory                     \
    {                                                               \
    public:                                                         \
        type##_variable_factory()                                   \
            : variable_factory(#type)                               \
        {}                                                          \
        virtual ::snaplogger::variable::pointer_t                   \
                create_variable() override final                    \
        {                                                           \
            return std::make_shared<type##_variable>();             \
        }                                                           \
    };                                                              \
    int __attribute__((unused))                                     \
        g_##type##_variable_factory = []() {                        \
            ::snaplogger::register_variable_factory                 \
                    (std::make_shared<                              \
                        type##_variable_factory>());                \
            return 0;                                               \
        } ();                                                       \
    void type##_variable::process_value(                            \
                  ::snaplogger::message const & msg                 \
                , std::string & value) const


#define DEFINE_LOGGER_VARIABLE(type) \
    DEFINE_LOGGER_VARIABLE_IMPL_(type, false)

#define DEFINE_LOGGER_VARIABLE_IGNORED_ON_NO_REPEAT(type) \
    DEFINE_LOGGER_VARIABLE_IMPL_(type, true)






class function_data
{
public:
    void                set_value(std::string value);
    void                set_value(std::u32string value);
    std::u32string &    get_value();
    void                set_param(std::string const & name, std::string const & value);
    void                set_param(std::string const & name, std::u32string const & value);
    std::u32string      get_param(std::string const & name, std::u32string const & default_value);

private:
    std::u32string      f_value = std::u32string();
    u8u32string_map_t   f_params = u8u32string_map_t();
};




class function
{
public:
    typedef std::shared_ptr<function>   pointer_t;

                        function(std::string const & function_name);
    virtual             ~function();

    std::string const & get_name() const;

    virtual void        apply(
                              message const & msg
                            , function_data & data
                            , param::pointer_t const & p) = 0;

private:
    std::string const   f_name;
};


void register_function(function::pointer_t func);


#define DECLARE_FUNCTION(name)                                              \
    class name##_function : public function                                 \
    {                                                                       \
    public:                                                                 \
        name##_function() : function(#name) {}                              \
        virtual void apply(                                                 \
              ::snaplogger::message const & msg                             \
            , ::snaplogger::function_data & d                               \
            , ::snaplogger::param::pointer_t const & p) override;           \
    };                                                                      \
    int __attribute__((unused)) g_##name##_function = []() {                \
            register_function(std::make_shared<name##_function>());         \
            return 0;                                                       \
        } ();                                                               \
    void name##_function::apply(                                            \
              ::snaplogger::message const & msg                             \
            , ::snaplogger::function_data & d                               \
            , ::snaplogger::param::pointer_t const & p)





} // snaplogger namespace
// vim: ts=4 sw=4 et
