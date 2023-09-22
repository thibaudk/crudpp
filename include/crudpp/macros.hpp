#pragma once

#ifdef SERVER
#ifdef DROGON
#include <crudpp/bindigs/drogon/ctrl_macro.hpp>
#endif
#endif

#define STRINGIFY_MACRO(x) STR(x)
#define STR(x) #x
