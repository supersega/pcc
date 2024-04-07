#include <functional>
#include <iostream>
#include <map>
#include <ostream>
#include <variant>

#include <pcc/pcc.hpp>

// Forward declate json structures
struct json_null;
struct json_boolean;
struct json_number;
struct json_string;
struct json_array;
struct json_object;

// Define json value type as a variant of possible json values
using json_value = std::variant<json_null, json_boolean, json_number,
                                json_string, json_array, json_object>;

struct json_null final {};

struct json_boolean final {
    bool value;
};

struct json_number final {
    double value;
};

struct json_string final {
    std::string value;
};

struct json_object;

struct json_array final {
    std::vector<json_value> value;
};

struct json_object final {
    std::map<std::string, json_value> value;
};

// Deine output operators for json values just ot print results
std::ostream &operator<<(std::ostream &os, const json_null &) {
    os << "null";
    return os;
}

std::ostream &operator<<(std::ostream &os, const json_boolean &b) {
    os << (b.value ? "true" : "false");
    return os;
}

std::ostream &operator<<(std::ostream &os, const json_number &n) {
    os << n.value;
    return os;
}

std::ostream &operator<<(std::ostream &os, const json_string &s) {
    os << "\"" << s.value << "\"";
    return os;
}

std::ostream &operator<<(std::ostream &os, const json_value &v);

std::ostream &operator<<(std::ostream &os, const json_array &a) {
    os << "[";
    for (size_t i = 0; i < a.value.size(); ++i) {
        os << a.value[i];
        if (i != a.value.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

std::ostream &operator<<(std::ostream &os, const json_object &o) {
    os << "{";
    size_t i = 0;
    for (const auto &[k, v] : o.value) {
        os << "\"" << k << "\": " << v;
        if (i != o.value.size() - 1) {
            os << ", ";
        }
        ++i;
    }
    os << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, const json_value &v) {
    std::visit([&os](const auto &v) { os << v; }, v);
    return os;
}

/// @brief Parse a string with escape characters
auto parse_string() {
    return pcc::escaped(pcc::alphanumeric1(), '\\', pcc::one_of("\"n\\"));
}

/// @brief Parse boolean values
auto boolean() {
    // constant is a parser that always returns the same value
    // if the parser is successful. For example, constant(true, tag("true"))
    // will return true if the input starts with "true" and false otherwise.
    auto true_parser = pcc::constant(true, pcc::tag("true"));
    auto false_parser = pcc::constant(false, pcc::tag("false"));

    // alternative is a parser that tries the first parser and if it fails
    // tries the second parser, then the third and so on. If all parsers fail
    // the result is a failure. Parsers should have the same return type.
    return pcc::alternative(std::move(true_parser), std::move(false_parser));
}

/// @brief Parse null values
auto null() { return pcc::constant(nullptr, pcc::tag("null")); }

/// @brief Parse a string
auto string() {
    // tag is a parser that returns the input if it starts with the given string
    // and fails otherwise. For example, tag("hello") will return "hello" if the
    // input starts with "hello" and fail otherwise. Then operator << is used to
    // apply next parser and return the result of the second parser. In this
    // case the second parser is parse_string. Then operator >> is used to apply
    // the third parser and return the result of the second parser. In this case
    // the third parser is pcc::tag("\""). The result of whole expression is the
    // string between quotes.
    return pcc::tag("\"") << parse_string() >> pcc::tag("\"");
}

auto json() {
    // map is a parser that applies a function to the result of the parser.
    // So if the parser is successful the function is applied to the result,
    // and transform the result to another type. For example, map([](auto n) ->
    // hex { return to_hex(n); }, pcc::arithmetic<int>()) will return a hex
    // number type if the parser is successful. Here we just transform the
    // result to json_value for each parser.
    auto null_parser =
        pcc::map([](auto) -> json_value { return json_null{}; }, null());
    auto boolean_parser = pcc::map(
        [](auto b) -> json_value { return json_boolean{b}; }, boolean());
    auto num_parser =
        pcc::map([](auto n) -> json_value { return json_number{n}; },
                 pcc::arithmetic<double>());
    auto string_parser = pcc::map(
        [](const auto &s) -> json_value {
            return json_string{std::string(s.begin(), s.end())};
        },
        string());

    // Make a parser to parse json values. This parser is recursive because
    // JSON values can be arrays or objects that contain other JSON values.
    // It takes two parsers as arguments, one for parsing arrays and one for
    // parsing objects. They are passed as arguments to the lambda because
    // they are undefined at the time of the definition of the lambda.
    auto json_value_parser = [null_parser, boolean_parser, num_parser,
                              string_parser](auto json_array_parser,
                                             auto json_object_parser) -> auto {
        // We again apply alternative to try to parse different types of JSON
        // values. We also apply spaces0 to ignore spaces between values.
        // We use lazy to allow recursive calls to the parser, otherwice we
        // will have a stack overflow. So each recursive parser should be
        // wrapped in a lambda and called with lazy.
        auto element =
            pcc::spaces0() << pcc::alternative(
                pcc::lazy([json_object_parser] {
                    return json_object_parser(json_object_parser);
                }),
                pcc::lazy([json_array_parser, json_object_parser] {
                    return json_array_parser(json_array_parser,
                                             json_object_parser);
                }),
                null_parser, boolean_parser, num_parser, string_parser) >>
            pcc::spaces0();
        return element;
    };

    // Make a parser to parse JSON arrays. This parser is recursive because
    // JSON arrays can contain other JSON values. It takes two parsers as
    // arguments, one for parsing arrays and one for parsing objects. They
    // are passed as arguments to the lambda because they are undefined at
    // the time of the definition of the lambda.
    auto json_array_parser =
        [null_parser, boolean_parser, num_parser, string_parser,
         json_value_parser](auto self, auto json_object_parser)
        -> pcc::parser<std::function<pcc::parse_result<json_value, char>(
            std::string_view)>> {
        // Element is a json value, json_array_parser and json_object_parser
        // are forwarded to the json_value_parser. We use lazy to allow
        // recursion.
        auto element = pcc::lazy([json_value_parser, self, json_object_parser] {
            return json_value_parser(self, json_object_parser);
        });

        // Elements is a list of elements separated by commas. We use
        // separated_list to parse a list of elements separated by commas.
        // It returns std::vector<T> where T is the type of the element parser.
        auto elements = pcc::separated_list(element, pcc::tag(","));

        // Array parser is a parser that parses elements between square
        // brackets.
        auto array_parser = pcc::tag("[") << elements >> pcc::tag("]");

        // We map the result of the parser to a json_array.
        return pcc::map(
            [](const auto &v) -> json_value { return json_array{v}; },
            array_parser);
    };

    // Make a parser to parse JSON objects. This parser is recursive again,
    // same statements as above are valid here.
    auto json_object_parser = [null_parser, boolean_parser, num_parser,
                               string_parser, json_array_parser,
                               json_value_parser](auto self)
        -> pcc::parser<std::function<pcc::parse_result<json_value, char>(
            std::string_view)>> {
        // JSON objects are key-value pairs. Key is a string followed by a
        // colon.
        auto key =
            pcc::spaces0() << string() >> pcc::spaces0() >> pcc::tag(":");

        // Value is a JSON value. We use lazy to allow recursion.
        auto value = pcc::lazy([json_value_parser, json_array_parser, self] {
            return json_value_parser(json_array_parser, self);
        });

        // Element is a key-value pair. We use product to make a product type
        // of applied parsers, it is a std::tuple<T1, T2> where T1 is the type
        // of the first parser and T2 is the type of the second parser.
        auto element =
            pcc::product<std::tuple<std::string, json_value>>(key, value);

        // Elements is a list of elements separated by commas.
        auto elements = pcc::separated_list(element, pcc::tag(","));

        // Object parser is a parser that parses elements between figure
        // brackets.
        auto object_parser = pcc::tag("{") << elements >> pcc::tag("}");

        // We map the result of the parser to a json_object.
        return pcc::map(
            [](const auto &v) -> json_value {
                std::map<std::string, json_value> value;
                for (const auto &[k, v] : v) {
                    value[k] = v;
                }
                return json_object{std::move(value)};
            },
            object_parser);
    };

    // The root parser is either a JSON object, a JSON array or null.
    return pcc::spaces0() << pcc::alternative(
               pcc::lazy([json_object_parser] {
                   return json_object_parser(json_object_parser);
               }),
               pcc::lazy([json_array_parser, json_object_parser] {
                   return json_array_parser(json_array_parser,
                                            json_object_parser);
               }),
               null_parser) >>
           pcc::spaces0();
}

int main() {
    // Create a JSON parser.
    auto json_parser = json();

    // JSON string to parse
    constexpr char json_str[] = R"(
    {
        "key1": "value1",
        "key2": 2,
        "key3": [ 1,  2 , 3 ] ,
        "key4": {
            "key5": "value5" ,
            "key6": 6
        }
    })";

    // Parse the JSON string
    auto result = json_parser(json_str);

    // If the parser is successful print the result, otherwise print the error.
    if (result.is_success()) {
        std::cout << result.value_unsafe().value << std::endl;
    } else {
        std::cout << "Failure: " << result.error_unsafe() << std::endl;
    }
}
