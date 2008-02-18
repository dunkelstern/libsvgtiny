/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2008 James Bursa <james@semichrome.net>
 */

#ifndef SVGTINY_INTERNAL_H
#define SVGTINY_INTERNAL_H

struct svgtiny_gradient_stop {
	float offset;
	svgtiny_colour color;
};

#define svgtiny_MAX_STOPS 10
#define svgtiny_LINEAR_GRADIENT 0x2000000

struct svgtiny_parse_state {
	struct svgtiny_diagram *diagram;
	xmlDoc *document;

	float viewport_width;
	float viewport_height;

	/* current transformation matrix */
	struct {
		float a, b, c, d, e, f;
	} ctm;

	/*struct css_style style;*/

	/* paint attributes */
	svgtiny_colour fill;
	svgtiny_colour stroke;
	int stroke_width;

	/* gradients */
	unsigned int linear_gradient_stop_count;
	struct svgtiny_gradient_stop gradient_stop[svgtiny_MAX_STOPS];
};


/* svgtiny.c */
void svgtiny_transform_path(float *p, unsigned int n,
		struct svgtiny_parse_state *state);
void svgtiny_parse_color(const char *s, svgtiny_colour *c,
		struct svgtiny_parse_state *state);
struct svgtiny_shape *svgtiny_add_shape(struct svgtiny_parse_state *state);

/* svgtiny_gradient.c */
void svgtiny_find_gradient(const char *id, struct svgtiny_parse_state *state);
svgtiny_code svgtiny_add_path_linear_gradient(float *p, unsigned int n,
		struct svgtiny_parse_state *state);
xmlNode *svgtiny_find_element_by_id(xmlNode *node, const char *id);

/* colors.gperf */
const struct svgtiny_named_color *
		svgtiny_color_lookup(register const char *str,
				register unsigned int len);

#endif
