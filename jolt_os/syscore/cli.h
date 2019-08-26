/**
 * @file cli.h
 * @brief
 */

#ifndef JOLT_SYSCORE_CLI_H__
#define JOLT_SYSCORE_CLI_H__

#include "stdint.h"

/**
 * @brief In a CLI function, return this if your function will explicitly call
 * jolt_cli_return(val) at a later time via a queued bg job.
 */
#define JOLT_CLI_NON_BLOCKING INT_MAX

typedef struct {
    char *line; /**< NULL-terminated line entered via some CLI */
    FILE *in;   /**< Where it came from */
    FILE *out;  /**< Where to print to */
    FILE *err;  /**< Where to print errors to */
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
char *jolt_cli_get_line(int16_t timeout);

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
void jolt_cli_set_src(jolt_cli_src_t *src);

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


#endif
