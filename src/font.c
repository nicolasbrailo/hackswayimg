// SPDX-License-Identifier: MIT
// Text renderer.
// Copyright (C) 2022 Artem Senichev <artemsen@gmail.com>

#include "font.h"

#include "config.h"
#include "str.h"

// font realted
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define SPACE_WH_REL 2
#define GLYPH_GW_REL 4

// Defaults
#define DEFALT_FONT  "monospace"
#define DEFALT_SIZE  14
#define DEFALT_SCALE 1

/** Font context. */
struct font {
    FT_Library lib; ///< Font lib instance
    FT_Face face;   ///< Font face instance
    char* name;     ///< Font face name
    size_t size;    ///< Font size (pt)
    size_t scale;   ///< Scale factor (HiDPI)
};
static struct font ctx;

/**
 * Get path to the font file by its name.
 * @param name font name
 * @param font_file output buffer for file path
 * @param len size of buffer
 * @return false if font not found
 */
static bool search_font_file(const char* name, char* font_file, size_t len)
{
    FcConfig* fc = NULL;

    font_file[0] = 0;
    font_file[len - 1] = 0;

    if (FcInit()) {
        fc = FcInitLoadConfigAndFonts();
        if (fc) {
            FcPattern* fc_name = NULL;
            fc_name = FcNameParse((const FcChar8*)name);
            if (fc_name) {
                FcPattern* fc_font = NULL;
                FcResult result;
                FcConfigSubstitute(fc, fc_name, FcMatchPattern);
                FcDefaultSubstitute(fc_name);
                fc_font = FcFontMatch(fc, fc_name, &result);
                if (fc_font) {
                    FcChar8* path = NULL;
                    if (FcPatternGetString(fc_font, FC_FILE, 0, &path) ==
                        FcResultMatch) {
                        strncpy(font_file, (const char*)path, len - 1);
                    }
                    FcPatternDestroy(fc_font);
                }
                FcPatternDestroy(fc_name);
            }
            FcConfigDestroy(fc);
        }
        FcFini();
    }

    return *font_file;
}

/**
 * Custom section loader, see `config_loader` for details.
 */
static enum config_status load_config(const char* key, const char* value)
{
    enum config_status status = cfgst_invalid_value;

    if (strcmp(key, "name") == 0) {
        str_dup(value, &ctx.name);
        status = cfgst_ok;
    } else if (strcmp(key, "size") == 0) {
        ssize_t num;
        if (str_to_num(value, 0, &num, 0) && num > 0 && num < 1024) {
            ctx.size = num;
            status = cfgst_ok;
        }
    } else {
        status = cfgst_invalid_key;
    }

    return status;
}

void font_create(void)
{
    // set defaults
    str_dup(DEFALT_FONT, &ctx.name);
    ctx.size = DEFALT_SIZE;
    ctx.scale = DEFALT_SCALE;

    // register configuration loader
    config_add_loader(FONT_CONFIG_SECTION, load_config);
}

void font_init(void)
{
    char file[256];
    const FT_F26Dot6 size = ctx.size * ctx.scale * 64;

    if (!search_font_file(ctx.name, file, sizeof(file))) {
        return;
    }
    if (FT_Init_FreeType(&ctx.lib) != 0) {
        return;
    }
    if (FT_New_Face(ctx.lib, file, 0, &ctx.face) != 0) {
        return;
    }

    FT_Set_Char_Size(ctx.face, size, 0, 96, 0);
}

void font_free(void)
{
    if (ctx.face) {
        FT_Done_Face(ctx.face);
    }
    if (ctx.lib) {
        FT_Done_FreeType(ctx.lib);
    }
    free(ctx.name);
}

void font_set_scale(size_t scale)
{
    ctx.scale = scale;
}

bool font_render(const char* text, struct text_surface* surface)
{
    size_t x;
    wchar_t* wide;
    const wchar_t* ptr;
    const size_t space_size = ctx.face->size->metrics.y_ppem / SPACE_WH_REL;

    wide = str_to_wide(text, NULL);
    if (!wide) {
        return false;
    }

    // get total width
    x = 0;
    ptr = wide;
    while (*ptr) {
        if (*ptr == L' ') {
            x += space_size;
        } else if (FT_Load_Char(ctx.face, *ptr, FT_LOAD_RENDER) == 0) {
            x += ctx.face->glyph->advance.x >>
                6; // why 6? from freetype tutorial!
        }
        ++ptr;
    }

    // allocate surface buffer
    surface->width = x;
    surface->height = ctx.face->size->metrics.y_ppem;
    surface->data = calloc(1, surface->height * surface->width);
    if (!surface->data) {
        free(wide);
        return false;
    }

    // draw glyphs
    x = 0;
    ptr = wide;
    while (*ptr) {
        if (*ptr == L' ') {
            x += space_size;
        } else if (FT_Load_Char(ctx.face, *ptr, FT_LOAD_RENDER) == 0) {
            const FT_GlyphSlot glyph = ctx.face->glyph;
            const FT_Bitmap* bmp = &glyph->bitmap;
            size_t y = ctx.face->size->metrics.y_ppem - glyph->bitmap_top;

            // it's a hack, but idk how to up the baseline
            const size_t baseline_offset = ctx.size / 3;
            if (y > baseline_offset) {
                y -= baseline_offset;
            } else {
                y = 0;
            }

            for (size_t glyph_y = 0; glyph_y < bmp->rows; ++glyph_y) {
                uint8_t* dst;
                if (glyph_y + y >= surface->height) {
                    break; // it's a hack too =)
                }
                dst = &surface->data[(glyph_y + y) * surface->width + x +
                                     glyph->bitmap_left];
                memcpy(dst, &bmp->buffer[glyph_y * bmp->pitch], bmp->width);
            }

            x += glyph->advance.x >> 6;
        }
        ++ptr;
    }

    free(wide);

    return true;
}
