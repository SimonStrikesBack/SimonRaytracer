/**
 * @file vec2.cpp
 * @author Simon Tanev
 * @brief Implementation file for vec2.hpp, see the header file for descriptions
 */

#include "vec2.hpp"
#include "vec3.hpp"

vec2 operator*(const float a, const vec2& vector) {
    return vector * a;
}
vec2 operator/(const float a, const vec2& vector) {
    return {a / vector.x, a / vector.y};
}

std::ostream& operator<<(std::ostream& os, const vec2& vec) {
    os << vec.x << " " << vec.y;
    return os;
}

vec2::vec2(const vec3 &v) {
    x = v.x;
    y = v.y;
}
