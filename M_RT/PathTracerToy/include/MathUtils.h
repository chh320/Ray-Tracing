#ifndef MATHUTILS_H
#define MATHUTILS_H

#define PI 3.14159265358979323846f

#include <cmath>
#include <algorithm>
#include <Config.h>

namespace GLSLPT
{
    struct Math
    {
    public:

        static inline float Degrees(float radians) { return radians * (180.f / PI); };
        static inline float Radians(float degrees) { return degrees * (PI / 180.f); };
        static inline float Clamp(float x, float lower, float upper) { return std::min(upper, std::max(x, lower)); };
    };
}
#endif // !MATHUTILS_H
