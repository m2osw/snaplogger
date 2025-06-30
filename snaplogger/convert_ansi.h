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
#pragma once

/** \file
 * \brief Convert an ANSI feed into something else.
 *
 * This class allows you to convert plain text from a console output
 * which may contain escape codes to a different format:
 *
 * * HTML
 * * Markdown
 * * Plain text
 */


// self
//
//#include    <snaplogger/message.h>



// C++
//
#include    <cstdint>
#include    <list>
#include    <memory>
#include    <vector>



namespace snaplogger
{



enum ansi_output_t
{
    ANSI_OUTPUT_PLAIN_TEXT,
    ANSI_OUTPUT_HTML,
    ANSI_OUTPUT_MARKDOWN,
};



class convert_ansi
{
public:
    typedef std::shared_ptr<convert_ansi>   pointer_t;

                                convert_ansi(ansi_output_t type = ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT);

    ansi_output_t               get_type() const;
    void                        set_optimize(bool optimize = true);
    bool                        get_optimize() const;

    void                        write(std::string const & in);
    std::string                 read();
    bool                        has_invalid_data() const;
    std::string                 get_styles(bool apply_to_detauls = false) const;

private:
    enum class state_t : std::uint8_t
    {
        ANSI_STATE_PLAIN_TEXT,
        ANSI_STATE_ESCAPE,
        ANSI_STATE_PARAMETERS,
    };
    typedef std::list<std::string>      list_t;

    typedef std::uint16_t               graphical_state_t;

    static constexpr graphical_state_t const    GRAPHICAL_STATE_NORMAL              = 0x0000;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_BOLD                = 0x0001;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_LIGHT               = 0x0002;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_ITALIC              = 0x0004;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_UNDERLINE           = 0x0008;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_OVERLINE            = 0x0010;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_DOUBLE_UNDERLINE    = 0x0020;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_CROSS_OUT           = 0x0040;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_SLOW_BLINK          = 0x0080;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_FAST_BLINK          = 0x0100;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_SUPERSCRIPT         = 0x0200;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_SUBSCRIPT           = 0x0400;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_INVERSE             = 0x0800;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_PROPORTIONAL        = 0x1000;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_FOREGROUND_COLOR    = 0x2000;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_BACKGROUND_COLOR    = 0x4000;
    static constexpr graphical_state_t const    GRAPHICAL_STATE_UNDERLINE_COLOR     = 0x8000;

    static constexpr graphical_state_t const    GRAPHICAL_STATE_ALL                 = 0xFFFF;

    void                        apply_graphical_rendition();
    std::int32_t                get_color(bool support_transparent);
    void                        update_style(graphical_state_t new_state, graphical_state_t replaced_states = GRAPHICAL_STATE_NORMAL);
    void                        start_style();
    void                        end_style();
    void                        open_span();
    void                        close_span();
    void                        start_markdown();
    void                        end_markdown();
    char32_t                    getc();

    ansi_output_t               f_type = ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT;
    list_t                      f_data = list_t();
    std::string::size_type      f_pos = 0;
    std::string                 f_result = std::string();
    graphical_state_t           f_graphical_state = GRAPHICAL_STATE_NORMAL;
    graphical_state_t           f_graphical_state_for_styles = GRAPHICAL_STATE_NORMAL;
    graphical_state_t           f_current_graphical_state = GRAPHICAL_STATE_NORMAL;
    std::int32_t                f_foreground_color = 0; // if state is not set, do not include colors
    std::int32_t                f_background_color = 0;
    std::int32_t                f_underline_color = 0;
    std::vector<std::int32_t>   f_parameters = std::vector<std::int32_t>();
    ssize_t                     f_param_pos = 0;
    state_t                     f_state = state_t::ANSI_STATE_PLAIN_TEXT;
    bool                        f_conceal = false;
    bool                        f_span_open = false;
    bool                        f_optimize = false;
    bool                        f_invalid_input = false;
};



} // snaplogger namespace
// vim: ts=4 sw=4 et
