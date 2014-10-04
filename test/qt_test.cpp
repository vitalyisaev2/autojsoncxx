// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <catch.hpp>

#define AUTOJSONCXX_HAS_MODERN_TYPES 1
#define AUTOJSONCXX_HAS_RVALUE 1

// Uncomment the next line if you are adventurous
// #define AUTOJSONCXX_HAS_VARIADIC_TEMPLATE 1

#ifndef AUTOJSONCXX_ROOT_DIRECTORY
#define AUTOJSONCXX_ROOT_DIRECTORY ".."
#endif

#include <autojsoncxx/qt_types.hpp>
#include "userdef_qt.hpp"

#include <fstream>
#include <sstream>

using namespace autojsoncxx;
using namespace config;
using namespace config::event;

inline bool operator==(Date d1, Date d2)
{
    return d1.year == d2.year && d1.month == d2.month && d1.day == d2.day;
}

inline bool operator!=(Date d1, Date d2)
{
    return !(d1 == d2);
}

inline std::string read_all(const char* file_name)
{
    std::ifstream input(file_name);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

inline Date create_date(int year, int month, int day)
{
    Date d;
    d.year = year;
    d.month = month;
    d.day = day;
    return d;
}

// If most of the cases fail, you probably set the work directory wrong.
// Point the work directory to the `test/` subdirectory
// or redefine the macro AUTOJSONCXX_ROOT_DIRECTORY.

TEST_CASE("Test for the constructor of generated class", "[code generator]")
{
    User user;

    REQUIRE(user.ID == 0ULL);

    // Do not use string literal because MSVC will mess up the encoding
    static const char default_nickname[] = { char(0xe2), char(0x9d), char(0xb6), char(0xe2),
                                             char(0x9d), char(0xb7), char(0xe2), char(0x9d),
                                             char(0xb8) };

    REQUIRE(user.nickname == QString::fromUtf8(default_nickname, sizeof(default_nickname)));

    REQUIRE(user.birthday == create_date(0, 0, 0));
    REQUIRE(user.dark_history.empty());
    REQUIRE(user.optional_attributes.empty());
    REQUIRE(!user.block_event);

    BlockEvent event;

    REQUIRE(event.admin_ID == 255ULL);
    REQUIRE(event.date == create_date(1970, 1, 1));
    REQUIRE(event.serial_number == 0ULL);
    REQUIRE(event.details.isEmpty());
}

TEST_CASE("Test for correct parsing", "[parsing]")
{
    SECTION("Test for an array of user", "[parsing]")
    {
        QList<User> users;
        ParsingResult err;

        bool success = from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", users, err);
        {
            CAPTURE(err.description());
            REQUIRE(success);
        }
        REQUIRE(users.size() == 2);

        {
            const User& u = users.front();
            REQUIRE(u.ID == 7947402710862746952ULL);
            REQUIRE(u.nickname == "bigger than bigger");
            REQUIRE(u.birthday == create_date(1984, 9, 2));

            REQUIRE(u.block_event.data() != 0);
            const BlockEvent& e = *u.block_event;

            REQUIRE(e.admin_ID > 0ULL);
            REQUIRE(e.date == create_date(1970, 12, 31));
            REQUIRE(e.description == "advertisement");
            REQUIRE(e.details.size() > 0);

            REQUIRE(u.dark_history.empty());
            REQUIRE(u.optional_attributes.empty());
        }

        {
            const User& u = users.back();
            REQUIRE(u.ID == 13478355757133566847ULL);
            REQUIRE(!u.block_event);
            REQUIRE(u.optional_attributes.size() == 3);
            REQUIRE(u.optional_attributes.find("Self description") != u.optional_attributes.end());
        }
    }

    SECTION("Test for a map of user", "[parsing]")
    {
        QHash<QString, User> users;
        ParsingResult err;

        bool success = from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_map.json", users, err);
        {
            CAPTURE(err.description());
            REQUIRE(success);
        }
        REQUIRE(users.size() == 2);

        {
            const User& u = users["First"];
            REQUIRE(u.ID == 7947402710862746952ULL);
            REQUIRE(u.nickname == "bigger than bigger");
            REQUIRE(u.birthday == create_date(1984, 9, 2));

            REQUIRE(u.block_event.data() != 0);
            const BlockEvent& e = *u.block_event;

            REQUIRE(e.admin_ID > 0ULL);
            REQUIRE(e.date == create_date(1970, 12, 31));
            REQUIRE(e.description == "advertisement");
            REQUIRE(e.details.size() > 0);

            REQUIRE(u.dark_history.empty());
            REQUIRE(u.optional_attributes.empty());
        }

        {
            const User& u = users["Second"];
            REQUIRE(u.ID == 13478355757133566847ULL);
            REQUIRE(u.nickname.toUtf8().size() == 15);
            REQUIRE(!u.block_event);
            REQUIRE(u.optional_attributes.size() == 3);
            REQUIRE(u.optional_attributes.find("Self description") != u.optional_attributes.end());
        }
    }
}

TEST_CASE("Test for mismatch between JSON and C++ class QList<config::User>", "[parsing], [error]")
{
    QList<User> users;
    ParsingResult err;

    SECTION("Mismatch between array and object", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/single_object.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
        REQUIRE(std::distance(err.begin(), err.end()) == 1);

        auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());

        REQUIRE(e.expected_type() == "array");

        REQUIRE(e.actual_type() == "object");
    }

    SECTION("Required field not present; test the path as well", "[parsing], [error], [missing required]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/missing_required.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(std::distance(err.begin(), err.end()) == 5);

        auto it = err.begin();
        REQUIRE(it->type() == error::MISSING_REQUIRED);

        ++it;
        REQUIRE(it->type() == error::OBJECT_MEMBER);
        REQUIRE(static_cast<const error::ObjectMemberError&>(*it).member_name() == "date");

        ++it;
        REQUIRE(it->type() == error::ARRAY_ELEMENT);
        REQUIRE(static_cast<const error::ArrayElementError&>(*it).index() == 0);

        ++it;
        REQUIRE(it->type() == error::OBJECT_MEMBER);
        REQUIRE(static_cast<const error::ObjectMemberError&>(*it).member_name() == "dark_history");
    }

    SECTION("Unknown field in strict parsed class Date", "[parsing], [error], [unknown field]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/unknown_field.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);

        REQUIRE(static_cast<const error::UnknownFieldError&>(*err.begin()).field_name() == "hour");
    }

    SECTION("Out of range", "[parsing], [error], [out of range]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/out_of_range.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::NUMBER_OUT_OF_RANGE);
    }

    SECTION("Mismatch between integer and string", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/integer_string.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
    }

    SECTION("Null character in key", "[parsing], [error], [null character]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/null_in_key.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);
    }
}

TEST_CASE("Test for mismatch between JSON and C++ class QMap<QString, config::User>", "[parsing], [error]")
{
    QMap<QString, config::User> users;
    ParsingResult err;

    SECTION("Mismatch between object and array", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);

        auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());
        REQUIRE(e.expected_type() == "object");
        REQUIRE(e.actual_type() == "array");
    }

    SECTION("Mismatch in mapped element", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/map_element_mismatch.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());
        {
            REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);

            auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());
        }
        {
            auto it = ++err.begin();
            REQUIRE(it != err.end());
            REQUIRE(it->type() == error::OBJECT_MEMBER);
            auto&& e = static_cast<const error::ObjectMemberError&>(*it);
            REQUIRE(e.member_name() == "Third");
        }
    }
}

TEST_CASE("Test for writing JSON", "[serialization]")
{
    QVector<User> users;
    ParsingResult err;

    bool success = from_json_file(AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", users, err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }
    REQUIRE(users.size() == 2);

    std::string output;
    to_json_string(output, users);

    REQUIRE(output == read_all(AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array_compact.json"));
}
