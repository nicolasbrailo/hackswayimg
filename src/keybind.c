// SPDX-License-Identifier: MIT
// Keyboard bindings.
// Copyright (C) 2023 Artem Senichev <artemsen@gmail.com>

#include "keybind.h"

#include "config.h"
#include "str.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Section names in the config file
#define CONFIG_SECTION_KEYS  "keys"
#define CONFIG_SECTION_MOUSE "mouse"

/** Action names. */
static const char* action_names[] = {
    [kb_none] = "none",
    [kb_help] = "help",
    [kb_first_file] = "first_file",
    [kb_last_file] = "last_file",
    [kb_prev_dir] = "prev_dir",
    [kb_next_dir] = "next_dir",
    [kb_prev_file] = "prev_file",
    [kb_next_file] = "next_file",
    [kb_skip_file] = "skip_file",
    [kb_prev_frame] = "prev_frame",
    [kb_next_frame] = "next_frame",
    [kb_animation] = "animation",
    [kb_slideshow] = "slideshow",
    [kb_fullscreen] = "fullscreen",
    [kb_step_left] = "step_left",
    [kb_step_right] = "step_right",
    [kb_step_up] = "step_up",
    [kb_step_down] = "step_down",
    [kb_zoom] = "zoom",
    [kb_rotate_left] = "rotate_left",
    [kb_rotate_right] = "rotate_right",
    [kb_flip_vertical] = "flip_vertical",
    [kb_flip_horizontal] = "flip_horizontal",
    [kb_reload] = "reload",
    [kb_antialiasing] = "antialiasing",
    [kb_info] = "info",
    [kb_exec] = "exec",
    [kb_exit] = "exit",
};

// Default key bindings
// clang-format off
static const struct key_binding default_bindings[] = {
    { .key = XKB_KEY_F1,    .action = kb_help },
    { .key = XKB_KEY_Home,  .action = kb_first_file },
    { .key = XKB_KEY_End,   .action = kb_last_file },
    { .key = XKB_KEY_space, .action = kb_next_file },
    { .key = XKB_KEY_SunPageDown, .action = kb_next_file },
    { .key = XKB_KEY_SunPageUp,   .action = kb_prev_file },
    { .key = XKB_KEY_c,   .action = kb_skip_file },
    { .key = XKB_KEY_d, .action = kb_next_dir },
    { .key = XKB_KEY_d, .mods = KEYMOD_SHIFT,.action = kb_prev_dir },
    { .key = XKB_KEY_o, .action = kb_next_frame },
    { .key = XKB_KEY_o, .mods = KEYMOD_SHIFT,.action = kb_prev_frame },
    { .key = XKB_KEY_s, .action = kb_animation },
    { .key = XKB_KEY_s, .mods = KEYMOD_SHIFT,.action = kb_slideshow },
    { .key = XKB_KEY_f, .action = kb_fullscreen },
    { .key = XKB_KEY_Left,  .action = kb_step_left },
    { .key = XKB_KEY_Right, .action = kb_step_right },
    { .key = XKB_KEY_Up,    .action = kb_step_up },
    { .key = XKB_KEY_Down,  .action = kb_step_down },
    { .key = XKB_KEY_equal, .action = kb_zoom, .params = "+10" },
    { .key = XKB_KEY_plus,  .action = kb_zoom, .params = "+10" },
    { .key = XKB_KEY_minus, .action = kb_zoom, .params = "-10" },
    { .key = XKB_KEY_w, .action = kb_zoom, .params = "width" },
    { .key = XKB_KEY_w, .mods = KEYMOD_SHIFT, .action = kb_zoom, .params = "height" },
    { .key = XKB_KEY_z, .action = kb_zoom, .params = "fit" },
    { .key = XKB_KEY_z, .mods = KEYMOD_SHIFT,.action = kb_zoom, .params = "fill" },
    { .key = XKB_KEY_0, .action = kb_zoom, .params = "real" },
    { .key = XKB_KEY_BackSpace, .action = kb_zoom, .params = "optimal" },
    { .key = XKB_KEY_bracketleft,  .action = kb_rotate_left },
    { .key = XKB_KEY_bracketright, .action = kb_rotate_right },
    { .key = XKB_KEY_m, .action = kb_flip_vertical },
    { .key = XKB_KEY_m, .mods = KEYMOD_SHIFT, .action = kb_flip_horizontal },
    { .key = XKB_KEY_a, .action = kb_antialiasing },
    { .key = XKB_KEY_r, .action = kb_reload },
    { .key = XKB_KEY_i, .action = kb_info },
    { .key = XKB_KEY_e, .action = kb_exec, .params = "echo \"Image: %\"" },
    { .key = XKB_KEY_Escape, .action = kb_exit },
    { .key = XKB_KEY_q,      .action = kb_exit },
    { .key = VKEY_SCROLL_LEFT,  .action = kb_step_right, .params = "5" },
    { .key = VKEY_SCROLL_RIGHT, .action = kb_step_left,  .params = "5" },
    { .key = VKEY_SCROLL_UP,    .action = kb_step_down,  .params = "5" },
    { .key = VKEY_SCROLL_DOWN,  .action = kb_step_up,    .params = "5" },
    { .key = VKEY_SCROLL_UP,   .mods = KEYMOD_CTRL, .action = kb_zoom, .params = "+10" },
    { .key = VKEY_SCROLL_DOWN, .mods = KEYMOD_CTRL, .action = kb_zoom, .params = "-10" },
    { .key = VKEY_SCROLL_UP,   .mods = KEYMOD_SHIFT, .action = kb_prev_file },
    { .key = VKEY_SCROLL_DOWN, .mods = KEYMOD_SHIFT, .action = kb_next_file },
    { .key = VKEY_SCROLL_UP,   .mods = KEYMOD_ALT, .action = kb_prev_frame },
    { .key = VKEY_SCROLL_DOWN, .mods = KEYMOD_ALT, .action = kb_next_frame },
};
// clang-format on

// Names of virtual keys
struct virtual_keys {
    xkb_keysym_t key;
    const char* name;
};
static const struct virtual_keys virtual_keys[] = {
    { VKEY_SCROLL_UP, "ScrollUp" },
    { VKEY_SCROLL_DOWN, "ScrollDown" },
    { VKEY_SCROLL_LEFT, "ScrollLeft" },
    { VKEY_SCROLL_RIGHT, "ScrollRight" },
};

struct key_binding* key_bindings;
size_t key_bindings_size;

/**
 * Set key binding.
 * @param key keyboard key
 * @param mods key modifiers (ctrl/alt/shift)
 * @param action action to set
 * @param params additional parameters (action specific)
 */
static void keybind_set(xkb_keysym_t key, uint8_t mods, enum kb_action action,
                        const char* params)
{
    char* key_name;
    struct key_binding* new_binding = NULL;

    for (size_t i = 0; i < key_bindings_size; ++i) {
        struct key_binding* binding = &key_bindings[i];
        if (binding->key == key && binding->mods == mods) {
            new_binding = binding; // replace existing
            break;
        } else if (binding->action == kb_none && !new_binding) {
            new_binding = binding; // reuse existing
        }
    }

    if (new_binding) {
        // use existing, clear previous buffers
        free(new_binding->params);
        new_binding->params = NULL;
        free(new_binding->help);
        new_binding->help = NULL;
        if (action == kb_none) {
            new_binding->action = kb_none; // mark as free
            return;
        }
    } else {
        // add new (reallocate)
        const size_t sz = (key_bindings_size + 1) * sizeof(struct key_binding);
        struct key_binding* bindings = realloc(key_bindings, sz);
        if (!bindings) {
            return;
        }
        new_binding = &bindings[key_bindings_size];
        memset(new_binding, 0, sizeof(*new_binding));
        key_bindings = bindings;
        ++key_bindings_size;
    }

    // set new parameters
    new_binding->key = key;
    new_binding->mods = mods;
    new_binding->action = action;
    if (params && *params) {
        str_dup(params, &new_binding->params);
    }

    // construct help description
    key_name = keybind_name(key, mods);
    if (key_name) {
        str_append(key_name, 0, &new_binding->help);
        str_append(": ", 2, &new_binding->help);
        str_append(action_names[action], 0, &new_binding->help);
        if (new_binding->params) {
            str_append(" ", 1, &new_binding->help);
            str_append(new_binding->params, 0, &new_binding->help);
        }
        free(key_name);
    }
}

/**
 * Get a keysym from its name.
 * @param key keyboard key
 * @param mods key modifiers (ctrl/alt/shift)
 * @return false if name is invalid
 */
static bool get_key_from_name(const char* name, xkb_keysym_t* key,
                              uint8_t* mods)
{
    struct str_slice slices[4]; // mod[alt+ctrl+shift]+key
    const size_t snum = str_split(name, '+', slices, ARRAY_SIZE(slices));
    if (snum == 0) {
        return false;
    }

    // get modifiers
    *mods = 0;
    for (size_t i = 0; i < snum - 1; ++i) {
        const char* mod_names[] = { "Ctrl", "Alt", "Shift" };
        const ssize_t index =
            str_index(mod_names, slices[i].value, slices[i].len);
        if (index < 0) {
            return false;
        }
        *mods |= 1 << index;
    }

    // get key
    *key = xkb_keysym_from_name(slices[snum - 1].value,
                                XKB_KEYSYM_CASE_INSENSITIVE);
    // check for virtual keys
    if (*key == XKB_KEY_NoSymbol) {
        for (size_t i = 0; i < ARRAY_SIZE(virtual_keys); ++i) {
            if (strcmp(slices[snum - 1].value, virtual_keys[i].name) == 0) {
                *key = virtual_keys[i].key;
                break;
            }
        }
    }
    // check for international symbols
    if (*key == XKB_KEY_NoSymbol) {
        wchar_t* wide = str_to_wide(slices[snum - 1].value, NULL);
        *key = xkb_utf32_to_keysym(wide[0]);
        free(wide);
    }

    return (*key != XKB_KEY_NoSymbol);
}

/**
 * Custom section loader, see `config_loader` for details.
 */
static enum config_status load_config(const char* key, const char* value)
{
    enum kb_action action_id = kb_none;
    size_t action_len;
    const char* params;
    xkb_keysym_t keysym;
    uint8_t mods;
    const char* action_end;
    ssize_t index;

    // get action length
    action_end = value;
    while (*action_end && !isspace(*action_end)) {
        ++action_end;
    }
    action_len = action_end - value;

    // get action id form its name
    index = str_index(action_names, value, action_len);
    if (index < 0) {
        return cfgst_invalid_value;
    }
    action_id = index;

    // get parameters
    params = action_end;
    while (*params && isspace(*params)) {
        ++params;
    }
    if (!*params) {
        params = NULL;
    }

    // convert key name to code
    if (!get_key_from_name(key, &keysym, &mods)) {
        return cfgst_invalid_key;
    }

    keybind_set(keysym, mods, action_id, params);

    return cfgst_ok;
}

void keybind_init(void)
{
    // set defaults
    for (size_t i = 0; i < ARRAY_SIZE(default_bindings); ++i) {
        const struct key_binding* kb = &default_bindings[i];
        keybind_set(kb->key, kb->mods, kb->action, kb->params);
    }

    // register configuration loader
    config_add_loader(CONFIG_SECTION_KEYS, load_config);
    config_add_loader(CONFIG_SECTION_MOUSE, load_config);
}

void keybind_free(void)
{
    for (size_t i = 0; i < key_bindings_size; ++i) {
        free(key_bindings[i].params);
        free(key_bindings[i].help);
    }
    free(key_bindings);
}

uint8_t keybind_mods(struct xkb_state* state)
{
    uint8_t mods = 0;

    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL,
                                     XKB_STATE_MODS_EFFECTIVE) > 0) {
        mods |= KEYMOD_CTRL;
    }
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT,
                                     XKB_STATE_MODS_EFFECTIVE) > 0) {
        mods |= KEYMOD_ALT;
    }
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT,
                                     XKB_STATE_MODS_EFFECTIVE) > 0) {
        mods |= KEYMOD_SHIFT;
    }

    return mods;
}

char* keybind_name(xkb_keysym_t key, uint8_t mods)
{
    char key_name[32];
    char* name = NULL;

    // skip modifiers
    switch (key) {
        case XKB_KEY_Shift_L:
        case XKB_KEY_Shift_R:
        case XKB_KEY_Control_L:
        case XKB_KEY_Control_R:
        case XKB_KEY_Meta_L:
        case XKB_KEY_Meta_R:
        case XKB_KEY_Alt_L:
        case XKB_KEY_Alt_R:
            return NULL;
    }

    if (xkb_keysym_get_name(key, key_name, sizeof(key_name)) > 0) {
        if (mods & KEYMOD_CTRL) {
            str_append("Ctrl+", 0, &name);
        }
        if (mods & KEYMOD_ALT) {
            str_append("Alt+", 0, &name);
        }
        if (mods & KEYMOD_SHIFT) {
            str_append("Shift+", 0, &name);
        }
        str_append(key_name, 0, &name);
    }

    return name;
}

const struct key_binding* keybind_get(xkb_keysym_t key, uint8_t mods)
{
    // we always use lowercase + Shift modifier
    key = xkb_keysym_to_lower(key);

    for (size_t i = 0; i < key_bindings_size; ++i) {
        struct key_binding* binding = &key_bindings[i];
        if (binding->key == key && binding->mods == mods) {
            return binding;
        }
    }

    return NULL;
}
