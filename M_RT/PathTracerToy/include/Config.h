#ifndef CONFIG_H
#define CONFIG_H

#include <GL/gl3w.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#undef min
#undef max
#endif
#endif // !CONFIG_H
