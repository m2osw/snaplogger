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
 * \brief Implementation of the ANSI converter.
 *
 * This file implements a class that takes output from a console and
 * convert it in various ways such as HTML, markdown, or plain text.
 */

// self
//
#include    "snaplogger/exception.h"
#include    "snaplogger/convert_ansi.h"


// libutf8
//
#include    <libutf8/base.h>
#include    <libutf8/iterator.h>
#include    <libutf8/libutf8.h>


// snapdev
//
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/join_strings.h>


// C++
//
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{


constexpr char32_t const        ESCAPE = U'\x1B';    // Escape code

constexpr std::uint32_t const   g_colors[] =
{
    0x000000, 0xde382b, 0x39b54a, 0xffc706, 0x006fb8, 0x762671, 0x2cb5e9, 0xcccccc,
    0x808080, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff,
    0x000000, 0x00005f, 0x000087, 0x0000af, 0x0000d7, 0x0000ff, 0x005f00, 0x005f5f,
    0x005f87, 0x005faf, 0x005fd7, 0x005fff, 0x008700, 0x00875f, 0x008787, 0x0087af,
    0x0087d7, 0x0087ff, 0x00af00, 0x00af5f, 0x00af87, 0x00afaf, 0x00afd7, 0x00afff,
    0x00d700, 0x00d75f, 0x00d787, 0x00d7af, 0x00d7d7, 0x00d7ff, 0x00ff00, 0x00ff5f,
    0x00ff87, 0x00ffaf, 0x00ffd7, 0x00ffff, 0x5f0000, 0x5f005f, 0x5f0087, 0x5f00af,
    0x5f00d7, 0x5f00ff, 0x5f5f00, 0x5f5f5f, 0x5f5f87, 0x5f5faf, 0x5f5fd7, 0x5f5fff,
    0x5f8700, 0x5f875f, 0x5f8787, 0x5f87af, 0x5f87d7, 0x5f87ff, 0x5faf00, 0x5faf5f,
    0x5faf87, 0x5fafaf, 0x5fafd7, 0x5fafff, 0x5fd700, 0x5fd75f, 0x5fd787, 0x5fd7af,
    0x5fd7d7, 0x5fd7ff, 0x5fff00, 0x5fff5f, 0x5fff87, 0x5fffaf, 0x5fffd7, 0x5fffff,
    0x870000, 0x87005f, 0x870087, 0x8700af, 0x8700d7, 0x8700ff, 0x875f00, 0x875f5f,
    0x875f87, 0x875faf, 0x875fd7, 0x875fff, 0x878700, 0x87875f, 0x878787, 0x8787af,
    0x8787d7, 0x8787ff, 0x87af00, 0x87af5f, 0x87af87, 0x87afaf, 0x87afd7, 0x87afff,
    0x87d700, 0x87d75f, 0x87d787, 0x87d7af, 0x87d7d7, 0x87d7ff, 0x87ff00, 0x87ff5f,
    0x87ff87, 0x87ffaf, 0x87ffd7, 0x87ffff, 0xaf0000, 0xaf005f, 0xaf0087, 0xaf00af,
    0xaf00d7, 0xaf00ff, 0xaf5f00, 0xaf5f5f, 0xaf5f87, 0xaf5faf, 0xaf5fd7, 0xaf5fff,
    0xaf8700, 0xaf875f, 0xaf8787, 0xaf87af, 0xaf87d7, 0xaf87ff, 0xafaf00, 0xafaf5f,
    0xafaf87, 0xafafaf, 0xafafd7, 0xafafff, 0xafd700, 0xafd75f, 0xafd787, 0xafd7af,
    0xafd7d7, 0xafd7ff, 0xafff00, 0xafff5f, 0xafff87, 0xafffaf, 0xafffd7, 0xafffff,
    0xd70000, 0xd7005f, 0xd70087, 0xd700af, 0xd700d7, 0xd700ff, 0xd75f00, 0xd75f5f,
    0xd75f87, 0xd75faf, 0xd75fd7, 0xd75fff, 0xd78700, 0xd7875f, 0xd78787, 0xd787af,
    0xd787d7, 0xd787ff, 0xd7af00, 0xd7af5f, 0xd7af87, 0xd7afaf, 0xd7afd7, 0xd7afff,
    0xd7d700, 0xd7d75f, 0xd7d787, 0xd7d7af, 0xd7d7d7, 0xd7d7ff, 0xd7ff00, 0xd7ff5f,
    0xd7ff87, 0xd7ffaf, 0xd7ffd7, 0xd7ffff, 0xff0000, 0xff005f, 0xff0087, 0xff00af,
    0xff00d7, 0xff00ff, 0xff5f00, 0xff5f5f, 0xff5f87, 0xff5faf, 0xff5fd7, 0xff5fff,
    0xff8700, 0xff875f, 0xff8787, 0xff87af, 0xff87d7, 0xff87ff, 0xffaf00, 0xffaf5f,
    0xffaf87, 0xffafaf, 0xffafd7, 0xffafff, 0xffd700, 0xffd75f, 0xffd787, 0xffd7af,
    0xffd7d7, 0xffd7ff, 0xffff00, 0xffff5f, 0xffff87, 0xffffaf, 0xffffd7, 0xffffff,
    0x080808, 0x121212, 0x1c1c1c, 0x262626, 0x303030, 0x3a3a3a, 0x444444, 0x4e4e4e,
    0x585858, 0x626262, 0x6c6c6c, 0x767676, 0x808080, 0x8a8a8a, 0x949494, 0x9e9e9e,
    0xa8a8a8, 0xb2b2b2, 0xbcbcbc, 0xc6c6c6, 0xd0d0d0, 0xdadada, 0xe4e4e4, 0xeeeeee,
};



}
// no name namespace




convert_ansi::convert_ansi(ansi_output_t type)
    : f_type(type)
{
}


ansi_output_t convert_ansi::get_type() const
{
    return f_type;
}


void convert_ansi::set_optimize(bool optimize)
{
    f_optimize = optimize;
}


bool convert_ansi::get_optimize() const
{
    return f_optimize;
}


void convert_ansi::set_br(bool br)
{
    f_br = br;
}


bool convert_ansi::get_br() const
{
    return f_br;
}


void convert_ansi::write(std::string const & in)
{
    f_data.push_back(in);
}


std::string convert_ansi::read()
{
    // we assume that on a following up call you have brand new data
    // so we need to make sure the state was properly reset
    //
    f_graphical_state = GRAPHICAL_STATE_NORMAL;
    f_graphical_state_for_styles = GRAPHICAL_STATE_NORMAL;
    f_current_graphical_state = GRAPHICAL_STATE_NORMAL;
    f_result.clear();
    //f_param_pos = 0;
    f_state = state_t::ANSI_STATE_PLAIN_TEXT;
    f_conceal = false;
    f_span_open = false;
    f_invalid_input = false;

    char32_t last(U'\0');
    for(;;)
    {
        char32_t wc(last);
        if(wc == U'\0')
        {
            wc = getc();
        }
        else
        {
            last = U'\0';
        }
        if(wc == U'\r')
        {
            wc = getc();
            if(wc != U'\n')
            {
                last = wc;
                wc = U'\n';
            }
        }
        if(wc == libutf8::EOS)
        {
            break;
        }

        switch(f_state)
        {
        case state_t::ANSI_STATE_PLAIN_TEXT:
            if(wc == ESCAPE)
            {
                f_state = state_t::ANSI_STATE_ESCAPE;
            }
            else if(wc != U'\0' && !f_conceal)
            {
                switch(f_type)
                {
                case ansi_output_t::ANSI_OUTPUT_HTML:
                    if(!iswspace(wc))
                    {
                        start_style();
                    }
                    switch(wc)
                    {
                    case U'"':
                        f_result += "&quot;";
                        break;

                    case U'&':
                        f_result += "&amp;";
                        break;

                    case U'\'':
                        f_result += "&apos;";
                        break;

                    case U'<':
                        f_result += "&lt;";
                        break;

                    case U'>': // important for xhtml
                        f_result += "&gt;";
                        break;

                    case U'\n':
                        if(f_br)
                        {
                            f_result += "<br/>";
                        }
                        f_result += '\n';
                        break;

                    default:
                        f_result += libutf8::to_u8string(wc);
                        break;

                    }
                    break;

                case ansi_output_t::ANSI_OUTPUT_MARKDOWN:
                    if(iswspace(wc))
                    {
                        if(f_graphical_state != f_current_graphical_state
                        && f_current_graphical_state != GRAPHICAL_STATE_NORMAL)
                        {
                            end_style();
                            f_current_graphical_state = GRAPHICAL_STATE_NORMAL;
                        }
                    }
                    else
                    {
                        start_style();
                    }
                    switch(wc)
                    {
                    case U'*':
                    case U'-':
                    case U'#':
                    case U'_':
                    case U'<':
                    case U'>':
                    case U'`':
                    case U'[':
                    case U'\\':
                        f_result += '\\';
                        f_result += static_cast<char>(wc);
                        break;

                    case U'\n':
                        end_style();
                        f_current_graphical_state = GRAPHICAL_STATE_NORMAL;
                        f_result += '\n';
                        break;

                    default:
                        f_result += libutf8::to_u8string(wc);
                        break;

                    }
                    break;

                case ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT:
                    f_result += libutf8::to_u8string(wc);
                    break;

                }
            }
            break;

        case state_t::ANSI_STATE_ESCAPE:
            if(wc == U'[')
            {
                // Escape + '[' = Control Sequence Introducer
                //
                f_state = state_t::ANSI_STATE_PARAMETERS;
                f_parameters.push_back(0);
            }
            else
            {
                // we lose the Escape in case of an invalid CSI start
                //
                f_result += libutf8::to_u8string(wc);

                f_state = state_t::ANSI_STATE_PLAIN_TEXT;
            }
            break;

        case state_t::ANSI_STATE_PARAMETERS:
            if(wc >= U'@' && wc <= U'\x7E')
            {
                // ends the sequence and process it
                //
                switch(wc)
                {
                case U'm':
                    apply_graphical_rendition();
                    break;

                case U'A':
                case U'B':
                case U'C':
                case U'D':
                case U'E':
                case U'F':
                case U'G':
                case U'H':
                case U'J':
                case U'K':
                case U'S':
                case U'T':
                case U'f':
                case U'h':
                case U'i':
                case U'l':
                case U'n':
                case U's':
                case U'u':
                    // valid but ignored
                    break;

                default:
                    // this is not really invalid, just unsupported
                    //
                    f_invalid_input = true;
                    break;

                }

                f_parameters.clear();

                f_state = state_t::ANSI_STATE_PLAIN_TEXT;
            }
            else if(wc >= U'0' && wc <= U'9')
            {
                f_parameters[f_parameters.size() - 1] *= 10;
                f_parameters[f_parameters.size() - 1] += wc - U'0';
            }
            else 
            {
                // we expect ';' or ':' here because all the sequences we
                // support are expected to have such as the separator
                // (the ':' is at times used for color definitions)
                //
                if(wc != U';'
                && wc != U':')
                {
                    // "invalid" is not automatically true, but we do not
                    // recognize other sequences at the moment
                    //
                    f_invalid_input = true;
                }
                f_parameters.push_back(0);
            }
            break;

        }
    }

    end_style();

    return f_result;
}


void convert_ansi::apply_graphical_rendition()
{
    for(f_param_pos = 0; static_cast<std::size_t>(f_param_pos) < f_parameters.size();)
    {
//std::cerr << "\n--- PARAMS! switch: " << f_parameters[f_param_pos] << "\n";
        switch(f_parameters[f_param_pos])
        {
        case 0:
            update_style(GRAPHICAL_STATE_NORMAL, GRAPHICAL_STATE_ALL);
            ++f_param_pos;
            f_conceal = false;
            break;

        case 1:
            update_style(GRAPHICAL_STATE_BOLD, GRAPHICAL_STATE_LIGHT);
            ++f_param_pos;
            break;

        case 2:
            update_style(GRAPHICAL_STATE_LIGHT, GRAPHICAL_STATE_BOLD);
            ++f_param_pos;
            break;

        case 3:
            update_style(GRAPHICAL_STATE_ITALIC);
            ++f_param_pos;
            break;

        case 4:
            update_style(GRAPHICAL_STATE_UNDERLINE, GRAPHICAL_STATE_DOUBLE_UNDERLINE);
            ++f_param_pos;
            break;

        case 5:
            update_style(GRAPHICAL_STATE_SLOW_BLINK, GRAPHICAL_STATE_FAST_BLINK);
            ++f_param_pos;
            break;

        case 6:
            update_style(GRAPHICAL_STATE_FAST_BLINK, GRAPHICAL_STATE_SLOW_BLINK);
            ++f_param_pos;
            break;

        case 7:
            update_style(GRAPHICAL_STATE_INVERSE);
            ++f_param_pos;
            break;

        case 8:
            f_conceal = true;
            ++f_param_pos;
            break;

        case 9:
            update_style(GRAPHICAL_STATE_CROSS_OUT);
            ++f_param_pos;
            break;

        case 21:
            update_style(GRAPHICAL_STATE_DOUBLE_UNDERLINE, GRAPHICAL_STATE_UNDERLINE);
            ++f_param_pos;
            break;

        case 22:
            update_style(0, GRAPHICAL_STATE_BOLD | GRAPHICAL_STATE_LIGHT);
            ++f_param_pos;
            break;

        case 23:
            update_style(0, GRAPHICAL_STATE_ITALIC);
            ++f_param_pos;
            break;

        case 24:
            update_style(0, GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_DOUBLE_UNDERLINE);
            ++f_param_pos;
            break;

        case 25:
            update_style(0, GRAPHICAL_STATE_SLOW_BLINK | GRAPHICAL_STATE_FAST_BLINK);
            ++f_param_pos;
            break;

        case 26:
            update_style(GRAPHICAL_STATE_PROPORTIONAL);
            ++f_param_pos;
            break;

        case 27:
            update_style(0, GRAPHICAL_STATE_INVERSE);
            ++f_param_pos;
            break;

        case 28:
            f_conceal = false;
            ++f_param_pos;
            break;

        case 29:
            update_style(0, GRAPHICAL_STATE_CROSS_OUT);
            ++f_param_pos;
            break;

        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
            // the first 8 colors are the "standard" CSI colors
            //
            f_foreground_color = g_colors[f_parameters[f_param_pos] - 30];
            update_style(GRAPHICAL_STATE_FOREGROUND_COLOR);
            ++f_param_pos;
            break;

        case 38:
            f_foreground_color = get_color(true);
            update_style(GRAPHICAL_STATE_FOREGROUND_COLOR);
            break;

        case 39:
            f_foreground_color = 0;
            update_style(0, GRAPHICAL_STATE_FOREGROUND_COLOR);
            ++f_param_pos;
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
            // the first 8 colors are the "standard" CSI colors
            //
            f_background_color = g_colors[f_parameters[f_param_pos] - 40];
            update_style(GRAPHICAL_STATE_BACKGROUND_COLOR);
            ++f_param_pos;
            break;

        case 48:
            f_background_color = get_color(false);
            update_style(GRAPHICAL_STATE_BACKGROUND_COLOR);
            break;

        case 49:
            f_background_color = 0;
            update_style(0, GRAPHICAL_STATE_BACKGROUND_COLOR);
            ++f_param_pos;
            break;

        case 50:
            update_style(0, GRAPHICAL_STATE_PROPORTIONAL);
            ++f_param_pos;
            break;

        case 53:
            update_style(GRAPHICAL_STATE_OVERLINE);
            ++f_param_pos;
            break;

        case 55:
            update_style(0, GRAPHICAL_STATE_OVERLINE);
            ++f_param_pos;
            break;

        case 58:
            f_underline_color = get_color(false);
            update_style(GRAPHICAL_STATE_UNDERLINE_COLOR);
            break;

        case 59:
            f_underline_color = 0;
            update_style(0, GRAPHICAL_STATE_UNDERLINE_COLOR);
            ++f_param_pos;
            break;

        case 73:
            update_style(GRAPHICAL_STATE_SUPERSCRIPT, GRAPHICAL_STATE_SUBSCRIPT);
            ++f_param_pos;
            break;

        case 74:
            update_style(GRAPHICAL_STATE_SUBSCRIPT, GRAPHICAL_STATE_SUPERSCRIPT);
            ++f_param_pos;
            break;

        case 75:
            update_style(0, GRAPHICAL_STATE_SUBSCRIPT | GRAPHICAL_STATE_SUPERSCRIPT);
            ++f_param_pos;
            break;

        case 90:
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
            // the next 8 colors are the "bright" CSI colors
            //
            f_foreground_color = g_colors[f_parameters[f_param_pos] - (90 - 8)];
            update_style(GRAPHICAL_STATE_FOREGROUND_COLOR);
            ++f_param_pos;
            break;

        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
            // the next 8 colors are the "bright" CSI colors
            //
            f_background_color = g_colors[f_parameters[f_param_pos] - (100 - 8)];
            update_style(GRAPHICAL_STATE_BACKGROUND_COLOR);
            ++f_param_pos;
            break;

        // ignore the following
        //
        case 10: // fonts
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 51: // framed
        case 52: // encircled
        case 54: // cancel framed / encircled
        case 60: // ideogram
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
            ++f_param_pos;
            break;

        default:
            // undefined, that means invalid
            //
            f_invalid_input = true;
            f_param_pos = -1;
            break;

        }
    }
}


std::int32_t convert_ansi::get_color(bool support_transparent)
{
    if(f_parameters.size() >= static_cast<std::size_t>(f_param_pos) + 2UL)
    {
        switch(f_parameters[f_param_pos + 1])
        {
        //case 0: // app. specific, we do not support that one

        case 1:
            if(support_transparent)
            {
                f_param_pos += 2;
                return -1;
            }
            break;

        case 2:
            if(f_parameters.size() >= static_cast<std::size_t>(f_param_pos) + 5UL)
            {
                if(f_parameters[f_param_pos + 2] < 256
                && f_parameters[f_param_pos + 3] < 256
                && f_parameters[f_param_pos + 4] < 256)
                {
                    // RGB
                    std::int32_t const color(
                          (f_parameters[f_param_pos + 2] << 16)
                        | (f_parameters[f_param_pos + 3] <<  8)
                        | (f_parameters[f_param_pos + 4] <<  0));
                    f_param_pos += 5;
                    return color;
                }
            }
            break;

        case 3:
            if(f_parameters.size() >= static_cast<std::size_t>(f_param_pos) + 5UL)
            {
                if(f_parameters[f_param_pos + 2] < 256
                && f_parameters[f_param_pos + 3] < 256
                && f_parameters[f_param_pos + 4] < 256)
                {
                    // CMY -> RGB
                    std::int32_t const color(
                          ((255 - f_parameters[f_param_pos + 2]) << 16)
                        | ((255 - f_parameters[f_param_pos + 3]) <<  8)
                        | ((255 - f_parameters[f_param_pos + 4]) <<  0));
                    f_param_pos += 5;
                    return color;
                }
            }
            break;

        case 4:
            if(f_parameters.size() >= static_cast<std::size_t>(f_param_pos) + 6UL)
            {
                if(f_parameters[f_param_pos + 2] < 256
                && f_parameters[f_param_pos + 3] < 256
                && f_parameters[f_param_pos + 4] < 256
                && f_parameters[f_param_pos + 5] < 256)
                {
                    // CMYK -> RGB
                    int const percent(255 - f_parameters[f_param_pos + 5]);
                    std::int32_t const color(
                          (((255 - f_parameters[f_param_pos + 2]) * percent / 255) << 16)
                        | (((255 - f_parameters[f_param_pos + 3]) * percent / 255) <<  8)
                        | (((255 - f_parameters[f_param_pos + 4]) * percent / 255) <<  0));
                    f_param_pos += 6;
                    return color;
                }
            }
            break;

        case 5:
            if(f_parameters.size() >= static_cast<std::size_t>(f_param_pos) + 3UL)
            {
                if(f_parameters[f_param_pos + 2] < 256)
                {
                    std::int32_t const color(g_colors[f_parameters[f_param_pos + 2]]);
                    f_param_pos += 3;
                    return color;
                }
            }
            break;

        }
    }
    f_param_pos = -1;
    f_invalid_input = true;

    return 0;
}


void convert_ansi::update_style(graphical_state_t new_state, graphical_state_t replaced_states)
{
    f_graphical_state |= new_state;
    f_graphical_state &= ~replaced_states;
}


void convert_ansi::start_style()
{
    // is that state already set?
    // if so, we have nothing to do here
    //
    if(f_graphical_state == f_current_graphical_state)
    {
        return;
    }
    end_style();
    f_current_graphical_state = f_graphical_state;

    // when clearing we may end up with no more graphical style
    //
    if(f_current_graphical_state == GRAPHICAL_STATE_NORMAL)
    {
        return;
    }

    if(f_type == ansi_output_t::ANSI_OUTPUT_HTML)
    {
        open_span();
    }
    else
    {
        start_markdown();
    }
}


void convert_ansi::end_style()
{
    if(f_current_graphical_state != GRAPHICAL_STATE_NORMAL)
    {
        if(f_type == ansi_output_t::ANSI_OUTPUT_HTML)
        {
            close_span();
        }
        else
        {
            end_markdown();
        }
    }
}


void convert_ansi::open_span()
{
    std::list<std::string> tags;
    std::list<std::string> classes;

    f_graphical_state_for_styles |= f_current_graphical_state;

    if((f_current_graphical_state & GRAPHICAL_STATE_BOLD) != 0)
    {
        if(f_optimize)
        {
            tags.push_back("<b>");
        }
        else
        {
            classes.push_back("ansi-b");
        }
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_LIGHT) != 0)
    {
        classes.push_back("ansi-l");
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_ITALIC) != 0)
    {
        if(f_optimize)
        {
            tags.push_back("<i>");
        }
        else
        {
            classes.push_back("ansi-i");
        }
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_SLOW_BLINK) != 0)
    {
        classes.push_back("ansi-sb");
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_FAST_BLINK) != 0)
    {
        classes.push_back("ansi-fb");
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_PROPORTIONAL) != 0)
    {
        classes.push_back("ansi-p");
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_SUPERSCRIPT) != 0)
    {
        if(f_optimize)
        {
            tags.push_back("<sup>");
        }
        else
        {
            classes.push_back("ansi-sup");
        }
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_SUBSCRIPT) != 0)
    {
        if(f_optimize)
        {
            tags.push_back("<sub>");
        }
        else
        {
            classes.push_back("ansi-sub");
        }
    }

    switch(f_current_graphical_state & (GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_DOUBLE_UNDERLINE | GRAPHICAL_STATE_OVERLINE | GRAPHICAL_STATE_CROSS_OUT))
    {
    case 0:
        break;

    case GRAPHICAL_STATE_UNDERLINE:
        if(f_optimize)
        {
            tags.push_back("<u>");
        }
        else
        {
            classes.push_back("ansi-u");
        }
        break;

    case GRAPHICAL_STATE_DOUBLE_UNDERLINE:
        classes.push_back("ansi-d");
        break;

    case GRAPHICAL_STATE_OVERLINE:
        classes.push_back("ansi-v");
        break;

    case GRAPHICAL_STATE_CROSS_OUT:
        if(f_optimize)
        {
            tags.push_back("<s>");
        }
        else
        {
            classes.push_back("ansi-s");
        }
        break;

    case GRAPHICAL_STATE_DOUBLE_UNDERLINE | GRAPHICAL_STATE_CROSS_OUT:
        classes.push_back("ansi-ds");
        break;

    case GRAPHICAL_STATE_DOUBLE_UNDERLINE | GRAPHICAL_STATE_OVERLINE:
        classes.push_back("ansi-dv");
        break;

    case GRAPHICAL_STATE_DOUBLE_UNDERLINE | GRAPHICAL_STATE_OVERLINE | GRAPHICAL_STATE_CROSS_OUT:
        classes.push_back("ansi-dvs");
        break;

    case GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_CROSS_OUT:
        if(f_optimize)
        {
            tags.push_back("<u><s>");
        }
        else
        {
            classes.push_back("ansi-us");
        }
        break;

    case GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_OVERLINE:
        classes.push_back("ansi-uv");
        break;

    case GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_OVERLINE | GRAPHICAL_STATE_CROSS_OUT:
        classes.push_back("ansi-uvs");
        break;

    case GRAPHICAL_STATE_OVERLINE | GRAPHICAL_STATE_CROSS_OUT:
        classes.push_back("ansi-vs");
        break;

    // LCOV_EXCL_START
    default:
        throw logger_logic_error(
              "unhandled underline + double underline + overline + cross out case (0x"
            + snapdev::int_to_hex(f_current_graphical_state & (GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_DOUBLE_UNDERLINE | GRAPHICAL_STATE_OVERLINE | GRAPHICAL_STATE_CROSS_OUT), false, 4)
            + ")");
    // LCOV_EXCL_STOP

    }

    f_result += snapdev::join_strings(tags, "");

    bool add_colors(false);
    std::int32_t foreground_color(0x000000);
    std::int32_t background_color(0xffffff);

    if((f_current_graphical_state & GRAPHICAL_STATE_FOREGROUND_COLOR) != 0)
    {
        add_colors = true;
        foreground_color = f_foreground_color;
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_BACKGROUND_COLOR) != 0)
    {
        add_colors = true;
        background_color = f_background_color;
    }

    if((f_current_graphical_state & GRAPHICAL_STATE_INVERSE) != 0
    && foreground_color != -1)
    {
        add_colors = true;
        std::swap(foreground_color, background_color);
    }

    bool const add_underline_color((f_current_graphical_state & GRAPHICAL_STATE_UNDERLINE_COLOR) != 0);

    if(!classes.empty() || add_colors || add_underline_color)
    {
        f_span_open = true;
        f_result += "<span";

        if(!classes.empty())
        {
            f_result += " class=\"";
            f_result += snapdev::join_strings(classes, " ");
            f_result += '"';
        }

        if(add_colors || add_underline_color)
        {
            f_result += " style=\"";
            if(add_colors)
            {
                if(foreground_color != -1)
                {
                    f_result += "color:#";
                    f_result += snapdev::int_to_hex(foreground_color, false, 6);
                }
                else
                {
                    f_result += "opacity:0%";
                }
                f_result += ";background-color:#";
                f_result += snapdev::int_to_hex(background_color, false, 6);
            }
            if(add_underline_color)
            {
                if(add_colors)
                {
                    f_result += ';';
                }
                f_result += "text-decoration-color:#";
                f_result += snapdev::int_to_hex(f_underline_color, false, 6);
            }
            f_result += '"';
        }

        f_result += '>';
    }
}


void convert_ansi::close_span()
{
    if(f_span_open)
    {
        f_result += "</span>";
        f_span_open = false;
    }

    if(f_optimize)
    {
        switch(f_current_graphical_state & (GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_DOUBLE_UNDERLINE | GRAPHICAL_STATE_OVERLINE | GRAPHICAL_STATE_CROSS_OUT))
        {
        case GRAPHICAL_STATE_UNDERLINE:
            f_result += "</u>";
            break;

        case GRAPHICAL_STATE_CROSS_OUT:
            f_result += "</s>";
            break;

        case GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_CROSS_OUT:
            f_result += "</s></u>";
            break;

        default:
            // other cases make use of the <span> tag
            break;

        }
        if((f_current_graphical_state & GRAPHICAL_STATE_SUBSCRIPT) != 0)
        {
            f_result += "</sub>";
        }
        if((f_current_graphical_state & GRAPHICAL_STATE_SUPERSCRIPT) != 0)
        {
            f_result += "</sup>";
        }
        if((f_current_graphical_state & GRAPHICAL_STATE_ITALIC) != 0)
        {
            f_result += "</i>";
        }
        if((f_current_graphical_state & GRAPHICAL_STATE_BOLD) != 0)
        {
            f_result += "</b>";
        }
    }
}


void convert_ansi::start_markdown()
{
    if((f_current_graphical_state & GRAPHICAL_STATE_BOLD) != 0)
    {
        f_result += "*";
    }
    if((f_current_graphical_state & GRAPHICAL_STATE_ITALIC) != 0)
    {
        f_result += "**";
    }
}


void convert_ansi::end_markdown()
{
    if((f_current_graphical_state & GRAPHICAL_STATE_ITALIC) != 0)
    {
        f_result += "**";
    }
    if((f_current_graphical_state & GRAPHICAL_STATE_BOLD) != 0)
    {
        f_result += "*";
    }
}


/** \brief Get the HTML styles to add to your style tag.
 *
 * After you are done with the read() function, you must call this function
 * to get all the styles. For the most part, the tags are assigned classes
 * and these are defined in these styles.
 *
 * Note that the function does not generate the `<style>` tag. That way you
 * can include these in your existing tag.
 *
 * I use abbreviations to make the names shorter:
 *
 * * bold -- b
 * * cross out -- s
 * * double underline -- d
 * * double underline + cross out -- ds
 * * double underline + overline -- dv
 * * double underline + overline + cross out -- dvs
 * * fast blink -- fb
 * * italic -- i
 * * light -- l
 * * outline -- o
 * * overline -- v
 * * overline + cross out -- vs
 * * proportional -- p
 * * slow blink -- sb
 * * subscript -- sub
 * * superscript -- sup
 * * underline -- u
 * * underline + cross out -- us
 * * underline + overline -- uv
 * * underline + overline + cross out -- uvs
 *
 * \param[in] apply_to_defaults  Whether the default optimization tags should
 * be tweaked by this CSS code.
 *
 * \return Styles that can be added inside your main \<style> tag.
 */
std::string convert_ansi::get_styles(bool apply_to_detauls) const
{
    std::string styles;

    if(f_type == ansi_output_t::ANSI_OUTPUT_HTML)
    {
        if((f_graphical_state_for_styles & GRAPHICAL_STATE_BOLD) != 0)
        {
            if(apply_to_detauls && f_optimize)
            {
                styles += "b,";
            }
            styles += ".ansi-b{font-weight:bold}\n";
        }

        if((f_graphical_state_for_styles & GRAPHICAL_STATE_LIGHT) != 0)
        {
            styles += ".ansi-l{font-weight:light}\n";
        }

        if((f_graphical_state_for_styles & GRAPHICAL_STATE_ITALIC) != 0)
        {
            if(apply_to_detauls && f_optimize)
            {
                styles += "i,";
            }
            styles += ".ansi-i{font-style:italic}\n";
        }

        if((f_graphical_state_for_styles & (GRAPHICAL_STATE_SLOW_BLINK | GRAPHICAL_STATE_FAST_BLINK)) != 0)
        {
            styles += "@keyframes ansi-blinker{50%{opacity:0}}\n";
        }

        if((f_graphical_state_for_styles & GRAPHICAL_STATE_SLOW_BLINK) != 0)
        {
            styles += ".ansi-sb{animation:ansi-blinker 2s linear infinite}\n";
        }

        if((f_graphical_state_for_styles & GRAPHICAL_STATE_FAST_BLINK) != 0)
        {
            styles += ".ansi-fb{animation:ansi-blinker 0.4s linear infinite}\n";
        }

        if((f_graphical_state_for_styles & GRAPHICAL_STATE_PROPORTIONAL) != 0)
        {
            styles += ".ansi-p{font-family:sans-serif}\n";
        }

        if((f_graphical_state_for_styles & GRAPHICAL_STATE_SUPERSCRIPT) != 0)
        {
            if(apply_to_detauls && f_optimize)
            {
                styles += "sup,";
            }
            styles += ".ansi-sup{font-size:60%;vertical-align:super}\n";
        }

        if((f_graphical_state_for_styles & GRAPHICAL_STATE_SUBSCRIPT) != 0)
        {
            if(apply_to_detauls && f_optimize)
            {
                styles += "sub,";
            }
            styles += ".ansi-sub{font-size:60%;vertical-align:sub}\n";
        }

        // TODO: the following if() test is a simplification, we could change
        //       that to only include the classes we used instead
        //
        if((f_graphical_state_for_styles & (GRAPHICAL_STATE_UNDERLINE | GRAPHICAL_STATE_DOUBLE_UNDERLINE | GRAPHICAL_STATE_OVERLINE | GRAPHICAL_STATE_CROSS_OUT)) != 0)
        {
            // Note:  The double style applies to both, the overline and
            //        the underline; in the XTerm console, though it
            //        only applies to the underline; to simplify here
            //        especially since I've never seen it used anyway,
            //        I keep it as is (double on both ends)

            // CROSS OUT on its own
            //
            if(apply_to_detauls && f_optimize)
            {
                styles += "s,";
            }
            styles += ".ansi-s{text-decoration-line:line-through}\n";

            // UNDERLINE on its own
            //
            if(apply_to_detauls && f_optimize)
            {
                styles += "u,";
            }
            styles += ".ansi-u{text-decoration-line:underline}\n";

            // DOUBLE UNDERLINE on its own
            //
            styles += ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n";

            // OVERLINE on its own
            //
            styles += ".ansi-v{text-decoration-line:overline;}\n";

            // UNDERLINE + CROSS OUT
            //
            styles += ".ansi-us{text-decoration-line:underline line-through}\n";

            // UNDERLINE + OVERLINE
            //
            styles += ".ansi-uv{text-decoration-line:underline overline}\n";

            // UNDERLINE + OVERLINE + CROSS OUT
            //
            styles += ".ansi-uvs{text-decoration-line:underline overline line-through}\n";

            // DOUBLE UNDERLINE + CROSS OUT
            //
            styles += ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n";

            // DOUBLE UNDERLINE + OVERLINE
            //
            styles += ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n";

            // DOUBLE UNDERLINE + OVERLINE + CROSS OUT
            //
            styles += ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n";

            // OVERLINE + CROSS OUT
            //
            styles += ".ansi-vs{text-decoration-line:overline line-through}\n";
        }
    }

    return styles;
} // LCOV_EXCL_LINE


/** \brief Check whether any of the input was considered invalid.
 *
 * If the input data could not be read 100% without errors (skipping
 * sequence of unknown character sequences) then this function returns
 * true to signal that there was at least one error.
 *
 * \return true if a sequence of bytes in the input was not considered valid.
 */
bool convert_ansi::has_invalid_data() const
{
    return f_invalid_input;
}


/** \brief Get the following character as a Unicode value.
 *
 * This function reads the input data for one character. It converts
 * the character to a full char32_t character (i.e. a Unicode rune).
 *
 * \todo
 * A Unicode character sequence that ends up being broken in two separate
 * strings (see write()) is not properly recognized. We'd need to support
 * such one day.
 *
 * \return The Unicode character or libutf8::EOS if no more data is available.
 */
char32_t convert_ansi::getc()
{
    while(!f_data.empty())
    {
        if(f_pos >= f_data.front().length())
        {
            f_data.pop_front();
            f_pos = 0;
        }
        else
        {
            char32_t wc(U'\0');
            char const * end(f_data.front().data() + f_pos);
            char const * start(end);
            std::size_t len(f_data.front().length() - f_pos);
            int const r(libutf8::mbstowc(wc, end, len));

            // on invalid UTF-8 sequences, we may skip many bytes at once
            // the following takes that in account
            //
            f_pos += end - start;

            if(r > 0)
            {
                return wc;
            }
            // if r == 0, we reached the end of the string
            // otherwise we got an error and want to make a note of that
            //
            if(r != 0)
            {
                f_invalid_input = true;
            }
        }
    }

    // done, no more input data
    //
    return libutf8::EOS;
}



}
// namespace snaplogger
// vim: ts=4 sw=4 et
