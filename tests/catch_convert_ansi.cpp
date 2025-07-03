// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/convert_ansi.h>
#include    <snaplogger/exception.h>


// snapdev
//
#include    <snapdev/string_replace_many.h>


// libutf8
//
#include    <libutf8/libutf8.h>


// C
//
//#include    <unistd.h>



namespace
{



struct convert_t
{
    char const *        f_input = nullptr;
    char const *        f_plain_text = nullptr;
    char const *        f_html = nullptr;
    char const *        f_optimized_html = nullptr;
    char const *        f_markdown = nullptr;
    char const *        f_styles = nullptr;
    char const *        f_styles_with_defaults = nullptr;
};


constexpr convert_t const g_convert_data[] =
{
    // plain text
    {
        .f_input = "plain",
        .f_plain_text = "plain",
        .f_html = "plain",
    },

    // normal
    {
        .f_input = "\033[0mnormal\033[m",
        .f_plain_text = "normal",
        .f_html = "normal",
    },

    // bold
    {
        .f_input = "\033[1mbold\033[m",
        .f_plain_text = "bold",
        .f_html = "<span class=\"ansi-b\">bold</span>",
        .f_optimized_html = "<b>bold</b>",
        .f_markdown = "*bold*",
        .f_styles = ".ansi-b{font-weight:bold}\n",
        .f_styles_with_defaults = "b,.ansi-b{font-weight:bold}\n",
    },

    // light
    {
        .f_input = "\033[2mlight\033[m",
        .f_plain_text = "light",
        .f_html = "<span class=\"ansi-l\">light</span>",
        .f_styles = ".ansi-l{font-weight:light}\n",
    },

    // italic
    {
        .f_input = "\033[3mitalic\033[m",
        .f_plain_text = "italic",
        .f_html = "<span class=\"ansi-i\">italic</span>",
        .f_optimized_html = "<i>italic</i>",
        .f_markdown = "**italic**",
        .f_styles = ".ansi-i{font-style:italic}\n",
        .f_styles_with_defaults = "i,.ansi-i{font-style:italic}\n",
    },

    // underline
    {
        .f_input = "\033[4munderline\033[m",
        .f_plain_text = "underline",
        .f_html = "<span class=\"ansi-u\">underline</span>",
        .f_optimized_html = "<u>underline</u>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // slow blink
    {
        .f_input = "\033[5mslow blink\033[m",
        .f_plain_text = "slow blink",
        .f_html = "<span class=\"ansi-sb\">slow blink</span>",
        .f_styles = "@keyframes ansi-blinker{50%{opacity:0}}\n.ansi-sb{animation:ansi-blinker 2s linear infinite}\n",
    },

    // fast blink
    {
        .f_input = "\033[6mfast blink\033[m",
        .f_plain_text = "fast blink",
        .f_html = "<span class=\"ansi-fb\">fast blink</span>",
        .f_styles = "@keyframes ansi-blinker{50%{opacity:0}}\n.ansi-fb{animation:ansi-blinker 0.4s linear infinite}\n",
    },

    // inverse
    {
        .f_input = "\033[7minverse\033[m",
        .f_plain_text = "inverse",
        .f_html = "<span style=\"color:#ffffff;background-color:#000000\">inverse</span>",
    },

    // conceal
    {
        .f_input = "\033[8mconceal this now\033[m",
        .f_plain_text = "",
        .f_html = "",
    },

    // cross out
    {
        .f_input = "\033[9mcross out\033[m",
        .f_plain_text = "cross out",
        .f_html = "<span class=\"ansi-s\">cross out</span>",
        .f_optimized_html = "<s>cross out</s>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // cross out + underline
    {
        .f_input = "\033[9;4mcross out + underline\033[m",
        .f_plain_text = "cross out + underline",
        .f_html = "<span class=\"ansi-us\">cross out + underline</span>",
        .f_optimized_html = "<u><s>cross out + underline</s></u>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // underline + cross out
    {
        .f_input = "\033[4;9munderline + cross out\033[m",
        .f_plain_text = "underline + cross out",
        .f_html = "<span class=\"ansi-us\">underline + cross out</span>",
        .f_optimized_html = "<u><s>underline + cross out</s></u>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // double underline
    {
        .f_input = "\033[21mdouble underline\033[m",
        .f_plain_text = "double underline",
        .f_html = "<span class=\"ansi-d\">double underline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // cross out + double underline
    {
        .f_input = "\033[9;21mcross out + double underline\033[m",
        .f_plain_text = "cross out + double underline",
        .f_html = "<span class=\"ansi-ds\">cross out + double underline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // double underline + cross out
    {
        .f_input = "\033[21;9mdouble underline + cross out\033[m",
        .f_plain_text = "double underline + cross out",
        .f_html = "<span class=\"ansi-ds\">double underline + cross out</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // bold + normal intensity
    {
        .f_input = "normal then \033[1mbold\033[22m and not\033[m",
        .f_plain_text = "normal then bold and not",
        .f_html = "normal then <span class=\"ansi-b\">bold </span>and not",
        .f_optimized_html = "normal then <b>bold </b>and not",
        .f_markdown = "normal then *bold* and not",
        .f_styles = ".ansi-b{font-weight:bold}\n",
        .f_styles_with_defaults = "b,.ansi-b{font-weight:bold}\n",
    },

    // light + normal intensity
    {
        .f_input = "normal then \033[2mlight\033[22m and not\033[m",
        .f_plain_text = "normal then light and not",
        .f_html = "normal then <span class=\"ansi-l\">light </span>and not",
        .f_styles = ".ansi-l{font-weight:light}\n",
    },

    // italic + normal intensity
    {
        .f_input = "normal then \033[3mitalic\033[23m and not\033[m",
        .f_plain_text = "normal then italic and not",
        .f_html = "normal then <span class=\"ansi-i\">italic </span>and not",
        .f_optimized_html = "normal then <i>italic </i>and not",
        .f_markdown = "normal then **italic** and not",
        .f_styles = ".ansi-i{font-style:italic}\n",
        .f_styles_with_defaults = "i,.ansi-i{font-style:italic}\n",
    },

    // underline + not-underline
    {
        .f_input = "normal then \033[4munderline\033[24m and not\033[m",
        .f_plain_text = "normal then underline and not",
        .f_html = "normal then <span class=\"ansi-u\">underline </span>and not",
        .f_optimized_html = "normal then <u>underline </u>and not",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // double underline + not-underline
    {
        .f_input = "normal then \033[21mdouble underline\033[24m and not\033[m",
        .f_plain_text = "normal then double underline and not",
        .f_html = "normal then <span class=\"ansi-d\">double underline </span>and not",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // slow blink + not blink
    {
        .f_input = "normal then \033[5mslow blink\033[25m and not\033[m",
        .f_plain_text = "normal then slow blink and not",
        .f_html = "normal then <span class=\"ansi-sb\">slow blink </span>and not",
        .f_styles = "@keyframes ansi-blinker{50%{opacity:0}}\n.ansi-sb{animation:ansi-blinker 2s linear infinite}\n",
    },

    // fast blink + not blink
    {
        .f_input = "normal then \033[6mfast blink\033[25m and not\033[m",
        .f_plain_text = "normal then fast blink and not",
        .f_html = "normal then <span class=\"ansi-fb\">fast blink </span>and not",
        .f_styles = "@keyframes ansi-blinker{50%{opacity:0}}\n.ansi-fb{animation:ansi-blinker 0.4s linear infinite}\n",
    },

    // proportional
    {
        .f_input = "normal then \033[26mproportional\033[50m and not\033[m",
        .f_plain_text = "normal then proportional and not",
        .f_html = "normal then <span class=\"ansi-p\">proportional </span>and not",
        .f_styles = ".ansi-p{font-family:sans-serif}\n",
    },

    // inverse + positive
    {
        .f_input = "normal then \033[7minverse\033[27m and not\033[m",
        .f_plain_text = "normal then inverse and not",
        .f_html = "normal then <span style=\"color:#ffffff;background-color:#000000\">inverse </span>and not",
    },

    // conceal + visible
    {
        .f_input = "normal then \033[8mconcealed\033[28m and not\033[m",
        .f_plain_text = "normal then  and not",
        .f_html = "normal then  and not",
    },

    // cross out + not cross out
    {
        .f_input = "normal then \033[9mcross out\033[29m and not\033[m",
        .f_plain_text = "normal then cross out and not",
        .f_html = "normal then <span class=\"ansi-s\">cross out </span>and not",
        .f_optimized_html = "normal then <s>cross out </s>and not",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // foreground black color
    {
        .f_input = "foreground \033[30mblack\033[39m color\033[m",
        .f_plain_text = "foreground black color",
        .f_html = "foreground <span style=\"color:#000000;background-color:#ffffff\">black </span>color",
    },

    // foreground red color
    {
        .f_input = "foreground \033[31mred\033[39m color\033[m",
        .f_plain_text = "foreground red color",
        .f_html = "foreground <span style=\"color:#de382b;background-color:#ffffff\">red </span>color",
    },

    // foreground green color
    {
        .f_input = "foreground \033[32mgreen\033[39m color\033[m",
        .f_plain_text = "foreground green color",
        .f_html = "foreground <span style=\"color:#39b54a;background-color:#ffffff\">green </span>color",
    },

    // foreground yellow color
    {
        .f_input = "foreground \033[33myellow\033[39m color\033[m",
        .f_plain_text = "foreground yellow color",
        .f_html = "foreground <span style=\"color:#ffc706;background-color:#ffffff\">yellow </span>color",
    },

    // foreground blue color
    {
        .f_input = "foreground \033[34mblue\033[39m color\033[m",
        .f_plain_text = "foreground blue color",
        .f_html = "foreground <span style=\"color:#006fb8;background-color:#ffffff\">blue </span>color",
    },

    // foreground magenta color
    {
        .f_input = "foreground \033[35mmagenta\033[39m color\033[m",
        .f_plain_text = "foreground magenta color",
        .f_html = "foreground <span style=\"color:#762671;background-color:#ffffff\">magenta </span>color",
    },

    // foreground cyan color
    {
        .f_input = "foreground \033[36mcyan\033[39m color\033[m",
        .f_plain_text = "foreground cyan color",
        .f_html = "foreground <span style=\"color:#2cb5e9;background-color:#ffffff\">cyan </span>color",
    },

    // foreground white color
    {
        .f_input = "foreground \033[37mwhite\033[39m color\033[m",
        .f_plain_text = "foreground white color",
        .f_html = "foreground <span style=\"color:#cccccc;background-color:#ffffff\">white </span>color",
    },

    // foreground transparent
    {
        .f_input = "foreground \033[38;1mtransparent\033[39m color\033[m",
        .f_plain_text = "foreground transparent color",
        .f_html = "foreground <span style=\"opacity:0%;background-color:#ffffff\">transparent </span>color",
    },

    // foreground RGB color
    {
        .f_input = "foreground \033[38;2;32;64;96mRGB\033[39m color\033[m",
        .f_plain_text = "foreground RGB color",
        .f_html = "foreground <span style=\"color:#204060;background-color:#ffffff\">RGB </span>color",
    },

    // foreground CMY color
    {
        .f_input = "foreground \033[38;3;32;64;96mCMY\033[39m color\033[m",
        .f_plain_text = "foreground CMY color",
        .f_html = "foreground <span style=\"color:#dfbf9f;background-color:#ffffff\">CMY </span>color",
    },

    // foreground CMYK color
    {
        .f_input = "foreground \033[38;4;32;64;96;128mCMYK\033[39m color\033[m",
        .f_plain_text = "foreground CMYK color",
        .f_html = "foreground <span style=\"color:#6f5f4f;background-color:#ffffff\">CMYK </span>color",
    },

    // foreground index color 100
    {
        .f_input = "foreground \033[38;5;100mindex color 100\033[39m color\033[m",
        .f_plain_text = "foreground index color 100 color",
        .f_html = "foreground <span style=\"color:#878700;background-color:#ffffff\">index color 100 </span>color",
    },

    // inverse does not work on foreground transparent
    // (i.e. the background color is #ffffff when foreground is transparent)
    {
        .f_input = "foreground \033[7minverse here \033[38;1mthen transparent\033[39m then not\033[m",
        .f_plain_text = "foreground inverse here then transparent then not",
        .f_html = "foreground <span style=\"color:#ffffff;background-color:#000000\">inverse here </span><span style=\"opacity:0%;background-color:#ffffff\">then transparent </span><span style=\"color:#ffffff;background-color:#000000\">then not</span>",
    },

    // background black color
    {
        .f_input = "background \033[40mblack\033[49m color\033[m",
        .f_plain_text = "background black color",
        .f_html = "background <span style=\"color:#000000;background-color:#000000\">black </span>color",
    },

    // background red color
    {
        .f_input = "background \033[41mred\033[49m color\033[m",
        .f_plain_text = "background red color",
        .f_html = "background <span style=\"color:#000000;background-color:#de382b\">red </span>color",
    },

    // background green color
    {
        .f_input = "background \033[42mgreen\033[49m color\033[m",
        .f_plain_text = "background green color",
        .f_html = "background <span style=\"color:#000000;background-color:#39b54a\">green </span>color",
    },

    // background yellow color
    {
        .f_input = "background \033[43myellow\033[49m color\033[m",
        .f_plain_text = "background yellow color",
        .f_html = "background <span style=\"color:#000000;background-color:#ffc706\">yellow </span>color",
    },

    // background blue color
    {
        .f_input = "background \033[44mblue\033[49m color\033[m",
        .f_plain_text = "background blue color",
        .f_html = "background <span style=\"color:#000000;background-color:#006fb8\">blue </span>color",
    },

    // background magenta color
    {
        .f_input = "background \033[45mmagenta\033[49m color\033[m",
        .f_plain_text = "background magenta color",
        .f_html = "background <span style=\"color:#000000;background-color:#762671\">magenta </span>color",
    },

    // background cyan color
    {
        .f_input = "background \033[46mcyan\033[49m color\033[m",
        .f_plain_text = "background cyan color",
        .f_html = "background <span style=\"color:#000000;background-color:#2cb5e9\">cyan </span>color",
    },

    // background white color
    {
        .f_input = "background \033[47mwhite\033[49m color\033[m",
        .f_plain_text = "background white color",
        .f_html = "background <span style=\"color:#000000;background-color:#cccccc\">white </span>color",
    },

    // background RGB color
    {
        .f_input = "background \033[48;2;32;64;96mRGB\033[49m color\033[m",
        .f_plain_text = "background RGB color",
        .f_html = "background <span style=\"color:#000000;background-color:#204060\">RGB </span>color",
    },

    // background CMY color
    {
        .f_input = "background \033[48;3;32;64;96mCMY\033[49m color\033[m",
        .f_plain_text = "background CMY color",
        .f_html = "background <span style=\"color:#000000;background-color:#dfbf9f\">CMY </span>color",
    },

    // background CMYK color
    {
        .f_input = "background \033[48;4;32;64;96;128mCMYK\033[49m color\033[m",
        .f_plain_text = "background CMYK color",
        .f_html = "background <span style=\"color:#000000;background-color:#6f5f4f\">CMYK </span>color",
    },

    // background index color 100
    {
        .f_input = "background \033[48;5;100mindex color 100\033[49m color\033[m",
        .f_plain_text = "background index color 100 color",
        .f_html = "background <span style=\"color:#000000;background-color:#878700\">index color 100 </span>color",
    },

    // overline
    {
        .f_input = "\033[53moverline\033[m",
        .f_plain_text = "overline",
        .f_html = "<span class=\"ansi-v\">overline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // overline + not overline
    {
        .f_input = "normal \033[53mthen overline\033[55m and not\033[m",
        .f_plain_text = "normal then overline and not",
        .f_html = "normal <span class=\"ansi-v\">then overline </span>and not",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // cross out + overline
    {
        .f_input = "\033[9;53mcross out + overline\033[m",
        .f_plain_text = "cross out + overline",
        .f_html = "<span class=\"ansi-vs\">cross out + overline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // overline + cross out
    {
        .f_input = "\033[53;9moverline + cross out\033[m",
        .f_plain_text = "overline + cross out",
        .f_html = "<span class=\"ansi-vs\">overline + cross out</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // cross out + overline + underline
    {
        .f_input = "\033[9;53;4mcross out + overline + underline\033[m",
        .f_plain_text = "cross out + overline + underline",
        .f_html = "<span class=\"ansi-uvs\">cross out + overline + underline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // overline + cross out + underline
    {
        .f_input = "\033[53;9;4moverline + cross out + underline\033[m",
        .f_plain_text = "overline + cross out + underline",
        .f_html = "<span class=\"ansi-uvs\">overline + cross out + underline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // underline + cross out + overline
    {
        .f_input = "\033[4;9;53munderline + cross out + overline\033[m",
        .f_plain_text = "underline + cross out + overline",
        .f_html = "<span class=\"ansi-uvs\">underline + cross out + overline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // underline + overline + cross out
    {
        .f_input = "\033[4;53;9munderline + overline + cross out\033[m",
        .f_plain_text = "underline + overline + cross out",
        .f_html = "<span class=\"ansi-uvs\">underline + overline + cross out</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // cross out + underline + overline
    {
        .f_input = "\033[9;4;53mcross out + underline + overline\033[m",
        .f_plain_text = "cross out + underline + overline",
        .f_html = "<span class=\"ansi-uvs\">cross out + underline + overline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // overline + underline + cross out
    {
        .f_input = "\033[53;4;9moverline + underline + cross out\033[m",
        .f_plain_text = "overline + underline + cross out",
        .f_html = "<span class=\"ansi-uvs\">overline + underline + cross out</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // cross out + overline + double underline
    {
        .f_input = "\033[9;53;21mcross out + overline + double underline\033[m",
        .f_plain_text = "cross out + overline + double underline",
        .f_html = "<span class=\"ansi-dvs\">cross out + overline + double underline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // overline + cross out + double underline
    {
        .f_input = "\033[53;9;21moverline + cross out + double underline\033[m",
        .f_plain_text = "overline + cross out + double underline",
        .f_html = "<span class=\"ansi-dvs\">overline + cross out + double underline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // double underline + cross out + overline
    {
        .f_input = "\033[21;9;53mdouble underline + cross out + overline\033[m",
        .f_plain_text = "double underline + cross out + overline",
        .f_html = "<span class=\"ansi-dvs\">double underline + cross out + overline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // double underline + overline + cross out
    {
        .f_input = "\033[21;53;9mdouble underline + overline + cross out\033[m",
        .f_plain_text = "double underline + overline + cross out",
        .f_html = "<span class=\"ansi-dvs\">double underline + overline + cross out</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // cross out + double underline + overline
    {
        .f_input = "\033[9;21;53mcross out + double underline + overline\033[m",
        .f_plain_text = "cross out + double underline + overline",
        .f_html = "<span class=\"ansi-dvs\">cross out + double underline + overline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // overline + double underline + cross out
    {
        .f_input = "\033[53;21;9moverline + double underline + cross out\033[m",
        .f_plain_text = "overline + double underline + cross out",
        .f_html = "<span class=\"ansi-dvs\">overline + double underline + cross out</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // underline + overline
    {
        .f_input = "\033[4;53munderline + overline\033[m",
        .f_plain_text = "underline + overline",
        .f_html = "<span class=\"ansi-uv\">underline + overline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // overline + underline
    {
        .f_input = "\033[53;4moverline + underline\033[m",
        .f_plain_text = "overline + underline",
        .f_html = "<span class=\"ansi-uv\">overline + underline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // double underline + overline
    {
        .f_input = "\033[21;53mdouble underline + overline\033[m",
        .f_plain_text = "double underline + overline",
        .f_html = "<span class=\"ansi-dv\">double underline + overline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // overline + double underline
    {
        .f_input = "\033[53;21moverline + double underline\033[m",
        .f_plain_text = "overline + double underline",
        .f_html = "<span class=\"ansi-dv\">overline + double underline</span>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // underline with a different color
    {
        .f_input = "normal \033[4mthen underline\033[58;5;200m with color\033[59m and not\033[m",
        .f_plain_text = "normal then underline with color and not",
        .f_html = "normal <span class=\"ansi-u\">then underline </span><span class=\"ansi-u\" style=\"text-decoration-color:#ff00d7\">with color </span><span class=\"ansi-u\">and not</span>",
        .f_optimized_html = "normal <u>then underline </u><u><span style=\"text-decoration-color:#ff00d7\">with color </span></u><u>and not</u>",
        .f_styles = ".ansi-s{text-decoration-line:line-through}\n"
                    ".ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
        .f_styles_with_defaults = "s,.ansi-s{text-decoration-line:line-through}\n"
                    "u,.ansi-u{text-decoration-line:underline}\n"
                    ".ansi-d{text-decoration-line:underline;text-decoration-style:double}\n"
                    ".ansi-v{text-decoration-line:overline;}\n"
                    ".ansi-us{text-decoration-line:underline line-through}\n"
                    ".ansi-uv{text-decoration-line:underline overline}\n"
                    ".ansi-uvs{text-decoration-line:underline overline line-through}\n"
                    ".ansi-ds{text-decoration-line:underline line-through;text-decoration-style:double}\n"
                    ".ansi-dv{text-decoration-line:underline overline;text-decoration-style:double}\n"
                    ".ansi-dvs{text-decoration-line:underline overline line-through;text-decoration-style:double}\n"
                    ".ansi-vs{text-decoration-line:overline line-through}\n",
    },

    // foreground, background, and underline colors
    {
        .f_input = "normal \033[38;2;5;15;35;48;2;65;85;95;58;2;4;44;144mcolorful\033[39;49;59m normal",
        .f_plain_text = "normal colorful normal",
        .f_html = "normal <span style=\"color:#050f23;background-color:#41555f;text-decoration-color:#042c90\">colorful </span>normal",
    },

    // superscript
    {
        .f_input = "\033[73msuperscript\033[m",
        .f_plain_text = "superscript",
        .f_html = "<span class=\"ansi-sup\">superscript</span>",
        .f_optimized_html = "<sup>superscript</sup>",
        .f_styles = ".ansi-sup{font-size:60%;vertical-align:super}\n",
        .f_styles_with_defaults = "sup,.ansi-sup{font-size:60%;vertical-align:super}\n",
    },

    // subscript
    {
        .f_input = "\033[74msubscript\033[m",
        .f_plain_text = "subscript",
        .f_html = "<span class=\"ansi-sub\">subscript</span>",
        .f_optimized_html = "<sub>subscript</sub>",
        .f_styles = ".ansi-sub{font-size:60%;vertical-align:sub}\n",
        .f_styles_with_defaults = "sub,.ansi-sub{font-size:60%;vertical-align:sub}\n",
    },

    // superscript + not superscript
    {
        .f_input = "normal \033[73mthen superscript\033[75m and not\033[m",
        .f_plain_text = "normal then superscript and not",
        .f_html = "normal <span class=\"ansi-sup\">then superscript </span>and not",
        .f_optimized_html = "normal <sup>then superscript </sup>and not",
        .f_styles = ".ansi-sup{font-size:60%;vertical-align:super}\n",
        .f_styles_with_defaults = "sup,.ansi-sup{font-size:60%;vertical-align:super}\n",
    },

    // subscript + not subscript
    {
        .f_input = "normal \033[74mthen subscript\033[75m and not\033[m",
        .f_plain_text = "normal then subscript and not",
        .f_html = "normal <span class=\"ansi-sub\">then subscript </span>and not",
        .f_optimized_html = "normal <sub>then subscript </sub>and not",
        .f_styles = ".ansi-sub{font-size:60%;vertical-align:sub}\n",
        .f_styles_with_defaults = "sub,.ansi-sub{font-size:60%;vertical-align:sub}\n",
    },

    // superscript + subscript are mutually exclusive
    {
        .f_input = "normal \033[73mthen superscript \033[74mthen subscript\033[75m and then not\033[m",
        .f_plain_text = "normal then superscript then subscript and then not",
        .f_html = "normal <span class=\"ansi-sup\">then superscript </span><span class=\"ansi-sub\">then subscript </span>and then not",
        .f_optimized_html = "normal <sup>then superscript </sup><sub>then subscript </sub>and then not",
        .f_styles = ".ansi-sup{font-size:60%;vertical-align:super}\n.ansi-sub{font-size:60%;vertical-align:sub}\n",
        .f_styles_with_defaults = "sup,.ansi-sup{font-size:60%;vertical-align:super}\nsub,.ansi-sub{font-size:60%;vertical-align:sub}\n",
    },

    // subscript + superscript are mutually exclusive
    {
        .f_input = "normal \033[74mthen subscript \033[73mthen superscript\033[75m and then not\033[m",
        .f_plain_text = "normal then subscript then superscript and then not",
        .f_html = "normal <span class=\"ansi-sub\">then subscript </span><span class=\"ansi-sup\">then superscript </span>and then not",
        .f_optimized_html = "normal <sub>then subscript </sub><sup>then superscript </sup>and then not",
        .f_styles = ".ansi-sup{font-size:60%;vertical-align:super}\n.ansi-sub{font-size:60%;vertical-align:sub}\n",
        .f_styles_with_defaults = "sup,.ansi-sup{font-size:60%;vertical-align:super}\nsub,.ansi-sub{font-size:60%;vertical-align:sub}\n",
    },

    // foreground gray color
    {
        .f_input = "foreground \033[90mgray\033[39m color\033[m",
        .f_plain_text = "foreground gray color",
        .f_html = "foreground <span style=\"color:#808080;background-color:#ffffff\">gray </span>color",
    },

    // foreground bright red color
    {
        .f_input = "foreground \033[91mbright red\033[39m color\033[m",
        .f_plain_text = "foreground bright red color",
        .f_html = "foreground <span style=\"color:#ff0000;background-color:#ffffff\">bright red </span>color",
    },

    // foreground bright green color
    {
        .f_input = "foreground \033[92mbright green\033[39m color\033[m",
        .f_plain_text = "foreground bright green color",
        .f_html = "foreground <span style=\"color:#00ff00;background-color:#ffffff\">bright green </span>color",
    },

    // foreground bright yellow color
    {
        .f_input = "foreground \033[93mbright yellow\033[39m color\033[m",
        .f_plain_text = "foreground bright yellow color",
        .f_html = "foreground <span style=\"color:#ffff00;background-color:#ffffff\">bright yellow </span>color",
    },

    // foreground bright blue color
    {
        .f_input = "foreground \033[94mbright blue\033[39m color\033[m",
        .f_plain_text = "foreground bright blue color",
        .f_html = "foreground <span style=\"color:#0000ff;background-color:#ffffff\">bright blue </span>color",
    },

    // foreground bright magenta color
    {
        .f_input = "foreground \033[95mbright magenta\033[39m color\033[m",
        .f_plain_text = "foreground bright magenta color",
        .f_html = "foreground <span style=\"color:#ff00ff;background-color:#ffffff\">bright magenta </span>color",
    },

    // foreground bright cyan color
    {
        .f_input = "foreground \033[96mbright cyan\033[39m color\033[m",
        .f_plain_text = "foreground bright cyan color",
        .f_html = "foreground <span style=\"color:#00ffff;background-color:#ffffff\">bright cyan </span>color",
    },

    // foreground bright white color
    {
        .f_input = "foreground \033[97mbright white\033[39m color\033[m",
        .f_plain_text = "foreground bright white color",
        .f_html = "foreground <span style=\"color:#ffffff;background-color:#ffffff\">bright white </span>color",
    },

    // background gray color
    {
        .f_input = "background \033[100mgray\033[49m color\033[m",
        .f_plain_text = "background gray color",
        .f_html = "background <span style=\"color:#000000;background-color:#808080\">gray </span>color",
    },

    // background bright red color
    {
        .f_input = "background \033[101mbright red\033[49m color\033[m",
        .f_plain_text = "background bright red color",
        .f_html = "background <span style=\"color:#000000;background-color:#ff0000\">bright red </span>color",
    },

    // background bright green color
    {
        .f_input = "background \033[102mbright green\033[49m color\033[m",
        .f_plain_text = "background bright green color",
        .f_html = "background <span style=\"color:#000000;background-color:#00ff00\">bright green </span>color",
    },

    // background bright yellow color
    {
        .f_input = "background \033[103mbright yellow\033[49m color\033[m",
        .f_plain_text = "background bright yellow color",
        .f_html = "background <span style=\"color:#000000;background-color:#ffff00\">bright yellow </span>color",
    },

    // background bright blue color
    {
        .f_input = "background \033[104mbright blue\033[49m color\033[m",
        .f_plain_text = "background bright blue color",
        .f_html = "background <span style=\"color:#000000;background-color:#0000ff\">bright blue </span>color",
    },

    // background bright magenta color
    {
        .f_input = "background \033[105mbright magenta\033[49m color\033[m",
        .f_plain_text = "background bright magenta color",
        .f_html = "background <span style=\"color:#000000;background-color:#ff00ff\">bright magenta </span>color",
    },

    // background bright cyan color
    {
        .f_input = "background \033[106mbright cyan\033[49m color\033[m",
        .f_plain_text = "background bright cyan color",
        .f_html = "background <span style=\"color:#000000;background-color:#00ffff\">bright cyan </span>color",
    },

    // background bright white color
    {
        .f_input = "background \033[107mbright white\033[49m color\033[m",
        .f_plain_text = "background bright white color",
        .f_html = "background <span style=\"color:#000000;background-color:#ffffff\">bright white </span>color",
    },

    // HTML special characters
    {
        .f_input = "quot - \", amp - &, apos - ', lt - <, and gt - >",
        .f_plain_text = "quot - \", amp - &, apos - ', lt - <, and gt - >",
        .f_html = "quot - &quot;, amp - &amp;, apos - &apos;, lt - &lt;, and gt - &gt;",
        .f_markdown = "quot \\- \", amp \\- &, apos \\- ', lt \\- \\<, and gt \\- \\>",
    },

    // markdown special characters
    {
        .f_input = "++ * - # _ < > ` [ \\ ++",
        .f_plain_text = "++ * - # _ < > ` [ \\ ++",
        .f_html = "++ * - # _ &lt; &gt; ` [ \\ ++",
        .f_markdown = "++ \\* \\- \\# \\_ \\< \\> \\` \\[ \\\\ ++",
    },

    // multiple lines
    {
        .f_input = "this is line 1\nthen comes line 2\r\nand we got 3\rfinally line 4",
        .f_plain_text = "this is line 1\nthen comes line 2\nand we got 3\nfinally line 4",
        .f_html = "this is line 1<br/>\nthen comes line 2<br/>\nand we got 3<br/>\nfinally line 4",
    },

    // unsupported system control
    {
        .f_input = "change title \033]0;This is the title\007",
        .f_plain_text = "change title ]0;This is the title\007",
        .f_html = "change title ]0;This is the title\007",
    },

    // unsupported ST characters (for example the console can move the cursor)
    {
        .f_input = "A\033[AB\033[BC\033[CD\033[DE\033[EF\033[FG\033[GH\033[HJ\033[JK\033[KS\033[ST\033[Tf\033[fh\033[hi\033[il\033[ln\033[ns\033[su\033[u",
        .f_plain_text = "ABCDEFGHJKSTfhilnsu",
        .f_html = "ABCDEFGHJKSTfhilnsu",
    },

    // fonts are ignored
    {
        .f_input = "\033[10mFon\033[11mt s\033[12mele\033[13mcti\033[14mons\033[15m ar\033[16me i\033[17mgno\033[18mred\033[19m Fr\033[20makt\033[mur.",
        .f_plain_text = "Font selections are ignored Fraktur.",
        .f_html = "Font selections are ignored Fraktur.",
    },

    // frames are ignored
    {
        .f_input = "Normal \033[51mFramed \033[52mCircled \033[54mNormal",
        .f_plain_text = "Normal Framed Circled Normal",
        .f_html = "Normal Framed Circled Normal",
    },

    // ideograms are ignored
    {
        .f_input = "Normal \033[60mIdeo0 \033[61mIdeo1 \033[62mIdeo2 \033[63mIdeo3 \033[64mIdeo4 \033[65mNormal",
        .f_plain_text = "Normal Ideo0 Ideo1 Ideo2 Ideo3 Ideo4 Normal",
        .f_html = "Normal Ideo0 Ideo1 Ideo2 Ideo3 Ideo4 Normal",
    },
};



}
// no name namespace


CATCH_TEST_CASE("convert_ansi", "[converter]")
{
    CATCH_START_SECTION("convert_ansi: simple conversions")
    {
        snaplogger::convert_ansi::pointer_t plain_text_converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT));
        snaplogger::convert_ansi::pointer_t html_converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_HTML));
        snaplogger::convert_ansi::pointer_t markdown_converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_MARKDOWN));

        CATCH_REQUIRE(plain_text_converter->get_type() == snaplogger::ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT);
        CATCH_REQUIRE(html_converter->get_type() == snaplogger::ansi_output_t::ANSI_OUTPUT_HTML);
        CATCH_REQUIRE(markdown_converter->get_type() == snaplogger::ansi_output_t::ANSI_OUTPUT_MARKDOWN);

        CATCH_REQUIRE_FALSE(plain_text_converter->get_optimize());
        CATCH_REQUIRE_FALSE(html_converter->get_optimize());
        CATCH_REQUIRE_FALSE(markdown_converter->get_optimize());

        // save cursor; we assume that the longest string will wrap one line
        //
        std::cout << "\n\n\033[s";

        for(auto const & convert : g_convert_data)
        {
            // to avoid the reuse of the line, change the \033[u with \033[s
            //
            std::cout << "\033[u\033[2A--- working on [" << snapdev::string_replace_many(std::string(convert.f_input), {
                    {"\r", "\\r"},
                    {"\n", "\\n"},
                    {"\033[A", "\\033[A"},
                    {"\033[B", "\\033[B"},
                    {"\033[C", "\\033[C"},
                    {"\033[D", "\\033[D"},
                    {"\033[E", "\\033[E"},
                    {"\033[F", "\\033[F"},
                    {"\033[G", "\\033[G"},
                    {"\033[H", "\\033[H"},
                    {"\033[J", "\\033[J"},
                    {"\033[K", "\\033[K"},
                    {"\033[S", "\\033[S"},
                    {"\033[T", "\\033[T"},
                    {"\033[f", "\\033[f"},
                    {"\033[h", "\\033[h"},
                    {"\033[i", "\\033[i"},
                    {"\033[l", "\\033[l"},
                    {"\033[n", "\\033[n"},
                    {"\033[s", "\\033[s"},
                    {"\033[u", "\\033[u"},
                }) << "]\033[K\n";

            // plain text
            //
            plain_text_converter->write(convert.f_input);
            std::string result(plain_text_converter->read());
            CATCH_REQUIRE_FALSE(plain_text_converter->has_invalid_data());
            CATCH_REQUIRE(convert.f_plain_text == result);
            CATCH_REQUIRE(plain_text_converter->get_styles().empty());

            // html
            //
            html_converter->write(convert.f_input);
            result = html_converter->read();
            CATCH_REQUIRE_FALSE(html_converter->has_invalid_data());
            CATCH_REQUIRE(convert.f_html == result);

            // clean html (to verify styles)
            //
            {
                snaplogger::convert_ansi::pointer_t clean_html_converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_HTML));
                clean_html_converter->write(convert.f_input);
                result = clean_html_converter->read();
                CATCH_REQUIRE_FALSE(clean_html_converter->has_invalid_data());
                CATCH_REQUIRE(convert.f_html == result);
                if(convert.f_styles != nullptr)
                {
//std::cout << " >>> (1)T:[" << snapdev::string_replace_many(std::string(convert.f_styles), {
//        {"\r", "\\r"},
//        {"\n", "\\n"},
//    }) << " && " << snapdev::string_replace_many(std::string(convert.f_styles_with_defaults == nullptr ? convert.f_styles : convert.f_styles_with_defaults), {
//        {"\r", "\\r"},
//        {"\n", "\\n"},
//    }) << "]\n";
//std::cout << " >>> (1)C:[" << snapdev::string_replace_many(clean_html_converter->get_styles(), {
//        {"\r", "\\r"},
//        {"\n", "\\n"},
//    }) << " && " << snapdev::string_replace_many(clean_html_converter->get_styles(true), {
//        {"\r", "\\r"},
//        {"\n", "\\n"},
//    }) << "]\n";
                    CATCH_REQUIRE(convert.f_styles == clean_html_converter->get_styles());
                    CATCH_REQUIRE(convert.f_styles == clean_html_converter->get_styles(true));
                    //CATCH_REQUIRE((convert.f_styles_with_defaults == nullptr ? convert.f_styles : convert.f_styles_with_defaults) == clean_html_converter->get_styles(true));
                }
                else
                {
                    if(!clean_html_converter->get_styles().empty())
                    {
                        std::cout << std::flush;
                        std::cerr << "error: styles are not empty:\n  "
                            << snapdev::string_replace_many(clean_html_converter->get_styles(), {{"\n", "\n  "}})
                            << "\n";
                    }
                    CATCH_REQUIRE(clean_html_converter->get_styles().empty());
                }
            }

            // optimized html
            //
            html_converter->set_optimize(true);
            CATCH_REQUIRE(html_converter->get_optimize());

            html_converter->write(convert.f_input);
            result = html_converter->read();
            CATCH_REQUIRE_FALSE(html_converter->has_invalid_data());
            CATCH_REQUIRE((convert.f_optimized_html == nullptr ? convert.f_html : convert.f_optimized_html) == result);

            html_converter->set_optimize(false);
            CATCH_REQUIRE_FALSE(html_converter->get_optimize());

            // clean optimized html
            //
            {
                snaplogger::convert_ansi::pointer_t clean_html_converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_HTML));
                clean_html_converter->set_optimize(true);
                CATCH_REQUIRE(clean_html_converter->get_optimize());

                clean_html_converter->write(convert.f_input);
                result = clean_html_converter->read();
                CATCH_REQUIRE_FALSE(clean_html_converter->has_invalid_data());
                CATCH_REQUIRE((convert.f_optimized_html == nullptr ? convert.f_html : convert.f_optimized_html) == result);
                if(convert.f_styles != nullptr)
                {
//std::cout << " >>> (2)T:[" << snapdev::string_replace_many(std::string(convert.f_styles), {
//        {"\r", "\\r"},
//        {"\n", "\\n"},
//    }) << " && " << snapdev::string_replace_many(std::string(convert.f_styles_with_defaults == nullptr ? convert.f_styles : convert.f_styles_with_defaults), {
//        {"\r", "\\r"},
//        {"\n", "\\n"},
//    }) << "]\n";
//std::cout << " >>> (2)C:[" << snapdev::string_replace_many(clean_html_converter->get_styles(), {
//        {"\r", "\\r"},
//        {"\n", "\\n"},
//    }) << " && " << snapdev::string_replace_many(clean_html_converter->get_styles(true), {
//        {"\r", "\\r"},
//        {"\n", "\\n"},
//    }) << "]\n";
                    //CATCH_REQUIRE(convert.f_styles == clean_html_converter->get_styles());
                    CATCH_REQUIRE((convert.f_styles_with_defaults == nullptr ? convert.f_styles : convert.f_styles_with_defaults) == clean_html_converter->get_styles(true));
                }
                else
                {
                    if(!clean_html_converter->get_styles().empty())
                    {
                        std::cout << std::flush;
                        std::cerr << "error: styles are not empty:\n  "
                            << snapdev::string_replace_many(clean_html_converter->get_styles(), {{"\n", "\n  "}})
                            << "\n";
                    }
                    CATCH_REQUIRE(clean_html_converter->get_styles().empty());
                }
            }

            // markdown
            //
            markdown_converter->write(convert.f_input);
            result = markdown_converter->read();
            CATCH_REQUIRE_FALSE(markdown_converter->has_invalid_data());
            CATCH_REQUIRE((convert.f_markdown == nullptr ? convert.f_plain_text : convert.f_markdown) == result);
            CATCH_REQUIRE(markdown_converter->get_styles().empty());
        }
        std::cout << "\033[K";
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("convert_ansi_invalid", "[converter][error]")
{
    CATCH_START_SECTION("convert_ansi_invalid: bad UTF-8 character")
    {
        snaplogger::convert_ansi::pointer_t converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT));
        converter->write("\033[4mBad char: \x83\033[m");
        std::string const result(converter->read());
        CATCH_REQUIRE(converter->has_invalid_data());
        CATCH_REQUIRE("Bad char: " == result);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_ansi_invalid: unsupported character closing CSI")
    {
        char buf[4] = {
            '\033',
            '[',
            '@',
            '\0',
        };
        for(; buf[2] <= '~'; ++buf[2])
        {
            switch(buf[2])
            {
            case 'm':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'J':
            case 'K':
            case 'S':
            case 'T':
            case 'f':
            case 'h':
            case 'i':
            case 'l':
            case 'n':
            case 's':
            case 'u':
                // supported characters do not generate an invalid sequence
                // (even if we do not support any of them except the 'm')
                //
                break;

            default:
                {
                    snaplogger::convert_ansi::pointer_t converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT));
                    converter->write(buf);
                    std::string const result(converter->read());
                    CATCH_REQUIRE(converter->has_invalid_data());
                    CATCH_REQUIRE(result.empty());
                }
                break;

            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_ansi_invalid: unsupported CSI value separator")
    {
        for(char32_t wc = 0x000001; wc <= 0x10FFFF; ++wc)
        {
            // skip invalid unicode (surrogates in char32_t are not valid)
            //
            if(wc >= 0xD800 && wc <= 0xDFFF)
            {
                continue;
            }

            // skip digits, those are understood
            //
            if(wc >= U'0' && wc <= U'9')
            {
                continue;
            }

            // skip closing characters
            //
            if(wc >= U'@' && wc <= U'~')
            {
                continue;
            }

            // skip valid separators
            //
            if(wc == ';' || wc == ':')
            {
                continue;
            }
//std::cerr << "--- checking character 0x" << std::hex << std::setw(6) << std::setfill('0') << static_cast<std::uint32_t>(wc) << std::dec << "\n";

            snaplogger::convert_ansi::pointer_t converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT));
            std::string sequence("\033[2");
            sequence += libutf8::to_u8string(wc);
            sequence += "3m";
            converter->write(sequence);
            std::string const result(converter->read());
            CATCH_REQUIRE(converter->has_invalid_data());
            CATCH_REQUIRE(result.empty());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_ansi_invalid: unsupported parameter numbers")
    {
        auto is_valid = [](int const value)
        {
            switch(value)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
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
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
            case 48:
            case 49:
            case 50:
            case 51:
            case 52:
            case 53:
            case 54:
            case 55:
            case 58:
            case 59:
            case 60:
            case 61:
            case 62:
            case 63:
            case 64:
            case 65:
            case 73:
            case 74:
            case 75:
            case 90:
            case 91:
            case 92:
            case 93:
            case 94:
            case 95:
            case 96:
            case 97:
            case 100:
            case 101:
            case 102:
            case 103:
            case 104:
            case 105:
            case 106:
            case 107:
                return true;

            default:
                return false;

            }
        };

        for(int count(0); count < 100; ++count)
        {
            int value(0);
            for(;;)
            {
                value = rand();
                if(!is_valid(value))
                {
                    break;
                }
            }

            snaplogger::convert_ansi::pointer_t converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT));
            std::string sequence("\033[");
            sequence += std::to_string(value);
            sequence += "m";
            converter->write(sequence);
            std::string const result(converter->read());
            CATCH_REQUIRE(converter->has_invalid_data());
            CATCH_REQUIRE(result.empty());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_ansi_invalid: invalid color definitions")
    {
        constexpr char const * invalid_colors[] =
        {
            "\033[48;1m -> transparent not supported with backgrounds",
            "\033[58;1m -> transparent not supported with underlines",
            "\033[38;2m -> RGB missing for foreground color",
            "\033[48;2;45m -> GB in RGB missing for background color",
            "\033[58;2;45;78m -> B in RGB missing for underline color",
            "\033[38;2;256;33;98m -> red value out of bound for foreground color",
            "\033[48;2;45;256;11m -> green value out of bound for background color",
            "\033[58;2;45;78;256m -> blue value out of bound for underline color",
            "\033[38;3m -> CMY missing for foreground color",
            "\033[48;3;45m -> MY in CMY missing for background color",
            "\033[58;3;45;78m -> Y in CMY missing for underline color",
            "\033[38;3;256;33;98m -> cyan value out of bound for foreground color",
            "\033[48;3;45;256;11m -> magenta value out of bound for background color",
            "\033[58;3;45;78;256m -> yellow value out of bound for underline color",
            "\033[38;4m -> CMYK missing for foreground color",
            "\033[48;4;45m -> MYK in CMYK missing for background color",
            "\033[58;4;45;78m -> YK in CMYK missing for underline color",
            "\033[38;4;45;78;14m -> K in CMYK missing for foreground color",
            "\033[38;4;256;33;98;100m -> cyan value out of bound for foreground color",
            "\033[48;4;45;256;11;200m -> magenta value out of bound for background color",
            "\033[58;4;45;78;256;180m -> yellow value out of bound for underline color",
            "\033[48;4;45;78;106;256m -> black value out of bound for background color",
            "\033[38;5m -> index missing for foreground color",
            "\033[48;5;256m -> index out of bound for background color",
        };
        for(auto const & bad_color : invalid_colors)
        {
            snaplogger::convert_ansi::pointer_t converter(std::make_shared<snaplogger::convert_ansi>(snaplogger::ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT));
            converter->write(bad_color);
            std::string const result(converter->read());
            CATCH_REQUIRE(converter->has_invalid_data());
            CATCH_REQUIRE_FALSE(result.empty()); // not super useful, but we include a changing comment
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
