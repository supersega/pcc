#include <array>
#include <fstream>
#include <iostream>
#include <string_view>

#include <pcc/pcc.hpp>
using namespace pcc;

struct material {
    std::array<double, 3> ka;
    std::array<double, 3> kd;
    std::array<double, 3> ks;
    std::array<double, 3> ke;
    double ni;
    double d;
    std::size_t illum;
};

struct vertex final {
    double x;
    double y;
    double z;
    double w;
};

struct texture_coordinate final {
    double u;
    double v;
    double w;
};

struct normal final {
    double x;
    double y;
    double z;
};

struct face_index final {
    std::size_t v;
    option<std::size_t> t;
    option<std::size_t> n;
};

struct face final {
    enum class smooth_shading { on, off };

    std::string_view material;
    smooth_shading shading;
    std::vector<face_index> fi;
};

struct mesh final {
    std::vector<vertex> vertices;
    std::vector<texture_coordinate> texture_coordinates;
    std::vector<normal> normals;
    std::vector<face> faces;
};

struct model final {
    // std::unordered_map<std::string_view, std::shared_ptr<material>>
    // material_lib;
    std::string_view material_file_name;
    std::vector<mesh> meshes;
};

auto eat_line = [] {
    return take_while([](auto ch) { return !(ch == '\n'); }) >>=
           take(exactly<1>());
};

auto comments0 = [] {
    return many(at_least<0>(), tag("#") >>= eat_line() >>= spaces0());
};

auto material_parser = [] {
    auto name = tag("newmtl") <<= spaces1() <<=
        take_while([](unsigned char c) { return !std::isspace(c); }) >>=
        spaces0();
    auto ns = tag("Ns") <<= spaces1() <<= arithmetic<double>() >>= spaces0();
    auto ka = tag("Ka") <<= spaces1() <<= product<std::array<double, 3>>(
        arithmetic<double>(), arithmetic<double>(), arithmetic<double>()) >>=
        spaces0();
};

auto vertex_parser = [] {
    auto builder = [](auto x, auto y, auto z, auto ow) -> vertex {
        return {x, y, z, ow.unwrap_or_else([] { return 1.0; })};
    };

    return comments0() <<= tag("v") <<= spaces1() <<=
           product(builder, arithmetic<double>() >>= spaces1(),
                   arithmetic<double>() >>= spaces1(),
                   arithmetic<double>() >>= spaces1(),
                   optional(arithmetic<double>() >>= spaces1()));
};

auto texture_coordinate_parser = [] {
    auto builder = [](auto u, auto v, auto ow) -> texture_coordinate {
        return {u, v, ow.unwrap_or_else([] { return 0.0; })};
    };

    return comments0() <<= tag("vt") <<= spaces1() <<=
           product(builder, arithmetic<double>() >>= spaces1(),
                   arithmetic<double>() >>= spaces1(),
                   optional(arithmetic<double>() >>= spaces1()));
};

auto normal_parser = [] {
    return comments0() <<= tag("vn") <<= spaces1() <<=
           product<normal>(arithmetic<double>() >>= spaces1(),
                           arithmetic<double>() >>= spaces1(),
                           arithmetic<double>() >>= spaces1());
};

auto material_name_parser = [] {
    return comments0() <<= tag("usemtl") <<= spaces1() <<=
           take_while([](unsigned char c) { return !std::isspace(c); }) >>=
           spaces1();
};

auto smooth_shading_parser = [] {
    auto builder = [](auto str) {
        return str == "off" ? face::smooth_shading::off
                            : face::smooth_shading::on;
    };
    return comments0() <<= tag("s") <<= spaces1() <<=
           product(builder, take_while([](unsigned char c) {
                       return !std::isspace(c);
                   })) >>= spaces1();
};

auto face_index_parser = [] {
    return product<face_index>(
        integral<std::size_t>(),
        optional(symbol('/') <<= integral<std::size_t>()),
        optional(symbol('/') <<= optional(symbol('/')) <<=
                 integral<std::size_t>()));
};

auto face_indecies_parser = [] {
    return comments0() <<= tag("f") <<= spaces1() <<=
           many(at_least<3>(), face_index_parser() >>= spaces1());
};

auto face_parser = []() {
    auto builder = [](auto omaterial_name, auto osmooth_shading,
                      auto &&fi) -> face {
        using namespace std::literals;

        auto material_name = omaterial_name.unwrap_or_else([] { return ""sv; });
        auto smooth_shading = osmooth_shading.unwrap_or_else(
            [] { return face::smooth_shading::on; });

        return {material_name, smooth_shading, std::move(fi)};
    };

    return product(builder, optional(material_name_parser()),
                   optional(smooth_shading_parser()), face_indecies_parser());
};

auto mesh_parser = [] {
    auto vertices = many(at_least<1>(), vertex_parser());
    auto texture_coordinates = many(at_least<0>(), texture_coordinate_parser());
    auto normals = many(at_least<1>(), normal_parser());
    auto faces = many(at_least<1>(), face_parser());

    return product<mesh>(vertices, texture_coordinates, normals, faces);
};

auto material_file_name_parser = [] {
    return spaces0() <<= comments0() <<= tag("mtllib") <<= spaces1() <<=
           take_while([](unsigned char c) { return !std::isspace(c); }) >>=
           spaces1();
};

auto model_parser = [] {
    return product<model>(material_file_name_parser(),
                          many(at_least<1>(), mesh_parser()));
};

int main() {
    std::ifstream t("/Users/kapitoshka/Development/pcc/examples/House.obj");
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());

    std::cout << str << '\n';

    auto material_file_name = material_file_name_parser()(str);

    std::cout << material_file_name.value_unsafe().value << '\n';

    auto vrtx = face_parser()(
        "#comment\n\n\nusemtl Glass\ns off\nf 43//7 41//7 42//7 44//7\n");
    std::cout << vrtx.value_unsafe().value.material << '\n';
}
