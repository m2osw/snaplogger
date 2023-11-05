// Copyright (c) 2011-2023  Made to Order Software Corp.  All Rights Reserved
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
// 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/** \file
 * \brief Functions to return a string representing the ordinal indicator.
 *
 * A few languages make use of ordinal indicators such as "st" for first
 * in English. These indicators may further have a feminin variant as
 * in French (1er and 1ere--accent missing here, to avoid UTF-8 in our
 * source code).
 *
 * The language is very important although we offer functions that return
 * the ordinal indicator in each language, you probably want to use the
 * main function that will return the indicator depending on the language.
 *
 * \note
 * The ordinal indicator changes even within one language. The default can
 * be used by more languages (those that do not otherwise offer an ordinal
 * indicator).
 *
 * \sa https://en.wikipedia.org/wiki/Ordinal_indicator
 * \sa https://fr.wikipedia.org/wiki/Indicateur_ordinal
 */

// self
//
#include    "snaplogger/ordinal_indicator.h"


// snapdev lib
//
#include    <snapdev/not_used.h>


// C++ lib
//
#include    <cstdint>
#include    <string>



namespace snaplogger
{


namespace
{



// WARNING: to simplify the usage in the find_values() functions, the flags
//          are bit numbers, not masks
//
typedef std::uint16_t           flag_t;

flag_t                          FLAG_TENTH = 0;  // use (n % 10) instead of n to do the search unless `(n % 100) E { 11, 12, 13 }` is true



struct indicator_t
{
    std::uint16_t       f_count = 0;
    std::uint16_t       f_flags = 0;
    std::uint32_t       f_languages = 0;
    std::uint32_t       f_genders = 0;
    std::uint32_t       f_numbers = 0;
    std::uint32_t       f_secondary_forms = 0;
    char const *        f_ordinal_indicator = nullptr;
};




template<typename T>
class indicator_value
{
public:
    typedef T   value_t;

    constexpr indicator_value<T>(T const v)
        : f_value(v)
    {
    }

    constexpr value_t get() const
    {
        return f_value;
    }

private:
    value_t     f_value;
};




class specific_count
    : public indicator_value<std::uint16_t>
{
public:
    constexpr specific_count()
        : indicator_value<std::uint16_t>(0)
    {
    }

    constexpr specific_count(std::uint16_t c)
        : indicator_value<std::uint16_t>(c)
    {
    }
};


class indicator_flag
    : public indicator_value<flag_t>
{
public:
    constexpr indicator_flag()
        : indicator_value<flag_t>(static_cast<flag_t>(-1))
    {
    }

    constexpr indicator_flag(flag_t f)
        : indicator_value<flag_t>(f)
    {
    }
};


class indicator_language
    : public indicator_value<language_t>
{
public:
    constexpr indicator_language()
        : indicator_value<language_t>(language_t::LANGUAGE_DEFAULT)
    {
    }

    constexpr indicator_language(language_t l)
        : indicator_value<language_t>(l)
    {
    }
};


class indicator_gender
    : public indicator_value<gender_t>
{
public:
    constexpr indicator_gender()
        : indicator_value<gender_t>(gender_t::GENDER_MASCULINE)
    {
    }

    constexpr indicator_gender(gender_t g)
        : indicator_value<gender_t>(g)
    {
    }
};


class indicator_number
    : public indicator_value<number_t>
{
public:
    constexpr indicator_number()
        : indicator_value<number_t>(number_t::NUMBER_SINGULAR)
    {
    }

    constexpr indicator_number(number_t g)
        : indicator_value<number_t>(g)
    {
    }
};


class indicator_secondary_form
    : public indicator_value<secondary_form_t>
{
public:
    constexpr indicator_secondary_form()
        : indicator_value<secondary_form_t>(secondary_form_t::SECONDARY_FORM_FORMAL)
    {
    }

    constexpr indicator_secondary_form(secondary_form_t sf)
        : indicator_value<secondary_form_t>(sf)
    {
    }
};


class ordinal_indicator_string
    : public indicator_value<char const *>
{
public:
    constexpr ordinal_indicator_string()
        : indicator_value<char const *>(nullptr)
    {
    }

    constexpr ordinal_indicator_string(char const * indicator)
        : indicator_value<char const *>(indicator)
    {
    }
};







//template<typename T, typename F, class ...ARGS>
template<typename T, typename F>
constexpr typename std::enable_if<std::is_same<T, F>::value, std::uint32_t>::type find_values(F first)
{
    std::uint32_t const value(static_cast<std::uint32_t>(first.get()));
    if(value >= 32)
    {
        return 0;
    }
    return 1 << value;      // LCOV_EXCL_LINE
}


template<typename T, typename F>
constexpr typename std::enable_if<!std::is_same<T, F>::value, std::uint32_t>::type find_values(F first)
{
    snapdev::NOT_USED(first);
    return 0;
}


template<typename T, typename F, class ...ARGS>
constexpr typename std::enable_if<std::is_same<T, F>::value, std::uint32_t>::type find_values(F first, ARGS ...args)
{
    std::uint32_t const value(static_cast<std::uint32_t>(first.get()));
    if(value == static_cast<std::uint32_t>(-1))
    {
        return find_values<T>(args...);      // LCOV_EXCL_LINE
    }
    return (1 << value) | find_values<T>(args...);
}


template<typename T, typename F, class ...ARGS>
constexpr typename std::enable_if<!std::is_same<T, F>::value, std::uint32_t>::type find_values(F first, ARGS ...args)
{
    snapdev::NOT_USED(first);
    return find_values<T>(args...);
}




template<typename T, typename F, class ...ARGS>
constexpr typename std::enable_if<std::is_same<T, F>::value, typename T::value_t>::type find_option(F first, ARGS ...args)
{
    snapdev::NOT_USED(args...);
    return first.get();
}


template<typename T, typename F, class ...ARGS>
constexpr typename std::enable_if<!std::is_same<T, F>::value, typename T::value_t>::type find_option(F first, ARGS ...args)
{
    snapdev::NOT_USED(first);
    return find_option<T>(args...);
}



template<class ...ARGS>
constexpr indicator_t define_indicator(ARGS ...args)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    indicator_t indicator =
    {
        .f_count =              find_option<specific_count>(args..., specific_count()),   // 0 = any
        .f_flags =              static_cast<flag_t>(find_values<indicator_flag>(args..., indicator_flag())),
        .f_languages =          find_values<indicator_language>(args...),
        .f_genders =            find_values<indicator_gender>(args...),
        .f_numbers =            find_values<indicator_number>(args...),
        .f_secondary_forms =    find_values<indicator_secondary_form>(args...),
        .f_ordinal_indicator =  find_option<ordinal_indicator_string>(args...),
    };
#pragma GCC diagnostic pop

    return indicator;
}



struct indicator_t g_indicators[]
{
    // Breton
    //
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_BRETON)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("a\xC3\xB1")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_BRETON)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("l")
    ),
    define_indicator(
          specific_count(3)
        , indicator_language(language_t::LANGUAGE_BRETON)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("de")
    ),
    define_indicator(
          specific_count(4)
        , indicator_language(language_t::LANGUAGE_BRETON)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("re")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_BRETON)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("vet")
    ),

    // English
    //
    define_indicator(
          specific_count(1)
        , indicator_flag(FLAG_TENTH)
        , indicator_language(language_t::LANGUAGE_ENGLISH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("st")
    ),
    define_indicator(
          specific_count(2)
        , indicator_flag(FLAG_TENTH)
        , indicator_language(language_t::LANGUAGE_ENGLISH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("nd")
    ),
    define_indicator(
          specific_count(2)
        , indicator_flag(FLAG_TENTH)
        , indicator_language(language_t::LANGUAGE_ENGLISH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_ONE)
        , ordinal_indicator_string("d")
    ),
    define_indicator(
          specific_count(3)
        , indicator_flag(FLAG_TENTH)
        , indicator_language(language_t::LANGUAGE_ENGLISH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("rd")
    ),
    define_indicator(
          specific_count(3)
        , indicator_flag(FLAG_TENTH)
        , indicator_language(language_t::LANGUAGE_ENGLISH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_ONE)
        , ordinal_indicator_string("d")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_ENGLISH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("th")
    ),

    // French
    //
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("er")
    ),
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("re")
    ),
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("ers")
    ),
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("res")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_THREE)
        , ordinal_indicator_string("d")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_THREE)
        , ordinal_indicator_string("de")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_THREE)
        , ordinal_indicator_string("ds")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_THREE)
        , ordinal_indicator_string("des")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FOUR)
        , ordinal_indicator_string("nd")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FOUR)
        , ordinal_indicator_string("nde")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FOUR)
        , ordinal_indicator_string("nds")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FOUR)
        , ordinal_indicator_string("ndes")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("e")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("es")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_ONE)
        , ordinal_indicator_string("\xC3\xA8me")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_ONE)
        , ordinal_indicator_string("\xC3\xA8mes")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_TWO)
        , ordinal_indicator_string("i\xC3\xA8me")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_FRENCH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_TWO)
        , ordinal_indicator_string("i\xC3\xA8mes")
    ),

    // Francoprovincal
    //
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_FRANCOPROVENCAL)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("\xC3\xA9r")
    ),
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_FRANCOPROVENCAL)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("\xC3\xA9re")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRANCOPROVENCAL)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_ONE)
        , ordinal_indicator_string("nd")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_FRANCOPROVENCAL)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_ONE)
        , ordinal_indicator_string("nda")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_FRANCOPROVENCAL)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("\xC3\xA9mo")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_FRANCOPROVENCAL)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("\xC3\xA9ma")
    ),

    // Occitan
    //
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_OCCITAN)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("\xC3\xA8r")
    ),
    define_indicator(
          specific_count(1)
        , indicator_language(language_t::LANGUAGE_OCCITAN)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("\xC3\xA8ra")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_OCCITAN)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("nd")
    ),
    define_indicator(
          specific_count(2)
        , indicator_language(language_t::LANGUAGE_OCCITAN)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("nda")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_OCCITAN)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("en")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_OCCITAN)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("ena")
    ),

    // Russian
    //
    define_indicator(
          indicator_language(language_t::LANGUAGE_RUSSIAN)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("-\xD0\xB9")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_RUSSIAN)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("-\xD1\x8F")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_RUSSIAN)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("-\xD0\xB5")
    ),

    // Other Latin languages use pretty much the same notation
    //
    define_indicator(
          indicator_language(language_t::LANGUAGE_SPANISH)
        , indicator_language(language_t::LANGUAGE_PORTUGUESE)
        , indicator_language(language_t::LANGUAGE_ITALIAN)
        , indicator_language(language_t::LANGUAGE_GALICIAN)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("\xC2\xBA")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_SPANISH)
        , indicator_language(language_t::LANGUAGE_PORTUGUESE)
        , indicator_language(language_t::LANGUAGE_ITALIAN)
        , indicator_language(language_t::LANGUAGE_GALICIAN)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string("\xC2\xAA")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_SPANISH)
        , indicator_language(language_t::LANGUAGE_PORTUGUESE)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_ONE)
        , ordinal_indicator_string(".\xC2\xBA")
    ),
    define_indicator(
          indicator_language(language_t::LANGUAGE_SPANISH)
        , indicator_language(language_t::LANGUAGE_PORTUGUESE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_ONE)
        , ordinal_indicator_string(".\xC2\xAA")
    ),

    // Period Indicator
    //
    define_indicator(
          indicator_language(language_t::LANGUAGE_BASQUE)
        , indicator_language(language_t::LANGUAGE_BOSNIAN)
        , indicator_language(language_t::LANGUAGE_CROATIAN)
        , indicator_language(language_t::LANGUAGE_CZECH)
        , indicator_language(language_t::LANGUAGE_DANISH)
        , indicator_language(language_t::LANGUAGE_FAROESE)
        , indicator_language(language_t::LANGUAGE_FINNISH)
        , indicator_language(language_t::LANGUAGE_GERMAN)
        , indicator_language(language_t::LANGUAGE_ICELANDIC)
        , indicator_language(language_t::LANGUAGE_LATVIAN)
        , indicator_language(language_t::LANGUAGE_MONTENEGRIN)
        , indicator_language(language_t::LANGUAGE_NORWEGIAN)
        , indicator_language(language_t::LANGUAGE_SERBIAN)      // <- TODO: this is only for Arabic numbers
        , indicator_language(language_t::LANGUAGE_SLOVAK)
        , indicator_language(language_t::LANGUAGE_SLOVENE)
        , indicator_language(language_t::LANGUAGE_TURKISH)
        , indicator_gender(gender_t::GENDER_MASCULINE)
        , indicator_gender(gender_t::GENDER_FEMININE)
        , indicator_gender(gender_t::GENDER_NEUTRAL)
        , indicator_number(number_t::NUMBER_SINGULAR)
        , indicator_number(number_t::NUMBER_PLURAL)
        , indicator_secondary_form(secondary_form_t::SECONDARY_FORM_FORMAL)
        , ordinal_indicator_string(".")
    ),
};

}



/** \brief Define the language.
 *
 * The language to use to generate the indicated.
 *
 * By default is LANGUAGE_DEFAULT is used. This means nothing happens
 * (i.e. the indicator() function will always return an empty string).
 *
 * \param[in] l  The language to use to generate the indicator.
 *
 * \sa language()
 */
void ordinal_indicator::language(language_t l)
{
    f_language = l;
}


/** \brief Retrieve the current language of the ordinal indicator.
 *
 * This function returns the language definition of the ordinal indicator
 * object.
 *
 * \return The current language of the ordinal indicator.
 *
 * \sa language(language_t l)
 */
language_t ordinal_indicator::language() const
{
    return f_language;
}


/** \brief Change the grammatical gender.
 *
 * The ordinal indicator often depends on the grammatical gender in the
 * sentence including the number.
 *
 * \param[in] g  The new gender.
 *
 * \sa gender()
 */
void ordinal_indicator::gender(gender_t g)
{
    f_gender = g;
}


/** \brief Retrieve the grammatical gender.
 *
 * This function retrieves the current grammatical gender.
 *
 * \return The current grammatical gender.
 *
 * \sa gender(gender_t g)
 */
gender_t ordinal_indicator::gender() const
{
    return f_gender;
}


/** \brief The number of the indicator.
 *
 * The number defines whether the singular or the plural version is to be
 * retrieved. For some languages (English) this makes no difference. For
 * others (French) it will force the plural version (i.e. append an 's'
 * to the French ordinal).
 *
 * By default, the number is set to singular.
 *
 * \param[in] n  The new number of the indicator.
 *
 * \sa number()
 */
void ordinal_indicator::number(number_t n)
{
    f_number = n;
}


/** \brief Retrieve the current indicator number.
 *
 * This function returns whether the singular or plural version of the
 * ordinal indicator is to be returned.
 *
 * \return The current number of the indicator.
 *
 * \sa number(number_t n)
 */
number_t ordinal_indicator::number() const
{
    return f_number;
}


/** \brief Select a secondary form.
 *
 * By default, the formal form is returned by the indicator() function.
 * Some languages offer multiple ordinals for various numbers.
 *
 * For example, the Spanish form just uses an ordinal mark (a small 'o'
 * with an underscore) by default, but it is possible to add a period
 * just before that character. In French, the number 2 can be folloed
 * by an 'e' (deuxieme) or 'nd' (second). Etc.
 *
 * \param[in] sf  The new secondary form to use with indicator().
 *
 * \sa secondary_form()
 */
void ordinal_indicator::secondary_form(secondary_form_t sf)
{
    f_secondary_form = sf;
}


/** \brief Retrieve the secondary form.
 *
 * This function returns the current secondary form.
 *
 * \return The current secondary form.
 *
 * \sa secondary_form(secondary_form_t sf)
 */
secondary_form_t ordinal_indicator::secondary_form() const
{
    return f_secondary_form;
}


/** \brief Determine the ordinal indicator for number \p n.
 *
 * This function returns the ordinal indicator for the input number \p n
 * as per all the specified parameters such as the language and
 * grammatical gender.
 *
 * This function can be called any number of times. Parameters can be changed
 * between calls and it will still work.
 *
 * \param[in] n  The number to be given an ordinal indicator.
 *
 * \return The ordinal indicator string or "" if no indicator is available.
 */
char const * ordinal_indicator::indicator(long long n) const
{
    if(n < 1)
    {
        return "";
    }

    int const tenth(n % 10);
    int const hundredth(n % 100);

    constexpr std::size_t const max(sizeof(g_indicators) / sizeof(g_indicators[0]));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        // language
        //
        if((g_indicators[idx].f_languages & (1 << static_cast<std::uint32_t>(f_language))) == 0)
        {
            continue;
        }

        // gender (masculine, feminine, neutral)
        //
        if((g_indicators[idx].f_genders & (1 << static_cast<std::uint32_t>(f_gender))) == 0)
        {
            continue;
        }

        // number (plurial, singular)
        //
        if((g_indicators[idx].f_numbers & (1 << static_cast<std::uint32_t>(f_number))) == 0)
        {
            continue;
        }

        // another form than the formal form
        //
        if((g_indicators[idx].f_secondary_forms & (1 << static_cast<std::uint32_t>(f_secondary_form))) == 0)
        {
            continue;
        }

        if(g_indicators[idx].f_count != 0
        && g_indicators[idx].f_count != n
        && ((g_indicators[idx].f_flags & (1 << FLAG_TENTH)) == 0
                    || g_indicators[idx].f_count != tenth
                    || hundredth == 11
                    || hundredth == 12
                    || hundredth == 13))
        {
            continue;
        }

        return g_indicators[idx].f_ordinal_indicator;
    }

    return "";
}






} // snaplogger namespace
// vim: ts=4 sw=4 et
