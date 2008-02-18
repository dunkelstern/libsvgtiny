/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2008 James Bursa <james@semichrome.net>
 */

#define _GNU_SOURCE  /* for strndup */
#include <assert.h>
#include <string.h>
#include "svgtiny.h"
#include "svgtiny_internal.h"

#define GRADIENT_DEBUG

static svgtiny_code svgtiny_parse_linear_gradient(xmlNode *linear,
		struct svgtiny_parse_state *state);
static float svgtiny_parse_gradient_offset(const char *s);
static void svgtiny_path_bbox(float *p, unsigned int n,
		float *x0, float *y0, float *x1, float *y1);


/**
 * Find a gradient by id and parse it.
 */

void svgtiny_find_gradient(const char *id, struct svgtiny_parse_state *state)
{
	fprintf(stderr, "svgtiny_find_gradient: id \"%s\"\n", id);

	state->linear_gradient_stop_count = 0;

	xmlNode *gradient = svgtiny_find_element_by_id(
			(xmlNode *) state->document, id);
	fprintf(stderr, "gradient %p\n", gradient);
	if (!gradient) {
		fprintf(stderr, "gradient \"%s\" not found\n", id);
		return;
	}

	fprintf(stderr, "gradient name \"%s\"\n", gradient->name);
	if (strcmp((const char *) gradient->name, "linearGradient") == 0) {
		svgtiny_parse_linear_gradient(gradient, state);
	}
}


/**
 * Parse a <linearGradient> element node.
 *
 * http://www.w3.org/TR/SVG11/pservers#LinearGradients
 */

svgtiny_code svgtiny_parse_linear_gradient(xmlNode *linear,
		struct svgtiny_parse_state *state)
{
	xmlAttr *href = xmlHasProp(linear, (const xmlChar *) "href");
	if (href && href->children->content[0] == '#')
		svgtiny_find_gradient((const char *) href->children->content
				+ 1, state);

	unsigned int i = 0;
	for (xmlNode *stop = linear->children; stop; stop = stop->next) {
		float offset = -1;
		svgtiny_colour color = svgtiny_TRANSPARENT;

		if (stop->type != XML_ELEMENT_NODE)
			continue;
		if (strcmp((const char *) stop->name, "stop") != 0)
			continue;

		for (xmlAttr *attr = stop->properties; attr;
				attr = attr->next) {
			const char *name = (const char *) attr->name;
			const char *content =
					(const char *) attr->children->content;
			if (strcmp(name, "offset") == 0)
				offset = svgtiny_parse_gradient_offset(content);
			else if (strcmp(name, "stop-color") == 0)
				svgtiny_parse_color(content, &color, state);
			else if (strcmp(name, "style") == 0) {
				const char *s;
				char *value;
				if ((s = strstr(content, "stop-color:"))) {
					s += 11;
					while (*s == ' ')
						s++;
					value = strndup(s, strcspn(s, "; "));
					svgtiny_parse_color(value, &color,
							state);
					free(value);
				}
			}
		}

		if (offset != -1 && color != svgtiny_TRANSPARENT) {
			fprintf(stderr, "stop %g %x\n", offset, color);
			state->gradient_stop[i].offset = offset;
			state->gradient_stop[i].color = color;
			i++;
		}

		if (i == svgtiny_MAX_STOPS)
			break;
	}

	if (i)
		state->linear_gradient_stop_count = i;

	return svgtiny_OK;
}


float svgtiny_parse_gradient_offset(const char *s)
{
	int num_length = strspn(s, "0123456789+-.");
	const char *unit = s + num_length;
	float n = atof((const char *) s);

	if (unit[0] == 0)
		;
	else if (unit[0] == '%')
		n /= 100.0;
	else
		return -1;

	if (n < 0)
		n = 0;
	if (1 < n)
		n = 1;
	return n;
}


/**
 * Add a path with a linear gradient fill to the svgtiny_diagram.
 */

svgtiny_code svgtiny_add_path_linear_gradient(float *p, unsigned int n,
		struct svgtiny_parse_state *state)
{
	/* determine object bounding box */
	float object_x0, object_y0, object_x1, object_y1;
	svgtiny_path_bbox(p, n, &object_x0, &object_y0, &object_x1, &object_y1);
	#ifdef GRADIENT_DEBUG
	fprintf(stderr, "object bbox: (%g %g) (%g %g)\n",
			object_x0, object_y0, object_x1, object_y1);
	#endif

	/* compute gradient vector */
	float gradient_x0 = 0, gradient_y0 = 0,
	      gradient_x1 = 1, gradient_y1 = 0.7,
	      gradient_dx, gradient_dy;
	gradient_x0 = object_x0 + gradient_x0 * (object_x1 - object_x0);
	gradient_y0 = object_y0 + gradient_y0 * (object_y1 - object_y0);
	gradient_x1 = object_x0 + gradient_x1 * (object_x1 - object_x0);
	gradient_y1 = object_y0 + gradient_y1 * (object_y1 - object_y0);
	gradient_dx = gradient_x1 - gradient_x0;
	gradient_dy = gradient_y1 - gradient_y0;
	#ifdef GRADIENT_DEBUG
	fprintf(stderr, "gradient vector: (%g %g) => (%g %g)\n",
			gradient_x0, gradient_y0, gradient_x1, gradient_y1);
	#endif

	/* show theoretical gradient strips for debugging */
	/*unsigned int strips = 10;
	for (unsigned int z = 0; z != strips; z++) {
		float f0, fd, strip_x0, strip_y0, strip_dx, strip_dy;
		f0 = (float) z / (float) strips;
		fd = (float) 1 / (float) strips;
		strip_x0 = gradient_x0 + f0 * gradient_dx;
		strip_y0 = gradient_y0 + f0 * gradient_dy;
		strip_dx = fd * gradient_dx;
		strip_dy = fd * gradient_dy;
		fprintf(stderr, "strip %i vector: (%g %g) + (%g %g)\n",
				z, strip_x0, strip_y0, strip_dx, strip_dy);

		float *p = malloc(13 * sizeof p[0]);
		if (!p)
			return svgtiny_OUT_OF_MEMORY;
		p[0] = svgtiny_PATH_MOVE;
		p[1] = strip_x0 + (strip_dy * 3);
		p[2] = strip_y0 - (strip_dx * 3);
		p[3] = svgtiny_PATH_LINE;
		p[4] = p[1] + strip_dx;
		p[5] = p[2] + strip_dy;
		p[6] = svgtiny_PATH_LINE;
		p[7] = p[4] - (strip_dy * 6);
		p[8] = p[5] + (strip_dx * 6);
		p[9] = svgtiny_PATH_LINE;
		p[10] = p[7] - strip_dx;
		p[11] = p[8] - strip_dy;
		p[12] = svgtiny_PATH_CLOSE;
		svgtiny_transform_path(p, 13, state);
		struct svgtiny_shape *shape = svgtiny_add_shape(state);
		if (!shape) {
			free(p);
			return svgtiny_OUT_OF_MEMORY;
		}
		shape->path = p;
		shape->path_length = 13;
		shape->fill = svgtiny_TRANSPARENT; 
		shape->stroke = svgtiny_RGB(0, 0xff, 0);
		state->diagram->shape_count++;
	}*/

	/* compute points on the path for triangle vertices */
	unsigned int steps = 10;
	float x0, y0, x1, y1;
	float gradient_norm_squared = gradient_dx * gradient_dx +
	                              gradient_dy * gradient_dy;
	struct grad_point {
		float x, y, r;
	};
	struct grad_point *pts = malloc(n * steps * sizeof pts[0]);
	if (!pts)
		return svgtiny_OUT_OF_MEMORY;
	unsigned int pts_count = 0;
	float min_r = 1000;
	unsigned int min_pt = 0;
	for (unsigned int j = 0; j != n; ) {
		switch ((int) p[j]) {
		case svgtiny_PATH_MOVE:
			x0 = p[j + 1];
			y0 = p[j + 2];
			j += 3;
			break;
		case svgtiny_PATH_LINE:
		case svgtiny_PATH_CLOSE:
			if (((int) p[j]) == svgtiny_PATH_LINE) {
				x1 = p[j + 1];
				y1 = p[j + 2];
				j += 3;
			} else {
				x1 = p[1];
				y1 = p[2];
				j++;
			}
			fprintf(stderr, "line: ");
			for (unsigned int z = 0; z != steps; z++) {
				float f, x, y, r;
				f = (float) z / (float) steps;
				x = x0 + f * (x1 - x0);
				y = y0 + f * (y1 - y0);
				r = ((x - gradient_x0) * gradient_dx +
					(y - gradient_y0) * gradient_dy) /
					gradient_norm_squared;
				fprintf(stderr, "(%g %g [%g]) ", x, y, r);
				pts[pts_count].x = x;
				pts[pts_count].y = y;
				pts[pts_count].r = r;
				if (r < min_r) {
					min_r = r;
					min_pt = pts_count;
				}
				pts_count++;
			}
			fprintf(stderr, "\n");
			x0 = x1;
			y0 = y1;
			break;
		case svgtiny_PATH_BEZIER:
			fprintf(stderr, "bezier: ");
			for (unsigned int z = 0; z != steps; z++) {
				float t, x, y, r;
				t = (float) z / (float) steps;
				x = (1-t) * (1-t) * (1-t) * x0 +
					3 * t * (1-t) * (1-t) * p[j + 1] +
					3 * t * t * (1-t) * p[j + 3] +
					t * t * t * p[j + 5];
				y = (1-t) * (1-t) * (1-t) * y0 +
					3 * t * (1-t) * (1-t) * p[j + 2] +
					3 * t * t * (1-t) * p[j + 4] +
					t * t * t * p[j + 6];
				r = ((x - gradient_x0) * gradient_dx +
					(y - gradient_y0) * gradient_dy) /
					gradient_norm_squared;
				fprintf(stderr, "(%g %g [%g]) ", x, y, r);
				pts[pts_count].x = x;
				pts[pts_count].y = y;
				pts[pts_count].r = r;
				if (r < min_r) {
					min_r = r;
					min_pt = pts_count;
				}
				pts_count++;
			}
			fprintf(stderr, "\n");
			x0 = p[j + 5];
			y0 = p[j + 6];
			j += 7;
			break;
		default:
			assert(0);
		}
	}
	fprintf(stderr, "pts_count %i, min_pt %i, min_r %.3f\n",
			pts_count, min_pt, min_r);

	unsigned int stop_count = state->linear_gradient_stop_count;
	assert(2 <= stop_count);
	unsigned int current_stop = 0;
	float last_stop_r = 0;
	float current_stop_r = state->gradient_stop[0].offset;
	int red0, green0, blue0, red1, green1, blue1;
	red0 = red1 = svgtiny_RED(state->gradient_stop[0].color);
	green0 = green1 = svgtiny_GREEN(state->gradient_stop[0].color);
	blue0 = blue1 = svgtiny_BLUE(state->gradient_stop[0].color);
	unsigned int t, a, b;
	t = min_pt;
	a = (min_pt + 1) % pts_count;
	b = min_pt == 0 ? pts_count - 1 : min_pt - 1;
	while (a != b) {
		float mean_r = (pts[t].r + pts[a].r + pts[b].r) / 3;
		fprintf(stderr, "triangle: t %i %.3f a %i %.3f b %i %.3f "
				"mean_r %.3f\n",
				t, pts[t].r, a, pts[a].r, b, pts[b].r,
				mean_r);
		while (current_stop != stop_count && current_stop_r < mean_r) {
			current_stop++;
			if (current_stop == stop_count)
				break;
			red0 = red1;
			green0 = green1;
			blue0 = blue1;
			red1 = svgtiny_RED(state->
					gradient_stop[current_stop].color);
			green1 = svgtiny_GREEN(state->
					gradient_stop[current_stop].color);
			blue1 = svgtiny_BLUE(state->
					gradient_stop[current_stop].color);
			last_stop_r = current_stop_r;
			current_stop_r = state->
					gradient_stop[current_stop].offset;
		}
		float *p = malloc(10 * sizeof p[0]);
		if (!p)
			return svgtiny_OUT_OF_MEMORY;
		p[0] = svgtiny_PATH_MOVE;
		p[1] = pts[t].x;
		p[2] = pts[t].y;
		p[3] = svgtiny_PATH_LINE;
		p[4] = pts[a].x;
		p[5] = pts[a].y;
		p[6] = svgtiny_PATH_LINE;
		p[7] = pts[b].x;
		p[8] = pts[b].y;
		p[9] = svgtiny_PATH_CLOSE;
		svgtiny_transform_path(p, 10, state);
		struct svgtiny_shape *shape = svgtiny_add_shape(state);
		if (!shape) {
			free(p);
			return svgtiny_OUT_OF_MEMORY;
		}
		shape->path = p;
		shape->path_length = 10;
		/*shape->fill = svgtiny_TRANSPARENT;*/
		if (current_stop == 0)
			shape->fill = state->gradient_stop[0].color;
		else if (current_stop == stop_count)
			shape->fill = state->
					gradient_stop[stop_count - 1].color;
		else {
			float stop_r = (mean_r - last_stop_r) /
				(current_stop_r - last_stop_r);
			shape->fill = svgtiny_RGB(
				(int) ((1 - stop_r) * red0 + stop_r * red1),
				(int) ((1 - stop_r) * green0 + stop_r * green1),
				(int) ((1 - stop_r) * blue0 + stop_r * blue1));
		}
		shape->stroke = svgtiny_TRANSPARENT;
		#ifdef GRADIENT_DEBUG
		shape->stroke = svgtiny_RGB(0, 0, 0xff);
		#endif
		state->diagram->shape_count++;
		if (pts[a].r < pts[b].r) {
			t = a;
			a = (a + 1) % pts_count;
		} else {
			t = b;
			b = b == 0 ? pts_count - 1 : b - 1;
		}
	}

	/* render gradient vector for debugging */
	#ifdef GRADIENT_DEBUG
	{
		float *p = malloc(7 * sizeof p[0]);
		if (!p)
			return svgtiny_OUT_OF_MEMORY;
		p[0] = svgtiny_PATH_MOVE;
		p[1] = gradient_x0;
		p[2] = gradient_y0;
		p[3] = svgtiny_PATH_LINE;
		p[4] = gradient_x1;
		p[5] = gradient_y1;
		p[6] = svgtiny_PATH_CLOSE;
		svgtiny_transform_path(p, 7, state);
		struct svgtiny_shape *shape = svgtiny_add_shape(state);
		if (!shape) {
			free(p);
			return svgtiny_OUT_OF_MEMORY;
		}
		shape->path = p;
		shape->path_length = 7;
		shape->fill = svgtiny_TRANSPARENT; 
		shape->stroke = svgtiny_RGB(0xff, 0, 0);
		state->diagram->shape_count++;
	}
	#endif

	/* render triangle vertices with r values for debugging */
	#ifdef GRADIENT_DEBUG
	for (unsigned int i = 0; i != pts_count; i++) {
		struct svgtiny_shape *shape = svgtiny_add_shape(state);
		if (!shape)
			return svgtiny_OUT_OF_MEMORY;
		char *text = malloc(20);
		if (!text)
			return svgtiny_OUT_OF_MEMORY;
		sprintf(text, "%i=%.3f", i, pts[i].r);
		shape->text = text;
		shape->text_x = state->ctm.a * pts[i].x +
				state->ctm.c * pts[i].y + state->ctm.e;
		shape->text_y = state->ctm.b * pts[i].x +
				state->ctm.d * pts[i].y + state->ctm.f;
		shape->fill = svgtiny_RGB(0, 0, 0);
		state->diagram->shape_count++;
	}
	#endif

	/* plot actual path outline */
	if (state->stroke != svgtiny_TRANSPARENT) {
		svgtiny_transform_path(p, n, state);

		struct svgtiny_shape *shape = svgtiny_add_shape(state);
		if (!shape) {
			free(p);
			return svgtiny_OUT_OF_MEMORY;
		}
		shape->path = p;
		shape->path_length = n;
		shape->fill = svgtiny_TRANSPARENT;
		state->diagram->shape_count++;
	}

	return svgtiny_OK;
}


/**
 * Get the bounding box of path.
 */

void svgtiny_path_bbox(float *p, unsigned int n,
		float *x0, float *y0, float *x1, float *y1)
{
	*x0 = *x1 = p[1];
	*y0 = *y1 = p[2];

	for (unsigned int j = 0; j != n; ) {
		unsigned int points = 0;
		switch ((int) p[j]) {
		case svgtiny_PATH_MOVE:
		case svgtiny_PATH_LINE:
			points = 1;
			break;
		case svgtiny_PATH_CLOSE:
			points = 0;
			break;
		case svgtiny_PATH_BEZIER:
			points = 3;
			break;
		default:
			assert(0);
		}
		j++;
		for (unsigned int k = 0; k != points; k++) {
			float x = p[j], y = p[j + 1];
			if (x < *x0)
				*x0 = x;
			else if (*x1 < x)
				*x1 = x;
			if (y < *y0)
				*y0 = y;
			else if (*y1 < y)
				*y1 = y;
			j += 2;
		}
	}
}


/**
 * Find an element in the document by id.
 */

xmlNode *svgtiny_find_element_by_id(xmlNode *node, const char *id)
{
	xmlNode *child;
	xmlNode *found;

	for (child = node->children; child; child = child->next) {
		if (child->type != XML_ELEMENT_NODE)
			continue;
		xmlAttr *attr = xmlHasProp(child, (const xmlChar *) "id");
		if (attr && strcmp(id, (const char *) attr->children->content)
				== 0)
			return child;
		found = svgtiny_find_element_by_id(child, id);
		if (found)
			return found;
	}

	return 0;
}

