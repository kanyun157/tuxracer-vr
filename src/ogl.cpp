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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "ogl.h"
#include "spx.h"
#include "winsys.h"
#include "env.h"
#include "physics.h"
#include "game_ctrl.h"
#include <iostream>
#include <cstdarg>
#include <stack>

TRenderMode currentMode = (TRenderMode)-1;

TRenderMode current_render_mode() { return currentMode; }

struct gl_value_t {
    char name[40];
    GLenum value;
    GLenum type;
};

static const gl_value_t gl_values[] = {
    { "maximum lights", GL_MAX_LIGHTS, GL_INT },
    { "modelview stack depth", GL_MAX_MODELVIEW_STACK_DEPTH, GL_INT },
    { "projection stack depth", GL_MAX_PROJECTION_STACK_DEPTH, GL_INT },
    { "max texture size", GL_MAX_TEXTURE_SIZE, GL_INT },
    { "double buffering", GL_DOUBLEBUFFER, GL_UNSIGNED_BYTE },
    { "red bits", GL_RED_BITS, GL_INT },
    { "green bits", GL_GREEN_BITS, GL_INT },
    { "blue bits", GL_BLUE_BITS, GL_INT },
    { "alpha bits", GL_ALPHA_BITS, GL_INT },
    { "depth bits", GL_DEPTH_BITS, GL_INT },
    { "stencil bits", GL_STENCIL_BITS, GL_INT } };

void check_gl_error() {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
		const char* errstr = (const char*)gluErrorString(error);
		Message ("OpenGL Error: ", errstr ? errstr : "");
    }
}

void init_glfloat_array (int num, GLfloat arr[], ...) {
    va_list args;
    va_start (args, arr);
    for (int i=0; i<num; i++) arr[i] = va_arg(args, double);
    va_end (args);
}


void InitOpenglExtensions () {
    if (!GLEW_EXT_framebuffer_object) {
        Message ("Oculus Rift support currently requires high-end cards w/ frame buffer object support.");
        Message ("This system configuration has been determined not to have this.  Aborting");
        // jdt TODO: Don't quit if oculus isn't the configured video mode.  give helpful hint otherwise.
        SDL_Quit();
    }

    if (!GLEW_EXT_compiled_vertex_array) {
        Message ("Failed to find OpenGL extension support for GL_EXT_compiled_vertex_array");
        SDL_Quit();
    }
}

void PrintGLInfo () {
    GLint int_val;
    GLfloat float_val;
    GLboolean boolean_val;
	string ss;

    Message ("Gl vendor: ", (char*)glGetString (GL_VENDOR));
    Message ("Gl renderer: ", (char*)glGetString (GL_RENDERER));
    Message ("Gl version: ", (char*)glGetString (GL_VERSION));
    string extensions = (char*)glGetString (GL_EXTENSIONS);
    Message ("", "");
	Message( "Status: Using GLEW: ", (char*)glewGetString(GLEW_VERSION));
	Message ("Gl extensions:", "");
	Message ("", "");

	size_t oldpos = 0;
	size_t pos;
    while ((pos = extensions.find(' ', oldpos)) != string::npos) {
		string s = extensions.substr(oldpos, pos-oldpos);
		Message(s, "");
		oldpos = pos+1;
    }
	Message(extensions.substr(oldpos), "");

	Message ("", "");
    for (int i=0; i<(int)(sizeof(gl_values)/sizeof(gl_values[0])); i++) {
		switch (gl_values[i].type) {
			case GL_INT:
	    	glGetIntegerv (gl_values[i].value, &int_val);
		    ss = Int_StrN (int_val);
			Message (gl_values[i].name, ss);
		    break;

			case GL_FLOAT:
		    glGetFloatv (gl_values[i].value, &float_val);
    		ss = Float_StrN (float_val, 2);
			Message (gl_values[i].name, ss);
		    break;

			case GL_UNSIGNED_BYTE:
	    	glGetBooleanv (gl_values[i].value, &boolean_val);
		    ss = Int_StrN (boolean_val);
			Message (gl_values[i].name, ss);
		    break;

			default:
			Message ("","");
		}
    }
}

void set_material (const TColor& diffuse_colour, const TColor& specular_colour, double specular_exp) {
	GLfloat mat_amb_diff[4];
	GLfloat mat_specular[4];

	mat_amb_diff[0] = diffuse_colour.r;
	mat_amb_diff[1] = diffuse_colour.g;
	mat_amb_diff[2] = diffuse_colour.b;
	mat_amb_diff[3] = diffuse_colour.a;
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);

	mat_specular[0] = specular_colour.r;
	mat_specular[1] = specular_colour.g;
	mat_specular[2] = specular_colour.b;
	mat_specular[3] = specular_colour.a;
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);

	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, specular_exp);

	glColor4f (diffuse_colour.r, diffuse_colour.g, diffuse_colour.b,
	     diffuse_colour.a);
}

void ClearDisplay () {
	glDepthMask (GL_TRUE);
	glClearColor (colBackgr.r, colBackgr.g, colBackgr.b, colBackgr.a);
	glClearStencil (0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ClearRenderContext () { }

void ClearRenderContext (const TColor& col) {
	glDepthMask (GL_TRUE);
	glClearColor (col.r, col.g, col.b, col.a);
	glClearStencil (0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void SetupGuiDisplay(bool displayFrame) {
    // Move x,y coordinate system to positive quandrant.
    float offsetx = (float)Winsys.resolution.width/2;
    float offsety = (float)Winsys.resolution.height/2;
    glTranslatef (-offsetx, -offsety, -offsety);

	if (displayFrame) { // semi-transparent frame behind gui elements
		GLfloat resx = (GLfloat)Winsys.resolution.width;
		GLfloat resy = (GLfloat)Winsys.resolution.height;
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
		glDepthMask (GL_TRUE);
		glDepthFunc (GL_ALWAYS);
		glColor4f (0, 0, 0, 0.4);
		glBegin(GL_QUADS);
		{
			GLfloat depth = -1; //-param.forward_clip_distance / 2;
			glNormal3f (0, 0, 1);
			glVertex3f (0, 0, depth);
			glVertex3f (0, resy, depth);
			glVertex3f (resx, resy, depth);
			glVertex3f (resx, 0, depth);
		}
		glEnd();
		glEnable (GL_TEXTURE_2D);

		glColor4f (1, 1, 1, 1);
	}
}

void SetupHudDisplay(bool attachToFace) {
    glDisable (GL_FOG);
    if (attachToFace || param.attach_hud_to_face) {
        glLoadIdentity ();
    }
    glScalef (0.08f, 0.08f, 0.15f); // err..bitrary

    // Move x,y coordinate system to positive quandrant.
    float offsetx = (float)Winsys.resolution.width/2;
    float offsety = (float)Winsys.resolution.height/2;
    glTranslatef (-offsetx, -offsety, -offsety);
}

void SetupDisplay (ovrEyeType eye, bool skybox) {
    // we'll just have to use the projection matrix supplied by the oculus SDK for this eye
    // note that libovr matrices are the transpose of what OpenGL expects, so we have
    // to use glLoadTransposeMatrixf instead of glLoadMatrixf to load it.
    //
    //double far_clip = currentMode == GUI ? 2000.0f : param.forward_clip_distance + FAR_CLIP_FUDGE_AMOUNT;
    double far_clip = 2000.0f; // jdt: trying to lessen the difference between GUI and 3D modes.
    // jdt: increase near_clip to get better depth buffer resolution if we turn that on.
    ovrMatrix4f proj = ovrMatrix4f_Projection(Winsys.hmd->DefaultEyeFov[eye], NEAR_CLIP_DIST, far_clip, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadTransposeMatrixf(proj.M[0]);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// retrieve the orientation quaternion and convert it to a rotation matrix 
	float rot_mat[16];
	quat_to_matrix(&Winsys.eyePose[ovrEye_Left].Orientation.x, rot_mat);

	// jdt: trick to render skybox without stereo.. up close to avoid fog.
	if (skybox) {
		glMultMatrixf(rot_mat);
		Env.DrawSkybox (TVector3(0,0,0), true);
		glLoadIdentity();
	}

	glTranslatef(Winsys.eye_rdesc[eye].HmdToEyeViewOffset.x * param.ipd_multiplier,
			Winsys.eye_rdesc[eye].HmdToEyeViewOffset.y * param.ipd_multiplier,
			Winsys.eye_rdesc[eye].HmdToEyeViewOffset.z * param.ipd_multiplier);

	glMultMatrixf(rot_mat);

	// translate the view matrix with the positional tracking
	glTranslatef(-Winsys.eyePose[eye].Position.x * param.ipd_multiplier,
			-Winsys.eyePose[eye].Position.y * param.ipd_multiplier,
			-Winsys.eyePose[eye].Position.z * param.ipd_multiplier);
	// move the camera to the eye level of the user
	//glTranslate(0, -ovrHmd_GetFloat(Winsys.hmd, OVR_KEY_EYE_HEIGHT, 1.65), 0);

	glColor4f (1.0, 1.0, 1.0, 1.0);
}

// Ripped from:
// Author: John Tsiombikas <nuclear@member.fsf.org>
// LICENSE: This code is in the public domain. Do whatever you like with it.
// DOC: 1 << (log(x-1, 2) + 1) next higher power of two for ogl texture sizing.
unsigned int next_pow2(unsigned int x)
{
    x -= 1;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

// Ripped from:
// Author: John Tsiombikas <nuclear@member.fsf.org>
// LICENSE: This code is in the public domain. Do whatever you like with it.
// DOC: convert a quaternion to a rotation matrix
void quat_to_matrix(const float *quat, float *mat)
{
    mat[0] = 1.0 - 2.0 * quat[1] * quat[1] - 2.0 * quat[2] * quat[2];
    mat[4] = 2.0 * quat[0] * quat[1] + 2.0 * quat[3] * quat[2];
    mat[8] = 2.0 * quat[2] * quat[0] - 2.0 * quat[3] * quat[1];
    mat[12] = 0.0f;

    mat[1] = 2.0 * quat[0] * quat[1] - 2.0 * quat[3] * quat[2];
    mat[5] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[2]*quat[2];
    mat[9] = 2.0 * quat[1] * quat[2] + 2.0 * quat[3] * quat[0];
    mat[13] = 0.0f;

    mat[2] = 2.0 * quat[2] * quat[0] + 2.0 * quat[3] * quat[1];
    mat[6] = 2.0 * quat[1] * quat[2] - 2.0 * quat[3] * quat[0];
    mat[10] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[1]*quat[1];
    mat[14] = 0.0f;

    mat[3] = mat[7] = mat[11] = 0.0f;
    mat[15] = 1.0f;
}

// Ripped from:
// Author: John Tsiombikas <nuclear@member.fsf.org>
// LICENSE: This code is in the public domain. Do whatever you like with it.
// DOC: creates (and/or resizes) the render target used to draw the two stero views.
unsigned int fbo, fb_tex[2], fb_depth;
unsigned int fb_width, fb_height;
int fb_tex_width, fb_tex_height;
unsigned int stereo_gl_list;

void UpdateRenderTarget(unsigned int width, unsigned int height)
{
    // save to globals for the heck of it.  
    fb_width = width;
    fb_height = height;

    if(!fbo) {
        // if fbo does not exist, then nothing does. create opengl objects
        glGenFramebuffers(1, &fbo);
        glGenTextures(2, fb_tex);
        glGenRenderbuffers(1, &fb_depth);

        glBindTexture(GL_TEXTURE_2D, fb_tex[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, fb_tex[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    check_gl_error();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // calculate the next power of two in both dimensions and use that as a texture size 
    fb_tex_width = next_pow2(width);
    fb_tex_height = next_pow2(height);

    // create and attach the texture that will be used as a color buffer
	glBindTexture(GL_TEXTURE_2D, fb_tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_tex_width, fb_tex_height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, fb_tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_tex_width, fb_tex_height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_tex[0], 0);

    check_gl_error();

    // create and attach the renderbuffer that will serve as our z-buffer
    glBindRenderbuffer(GL_RENDERBUFFER, fb_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fb_tex_width, fb_tex_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb_depth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        check_gl_error();
        fprintf(stderr, "Failed to create Complete Framebuffer!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    printf("created render target: %dx%d (texture size: %dx%d)\n", width, height, fb_tex_width, fb_tex_height);

    // jdt: cache geometry for second eye.
    if (glIsList(stereo_gl_list)) {
        glDeleteLists(stereo_gl_list, 1);
    }
    stereo_gl_list = glGenLists(1);
}

void Reshape (int w, int h) {
	// jdt: Overriding with nothing because we handle this in SetupDisplay now.
}

// ====================================================================
//					GL options
// ====================================================================

void set_gl_options (TRenderMode mode)
{
	currentMode = mode;
	switch (mode) {
    case GUI:
        glEnable (GL_TEXTURE_2D);
        glEnable (GL_DEPTH_TEST); // needed for lookAt widget selection.
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_ALWAYS);
		glDisable (GL_FOG);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        break;

	case GAUGE_BARS:
        glEnable (GL_TEXTURE_2D);
        glEnable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glEnable (GL_TEXTURE_GEN_S);
		glEnable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_ALWAYS);

		glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        break;

    case TEXFONT:
        glEnable (GL_TEXTURE_2D);
        glEnable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_ALWAYS);
        break;

	case COURSE:
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);
		glEnable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glEnable (GL_TEXTURE_GEN_S);
		glEnable (GL_TEXTURE_GEN_T);
		glEnable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LEQUAL);

		glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		break;

    case TREES:
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glEnable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
        glEnable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LEQUAL);

        glAlphaFunc (GL_GEQUAL, 0.5);
        break;

    case PARTICLES:
        glEnable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glEnable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LEQUAL);

        glAlphaFunc (GL_GEQUAL, 0.5);
        break;

	case SKY:
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
		glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_FALSE); // render skybox closer than other things
		glDepthFunc (GL_ALWAYS);
		//glDisable (GL_FOG);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LEQUAL);
		break;

    case FOG_PLANE:
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
		glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LEQUAL);
		break;

    case TUX:
	    glDisable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);
    	glEnable (GL_LIGHTING);
		glEnable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LEQUAL);
    	break;

    case TUX_SHADOW:
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_COLOR_MATERIAL);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
	#ifdef USE_STENCIL_BUFFER
		glDisable (GL_CULL_FACE);
		glEnable (GL_STENCIL_TEST);
		glDepthMask (GL_FALSE);

		glStencilFunc (GL_EQUAL, 0, ~0);
		glStencilOp (GL_KEEP, GL_KEEP, GL_INCR);
	#else
		glEnable (GL_CULL_FACE);
		glDisable (GL_STENCIL_TEST);
		glDepthMask (GL_TRUE);
	#endif
		break;

    case TRACK_MARKS:
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
		glDisable (GL_CULL_FACE);
		glEnable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_COLOR_MATERIAL);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDepthMask (GL_FALSE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LEQUAL);
		break;

	default:
		Message ("not a valid render mode", "");
    }
}
/* defined but not used
    case TEXT:
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

	case SPLASH_SCREEN:
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

    case PARTICLE_SHADOWS:
        glDisable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

    case OVERLAYS:
	    glEnable (GL_TEXTURE_2D);
    	glDisable (GL_DEPTH_TEST);
	    glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glEnable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
    	glAlphaFunc (GL_GEQUAL, 0.5);
    break;
*/

stack<TRenderMode> modestack;
void PushRenderMode(TRenderMode mode)
{
	set_gl_options(mode);
	//if(currentMode != mode)
	//	set_gl_options(mode);
	//modestack.push(mode);
}

void PopRenderMode()
{
	//TRenderMode mode = modestack.top();
	//modestack.pop();
	//if(!modestack.empty() && modestack.top() != mode)
	//	set_gl_options(modestack.top());
}

#define MAX_SHADER_SIZE (1 << 15)
int load_shader(GLenum type, const char* filename)
{
	GLuint shader;
	GLint chars_read;
	SDL_RWops* file;
	GLint compiled;

	file=SDL_RWFromFile(filename, "r");

	if (file==NULL)
	{
		printf("shader %s wasn't opened\n", filename);
		exit(1);
	}

	shader=glCreateShader(type);

	char* shader_source = new char[MAX_SHADER_SIZE];
	chars_read = (GLint)SDL_RWread(file, shader_source, 1, MAX_SHADER_SIZE-1);

	//const GLchar* sources[] = {shader_source, 0};
	glShaderSource(shader, 1, (const GLchar**)&shader_source, &chars_read);

	glCompileShader(shader);

	delete[] shader_source;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled==GL_FALSE) {
		printf("shader %s failed to compile\n", filename);

		GLsizei info_len;
		glGetShaderInfoLog(shader, MAX_SHADER_SIZE, &info_len, shader_source);
		cout << shader_source << std::endl;
		exit(1);
	}

	return shader;
}

void init_shader_program(GLuint* program, const char* vertfile, const char* fragfile)
{
	GLint linked;

	*program=glCreateProgram();

	if (vertfile) glAttachShader(*program, load_shader(GL_VERTEX_SHADER, vertfile));
	if (fragfile) glAttachShader(*program, load_shader(GL_FRAGMENT_SHADER, fragfile));

	glLinkProgram(*program);

	glGetProgramiv(*program, GL_LINK_STATUS, &linked);
	if (linked==GL_FALSE)
	{
		printf("shader failed to link\n");
		exit(1);
	}
}

