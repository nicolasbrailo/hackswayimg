// SPDX-License-Identifier: MIT
// Program configuration.
// Copyright (C) 2022 Artem Senichev <artemsen@gmail.com>

#pragma once

#include "pixmap.h"

// Name of the general configuration section
#define GENERAL_CONFIG_SECTION "general"

/** Load status. */
enum config_status {
    cfgst_ok,
    cfgst_invalid_section,
    cfgst_invalid_key,
    cfgst_invalid_value,
};

/**
 * Custom section loader.
 * @param key,value configuration parameters
 * @return load status
 */
typedef enum config_status (*config_loader)(const char* key, const char* value);

/**
 * Initialize configuration: set defaults and load from file.
 */
bool config_init(const char* path);

/**
 * Free configuration instance.
 */
void config_free(void);

/**
 * Set option.
 * @param section section name
 * @param key,value configuration parameters
 * @return load status
 */
enum config_status config_set(const char* section, const char* key,
                              const char* value);
/**
 * Execute config command to set option.
 * @param cmd command in format: "section.key=value"
 * @return false if error
 */
bool config_command(const char* cmd);

/**
 * Register custom section loader.
 * @param name statically allocated name of the section
 * @param loader section data handler
 */
void config_add_loader(const char* section, config_loader loader);

/**
 * Convert text value to boolean.
 * @param text text to convert
 * @param flag target variable
 * @return false if text has invalid format
 */
bool config_to_bool(const char* text, bool* flag);

/**
 * Convert text value to ARGB color.
 * @param text text to convert
 * @param color output variable
 * @return false if text has invalid format
 */
bool config_to_color(const char* text, argb_t* color);
