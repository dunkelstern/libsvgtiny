/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2008 James Bursa <james@semichrome.net>
 */

#ifndef SVGTINY_H
#define SVGTINY_H

#include <libxml/parser.h>

typedef int svgtiny_colour;
#define svgtiny_TRANSPARENT 0x1000000
#define svgtiny_RGB(r, g, b) ((r) << 16 | (g) << 8 | (b))

struct svgtiny_shape {
	float *path;
	unsigned int path_length;
	char *text;
	float text_x, text_y;
	svgtiny_colour fill;
	svgtiny_colour stroke;
	int stroke_width;
};

struct svgtiny_diagram {
	xmlDoc *doc;
	xmlNode *svg;

	int width, height;

	struct svgtiny_shape *shape;
	unsigned int shape_count;
};

typedef enum {
	svgtiny_OK,
	svgtiny_OUT_OF_MEMORY,
	svgtiny_LIBXML_ERROR,
	svgtiny_NOT_SVG,
} svgtiny_code;

enum {
	svgtiny_PATH_MOVE,
	svgtiny_PATH_CLOSE,
	svgtiny_PATH_LINE,
	svgtiny_PATH_BEZIER,
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

const struct svgtiny_named_color *
svgtiny_color_lookup (register const char *str, register unsigned int len);

#endif
