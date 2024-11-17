// SPDX-License-Identifier: MIT
// Program entry point.
// Copyright (C) 2020 Artem Senichev <artemsen@gmail.com>

#include "application.h"
#include "buildcfg.h"
#include "config.h"
#include "imagelist.h"
#include "loader.h"
#include "ui.h"
#include "viewer.h"

#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Print usage info.
 */
static void print_help(void)
{
    puts("Usage: " APP_NAME " [OPTION]... [FILE]...");
}

/**
 * Application entry point.
 */
int main(int argc, char* argv[])
{
    bool rc;

    setlocale(LC_ALL, "");

    if (argc <= 1) {
        print_help();
        return 1;
    }

    struct config* cfg = config_load(argv[1]);
    if (!cfg) {
        print_help();
        return 1;
    }

    const char* imgsource = config_get(cfg, "general", "imgsource");
    rc = app_init(cfg, &imgsource, 1);

    if (cfg && rc) {
        config_check(cfg);
    }
    config_free(cfg);

    if (rc) {
        rc = app_run();
        app_destroy();
    }

    return rc ? EXIT_SUCCESS : EXIT_FAILURE;
}
