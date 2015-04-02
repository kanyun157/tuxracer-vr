/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifndef OGL_H
#define OGL_H

#include "bh.h"

#include "OVR_CAPI.h"

#define FAR_CLIP_FUDGE_AMOUNT 5
#define NEAR_CLIP_DIST 0.1

// jdt TODO: get rid of globals
extern unsigned int fbo, fb_tex, fb_depth;
extern unsigned int fb_width, fb_height;
extern int fb_tex_width, fb_tex_height;
extern unsigned int stereo_gl_list;

enum TRenderMode {
    GUI,
    GAUGE_BARS,
    TEXFONT,
    COURSE,
    TREES,
    PARTICLES,
    TUX,
    TUX_SHADOW,
    SKY,
    FOG_PLANE,
    TRACK_MARKS
};

TRenderMode current_render_mode ();
void set_gl_options (TRenderMode mode);

void check_gl_error();
void init_glfloat_array( int num, GLfloat arr[], ... );
void InitOpenglExtensions();
void PrintGLInfo();

void set_material (const TColor& diffuse_colour,
		const TColor& specular_colour,
		double specular_exp);


void PushRenderMode(TRenderMode mode);
void PopRenderMode();

struct ScopedRenderMode {
	ScopedRenderMode(TRenderMode mode) {
		PushRenderMode(mode);
	}
	~ScopedRenderMode() {
		PopRenderMode();
	}
};

void ClearRenderContext (); // jdt: replaced by ClearDisplay
void ClearDisplay ();
void ClearRenderContext (const TColor& col);
void SetupGuiDisplay ();
void SetupHudDisplay (bool attachToFace=false);
void SetupDisplay (ovrEyeType eye);
void UpdateRenderTarget (unsigned int width, unsigned int height);
void Reshape (int w, int h);

// convert a quaternion to a rotation matrix
void quat_to_matrix(const float *quat, float *mat);

#endif
