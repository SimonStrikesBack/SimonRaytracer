/**
 * @file vec3.cpp
 * @author Simon Tanev
 * @brief Implementation file for vec3.hpp, see the header file for descriptions
 */
#include "vec3.hpp"
#include "vec2.hpp"
#include "geomath.hpp"

vec3 operator*(const float a, const vec3& vector){
    return vector * a;
}

vec3 operator/(const float a, const vec3& vector) {
    return {a / vector.x, a / vector.y, a / vector.z};
}

std::ostream& operator<<(std::ostream& os, const vec3& vec) {
    os << vec.x << " " << vec.y << " " << vec.z;
    return os;
}

vec3::vec3(const vec2 &v) {
    x = v.x;
    y = v.y;
    z = 0;
}

vec3::vec3(const vec2 &v, const float z): z(z) {
    x = v.x;
    y = v.y;
}

void vec3::vec_clamp(const float low, const float high) {
    x = clamp(x, low, high);
    y = clamp(y, low, high);
    z = clamp(z, low, high);
}
