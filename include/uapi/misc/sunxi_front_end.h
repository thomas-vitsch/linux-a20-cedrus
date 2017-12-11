/*
 * Copyright (C) 2017 Vitsch Electronics
 *
 * Thomas van Kleef <linux-dev@vitsch.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef UAPI_SUNXI_FRONT_END_H
#define UAPI_SUNXI_FRONT_END_H

#include <linux/ioctl.h>
 
#define IOCTL_CODE		'e' /* randomly chosen */
#define SFE_IOCTL_TEST 		_IO(IOCTL_CODE, 0x01)
#define SFE_IOCTL_SET_INPUT 	_IO(IOCTL_CODE, 0x02)
#define SFE_IOCTL_GET_OUTPUT 	_IO(IOCTL_CODE, 0x03)
#define SFE_IOCTL_SET_CONFIG	_IO(IOCTL_CODE, 0x04)
#define SFE_IOCTL_UPDATE_BUFFER	_IO(IOCTL_CODE, 0x05)

/* The front end is able to parse 3
 * inputs. At the output the mixer can write output to 
 * max 3 buffers (e.g. Y, U, V, data seperately). But the current 
 * implementation has its output fixed to the back end. */
#define MAX_INPUT_BUFFERS	3
#define MAX_OUTPUT_BUFFERS	3

#define Y_BUFFER_IDX		0
#define UV_BUFFER_IDX		1

#define XRGB_BUFFER_IDX		0

/* Todo check if these values are sane */
#define MIN_WIDTH		160
#define MIN_HEIGHT		160
#define MAX_WIDTH		2000
#define MAX_HEIGHT		1100

/* Configures the mixer processor 
 * Only YUV420 TILED Y and TILED UV to XRGB8888 is supported.
 * The video decoder outputs yuv420 tiled format where the tile size 
 * is 32 bytes.
 * Todo find/create a YUV420 tiled def
 */
struct sfe_config {
    uint32_t input_fmt, output_fmt;
    uint32_t in_width, in_height;
    uint32_t out_width, out_height;
};

/* Contains data about input/output buffers */
struct sfe_buffer {
    uint32_t *base;
    uint32_t size_in_bytes;
};

/* Holds input for front end
 * case of YUV420 
 *    buf_a = y_buffer.
 *    buf_b = uv_buffer.
 */
struct sfe_input_buffers {
    struct sfe_buffer buf[MAX_INPUT_BUFFERS];
};

/* Holds output from front end */
/*
struct sfe_output_buffers {
    struct sfe_buffer buf[MAX_OUTPUT_BUFFERS];
};
*/

#endif