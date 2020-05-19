/*
 * ESP32 YModem driver
 *
 * Copyright (C) LoBo 2017
 *
 * Author: Boris Lovosevic (loboris@gmail.com)
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>
#include "jolttypes.h"

/**
 * @brief Receive a file
 * @param[in] ffd File Descriptor
 * @param[in] maxsize Maximum file size to accept.
 * @param[in, out] getname (optional) buffer to store filename. If provided and
 *                 first character is NOT '\0', ymodem will test to see if
 *                 the incoming filename matches. If it doesn't match, will
 *                 abort transfer. Must be at least JOLT_FS_MAX_FILENAME_BUF_LEN
 *                 long to store incoming filename.
 * @param[in] write_fun A writer function to handle the incoming stream.
 * @param[out] progress (optional) updated with transfer progress 0~100
 *
 * @return 0 on success.
 */
int ymodem_receive_write( void *ffd, unsigned int maxsize, char *getname, write_fun_t write_fun, int8_t *progress );

/**
 * @brief Convenience function that will write the transfer to disk
 * @param[in] ffd File Descriptor
 * @param[in] maxsize Maximum file size to accept.
 * @param[out] getname (optional) buffer to store filename.
 * @param[out] progress (optional) updated with transfer progress 0~100
 *
 * @return 0 on success.
 */
int ymodem_receive( FILE *ffd, unsigned int maxsize, char *getname, int8_t *progress );

/**
 * @brief Transfer a file out of Jolt
 * @param sendFileName advertising name of file
 * @param filesize
 * @param ffd file descriptor
 * @return 0 on success.
 */
int ymodem_transmit( char *sendFileName, unsigned int sizeFile, FILE *ffd );

#endif
