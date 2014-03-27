/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2008 James Bursa <james@semichrome.net>
 */

#ifndef SVGTINY_H
#define SVGTINY_H

typedef int svgtiny_colour;
#define svgtiny_TRANSPARENT 0x1000000
#ifdef __riscos__
#define svgtiny_RGB(r, g, b) ((b) << 16 | (g) << 8 | (r))
#define svgtiny_RED(c) ((c) & 0xff)
#define svgtiny_GREEN(c) (((c) >> 8) & 0xff)
#define svgtiny_BLUE(c) (((c) >> 16) & 0xff)
#else
#define svgtiny_RGB(r, g, b) ((r) << 16 | (g) << 8 | (b))
#define svgtiny_RED(c) (((c) >> 16) & 0xff)
#define svgtiny_GREEN(c) (((c) >> 8) & 0xff)
#define svgtiny_BLUE(c) ((c) & 0xff)
#endif

#include <dom/dom.h>

typedef enum {
    svgtiny_TextAlignmentLeft = 0,
    svgtiny_TextAlignmentCenter,
    svgtiny_TextAlignmentRight
} svgtiny_TextAlignment;

typedef enum {
    svgtiny_ShapeTypePath = 0,
    svgtiny_ShapeTypeText,
    svgtiny_ShapeTypeTextArea,
    svgtiny_ShapeTypeUnused
} svgtiny_ShapeType;

struct svgtiny_shape {
    union {
        struct {
            float *path;
            unsigned int length;
        } path;
        struct {
            char *text;
            float x, y;
            float transform_matrix[6];
            char *font_family;
            char *font_style;
            char *font_variant;
            unsigned int font_weight;
            float font_size;
            svgtiny_TextAlignment text_alignment;
        } text;
    };
    svgtiny_ShapeType type;
	svgtiny_colour fill;
	svgtiny_colour stroke;
	int stroke_width;
};

struct svgtiny_diagram {
	int width, height;

	struct svgtiny_shape *shape;
	unsigned int shape_count;

	unsigned short error_line;
	const char *error_message;
};

typedef enum {
	svgtiny_OK,
	svgtiny_OUT_OF_MEMORY,
	svgtiny_LIBDOM_ERROR,
	svgtiny_NOT_SVG,
	svgtiny_SVG_ERROR
} svgtiny_code;

enum {
	svgtiny_PATH_MOVE,
	svgtiny_PATH_CLOSE,
	svgtiny_PATH_LINE,
	svgtiny_PATH_BEZIER
};

struct svgtiny_named_color {
	const char *name;
	svgtiny_colour color;
};


struct svgtiny_diagram *svgtiny_create(void);
svgtiny_code svgtiny_parse(struct svgtiny_diagram *diagram,
		const char *buffer, size_t size, const char *url,
		int width, int height);
void svgtiny_free(struct svgtiny_diagram *svg);

svgtiny_code svgtiny_parse_dom(const char *buffer, size_t size, const char *url, dom_document **output_dom);
svgtiny_code svgtiny_parse_svg_from_dom(struct svgtiny_diagram *diagram, dom_document *dom, int width, int height);
void svgtiny_free_dom(dom_document *dom);

#endif
