

#pragma once

#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif

#define CAF_ASSERT(expr) assert(expr)


