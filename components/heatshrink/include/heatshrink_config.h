#ifndef HEATSHRINK_CONFIG_H
#define HEATSHRINK_CONFIG_H

/* Should functionality assuming dynamic allocation be used? */
#ifndef CONFIG_HEATSHRINK_DYNAMIC_ALLOC
#define CONFIG_HEATSHRINK_DYNAMIC_ALLOC 1
#endif

#if CONFIG_HEATSHRINK_DYNAMIC_ALLOC
    /* Optional replacement of malloc/free */
    #define HEATSHRINK_MALLOC(SZ) malloc(SZ)
    #define HEATSHRINK_FREE(P, SZ) free(P)
#else
    /* Required parameters for static configuration */
    #define HEATSHRINK_STATIC_INPUT_BUFFER_SIZE 32
    #define HEATSHRINK_STATIC_WINDOW_BITS 8
    #define HEATSHRINK_STATIC_LOOKAHEAD_BITS 4
#endif

/* Turn on logging for debugging. */
#define CONFIG_HEATSHRINK_DEBUGGING_LOGS 0

/* Use indexing for faster compression. (This requires additional space.) */
#define CONFIG_HEATSHRINK_USE_INDEX 1

#endif
