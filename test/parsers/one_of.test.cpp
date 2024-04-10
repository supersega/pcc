#include <catch2/catch_all.hpp>
#include <pcc/pcc.hpp>
using namespace pcc;

SCENARIO("one_of parser happy path") {
    GIVEN("string which contains a one of character only") {
        WHEN("one_of parser applied") {
            auto one_of_parser = one_of("123");
            auto parse_result = one_of_parser("1");
            REQUIRE(parse_result.is_success());
            auto [value, rest] = parse_result.value_unsafe();
            THEN("it should return the character") { REQUIRE(value == '1'); }
            THEN("it should return the rest of the string") {
                REQUIRE(rest == "");
            }
        }
    }

    GIVEN("string which contains a one of character at the beginning") {
        WHEN("one_of parser applied") {
            auto one_of_parser = one_of("123");
            auto parse_result = one_of_parser("123");
            REQUIRE(parse_result.is_success());
            auto [value, rest] = parse_result.value_unsafe();
            THEN("it should return the character") { REQUIRE(value == '1'); }
            THEN("it should return the rest of the string") {
                REQUIRE(rest == "23");
            }
        }
    }
}

SCENARIO("one_of parser error path") {
    GIVEN("string which does not contain a one of character") {
        WHEN("one_of parser applied") {
            auto one_of_parser = one_of("123");
            auto parse_result = one_of_parser("4");
            REQUIRE(!parse_result.is_success());
            THEN("it should return an error message") {
                REQUIRE(parse_result.error_unsafe() ==
                        "A character is not in the input");
            }
        }
    }

    GIVEN("empty string") {
        WHEN("one_of parser applied") {
            auto one_of_parser = one_of("123");
            auto parse_result = one_of_parser("");
            REQUIRE(!parse_result.is_success());
            THEN("it should return an error message") {
                REQUIRE(parse_result.error_unsafe() ==
                        "A character is not in the input");
            }
        }
    }
}
