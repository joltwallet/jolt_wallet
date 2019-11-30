/**
 * @file cli.h
 * @brief
 */

#ifndef JOLT_SYSCORE_CLI_H__
#define JOLT_SYSCORE_CLI_H__

#include "cli_helpers.h"
#include "stdint.h"

/**
 * @brief In a CLI function, return this if your function will explicitly call
 * jolt_cli_return(val) at a later time via a queued bg job.
 */
#define JOLT_CLI_NON_BLOCKING INT_MAX

extern volatile bool jolt_cli_display_prompt;

typedef struct {
    char *line;         /**< NULL-terminated line entered via some CLI */
    FILE *in;           /**< Where it came from */
    FILE *out;          /**< Where to print to */
    FILE *err;          /**< Where to print errors to */
    const char *prompt; /**< Prompt to print */
} jolt_cli_src_t;

/**
 * @param Starts up the cli-engine.
 *
 * 1. Inits the message queue.
 * 2. Inits the Job mutex.
 * 3. Starts the job-from-cli-dispatcher task.
 * 4. Registers system commands
 */
void jolt_cli_init();

/**
 * @brief Gets a NULL-terminated string from some input source.
 *
 * Internally sets stdin/stdout/stderr according to the source.
 *
 * @param timeout Maximum time in milliseconds to wait. -1 for forever.
 * @return pointer to NULL-terminated string. Caller must free. NULL on timeout.
 */
char *jolt_cli_get_line( int16_t timeout );

/**
 * @brief Adds NULL-terminated string to the cmd queue.
 *
 * Will block until the data is queued.
 *
 * NOTE: Only the pointer is added to the queue, meaning the actual string
 * *must* be in the heap. Recipient must free the allocated memory.
 *
 * @param[in] src Data to add to queue.
 */
void jolt_cli_set_src( jolt_cli_src_t *src );

/**
 * @brief Pause the task that is currently feeding data into the msg_queue
 *
 * Used if complete control over the stream is required.
 */
void jolt_cli_suspend();

/**
 * @brief Resume the paused task
 */
void jolt_cli_resume();

/**
 * @brief Unblocks the CLI and returns the passed in value.
 *
 * if val==JOLT_CLI_NON_BLOCKING, then don't unblock.
 *
 * @param[in] val Return value for the CLI cmd.
 */
void jolt_cli_return( int val );

#if UNIT_TESTING

/**
 * @brief Get the latest returned value.
 *
 * ONLY to be used in internally or in Unity unit testing.
 *
 * Blocks until a non_blocking return value is returned.
 * @return return value.
 */
int jolt_cli_get_return();

    #include "jolt_helpers.h"
    #include "mp.h"
    #include "unity.h"

    /**
     * @brief Context for unit testing command line commands.
     *
     * Creates a scope where `stdout` is redirected to static buffer `buf`.
     * Logging is also disabled within the context.
     * `stdout` is restored upon exit.
     *
     * @param[in] buf_size Size of static buffer to allocate.
     */
    #define JOLT_CLI_UNIT_TEST_CTX( buf_size )                                               \
        MPP_DECLARE( 1, char buf[buf_size] = {0} )                                           \
        MPP_DECLARE( 2, FILE *old_stdout = stdout )                                          \
        MPP_BEFORE( 3, jolt_suspend_logging() )                                              \
        MPP_AFTER( 4, jolt_resume_logging() )                                                \
        MPP_DO_WHILE( 5, false )                                                             \
        MPP_BEFORE( 6, stdout = fmemopen( buf, sizeof( buf ), "w" ); setbuf( stdout, NULL ); \
                    jolt_gui_set_stdstream( stdin, stdout, stderr ) )                        \
        MPP_AFTER( 7, stdout = old_stdout; jolt_gui_set_stdstream( NULL, NULL, NULL ) )

    /**
     * @brief Dummy context to aid debugging with JOLT_CLI_UNIT_TEST_CTX.
     *     * Doesn't have logging disabled.
     *     * Doesn't redirect stdout.
     *
     * Used for faster debugging during the creation or regression of a unit test.
     */
    #define JOLT_CLI_UNIT_TEST_DBG_CTX( buf_size ) MPP_DECLARE( 1, char buf[buf_size] = {0} )

#endif

#endif
