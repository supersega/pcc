#include <catch.hpp>

#include <pcc/pcc.hpp>
using namespace pcc;

SCENARIO("escaped parser test happy path") {
    GIVEN("escaped parser") {
        WHEN("escaped parser applied") {
            auto escaped_parser = escaped(alphanumeric1(), '\\', tag("some"));
            auto parse_result = escaped_parser("123");
            REQUIRE(parse_result.is_success());
            REQUIRE(parse_result.value_unsafe().value == "123");
        }
    }

    GIVEN("string with escaped symbols") {
        WHEN("escaped parser applied") {
            auto escaped_parser = escaped(alphanumeric1(), '\\', tag(";"));
            auto parse_result = escaped_parser("123\\;");
            REQUIRE(parse_result.is_success());
            REQUIRE(parse_result.value_unsafe().value == "123\\;");
        }
    }

    GIVEN("string with escaped symbols and stuff") {
        WHEN("escaped parser applied") {
            auto escaped_parser = escaped(alphanumeric1(), '\\', tag(";"));
            auto parse_result = escaped_parser("123\\;abc");
            REQUIRE(parse_result.is_success());
            REQUIRE(parse_result.value_unsafe().value == "123\\;abc");
        }
    }

    GIVEN("string with escaped symbols and stuff") {
        WHEN("escaped parser applied") {
            auto escaped_parser = escaped(alphanumeric1(), '\\', tag(";"));
            auto parse_result = escaped_parser("123\\;abc:");
            REQUIRE(parse_result.is_success());
            REQUIRE(parse_result.value_unsafe().value == "123\\;abc");
        }
    }

    GIVEN("examples from nom") {
        WHEN("escaped parser applied") {
            auto escaped_parser =
                escaped(alphanumeric1(), '\\', one_of("\"n\\"));
            auto parse_result = escaped_parser("123;");
            REQUIRE(parse_result.is_success());
            auto [value, rest] = parse_result.value_unsafe();
            THEN("value is correct") { REQUIRE(value == "123"); }
            THEN("rest is correct") { REQUIRE(rest == ";"); }
            auto parse_result2 = escaped_parser(R"(12\"34;)");
            REQUIRE(parse_result2.is_success());
            auto [value2, rest2] = parse_result2.value_unsafe();
            THEN("value is correct") { REQUIRE(value2 == R"(12\"34)"); }
            THEN("rest is correct") { REQUIRE(rest2 == ";"); }
        }
    }

    GIVEN("empty string") {
        WHEN("escaped parser applied") {
            auto escaped_parser = escaped(alphanumeric1(), '\\', tag(";"));
            auto parse_result = escaped_parser("");
            REQUIRE(parse_result.is_success());
            REQUIRE(parse_result.value_unsafe().value == "");
        }
    }
}

SCENARIO("escaped parser test non happy path") {
    GIVEN("string with non escapable symbol") {
        WHEN("escaped parser applied") {
            auto escaped_parser = escaped(alphanumeric1(), '\\', tag(";"));
            auto parse_result = escaped_parser("123\\:abc");
            REQUIRE(!parse_result.is_success());
            REQUIRE(parse_result.error_unsafe() == "Not escapable symbol");
        }
    }

    GIVEN("string with escape symbol at the end") {
        WHEN("escaped parser applied") {
            auto escaped_parser = escaped(alphanumeric1(), '\\', tag(";"));
            auto parse_result = escaped_parser("123\\");
            REQUIRE(!parse_result.is_success());
            REQUIRE(parse_result.error_unsafe() == "End with control symbol");
        }
    }

    GIVEN("string with escape symbol at the end") {
        WHEN("escaped parser applied") {
            auto escaped_parser = escaped(alphanumeric1(), '\\', tag(";"));
            auto parse_result = escaped_parser("#123");
            REQUIRE(!parse_result.is_success());
            REQUIRE(parse_result.error_unsafe() ==
                    "Non normal or control symbol");
        }
    }
}
