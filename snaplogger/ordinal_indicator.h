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
#pragma once

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
 * main function that returns the indicator depending on the language.
 *
 * \note
 * The ordinal indicator changes even within one language. The default can
 * be used by more languages (those that do not otherwise offer an ordinal
 * indicator).
 */

// C++
//
#include    <string>



namespace snaplogger
{



/** \brief The list of languages defining an ordinal indicator.
 *
 * Most languages do not make use of an ordinal indicator. The few defined
 * below do. The "default" language template returns an empty string.
 *
 * \todo
 * Write the necessary function(s) to add convert the conventional LANG
 * value in one of our language enumeration.
 */
enum class language_t
{
    LANGUAGE_DEFAULT,

    LANGUAGE_BASQUE,
    LANGUAGE_BOSNIAN,
    LANGUAGE_BRETON,
    LANGUAGE_CROATIAN,
    LANGUAGE_CZECH,
    LANGUAGE_DANISH,
    LANGUAGE_ENGLISH,
    LANGUAGE_ESTONIAN,
    LANGUAGE_FAROESE,
    LANGUAGE_FINNISH,
    LANGUAGE_FRANCOPROVENCAL,
    LANGUAGE_FRENCH,
    LANGUAGE_GALICIAN,
    LANGUAGE_GERMAN,
    LANGUAGE_HUNGARIAN,
    LANGUAGE_ICELANDIC,
    LANGUAGE_ITALIAN,
    LANGUAGE_LATVIAN,
    LANGUAGE_MONTENEGRIN,
    LANGUAGE_NORWEGIAN,
    LANGUAGE_OCCITAN,
    LANGUAGE_PORTUGUESE,
    LANGUAGE_RUSSIAN,
    LANGUAGE_SERBIAN,
    LANGUAGE_SLOVAK,
    LANGUAGE_SLOVENE,
    LANGUAGE_SPANISH,
    LANGUAGE_TURKISH,

    LANGUAGE_FIRST = LANGUAGE_DEFAULT,
    LANGUAGE_LAST = LANGUAGE_TURKISH,
};


/** \brief The grammatical gender of the word.
 *
 * The words in English are generally viewed as not having a grammatical
 * gender. Many other languages have words that are feminine and words
 * that are masculine. This enumeration captures that information since
 * it makes a difference in several ordinal indicators.
 */
enum class gender_t
{
    GENDER_MASCULINE,
    GENDER_FEMININE,
    GENDER_NEUTRAL,
};


/** \brief The word number.
 *
 * This enumeration is used to indicate whether there is one (singular)
 * or more (plural) items. In some languages, the ordinal indicator
 * varies depending on the number.
 */
enum class number_t
{
    NUMBER_SINGULAR,
    NUMBER_PLURAL,
};


/** \brief Various additional forms.
 *
 * Some languages also make use of multiple forms in various cases. For
 * example, the spanish language often adds a period between the number
 * and the ordinal indicator. This enumeration allows us to reach these
 * other forms.
 */
enum class secondary_form_t
{
    SECONDARY_FORM_FORMAL,
    SECONDARY_FORM_ONE,
    SECONDARY_FORM_TWO,
    SECONDARY_FORM_THREE,
    SECONDARY_FORM_FOUR,
};


class ordinal_indicator
{
public:
    void                language(language_t l);
    //void                language(std::string l) -- TODO ("LANG" var to our language)
    language_t          language() const;
    void                gender(gender_t g);
    gender_t            gender() const;
    void                number(number_t n);
    number_t            number() const;
    void                secondary_form(secondary_form_t sf);
    secondary_form_t    secondary_form() const;

    char const *        indicator(long long n) const;

private:
    language_t          f_language = language_t::LANGUAGE_DEFAULT;
    gender_t            f_gender = gender_t::GENDER_MASCULINE;
    number_t            f_number = number_t::NUMBER_SINGULAR;
    secondary_form_t    f_secondary_form = secondary_form_t::SECONDARY_FORM_FORMAL;
};


} // snaplogger namespace
// vim: ts=4 sw=4 et
