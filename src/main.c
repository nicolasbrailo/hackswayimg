// SPDX-License-Identifier: MIT
// Program entry point.
// Copyright (C) 2020 Artem Senichev <artemsen@gmail.com>

#include "buildcfg.h"
#include "canvas.h"
#include "config.h"
#include "font.h"
#include "formats/loader.h"
#include "imagelist.h"
#include "info.h"
#include "keybind.h"
#include "sway.h"
#include "ui.h"
#include "viewer.h"

#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Command line options. */
struct cmdarg {
    const char short_opt; ///< Short option character
    const char* long_opt; ///< Long option name
    const char* format;   ///< Format description
    const char* help;     ///< Help string
    const char* section;  ///< Section name of the param
    const char* key;      ///< Key of the param
    const char* value;    ///< Static value to set
};


/**
 * Setup window position via Sway IPC.
 */
static void sway_setup(void)
{
    struct rect parent;
    bool fullscreen;
    bool absolute;
    int ipc;

    ipc = sway_connect();
    if (ipc == INVALID_SWAY_IPC) {
        return;
    }

    absolute = ui_get_x() != ((ssize_t)POS_FROM_PARENT) && ui_get_y() != ((ssize_t)POS_FROM_PARENT);

    if (sway_current(ipc, &parent, &fullscreen)) {
        if (fullscreen && !ui_get_fullscreen()) {
            // force set full screen mode if current window in it
            ui_toggle_fullscreen();
        }
        if (ui_get_fullscreen()) {
        }

        // set window position and size from the parent one
        if (!absolute) {
            ui_set_position(parent.x, parent.y);
        }
        if (ui_get_width() == SIZE_FROM_PARENT ||
            ui_get_height() == SIZE_FROM_PARENT) {
            ui_set_size(parent.width, parent.height);
        }
    }

    if (!ui_get_fullscreen()) {
        sway_add_rules(ipc, ui_get_appid(), ui_get_x(), ui_get_y(), absolute);
    }

    sway_disconnect(ipc);
}

/**
 * Application entry point.
 */
int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s path_to_config\n", argv[0]);
    }

    bool rc = false;

    setlocale(LC_ALL, "");

    keybind_init();
    font_create();
    info_create();
    image_list_init();
    canvas_init();
    ui_init();
    viewer_init();
    if (!config_init(argv[1])) {
        fprintf(stderr, "Failed to load config\n");
        goto done;
    }

    font_init();
    info_init();

    // compose file list
    if (!image_list_scan()) {
        fprintf(stderr, "No images to view, exit\n");
        goto done;
    }

    //setup window position and size
    if (!ui_get_fullscreen()) {
        sway_setup();
    }
    // fixup window size form the first image
    if (ui_get_width() == SIZE_FROM_IMAGE ||
        ui_get_height() == SIZE_FROM_IMAGE ||
        ui_get_width() == SIZE_FROM_PARENT ||
        ui_get_height() == SIZE_FROM_PARENT) {
        const struct pixmap* pm = &image_list_current().image->frames[0].pm;
        ui_set_size(pm->width, pm->height);
    }

    // run ui event loop
    rc = ui_run();

done:
    config_free();
    viewer_free();
    ui_free();
    image_list_free();
    info_free();
    font_free();
    keybind_free();

    return rc ? EXIT_SUCCESS : EXIT_FAILURE;
}
