#include <catch.hpp>

#include <pcc/pcc.hpp>
using namespace pcc;

SCENARIO("map parser happy path") {
    GIVEN("true string") {
        WHEN("map parser applied") {
            auto true_parser = map([](auto) { return true; }, tag("true"));
            auto parse_result = true_parser("true");
            REQUIRE(parse_result.is_success());
            auto [value, rest] = parse_result.value_unsafe();
            THEN("it should return the mapped value") {
                REQUIRE(value == true);
            }
            THEN("it should return the rest of the string") {
                REQUIRE(rest == "");
            }
        }
    }
}

SCENARIO("map parser error path") {
    GIVEN("false string") {
        WHEN("map parser applied") {
            auto false_parser = map([](auto) { return false; }, tag("false"));
            auto parse_result = false_parser("true");
            REQUIRE(!parse_result.is_success());
            THEN("it should return an error message") {
                REQUIRE(parse_result.error_unsafe() == "Tag not found");
            }
        }
    }

    GIVEN("empty string") {
        WHEN("map parser applied") {
            auto true_parser = map([](auto) { return true; }, tag("true"));
            auto parse_result = true_parser("");
            REQUIRE(!parse_result.is_success());
            THEN("it should return an error message") {
                REQUIRE(parse_result.error_unsafe() == "Tag not found");
            }
        }
    }
}

SCENARIO("constant parser happy path") {
    GIVEN("true string") {
        WHEN("constant parser applied") {
            auto true_parser = constant(true, tag("true"));
            auto parse_result = true_parser("true");
            REQUIRE(parse_result.is_success());
            auto [value, rest] = parse_result.value_unsafe();
            THEN("it should return the constant value") {
                REQUIRE(value == true);
            }
            THEN("it should return the rest of the string") {
                REQUIRE(rest == "");
            }
        }
    }
}

SCENARIO("constant parser error path") {
    GIVEN("false string") {
        WHEN("constant parser applied") {
            auto false_parser = constant(true, tag("true"));
            auto parse_result = false_parser("nethier");
            REQUIRE(!parse_result.is_success());
            THEN("it should return an error message") {
                REQUIRE(parse_result.error_unsafe() == "Tag not found");
            }
        }
    }
}
