#include <map>
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

auto parse_string() {
    return pcc::escaped(pcc::alphanumeric1(), '\\', pcc::one_of("\"n\\"));
}

auto boolean() {
    auto true_parser = pcc::constant(true, pcc::tag("true"));
    auto false_parser = pcc::constant(false, pcc::tag("false"));

    return pcc::alternative(std::move(true_parser), std::move(false_parser));
}

auto null() { return pcc::constant(nullptr, pcc::tag("null")); }

int main() { return 0; }
