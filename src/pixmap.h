// SPDX-License-Identifier: MIT
// Pixel map.
// Copyright (C) 2024 Artem Senichev <artemsen@gmail.com>

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/** ARGB color. */
typedef uint32_t argb_t;

// shifts for each channel in argb_t
#define ARGB_A_SHIFT 24
#define ARGB_R_SHIFT 16
#define ARGB_G_SHIFT 8
#define ARGB_B_SHIFT 0

// get channel value from argb_t
#define ARGB_GET_A(c) (((c) >> ARGB_A_SHIFT) & 0xff)
#define ARGB_GET_R(c) (((c) >> ARGB_R_SHIFT) & 0xff)
#define ARGB_GET_G(c) (((c) >> ARGB_G_SHIFT) & 0xff)
#define ARGB_GET_B(c) (((c) >> ARGB_B_SHIFT) & 0xff)

// create argb_t from channel value
#define ARGB_SET_A(a) (((a)&0xff) << ARGB_A_SHIFT)
#define ARGB_SET_R(r) (((r)&0xff) << ARGB_R_SHIFT)
#define ARGB_SET_G(g) (((g)&0xff) << ARGB_G_SHIFT)
#define ARGB_SET_B(b) (((b)&0xff) << ARGB_B_SHIFT)

// convert RGBA to ARGB
#define ARGB_SET_ABGR(c) \
    ((c & 0xff00ff00) | ARGB_SET_R(ARGB_GET_B(c)) | ARGB_SET_B(ARGB_GET_R(c)))

/** Size description. */
struct size {
    size_t width;
    size_t height;
};

/** Rectangle description. */
struct rect {
    ssize_t x;
    ssize_t y;
    size_t width;
    size_t height;
};

/** Pixel map. */
struct pixmap {
    size_t width;  ///< Width (px)
    size_t height; ///< Height (px)
    argb_t* data;  ///< Pixel data
};

/**
 * Allocate/reallocate pixel map.
 * @param pm pixmap context to create
 * @param width,height pixmap size
 * @return true pixmap was allocated
 */
bool pixmap_create(struct pixmap* pm, size_t width, size_t height);

/**
 * Free pixel map created with `pixmap_create`.
 * @param pm pixmap context to free
 */
void pixmap_free(struct pixmap* pm);

/**
 * Fill pixmap with specified color.
 * @param pm pixmap context
 * @param x,y start coordinates, left top point
 * @param width,height region size
 * @param color color to set
 */
void pixmap_fill(struct pixmap* pm, size_t x, size_t y, size_t width,
                 size_t height, argb_t color);

/**
 * Fill pixmap with grid.
 * @param pm pixmap context
 * @param x,y start coordinates, left top point
 * @param width,height region size
 * @param tail_sz size of a single tail
 * @param color0 first grid color
 * @param color1 second grid color
 */
void pixmap_grid(struct pixmap* pm, size_t x, size_t y, size_t width,
                 size_t height, size_t tail_sz, argb_t color0, argb_t color1);

/**
 * Apply mask to pixmap: change color according alpha channel.
 * @param dst destination pixmap
 * @param x,y destination left top point
 * @param mask array with alpha channel mask
 * @param width,height mask size
 * @param color color to set
 */
void pixmap_apply_mask(struct pixmap* dst, size_t x, size_t y,
                       const uint8_t* mask, size_t width, size_t height,
                       argb_t color);

/**
 * Put one pixmap on another.
 * @param dst destination pixmap
 * @param x,y destination left top point
 * @param src source pixmap
 * @param width,height source pixmap area to copy
 */
void pixmap_copy(struct pixmap* dst, size_t x, size_t y,
                 const struct pixmap* src, size_t width, size_t height);

/**
 * Put one pixmap on another with alpha blending.
 * @param dst destination pixmap
 * @param x,y destination left top point
 * @param src source pixmap
 * @param width,height source pixmap area to copy
 */
void pixmap_over(struct pixmap* dst, size_t x, size_t y,
                 const struct pixmap* src, size_t width, size_t height);

/**
 * Put one scaled pixmap on another.
 * @param dst destination pixmap
 * @param dst_x,dst_y destination left top point
 * @param src source pixmap
 * @param src_x,src_y left top point of source pixmap
 * @param src_scale scale of source pixmap
 * @param alpha flag to use alpha blending
 * @param antialiasing flag to use antialiasing
 */
void pixmap_put(struct pixmap* dst, ssize_t dst_x, ssize_t dst_y,
                const struct pixmap* src, ssize_t src_x, ssize_t src_y,
                float src_scale, bool alpha, bool antialiasing);

/**
 * Flip pixel map vertically.
 * @param pm pixmap context
 */
void pixmap_flip_vertical(struct pixmap* pm);

/**
 * Flip pixel map horizontally.
 * @param pm pixmap context
 */
void pixmap_flip_horizontal(struct pixmap* pm);

/**
 * Rotate pixel map.
 * @param pm pixmap context
 * @param angle rotation angle (only 90, 180, or 270)
 */
void pixmap_rotate(struct pixmap* pm, size_t angle);
