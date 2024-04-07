#include <functional>
#include <iostream>
#include <map>
#include <ostream>
#include <variant>

#include <pcc/pcc.hpp>

struct json_null;
struct json_boolean;
struct json_number;
struct json_string;
struct json_array;
struct json_object;

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

auto parse_string() {
    return pcc::escaped(pcc::alphanumeric1(), '\\', pcc::one_of("\"n\\"));
}

auto boolean() {
    auto true_parser = pcc::constant(true, pcc::tag("true"));
    auto false_parser = pcc::constant(false, pcc::tag("false"));

    return pcc::alternative(std::move(true_parser), std::move(false_parser));
}

auto null() { return pcc::constant(nullptr, pcc::tag("null")); }

auto string() { return pcc::tag("\"") << parse_string() >> pcc::tag("\""); }

auto json() {
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

    auto json_value_parser = [null_parser, boolean_parser, num_parser,
                              string_parser](auto json_array_parser,
                                             auto json_object_parser) -> auto {
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

    auto json_array_parser =
        [null_parser, boolean_parser, num_parser, string_parser,
         json_value_parser](auto self, auto json_object_parser)
        -> pcc::parser<std::function<pcc::parse_result<json_value, char>(
            std::string_view)>> {
        auto element = pcc::lazy([json_value_parser, self, json_object_parser] {
            return json_value_parser(self, json_object_parser);
        });

        auto elements = pcc::separated_list(element, pcc::tag(","));
        auto array_parser = pcc::tag("[") << elements >> pcc::tag("]");

        return pcc::map(
            [](const auto &v) -> json_value { return json_array{v}; },
            array_parser);
    };

    auto json_object_parser = [null_parser, boolean_parser, num_parser,
                               string_parser, json_array_parser,
                               json_value_parser](auto self)
        -> pcc::parser<std::function<pcc::parse_result<json_value, char>(
            std::string_view)>> {
        auto key =
            pcc::spaces0() << string() >> pcc::spaces0() >> pcc::tag(":");
        auto value = pcc::lazy([json_value_parser, json_array_parser, self] {
            return json_value_parser(json_array_parser, self);
        });

        auto element =
            pcc::product<std::tuple<std::string, json_value>>(key, value);
        auto elements = pcc::separated_list(element, pcc::tag(","));
        auto object_parser = pcc::tag("{") << elements >> pcc::tag("}");

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
    auto json_parser = json();
    auto json_str = R"(
    {
    "key1": "value1",
    "key2": 2,
    "key3":  [ 1,  2 , 3 ] ,
    "key4": {
        "key5": "value5" ,
        "key6": 6
    }
    })";
    auto result = json_parser(json_str);
    if (result.is_success()) {
        std::cout << result.value_unsafe().value << std::endl;
    } else {
        std::cout << "Failure: " << result.error_unsafe() << std::endl;
    }
}
