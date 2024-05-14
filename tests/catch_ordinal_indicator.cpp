// Copyright (c) 2006-2023  Made to Order Software Corp.  All Rights Reserved
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


// snaplogger lib
//
#include    <snaplogger/ordinal_indicator.h>


// C lib
//
#include    <unistd.h>



CATCH_TEST_CASE("ordinal_indicator_common", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: default settings")
    {
        snaplogger::ordinal_indicator ind;

        CATCH_REQUIRE(ind.language() == snaplogger::language_t::LANGUAGE_DEFAULT);
        CATCH_REQUIRE(ind.gender() == snaplogger::gender_t::GENDER_MASCULINE);
        CATCH_REQUIRE(ind.number() == snaplogger::number_t::NUMBER_SINGULAR);
        CATCH_REQUIRE(ind.secondary_form() == snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: test all possible parameters")
    {
        snaplogger::ordinal_indicator ind;

        for(snaplogger::language_t language(snaplogger::language_t::LANGUAGE_FIRST);
            language <= snaplogger::language_t::LANGUAGE_LAST;
            language = static_cast<snaplogger::language_t>(static_cast<int>(language) + 1))
        {
            for(snaplogger::gender_t gender(snaplogger::gender_t::GENDER_MASCULINE);
                gender <= snaplogger::gender_t::GENDER_NEUTRAL;
                gender = static_cast<snaplogger::gender_t>(static_cast<int>(gender) + 1))
            {
                for(snaplogger::number_t number(snaplogger::number_t::NUMBER_SINGULAR);
                    number <= snaplogger::number_t::NUMBER_PLURAL;
                    number = static_cast<snaplogger::number_t>(static_cast<int>(number) + 1))
                {
                    for(snaplogger::secondary_form_t form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
                        form <= snaplogger::secondary_form_t::SECONDARY_FORM_THREE;
                        form = static_cast<snaplogger::secondary_form_t>(static_cast<int>(form) + 1))
                    {
                        ind.language(language);
                        ind.gender(gender);
                        ind.number(number);
                        ind.secondary_form(form);

                        CATCH_REQUIRE(ind.language() == language);
                        CATCH_REQUIRE(ind.gender() == gender);
                        CATCH_REQUIRE(ind.number() == number);
                        CATCH_REQUIRE(ind.secondary_form() == form);
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("ordinal_indicator_english", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: 1st 2nd 3rd 4th ... 20th")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_ENGLISH);

        CATCH_REQUIRE(ind.indicator(1) == std::string("st"));
        CATCH_REQUIRE(ind.indicator(2) == std::string("nd"));
        CATCH_REQUIRE(ind.indicator(3) == std::string("rd"));
        for(int n(4); n <= 20; ++n)
        {
            CATCH_REQUIRE(ind.indicator(n) == std::string("th"));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: larger numbers")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_ENGLISH);

        CATCH_REQUIRE(ind.indicator(11) == std::string("th"));
        CATCH_REQUIRE(ind.indicator(111) == std::string("th"));
        CATCH_REQUIRE(ind.indicator(37'511) == std::string("th"));

        CATCH_REQUIRE(ind.indicator(12) == std::string("th"));
        CATCH_REQUIRE(ind.indicator(112) == std::string("th"));

        CATCH_REQUIRE(ind.indicator(13) == std::string("th"));
        CATCH_REQUIRE(ind.indicator(513) == std::string("th"));
        CATCH_REQUIRE(ind.indicator(915'113) == std::string("th"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: 1st 2nd 3rd 4th with any gender & number")
    {
        for(snaplogger::gender_t gender(snaplogger::gender_t::GENDER_MASCULINE);
            gender <= snaplogger::gender_t::GENDER_NEUTRAL;
            gender = static_cast<snaplogger::gender_t>(static_cast<int>(gender) + 1))
        {
            for(snaplogger::number_t number(snaplogger::number_t::NUMBER_SINGULAR);
                number <= snaplogger::number_t::NUMBER_PLURAL;
                number = static_cast<snaplogger::number_t>(static_cast<int>(number) + 1))
            {
//std::cerr << "test with " << static_cast<int>(gender) << " & " << static_cast<int>(number) << "\n";
                snaplogger::ordinal_indicator ind;
                ind.language(snaplogger::language_t::LANGUAGE_ENGLISH);
                ind.gender(gender);
                ind.number(number);

                CATCH_REQUIRE(ind.indicator(1) == std::string("st"));
                CATCH_REQUIRE(ind.indicator(2) == std::string("nd"));
                CATCH_REQUIRE(ind.indicator(3) == std::string("rd"));
                for(int n(4); n <= 20; ++n)
                {
                    CATCH_REQUIRE(ind.indicator(n) == std::string("th"));
                }

                // and a few of the special cases
                //
                CATCH_REQUIRE(ind.indicator(11) == std::string("th"));
                CATCH_REQUIRE(ind.indicator(112) == std::string("th"));
                CATCH_REQUIRE(ind.indicator(915'113) == std::string("th"));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: no other forms")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_ENGLISH);

        for(snaplogger::gender_t gender(snaplogger::gender_t::GENDER_MASCULINE);
            gender <= snaplogger::gender_t::GENDER_NEUTRAL;
            gender = static_cast<snaplogger::gender_t>(static_cast<int>(gender) + 1))
        {
            for(snaplogger::number_t number(snaplogger::number_t::NUMBER_SINGULAR);
                number <= snaplogger::number_t::NUMBER_PLURAL;
                number = static_cast<snaplogger::number_t>(static_cast<int>(number) + 1))
            {
                for(snaplogger::secondary_form_t form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
                    form <= snaplogger::secondary_form_t::SECONDARY_FORM_THREE;
                    form = static_cast<snaplogger::secondary_form_t>(static_cast<int>(form) + 1))
                {
//std::cerr << "test with " << static_cast<int>(gender) << " & " << static_cast<int>(number) << "\n";
                    ind.gender(gender);
                    ind.number(number);
                    ind.secondary_form(form);

                    CATCH_REQUIRE(ind.indicator(1) == std::string());
                    if(form == snaplogger::secondary_form_t::SECONDARY_FORM_ONE)
                    {
                        CATCH_REQUIRE(ind.indicator(2) == std::string("d"));
                        CATCH_REQUIRE(ind.indicator(3) == std::string("d"));
                    }
                    else
                    {
                        CATCH_REQUIRE(ind.indicator(2) == std::string());
                        CATCH_REQUIRE(ind.indicator(3) == std::string());
                    }
                    for(int n(4); n <= 20; ++n)
                    {
                        CATCH_REQUIRE(ind.indicator(n) == std::string());
                    }

                    // and a few of the special cases
                    //
                    CATCH_REQUIRE(ind.indicator(11) == std::string());
                    CATCH_REQUIRE(ind.indicator(112) == std::string());
                    CATCH_REQUIRE(ind.indicator(915'113) == std::string());
                }
            }
        }
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("ordinal_indicator_french", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: 1re 1res 1er 1ers (first)")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_FRENCH);

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("er"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("ers"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("re"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("res"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: 2e 2es 2d 2ds 2de 2des 2nd 2nds 2nde 2ndes 2eme 2emes (second)")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_FRENCH);

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("e"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("es"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("e"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("es"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("\xC3\xA8me"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("\xC3\xA8mes"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("\xC3\xA8me"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("\xC3\xA8mes"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_TWO);
        CATCH_REQUIRE(ind.indicator(2) == std::string("i\xC3\xA8me"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_TWO);
        CATCH_REQUIRE(ind.indicator(2) == std::string("i\xC3\xA8mes"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_TWO);
        CATCH_REQUIRE(ind.indicator(2) == std::string("i\xC3\xA8me"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_TWO);
        CATCH_REQUIRE(ind.indicator(2) == std::string("i\xC3\xA8mes"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_THREE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("d"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_THREE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("ds"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_THREE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("de"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_THREE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("des"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FOUR);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nd"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FOUR);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nds"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FOUR);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nde"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FOUR);
        CATCH_REQUIRE(ind.indicator(2) == std::string("ndes"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: Ne Nes Neme Nemes Nieme Niemes (second)")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_FRENCH);

        for(int n(3); n < 1'000; ++n)
        {
            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("e"));

            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("es"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("e"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("es"));

            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
            CATCH_REQUIRE(ind.indicator(n) == std::string("\xC3\xA8me"));

            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
            CATCH_REQUIRE(ind.indicator(n) == std::string("\xC3\xA8mes"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
            CATCH_REQUIRE(ind.indicator(n) == std::string("\xC3\xA8me"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
            CATCH_REQUIRE(ind.indicator(n) == std::string("\xC3\xA8mes"));

            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_TWO);
            CATCH_REQUIRE(ind.indicator(n) == std::string("i\xC3\xA8me"));

            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_TWO);
            CATCH_REQUIRE(ind.indicator(n) == std::string("i\xC3\xA8mes"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_TWO);
            CATCH_REQUIRE(ind.indicator(n) == std::string("i\xC3\xA8me"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_TWO);
            CATCH_REQUIRE(ind.indicator(n) == std::string("i\xC3\xA8mes"));
        }
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("ordinal_indicator_provincial", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: 1r 1re")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_FRANCOPROVENCAL);

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("\xC3\xA9r"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("\xC3\xA9r"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("\xC3\xA9re"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("\xC3\xA9re"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: 2nd 2nda 2emo 2ema (second)")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_FRANCOPROVENCAL);

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("\xC3\xA9mo"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("\xC3\xA9mo"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("\xC3\xA9ma"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("\xC3\xA9ma"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nd"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nd"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nda"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nda"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: Nemo Nema")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_FRANCOPROVENCAL);

        for(int n(3); n < 1'000; ++n)
        {
            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("\xC3\xA9mo"));

            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("\xC3\xA9mo"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("\xC3\xA9ma"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("\xC3\xA9ma"));
        }
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("ordinal_indicator_breton", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: 1an 2l 3de 4re 5vet")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_BRETON);

        for(snaplogger::gender_t gender(snaplogger::gender_t::GENDER_MASCULINE);
            gender <= snaplogger::gender_t::GENDER_NEUTRAL;
            gender = static_cast<snaplogger::gender_t>(static_cast<int>(gender) + 1))
        {
            for(snaplogger::number_t number(snaplogger::number_t::NUMBER_SINGULAR);
                number <= snaplogger::number_t::NUMBER_PLURAL;
                number = static_cast<snaplogger::number_t>(static_cast<int>(number) + 1))
            {
                ind.gender(gender);
                ind.number(number);

                CATCH_REQUIRE(ind.indicator(1) == std::string("a\xC3\xB1"));
                CATCH_REQUIRE(ind.indicator(2) == std::string("l"));
                CATCH_REQUIRE(ind.indicator(3) == std::string("de"));
                CATCH_REQUIRE(ind.indicator(4) == std::string("re"));

                for(int n(5); n < 1'000; ++n)
                {
                    CATCH_REQUIRE(ind.indicator(n) == std::string("vet"));
                }
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("ordinal_indicator_occitan", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: occitan: 1er 1era (first)")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_OCCITAN);

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("\xC3\xA8r"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("\xC3\xA8r"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("\xC3\xA8ra"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(1) == std::string("\xC3\xA8ra"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: occitan: 2nd 2nda (second)")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_OCCITAN);

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nd"));

        ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nd"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_SINGULAR);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nda"));

        ind.gender(snaplogger::gender_t::GENDER_FEMININE);
        ind.number(snaplogger::number_t::NUMBER_PLURAL);
        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
        CATCH_REQUIRE(ind.indicator(2) == std::string("nda"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: Nen Nena")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_OCCITAN);

        for(int n(3); n < 1'000; ++n)
        {
            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("en"));

            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("en"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_SINGULAR);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("ena"));

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            ind.number(snaplogger::number_t::NUMBER_PLURAL);
            ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_FORMAL);
            CATCH_REQUIRE(ind.indicator(n) == std::string("ena"));
        }
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("ordinal_indicator_russian", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: singular plural neutral in russian")
    {
        snaplogger::ordinal_indicator ind;

        ind.language(snaplogger::language_t::LANGUAGE_RUSSIAN);

        for(snaplogger::number_t number(snaplogger::number_t::NUMBER_SINGULAR);
            number <= snaplogger::number_t::NUMBER_PLURAL;
            number = static_cast<snaplogger::number_t>(static_cast<int>(number) + 1))
        {
            ind.number(number);

            ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
            for(int n(1); n <= 10; ++n)
            {
                CATCH_REQUIRE(ind.indicator(n) == std::string("-\xD0\xB9"));
            }

            ind.gender(snaplogger::gender_t::GENDER_FEMININE);
            for(int n(1); n <= 10; ++n)
            {
                CATCH_REQUIRE(ind.indicator(n) == std::string("-\xD1\x8F"));
            }

            ind.gender(snaplogger::gender_t::GENDER_NEUTRAL);
            for(int n(1); n <= 10; ++n)
            {
                CATCH_REQUIRE(ind.indicator(n) == std::string("-\xD0\xB5"));
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("ordinal_indicator_period", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: languages using a period")
    {
        snaplogger::ordinal_indicator ind;

        snaplogger::language_t lang[] = {
            snaplogger::language_t::LANGUAGE_BASQUE,
            snaplogger::language_t::LANGUAGE_BOSNIAN,
            snaplogger::language_t::LANGUAGE_CROATIAN,
            snaplogger::language_t::LANGUAGE_CZECH,
            snaplogger::language_t::LANGUAGE_FAROESE,
            snaplogger::language_t::LANGUAGE_FINNISH,
            snaplogger::language_t::LANGUAGE_GERMAN,
            snaplogger::language_t::LANGUAGE_ICELANDIC,
            snaplogger::language_t::LANGUAGE_LATVIAN,
            snaplogger::language_t::LANGUAGE_MONTENEGRIN,
            snaplogger::language_t::LANGUAGE_NORWEGIAN,
            snaplogger::language_t::LANGUAGE_SERBIAN,
            snaplogger::language_t::LANGUAGE_SLOVAK,
            snaplogger::language_t::LANGUAGE_SLOVENE,
            snaplogger::language_t::LANGUAGE_TURKISH,
        };

        for(auto l : lang)
        {
            ind.language(l);

            for(snaplogger::number_t number(snaplogger::number_t::NUMBER_SINGULAR);
                number <= snaplogger::number_t::NUMBER_PLURAL;
                number = static_cast<snaplogger::number_t>(static_cast<int>(number) + 1))
            {
                for(snaplogger::gender_t gender(snaplogger::gender_t::GENDER_MASCULINE);
                    gender <= snaplogger::gender_t::GENDER_NEUTRAL;
                    gender = static_cast<snaplogger::gender_t>(static_cast<int>(gender) + 1))
                {
                    ind.number(number);
                    ind.gender(gender);

                    for(int n(1); n <= 1'000; ++n)
                    {
                        CATCH_REQUIRE(ind.indicator(n) == std::string("."));
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("ordinal_indicator_other_latin", "[ordinal_indicator]")
{
    CATCH_START_SECTION("ordinal_indicator: other latin languages")
    {
        snaplogger::ordinal_indicator ind;

        snaplogger::language_t lang[] = {
            snaplogger::language_t::LANGUAGE_SPANISH,
            snaplogger::language_t::LANGUAGE_PORTUGUESE,
            snaplogger::language_t::LANGUAGE_ITALIAN,
            snaplogger::language_t::LANGUAGE_GALICIAN,
        };

        for(auto l : lang)
        {
            ind.language(l);

            for(snaplogger::number_t number(snaplogger::number_t::NUMBER_SINGULAR);
                number <= snaplogger::number_t::NUMBER_PLURAL;
                number = static_cast<snaplogger::number_t>(static_cast<int>(number) + 1))
            {
                ind.number(number);

                ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
                for(int n(1); n <= 10; ++n)
                {
                    CATCH_REQUIRE(ind.indicator(n) == std::string("\xC2\xBA"));
                }

                ind.gender(snaplogger::gender_t::GENDER_FEMININE);
                for(int n(1); n <= 10; ++n)
                {
                    CATCH_REQUIRE(ind.indicator(n) == std::string("\xC2\xAA"));
                }

                ind.gender(snaplogger::gender_t::GENDER_NEUTRAL);
                for(int n(1); n <= 10; ++n)
                {
                    CATCH_REQUIRE(ind.indicator(n) == std::string());
                }
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("ordinal_indicator: spanish/portuguese secondary")
    {
        snaplogger::ordinal_indicator ind;

        ind.secondary_form(snaplogger::secondary_form_t::SECONDARY_FORM_ONE);

        snaplogger::language_t lang[] = {
            snaplogger::language_t::LANGUAGE_SPANISH,
            snaplogger::language_t::LANGUAGE_PORTUGUESE,
        };

        for(auto l : lang)
        {
            ind.language(l);

            for(snaplogger::number_t number(snaplogger::number_t::NUMBER_SINGULAR);
                number <= snaplogger::number_t::NUMBER_PLURAL;
                number = static_cast<snaplogger::number_t>(static_cast<int>(number) + 1))
            {
                ind.number(number);

                ind.gender(snaplogger::gender_t::GENDER_MASCULINE);
                for(int n(1); n <= 10; ++n)
                {
                    CATCH_REQUIRE(ind.indicator(n) == std::string(".\xC2\xBA"));
                }

                ind.gender(snaplogger::gender_t::GENDER_FEMININE);
                for(int n(1); n <= 10; ++n)
                {
                    CATCH_REQUIRE(ind.indicator(n) == std::string(".\xC2\xAA"));
                }

                ind.gender(snaplogger::gender_t::GENDER_NEUTRAL);
                for(int n(1); n <= 10; ++n)
                {
                    CATCH_REQUIRE(ind.indicator(n) == std::string());
                }
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("ordinal_indicator_error", "[ordinal_indicator][error]")
{
    CATCH_START_SECTION("ordinal_indicator: negative and zero return an empty string")
    {
        snaplogger::ordinal_indicator ind;

        for(int n(-1'000); n <= 0; ++n)
        {
            CATCH_REQUIRE(ind.indicator(n) == std::string());
        }
    }
    CATCH_END_SECTION()
}




// vim: ts=4 sw=4 et
