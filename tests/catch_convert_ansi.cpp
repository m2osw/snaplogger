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
};


constexpr convert_t const g_convert_data[] =
{
    // plain text
    {
        .f_input = "plain",
        .f_plain_text = "plain",
        .f_html = "plain",
        .f_optimized_html = "plain",
        .f_markdown = "plain",
    },

    // normal
    {
        .f_input = "\033[0mnormal\033[m",
        .f_plain_text = "normal",
        .f_html = "normal",
        .f_optimized_html = "normal",
        .f_markdown = "normal",
    },

    // bold
    {
        .f_input = "\033[1mbold\033[m",
        .f_plain_text = "bold",
        .f_html = "<span class=\"ansi-b\">bold</span>",
        .f_optimized_html = "<b>bold</b>",
        .f_markdown = "*bold*",
    },

    // light
    {
        .f_input = "\033[2mlight\033[m",
        .f_plain_text = "light",
        .f_html = "<span class=\"ansi-l\">light</span>",
        .f_optimized_html = nullptr,
        .f_markdown = "light",
    },

    // italic
    {
        .f_input = "\033[3mitalic\033[m",
        .f_plain_text = "italic",
        .f_html = "<span class=\"ansi-i\">italic</span>",
        .f_optimized_html = "<i>italic</i>",
        .f_markdown = "**italic**",
    },

    // underline
    {
        .f_input = "\033[4munderline\033[m",
        .f_plain_text = "underline",
        .f_html = "<span class=\"ansi-u\">underline</span>",
        .f_optimized_html = "<u>underline</u>",
        .f_markdown = "underline",
    },

    // slow blink
    {
        .f_input = "\033[5mslow blink\033[m",
        .f_plain_text = "slow blink",
        .f_html = "<span class=\"ansi-sb\">slow blink</span>",
        .f_optimized_html = nullptr,
        .f_markdown = "slow blink",
    },

    // fast blink
    {
        .f_input = "\033[6mfast blink\033[m",
        .f_plain_text = "fast blink",
        .f_html = "<span class=\"ansi-fb\">fast blink</span>",
        .f_optimized_html = nullptr,
        .f_markdown = "fast blink",
    },

    // inverse
    {
        .f_input = "\033[7minverse\033[m",
        .f_plain_text = "inverse",
        .f_html = "<span style=\"color:#ffffff;background-color:#000000\">inverse</span>",
        .f_optimized_html = nullptr,
        .f_markdown = "inverse",
    },

    // conceal
    {
        .f_input = "\033[8mconceal this now\033[m",
        .f_plain_text = "",
        .f_html = "",
        .f_optimized_html = nullptr,
        .f_markdown = "",
    },

    // cross out
    {
        .f_input = "\033[9mcross out\033[m",
        .f_plain_text = "cross out",
        .f_html = "<span class=\"ansi-s\">cross out</span>",
        .f_optimized_html = "<s>cross out</s>",
        .f_markdown = "cross out",
    },

    // double underline
    {
        .f_input = "\033[21mdouble underline\033[m",
        .f_plain_text = "double underline",
        .f_html = "<span class=\"ansi-d\">double underline</span>",
        .f_optimized_html = nullptr,
        .f_markdown = "double underline",
    },

    // bold + normal intensity
    {
        .f_input = "normal then \033[1mbold\033[22m and not\033[m",
        .f_plain_text = "normal then bold and not",
        .f_html = "normal then <span class=\"ansi-b\">bold </span>and not",
        .f_optimized_html = "normal then <b>bold </b>and not",
        .f_markdown = "normal then *bold* and not",
    },

    // light + normal intensity
    {
        .f_input = "normal then \033[2mlight\033[22m and not\033[m",
        .f_plain_text = "normal then light and not",
        .f_html = "normal then <span class=\"ansi-l\">light </span>and not",
        .f_optimized_html = nullptr,
        .f_markdown = "normal then light and not",
    },

    // italic + normal intensity
    {
        .f_input = "normal then \033[3mitalic\033[23m and not\033[m",
        .f_plain_text = "normal then italic and not",
        .f_html = "normal then <span class=\"ansi-i\">italic </span>and not",
        .f_optimized_html = "normal then <i>italic </i>and not",
        .f_markdown = "normal then **italic** and not",
    },

    // underline + not-underline
    {
        .f_input = "normal then \033[4munderline\033[24m and not\033[m",
        .f_plain_text = "normal then underline and not",
        .f_html = "normal then <span class=\"ansi-u\">underline </span>and not",
        .f_optimized_html = "normal then <u>underline </u>and not",
        .f_markdown = "normal then underline and not",
    },

    // double underline + not-underline
    {
        .f_input = "normal then \033[21mdouble underline\033[24m and not\033[m",
        .f_plain_text = "normal then double underline and not",
        .f_html = "normal then <span class=\"ansi-d\">double underline </span>and not",
        .f_optimized_html = nullptr,
        .f_markdown = "normal then double underline and not",
    },

    // slow blink + not blink
    {
        .f_input = "normal then \033[5mslow blink\033[25m and not\033[m",
        .f_plain_text = "normal then slow blink and not",
        .f_html = "normal then <span class=\"ansi-sb\">slow blink </span>and not",
        .f_optimized_html = nullptr,
        .f_markdown = "normal then slow blink and not",
    },

    // fast blink + not blink
    {
        .f_input = "normal then \033[6mfast blink\033[25m and not\033[m",
        .f_plain_text = "normal then fast blink and not",
        .f_html = "normal then <span class=\"ansi-fb\">fast blink </span>and not",
        .f_optimized_html = nullptr,
        .f_markdown = "normal then fast blink and not",
    },

    // proportional
    {
        .f_input = "\033[26mproportional\033[m",
        .f_plain_text = "proportional",
        .f_html = "<span class=\"ansi-p\">proportional</span>",
        .f_optimized_html = nullptr,
        .f_markdown = "proportional",
    },

    // inverse + positive
    {
        .f_input = "normal then \033[7minverse\033[27m and not\033[m",
        .f_plain_text = "normal then inverse and not",
        .f_html = "normal then <span style=\"color:#ffffff;background-color:#000000\">inverse </span>and not",
        .f_optimized_html = nullptr,
        .f_markdown = "normal then inverse and not",
    },

    // conceal + visible
    {
        .f_input = "normal then \033[8mconcealed\033[28m and not\033[m",
        .f_plain_text = "normal then  and not",
        .f_html = "normal then  and not",
        .f_optimized_html = nullptr,
        .f_markdown = "normal then  and not",
    },

    // cross out + not cross out
    {
        .f_input = "normal then \033[9mcross out\033[29m and not\033[m",
        .f_plain_text = "normal then cross out and not",
        .f_html = "normal then <span class=\"ansi-s\">cross out </span>and not",
        .f_optimized_html = "normal then <s>cross out </s>and not",
        .f_markdown = "normal then cross out and not",
    },

    // foreground black color
    {
        .f_input = "foreground \033[30mblack\033[39m color\033[m",
        .f_plain_text = "foreground black color",
        .f_html = "foreground <span style=\"color:#000000;background-color:#ffffff\">black </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground black color",
    },

    // foreground red color
    {
        .f_input = "foreground \033[31mred\033[39m color\033[m",
        .f_plain_text = "foreground red color",
        .f_html = "foreground <span style=\"color:#de382b;background-color:#ffffff\">red </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground red color",
    },

    // foreground green color
    {
        .f_input = "foreground \033[32mgreen\033[39m color\033[m",
        .f_plain_text = "foreground green color",
        .f_html = "foreground <span style=\"color:#39b54a;background-color:#ffffff\">green </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground green color",
    },

    // foreground yellow color
    {
        .f_input = "foreground \033[33myellow\033[39m color\033[m",
        .f_plain_text = "foreground yellow color",
        .f_html = "foreground <span style=\"color:#ffc706;background-color:#ffffff\">yellow </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground yellow color",
    },

    // foreground blue color
    {
        .f_input = "foreground \033[34mblue\033[39m color\033[m",
        .f_plain_text = "foreground blue color",
        .f_html = "foreground <span style=\"color:#006fb8;background-color:#ffffff\">blue </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground blue color",
    },

    // foreground magenta color
    {
        .f_input = "foreground \033[35mmagenta\033[39m color\033[m",
        .f_plain_text = "foreground magenta color",
        .f_html = "foreground <span style=\"color:#762671;background-color:#ffffff\">magenta </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground magenta color",
    },

    // foreground cyan color
    {
        .f_input = "foreground \033[36mcyan\033[39m color\033[m",
        .f_plain_text = "foreground cyan color",
        .f_html = "foreground <span style=\"color:#2cb5e9;background-color:#ffffff\">cyan </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground cyan color",
    },

    // foreground white color
    {
        .f_input = "foreground \033[37mwhite\033[39m color\033[m",
        .f_plain_text = "foreground white color",
        .f_html = "foreground <span style=\"color:#cccccc;background-color:#ffffff\">white </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground white color",
    },

    // foreground transparent
    {
        .f_input = "foreground \033[38;1mtransparent\033[39m color\033[m",
        .f_plain_text = "foreground transparent color",
        .f_html = "foreground <span style=\"opacity:0%;background-color:#ffffff\">transparent </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground transparent color",
    },

    // foreground RGB color
    {
        .f_input = "foreground \033[38;2;32;64;96mRGB\033[39m color\033[m",
        .f_plain_text = "foreground RGB color",
        .f_html = "foreground <span style=\"color:#204060;background-color:#ffffff\">RGB </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground RGB color",
    },

    // foreground CMY color
    {
        .f_input = "foreground \033[38;3;32;64;96mCMY\033[39m color\033[m",
        .f_plain_text = "foreground CMY color",
        .f_html = "foreground <span style=\"color:#dfbf9f;background-color:#ffffff\">CMY </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground CMY color",
    },

    // foreground CMYK color
    {
        .f_input = "foreground \033[38;4;32;64;96;128mCMYK\033[39m color\033[m",
        .f_plain_text = "foreground CMYK color",
        .f_html = "foreground <span style=\"color:#6f5f4f;background-color:#ffffff\">CMYK </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground CMYK color",
    },

    // foreground index color 100
    {
        .f_input = "foreground \033[38;5;100mindex color 100\033[39m color\033[m",
        .f_plain_text = "foreground index color 100 color",
        .f_html = "foreground <span style=\"color:#878700;background-color:#ffffff\">index color 100 </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground index color 100 color",
    },

    // inverse does not work on foreground transparent
    // (i.e. the background color is #ffffff when foreground is transparent)
    {
        .f_input = "foreground \033[7minverse here \033[38;1mthen transparent\033[39m then not\033[m",
        .f_plain_text = "foreground inverse here then transparent then not",
        .f_html = "foreground <span style=\"color:#ffffff;background-color:#000000\">inverse here </span><span style=\"opacity:0%;background-color:#ffffff\">then transparent </span><span style=\"color:#ffffff;background-color:#000000\">then not</span>",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground inverse here then transparent then not",
    },

    // background black color
    {
        .f_input = "background \033[40mblack\033[49m color\033[m",
        .f_plain_text = "background black color",
        .f_html = "background <span style=\"color:#000000;background-color:#000000\">black </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background black color",
    },

    // background red color
    {
        .f_input = "background \033[41mred\033[49m color\033[m",
        .f_plain_text = "background red color",
        .f_html = "background <span style=\"color:#000000;background-color:#de382b\">red </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background red color",
    },

    // background green color
    {
        .f_input = "background \033[42mgreen\033[49m color\033[m",
        .f_plain_text = "background green color",
        .f_html = "background <span style=\"color:#000000;background-color:#39b54a\">green </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background green color",
    },

    // background yellow color
    {
        .f_input = "background \033[43myellow\033[49m color\033[m",
        .f_plain_text = "background yellow color",
        .f_html = "background <span style=\"color:#000000;background-color:#ffc706\">yellow </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background yellow color",
    },

    // background blue color
    {
        .f_input = "background \033[44mblue\033[49m color\033[m",
        .f_plain_text = "background blue color",
        .f_html = "background <span style=\"color:#000000;background-color:#006fb8\">blue </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background blue color",
    },

    // background magenta color
    {
        .f_input = "background \033[45mmagenta\033[49m color\033[m",
        .f_plain_text = "background magenta color",
        .f_html = "background <span style=\"color:#000000;background-color:#762671\">magenta </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background magenta color",
    },

    // background cyan color
    {
        .f_input = "background \033[46mcyan\033[49m color\033[m",
        .f_plain_text = "background cyan color",
        .f_html = "background <span style=\"color:#000000;background-color:#2cb5e9\">cyan </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background cyan color",
    },

    // background white color
    {
        .f_input = "background \033[47mwhite\033[49m color\033[m",
        .f_plain_text = "background white color",
        .f_html = "background <span style=\"color:#000000;background-color:#cccccc\">white </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background white color",
    },

    // background RGB color
    {
        .f_input = "background \033[48;2;32;64;96mRGB\033[49m color\033[m",
        .f_plain_text = "background RGB color",
        .f_html = "background <span style=\"color:#000000;background-color:#204060\">RGB </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background RGB color",
    },

    // background CMY color
    {
        .f_input = "background \033[48;3;32;64;96mCMY\033[49m color\033[m",
        .f_plain_text = "background CMY color",
        .f_html = "background <span style=\"color:#000000;background-color:#dfbf9f\">CMY </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background CMY color",
    },

    // background CMYK color
    {
        .f_input = "background \033[48;4;32;64;96;128mCMYK\033[49m color\033[m",
        .f_plain_text = "background CMYK color",
        .f_html = "background <span style=\"color:#000000;background-color:#6f5f4f\">CMYK </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background CMYK color",
    },

    // background index color 100
    {
        .f_input = "background \033[48;5;100mindex color 100\033[49m color\033[m",
        .f_plain_text = "background index color 100 color",
        .f_html = "background <span style=\"color:#000000;background-color:#878700\">index color 100 </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background index color 100 color",
    },

    // overline
    {
        .f_input = "\033[53moverline\033[m",
        .f_plain_text = "overline",
        .f_html = "<span class=\"ansi-v\">overline</span>",
        .f_optimized_html = nullptr,
        .f_markdown = "overline",
    },

    // overline + not overline
    {
        .f_input = "normal \033[53mthen overline\033[55m and not\033[m",
        .f_plain_text = "normal then overline and not",
        .f_html = "normal <span class=\"ansi-v\">then overline </span>and not",
        .f_optimized_html = nullptr,
        .f_markdown = "normal then overline and not",
    },

    // underline with a different color
    {
        .f_input = "normal \033[4mthen underline\033[58;5;200m with color\033[59m and not\033[m",
        .f_plain_text = "normal then underline with color and not",
        .f_html = "normal <span class=\"ansi-u\">then underline </span><span class=\"ansi-u\" style=\"text-decoration-color:#ff00d7\">with color </span><span class=\"ansi-u\">and not</span>",
        .f_optimized_html = "normal <u>then underline </u><u><span style=\"text-decoration-color:#ff00d7\">with color </span></u><u>and not</u>",
        .f_markdown = "normal then underline with color and not",
    },

    // superscript
    {
        .f_input = "\033[73msuperscript\033[m",
        .f_plain_text = "superscript",
        .f_html = "<span class=\"ansi-sup\">superscript</span>",
        .f_optimized_html = "<sup>superscript</sup>",
        .f_markdown = "superscript",
    },

    // subscript
    {
        .f_input = "\033[74msubscript\033[m",
        .f_plain_text = "subscript",
        .f_html = "<span class=\"ansi-sub\">subscript</span>",
        .f_optimized_html = "<sub>subscript</sub>",
        .f_markdown = "subscript",
    },

    // superscript + not superscript
    {
        .f_input = "normal \033[73mthen superscript\033[75m and not\033[m",
        .f_plain_text = "normal then superscript and not",
        .f_html = "normal <span class=\"ansi-sup\">then superscript </span>and not",
        .f_optimized_html = "normal <sup>then superscript </sup>and not",
        .f_markdown = "normal then superscript and not",
    },

    // subscript + not subscript
    {
        .f_input = "normal \033[74mthen subscript\033[75m and not\033[m",
        .f_plain_text = "normal then subscript and not",
        .f_html = "normal <span class=\"ansi-sub\">then subscript </span>and not",
        .f_optimized_html = "normal <sub>then subscript </sub>and not",
        .f_markdown = "normal then subscript and not",
    },

    // superscript + subscript are mutually exclusive
    {
        .f_input = "normal \033[73mthen superscript \033[74mthen subscript\033[75m and then not\033[m",
        .f_plain_text = "normal then superscript then subscript and then not",
        .f_html = "normal <span class=\"ansi-sup\">then superscript </span><span class=\"ansi-sub\">then subscript </span>and then not",
        .f_optimized_html = "normal <sup>then superscript </sup><sub>then subscript </sub>and then not",
        .f_markdown = "normal then superscript then subscript and then not",
    },

    // subscript + superscript are mutually exclusive
    {
        .f_input = "normal \033[74mthen subscript \033[73mthen superscript\033[75m and then not\033[m",
        .f_plain_text = "normal then subscript then superscript and then not",
        .f_html = "normal <span class=\"ansi-sub\">then subscript </span><span class=\"ansi-sup\">then superscript </span>and then not",
        .f_optimized_html = "normal <sub>then subscript </sub><sup>then superscript </sup>and then not",
        .f_markdown = "normal then subscript then superscript and then not",
    },

    // foreground gray color
    {
        .f_input = "foreground \033[90mgray\033[39m color\033[m",
        .f_plain_text = "foreground gray color",
        .f_html = "foreground <span style=\"color:#808080;background-color:#ffffff\">gray </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground gray color",
    },

    // foreground bright red color
    {
        .f_input = "foreground \033[91mbright red\033[39m color\033[m",
        .f_plain_text = "foreground bright red color",
        .f_html = "foreground <span style=\"color:#ff0000;background-color:#ffffff\">bright red </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground bright red color",
    },

    // foreground bright green color
    {
        .f_input = "foreground \033[92mbright green\033[39m color\033[m",
        .f_plain_text = "foreground bright green color",
        .f_html = "foreground <span style=\"color:#00ff00;background-color:#ffffff\">bright green </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground bright green color",
    },

    // foreground bright yellow color
    {
        .f_input = "foreground \033[93mbright yellow\033[39m color\033[m",
        .f_plain_text = "foreground bright yellow color",
        .f_html = "foreground <span style=\"color:#ffff00;background-color:#ffffff\">bright yellow </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground bright yellow color",
    },

    // foreground bright blue color
    {
        .f_input = "foreground \033[94mbright blue\033[39m color\033[m",
        .f_plain_text = "foreground bright blue color",
        .f_html = "foreground <span style=\"color:#0000ff;background-color:#ffffff\">bright blue </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground bright blue color",
    },

    // foreground bright magenta color
    {
        .f_input = "foreground \033[95mbright magenta\033[39m color\033[m",
        .f_plain_text = "foreground bright magenta color",
        .f_html = "foreground <span style=\"color:#ff00ff;background-color:#ffffff\">bright magenta </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground bright magenta color",
    },

    // foreground bright cyan color
    {
        .f_input = "foreground \033[96mbright cyan\033[39m color\033[m",
        .f_plain_text = "foreground bright cyan color",
        .f_html = "foreground <span style=\"color:#00ffff;background-color:#ffffff\">bright cyan </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground bright cyan color",
    },

    // foreground bright white color
    {
        .f_input = "foreground \033[97mbright white\033[39m color\033[m",
        .f_plain_text = "foreground bright white color",
        .f_html = "foreground <span style=\"color:#ffffff;background-color:#ffffff\">bright white </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "foreground bright white color",
    },

    // background gray color
    {
        .f_input = "background \033[100mgray\033[49m color\033[m",
        .f_plain_text = "background gray color",
        .f_html = "background <span style=\"color:#000000;background-color:#808080\">gray </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background gray color",
    },

    // background bright red color
    {
        .f_input = "background \033[101mbright red\033[49m color\033[m",
        .f_plain_text = "background bright red color",
        .f_html = "background <span style=\"color:#000000;background-color:#ff0000\">bright red </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background bright red color",
    },

    // background bright green color
    {
        .f_input = "background \033[102mbright green\033[49m color\033[m",
        .f_plain_text = "background bright green color",
        .f_html = "background <span style=\"color:#000000;background-color:#00ff00\">bright green </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background bright green color",
    },

    // background bright yellow color
    {
        .f_input = "background \033[103mbright yellow\033[49m color\033[m",
        .f_plain_text = "background bright yellow color",
        .f_html = "background <span style=\"color:#000000;background-color:#ffff00\">bright yellow </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background bright yellow color",
    },

    // background bright blue color
    {
        .f_input = "background \033[104mbright blue\033[49m color\033[m",
        .f_plain_text = "background bright blue color",
        .f_html = "background <span style=\"color:#000000;background-color:#0000ff\">bright blue </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background bright blue color",
    },

    // background bright magenta color
    {
        .f_input = "background \033[105mbright magenta\033[49m color\033[m",
        .f_plain_text = "background bright magenta color",
        .f_html = "background <span style=\"color:#000000;background-color:#ff00ff\">bright magenta </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background bright magenta color",
    },

    // background bright cyan color
    {
        .f_input = "background \033[106mbright cyan\033[49m color\033[m",
        .f_plain_text = "background bright cyan color",
        .f_html = "background <span style=\"color:#000000;background-color:#00ffff\">bright cyan </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background bright cyan color",
    },

    // background bright white color
    {
        .f_input = "background \033[107mbright white\033[49m color\033[m",
        .f_plain_text = "background bright white color",
        .f_html = "background <span style=\"color:#000000;background-color:#ffffff\">bright white </span>color",
        .f_optimized_html = nullptr,
        .f_markdown = "background bright white color",
    },

    // HTML special characters
    {
        .f_input = "quot - \", amp - &, apos - ', lt - <, and gt - >",
        .f_plain_text = "quot - \", amp - &, apos - ', lt - <, and gt - >",
        .f_html = "quot - &quot;, amp - &amp;, apos - &apos;, lt - &lt;, and gt - &gt;",
        .f_optimized_html = nullptr,
        .f_markdown = "quot \\- \", amp \\- &, apos \\- ', lt \\- \\<, and gt \\- \\>",
    },

    // markdown special characters
    {
        .f_input = "++ * - # _ < > ` [ \\ ++",
        .f_plain_text = "++ * - # _ < > ` [ \\ ++",
        .f_html = "++ * - # _ &lt; &gt; ` [ \\ ++",
        .f_optimized_html = nullptr,
        .f_markdown = "++ \\* \\- \\# \\_ \\< \\> \\` \\[ \\\\ ++",
    },

    // multiple lines
    {
        .f_input = "this is line 1\nthen comes line 2\r\nand we got 3\rfinally line 4",
        .f_plain_text = "this is line 1\nthen comes line 2\nand we got 3\nfinally line 4",
        .f_html = "this is line 1<br/>\nthen comes line 2<br/>\nand we got 3<br/>\nfinally line 4",
        .f_optimized_html = nullptr,
        .f_markdown = "this is line 1\nthen comes line 2\nand we got 3\nfinally line 4",
    },

    // unsupported system control
    {
        .f_input = "change title \033]0;This is the title\007",
        .f_plain_text = "change title ]0;This is the title\007",
        .f_html = "change title ]0;This is the title\007",
        .f_optimized_html = nullptr,
        .f_markdown = "change title ]0;This is the title\007",
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

        for(auto const & convert : g_convert_data)
        {
            // plain text
            //
            plain_text_converter->write(convert.f_input);
            std::string result(plain_text_converter->read());
            CATCH_REQUIRE(convert.f_plain_text == result);

            // html
            //
            html_converter->write(convert.f_input);
            result = html_converter->read();
            CATCH_REQUIRE(convert.f_html == result);

            // optimized html
            //
            html_converter->set_optimize(true);
            CATCH_REQUIRE(html_converter->get_optimize());

            html_converter->write(convert.f_input);
            result = html_converter->read();
            CATCH_REQUIRE((convert.f_optimized_html == nullptr ? convert.f_html : convert.f_optimized_html) == result);

            html_converter->set_optimize(false);
            CATCH_REQUIRE_FALSE(html_converter->get_optimize());

            // markdown
            //
            markdown_converter->write(convert.f_input);
            result = markdown_converter->read();
            CATCH_REQUIRE(convert.f_markdown == result);
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
