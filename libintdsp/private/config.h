#pragma once

#ifndef DEBUG
    #define LOGL(X) 
#else
    #ifndef LOGL
        #define LOGL(X) 
    #endif
#endif

#ifndef SPL_BITS
    #define SPL_BITS 16
#endif

#ifndef SPL_BITS_PROC
    #define SPL_BITS_PROC 32
#endif

#ifdef SPL_TYPE
    typedef spl_t SPL_TYPE
#else
    typedef spl_t int16_t
#endif

#ifndef LUT_COUNT 
    #define LUT_COUNT 256
#endif

#ifndef SIN_FUNC
    #define SIN_FUNC(X) sin(X)
#endif