// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
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

/** \file
 * \brief Format a message.
 *
 * This file declares the format class used to transform a message with
 * the administrator defined format.
 */


// self
//
#include    "snaplogger/format.h"

#include    "snaplogger/exception.h"


// C++
//
#include    <numeric>
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{



class parser
{
public:
    enum class token_t
    {
        TOKEN_EOF,
        TOKEN_COLON,
        TOKEN_EQUAL,
        TOKEN_STRING,
        TOKEN_INTEGER,
        TOKEN_IDENTIFIER,
        TOKEN_END,

        TOKEN_ERROR, // mark an error
    };

    parser(std::string const & f, variable::vector_t & variables)
        : f_input(f)
        , f_variables(variables)
    {
    }

    int getc()
    {
        if(f_pos >= f_input.length())
        {
            return EOF;
        }

        int const c(f_input[f_pos]);
        ++f_pos;
        return c;
    }

    void ungetc(int c)
    {
        if(f_pos == 0)
        {
            throw logger_logic_error("ungetc() called too many times.");                // LCOV_EXCL_LINE
        }
        --f_pos;
        if(f_input[f_pos] != c)
        {
            // LCOV_EXCL_START
            throw logger_logic_error(
                  std::string("ungetc() called with the wrong character (expected '")
                + static_cast<char>(f_input[f_pos])
                + "', got '"
                + static_cast<char>(c)
                + "').");
            // LCOV_EXCL_STOP
        }
    }

    token_t get_token()
    {
        for(;;)
        {
            int c(getc());
            if(c == EOF)
            {
                return token_t::TOKEN_EOF;
            }
            switch(c)
            {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                // ignore spaces
                break;

            case ':':
                return token_t::TOKEN_COLON;

            case '=':
                return token_t::TOKEN_EQUAL;

            case '}':
                return token_t::TOKEN_END;

            case '"':
            case '\'':
            case '`':
                {
                    f_text.clear();
                    int quote(c);
                    for(;;)
                    {
                        c = getc();
                        if(c == '\\')
                        {
                            c = getc();
                        }
                        else if(c == quote)
                        {
                            break;
                        }
                        if(c == EOF)
                        {
                            return token_t::TOKEN_ERROR;
                        }
                        f_text += c;
                    }
                    return token_t::TOKEN_STRING;
                }
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                {
                    f_integer = c - '0';
                    for(;;)
                    {
                        c = getc();
                        if(c < '0' || c > '9')
                        {
                            ungetc(c);
                            break;
                        }
                        f_integer *= 10;
                        f_integer += c - '0';
                    }
                    return token_t::TOKEN_INTEGER;
                }
                break;

            default:
                if((c >= 'a' && c <= 'z')
                || (c >= 'A' && c <= 'Z')
                || c == '_')
                {
                    f_text.clear();
                    f_text += c;
                    for(;;)
                    {
                        c = getc();
                        if((c < 'a' || c > 'z')
                        && (c < 'A' || c > 'Z')
                        && (c < '0' || c > '9')
                        && c != '_')
                        {
                            ungetc(c);
                            break;
                        }
                        f_text += c;
                    }
                    return token_t::TOKEN_IDENTIFIER;
                }
                else
                {
                    return token_t::TOKEN_ERROR;
                }
                break;

            }
        }
    }

    bool parse_variable()
    {
        token_t tok(get_token());
        if(tok != token_t::TOKEN_IDENTIFIER)
        {
            return false;
        }
        variable::pointer_t var(get_variable(f_text));
        if(var == nullptr)
        {
            return false;
        }
        f_variables.push_back(var);

        tok = get_token();
        for(;;)
        {
            if(tok == token_t::TOKEN_END)
            {
                break;
            }
            if(tok != token_t::TOKEN_COLON)
            {
                return false;
            }
            tok = get_token();
            if(tok != token_t::TOKEN_IDENTIFIER)
            {
                return false;
            }
            param::pointer_t p(std::make_shared<param>(f_text));
            var->add_param(p);

            tok = get_token();
            if(tok == token_t::TOKEN_EQUAL)
            {
                // the token is followed by a value
                //
                tok = get_token();
                if(tok == token_t::TOKEN_STRING
                || tok == token_t::TOKEN_IDENTIFIER)
                {
                    p->set_value(f_text);
                }
                else if(tok == token_t::TOKEN_INTEGER)
                {
                    p->set_integer(f_integer);
                }
                else
                {
                    return false;
                }

                tok = get_token();
            }
        }

        return true;
    }

    void parse()
    {
        auto add_text = [this](std::string const & text)
        {
            if(!text.empty())
            {
                variable::pointer_t var(get_variable("direct"));
                if(var == nullptr)
                {
                    throw logger_logic_error("variable type \"direct\" not registered?");
                }
                param::pointer_t p(std::make_shared<param>("msg"));
                var->add_param(p);
                p->set_value(text);
                f_variables.push_back(var);
            }
        };

        std::string text;

        for(;;)
        {
            int c(getc());
            if(c == EOF)
            {
                break;
            }
            if(c == '$')
            {
                c = getc();
                if(c == '{')
                {
                    // we found what looks like a variable definition
                    //
                    add_text(text);
                    text.clear();

                    // try parsing the variable, if that fails, ignore
                    // the variable and instead add the "${" introducer
                    // on its own
                    //
                    auto const save_pos(f_pos);
                    if(!parse_variable())
                    {
                        text += "${";
                        f_pos = save_pos;
                    }
                }
                else
                {
                    text += '$';
                    ungetc(c);
                }
            }
            else
            {
                text += c;
            }
        }
        add_text(text);
    }

private:
    std::string const       f_input;
    size_t                  f_pos = 0;
    variable::vector_t &    f_variables;
    std::string             f_text = std::string();
    std::int64_t            f_integer = 0;
};



}
// no name namespace


format::format(std::string const & f)
    : f_format(f)
{
    parser p(f, f_variables);
    p.parse();
}


std::string format::get_format() const
{
    return f_format;
}


std::string format::process_message(message const & msg, bool ignore_on_no_repeat)
{
    return std::accumulate(
              f_variables.begin()
            , f_variables.end()
            , std::string()
            , [&msg, &ignore_on_no_repeat](std::string const & r, variable::pointer_t v)
            {
                if(ignore_on_no_repeat
                && v->ignore_on_no_repeat())
                {
                    // do not include this variable to generate the "no-repeat"
                    // message (i.e. probably a time base message)
                    //
                    return r;
                }
                return r + v->get_value(msg);
            });
}





} // snaplogger namespace
// vim: ts=4 sw=4 et
