#pragma once

#include "defines.h"

typedef union vec2_u {
    // If we want some other type
    f32 elements[2];
    struct {
        union {
            f32 x, r, s, u;
        };
        union {
            f32 y, g, t, v;
        };
    };
} vec2;

typedef union vec3_u {
    // If we want some other type
    f32 elements[3];
    struct {
        union {
            f32 x, r, s, u;
        };
        union {
            f32 y, g, t, v;
        };
        union {
            f32 z, b, p, w;
        };
    };
} vec3;

typedef union vec4_u {
    // SIMD stands for single instruction multiple data
    // it is a type of parallel processing with a specialized register
    // used pretty much explicitly for dealing with stuff like vectors
    // we need to load our variables into the special register
    // that has its value stored in a __m128 type
    // and call another special register with whatever operation we want to make to our data
    // after which we can take it out
    // we EXPLICITLY align the data because, since it is an assembly register
    // we need to be very particular about its memory layour, or things won't work

    // Here's an example of SIMD
    /*
    #include <xmmintrin.h>  // Header for SSE instructions

    float a[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float b[4] = {5.0f, 6.0f, 7.0f, 8.0f};
    float result[4];

    // Load the arrays into SSE registers
    __m128 vecA = _mm_loadu_ps(a);  // Load array 'a' into a 128-bit register
    __m128 vecB = _mm_loadu_ps(b);  // Load array 'b' into a 128-bit register

    // Perform the SIMD addition (4 floats added in parallel)
    __m128 vecResult = _mm_add_ps(vecA, vecB);  // Add vecA and vecB

    // Store the result back to the result array
    _mm_storeu_ps(result, vecResult);
    */
    #if defined(KUSE_SIMD)
    alignas(16) __m128 data;
    #endif
    // If we want some other type
    KALIGN(16) f32 elements[4]; // if we use SIMD I'm pretty sure we'll only use this since only this is aligned
    struct {
        union {
            f32 x, r, s;
        };
        union {
            f32 y, g, t;
        };
        union {
            f32 z, b, p;
        };
        union {
            f32 w, a, q; // we remove the uvw component since it doesn't make sense in 4D and we need the w
        };
    };
} vec4;

typedef vec4 quat; // yasss

typedef union mat4_u {
    f32 data[16];
} mat4;

typedef struct vertex_3d{
    vec3 position;
    vec3 color;
} vertex_3d;