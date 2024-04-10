#include <catch2/catch_all.hpp>
#include <iostream>

#include <pcc/pcc.hpp>
using namespace pcc;

SCENARIO("parse tag from string") {
    GIVEN("correct string") {
        WHEN("tag is non empty sting") {
            std::string text = "some text";
            auto some_parser = tag(text);
            auto parse_result = some_parser(text);
            REQUIRE(parse_result.is_success());
        }
    }

    GIVEN("correct string") {
        WHEN("tag is non empty sting") {
            std::wstring text = L"some text";
            auto some_parser = tag(L"some");
            auto parse_result = some_parser(text);
            REQUIRE(parse_result.is_success());
        }
    }

    GIVEN("complex string") {
        WHEN("complex parser applied") {
            struct point3d {
                int x;
                int y;
                int z;
            };

            auto point_parser = product<point3d>(
                tag("point") << symbol(' ') << arithmetic<int>(),
                symbol(' ') << arithmetic<int>(),
                symbol(' ') << arithmetic<int>());
            auto parse_result = point_parser("point 1 1 1");
            REQUIRE(parse_result.is_success());
        }
    }

    GIVEN("some string") {
        WHEN("optional parser applied") {
            std::string text = "some text";
            auto optional_parser = optional(tag(text));
            auto parse_result = optional_parser(text);
            REQUIRE(parse_result.is_success());
        }
    }

    GIVEN("some string") {
        WHEN("optional parser applied") {
            std::string text = "some text";
            auto optional_parser = many(at_least<0>(), tag(text));
            many(at_least<1>(), tag(text));
            auto parse_result = optional_parser(text);
            REQUIRE(parse_result.is_success());
        }
    }

    GIVEN("integral string") {
        WHEN("string is hex based") {
            auto itegral_cool_parser = alternative(
                integral<int>(2) >> alternative(symbol('b'), symbol('B')),
                integral<int>(16) >> alternative(symbol('h'), symbol('H')),
                integral<int>());
            auto parse_result1 = itegral_cool_parser("000718");
            REQUIRE(parse_result1.is_success());
        }
    }

    GIVEN("sum combinator") {
        WHEN("some stuff") {
            auto int_or_double =
                sum<double, std::string_view>(arithmetic<double>(), tag("lol"));
            auto d = int_or_double("3.14");
            REQUIRE(d.is_success());
            auto i = int_or_double("lol");
            REQUIRE(d.is_success());
        }
    }

    GIVEN("obj file") {
        WHEN("it is valid we can parse it") {
            auto vertex = tag("v")
                          << symbol(' ')
                          << product(arithmetic<double>() >> symbol(' '),
                                     arithmetic<double>() >> symbol(' '),
                                     arithmetic<double>() >> symbol(' '),
                                     optional(arithmetic<double>()));

            auto texture_coordinate =
                tag("vt") << symbol(' ')
                          << product(
                                 arithmetic<double>() >> symbol(' '),
                                 optional(arithmetic<double>() >> symbol(' ')),
                                 optional(arithmetic<double>() >> symbol(' ')));

            auto vertex_normal =
                tag("vn") << symbol(' ')
                          << product(arithmetic<double>() >> symbol(' '),
                                     arithmetic<double>() >> symbol(' '),
                                     arithmetic<double>() >> symbol(' '));

            auto parameter_space_vertex =
                tag("vp") << symbol(' ')
                          << product(
                                 arithmetic<double>() >> symbol(' '),
                                 optional(arithmetic<double>() >> symbol(' ')),
                                 optional(arithmetic<double>() >> symbol(' ')));

            // 1/2/3
            auto face_index = product(
                integral<int>(), optional(symbol('/') << integral<int>()),
                optional(symbol('/')
                         << optional(symbol('/')) << integral<int>()));

            struct vertex_indecies final {
                int indecies[3];
            };

            struct texture_indecies final {
                int indecies[3];
            };

            struct normal_indecies final {
                int indecies[3];
            };

            vertex_indecies v{1, 2, 3};
            std::cout << v.indecies[2] << '\n';
            /// Structure to describe face
            struct face final {
                // Possible values to construct face
                using potential =
                    std::variant<vertex_indecies,
                                 std::tuple<vertex_indecies, texture_indecies>,
                                 std::tuple<vertex_indecies, normal_indecies>,
                                 std::tuple<vertex_indecies, texture_indecies,
                                            normal_indecies>>;

                explicit face(potential pv);

                vertex_indecies vi;
                option<texture_indecies> ti{none{}}; // opt
                option<normal_indecies> ni{none{}};  // opt
            };

            // variant<tuple<FVI>, tuple<FVI, FVTI>, tuple<FVI, FVNI>,
            // tuple<FVI, FVTI, FVNI>> tuple(tuple(vi1, [vt1], [vn1]),
            // tuple(vi1, [vt1], [vn1]), tuple(vi1, [vt1], [vn1]))
            // ||
            // ||
            // face(vi, [vt], [vn])
            // auto polygonal_face_element = tag("f") << symbol(' ') <<
            //     product(face_index >> symbol(' '),
            //             face_index >> symbol(' '),
            //             face_index >> symbol(' '));
        }
    }
}
