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

#include "env.h"
#include "ogl.h"
#include "textures.h"
#include "spx.h"
#include "view.h"
#include "course.h"
#include "winsys.h"

// --------------------------------------------------------------------
//					defaults
// --------------------------------------------------------------------

static const float def_amb[]     = {0.45, 0.53, 0.75, 1.0};
static const float def_diff[]    = {1.0, 0.9, 1.0, 1.0};
static const float def_spec[]    = {0.6, 0.6, 0.6, 1.0};
static const float def_pos[]     = {1, 2, 2, 0.0};
static const float def_fogcol[]  = {0.9, 0.9, 1.0, 0.0};
static const TColor def_partcol    (0.8, 0.8, 0.9, 0.0);

CEnvironment Env;

CEnvironment::CEnvironment ()
{
	EnvID = -1;
	lightcond[0].name = "sunny";
	lightcond[1].name = "cloudy";
	lightcond[2].name = "evening";
	lightcond[3].name = "night";
	for(size_t i = 0; i < 4; i++)
		LightIndex[lightcond[i].name] = i;
	Skybox = NULL;

	default_light.is_on = true;
	for (int i=0; i<4; i++) {
		default_light.ambient[i]  = def_amb[i];
		default_light.diffuse[i]  = def_diff[i];
		default_light.specular[i] = def_spec[i];
		default_light.position[i] = def_pos[i];
	}

	default_fog.is_on = true;
	default_fog.mode = GL_LINEAR;
	default_fog.start = 10.0;
	default_fog.end = 7000.0;
	default_fog.height = 0;
	for (int i=0; i<4; i++)
		default_fog.color[i] = def_fogcol[i];
	default_fog.part_color = def_partcol;
}

void CEnvironment::ResetSkybox () {
	delete[] Skybox;
	Skybox = NULL;
}

void CEnvironment::SetupLight () {
	glLightfv (GL_LIGHT0, GL_POSITION, lights[0].position);
	glLightfv (GL_LIGHT0, GL_AMBIENT, lights[0].ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, lights[0].diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, lights[0].specular);
	glEnable  (GL_LIGHT0);
	if (lights[1].is_on) {
		glLightfv (GL_LIGHT1, GL_POSITION, lights[1].position);
		glLightfv (GL_LIGHT1, GL_AMBIENT, lights[1].ambient);
		glLightfv (GL_LIGHT1, GL_DIFFUSE, lights[1].diffuse);
		glLightfv (GL_LIGHT1, GL_SPECULAR, lights[1].specular);
		glEnable  (GL_LIGHT1);
	}
	if (lights[2].is_on) {
		glLightfv (GL_LIGHT2, GL_POSITION, lights[2].position);
		glLightfv (GL_LIGHT2, GL_AMBIENT, lights[2].ambient);
		glLightfv (GL_LIGHT2, GL_DIFFUSE, lights[2].diffuse);
		glLightfv (GL_LIGHT2, GL_SPECULAR, lights[2].specular);
		glEnable  (GL_LIGHT2);
	}
	if (lights[3].is_on) {
		glLightfv (GL_LIGHT3, GL_POSITION, lights[3].position);
		glLightfv (GL_LIGHT3, GL_AMBIENT, lights[3].ambient);
		glLightfv (GL_LIGHT3, GL_DIFFUSE, lights[3].diffuse);
		glLightfv (GL_LIGHT3, GL_SPECULAR, lights[3].specular);
		glEnable  (GL_LIGHT3);
	}
	glEnable  (GL_LIGHTING);
}

void CEnvironment::SetupFog () {
	//if (!fog.is_on) {
	//	glDisable (GL_FOG);
	//	return;
	//}
    glEnable (GL_FOG);
    glFogi   (GL_FOG_MODE, fog.mode);
    glFogf   (GL_FOG_START, fog.start);
    glFogf   (GL_FOG_END, fog.end);
    glFogfv  (GL_FOG_COLOR, fog.color);

    if (param.perf_level > 1) {
		glHint (GL_FOG_HINT, GL_NICEST);
    } else {
		glHint (GL_FOG_HINT, GL_FASTEST);
    }
}

void CEnvironment::ResetLight () {
	lights[0] = default_light;
	for (int i=1; i<4; i++) lights[i].is_on = false;
	glDisable (GL_LIGHT1);
	glDisable (GL_LIGHT2);
	glDisable (GL_LIGHT3);
}

void CEnvironment::ResetFog () {
	fog = default_fog;
}

void CEnvironment::Reset () {
	ResetSkybox ();
	ResetLight ();
	ResetFog ();
}

bool CEnvironment::LoadEnvironmentList () {
	CSPList list (32, true);
	if (!list.Load (param.env_dir2, "environment.lst")) {
		Message ("could not load environment.lst");
		return false;
	}

	locs.resize(list.Count());
	for (size_t i=0; i<list.Count(); i++) {
		const string& line = list.Line(i);
		locs[i].name = SPStrN (line, "location", "");
	}
	list.MakeIndex (EnvIndex, "location");
	return true;
}

string CEnvironment::GetDir (size_t location, size_t light) const {
	if (location >= locs.size()) return "";
	if (light >= 4) return "";
	string res =
		param.env_dir2 + SEP +
		locs[location].name + SEP + lightcond[light].name;
	return res;
}

void CEnvironment::LoadSkybox () {
	if (param.no_skybox) return; // jdt: has issue w/ fog and stereo, atm
	Skybox = new TTexture[param.full_skybox?6:3];
	Skybox[0].Load(EnvDir, "front.png");
	Skybox[1].Load(EnvDir, "left.png");
	Skybox[2].Load(EnvDir, "right.png");
	if (param.full_skybox) {
		Skybox[3].Load(EnvDir, "top.png");
		Skybox[4].Load(EnvDir, "bottom.png");
		Skybox[5].Load(EnvDir, "back.png");
	}
}

void CEnvironment::LoadLight () {
	static const string idxstr = "[fog]-1[0]0[1]1[2]2[3]3[4]4[5]5[6]6";

	CSPList list(24);
	if (!list.Load (EnvDir, "light.lst")) {
		Message ("could not load light file", "");
		return;
	}

	for (size_t i=0; i<list.Count(); i++) {
		const string& line = list.Line(i);
		string item = SPStrN (line, "light", "none");
		int idx = SPIntN (idxstr, item, -1);
		if (idx < 0) {
			fog.is_on = SPBoolN (line, "fog", true);
			fog.start = SPFloatN (line, "fogstart", 20);
			fog.end = SPFloatN (line, "fogend", param.forward_clip_distance);
			fog.height = SPFloatN (line, "fogheight", 0);
			SPArrN (line, "fogcol", fog.color, 4, 1);
			fog.part_color = SPColorN (line, "partcol", def_partcol);
		} else if (idx < 4) {
			lights[idx].is_on = true;
			SPArrN (line, "amb", lights[idx].ambient, 4, 1);
			SPArrN (line, "diff", lights[idx].diffuse, 4, 1);
			SPArrN (line, "spec", lights[idx].specular, 4, 1);
			SPArrN (line, "pos", lights[idx].position, 4, 1);
		}
	}
}

void CEnvironment::DrawSkyboxGui () {
	// Skybox is now done from SetupDisplay in order to render it 
	// up close, but from the same eye offset on both viewports
	// ie. infinite stereo distance.
	//DrawSkybox (TVector3(0,0,0), true);
}

void CEnvironment::DrawSkyboxRacing (const TVector3& pos) {
	//DrawSkybox (pos, false);
}

void CEnvironment::DrawSkybox (const TVector3& pos, bool textures) {
	if (textures && !Skybox) return;
	if (textures) ScopedRenderMode rm(SKY);
	else { glDisable (GL_TEXTURE_2D); } // jdt: avoid unnecessary ogl state changes

	glDepthMask (GL_FALSE);

	GLfloat aa, bb;
	aa = 0.005f;
	bb = 0.995f;

	glColor4f (1.0, 1.0, 1.0, 1.0);

	if (textures) Skybox[0].Bind();
	glBegin(GL_QUADS);
		glTexCoord2f (aa, aa); glVertex3f (-1, -1, -1);
		glTexCoord2f (bb, aa); glVertex3f ( 1, -1, -1);
		glTexCoord2f (bb, bb); glVertex3f ( 1,  1, -1);
		glTexCoord2f (aa, bb); glVertex3f (-1,  1, -1);
	glEnd();

	// left
	if (textures) Skybox[1].Bind();
	glBegin(GL_QUADS);
		glTexCoord2f (aa, aa); glVertex3f (-1, -1,  1);
		glTexCoord2f (bb, aa); glVertex3f (-1, -1, -1);
		glTexCoord2f (bb, bb); glVertex3f (-1,  1, -1);
		glTexCoord2f (aa, bb); glVertex3f (-1,  1,  1);
	glEnd();

	// right
	if (textures) Skybox[2].Bind();
	glBegin(GL_QUADS);
		glTexCoord2f (aa, aa); glVertex3f (1, -1, -1);
		glTexCoord2f (bb, aa); glVertex3f (1, -1,  1);
		glTexCoord2f (bb, bb); glVertex3f (1,  1,  1);
		glTexCoord2f (aa, bb); glVertex3f (1,  1, -1);
	glEnd();

	// normally, the following textures are unvisible
	// see game_config.cpp (param.full_skybox)
	if (param.full_skybox) {
		// top
		if (textures) Skybox[3].Bind();
		glBegin(GL_QUADS);
			glTexCoord2f (aa, aa); glVertex3f (-1, 1, -1);
			glTexCoord2f (bb, aa); glVertex3f ( 1, 1, -1);
			glTexCoord2f (bb, bb); glVertex3f ( 1, 1,  1);
			glTexCoord2f (aa, bb); glVertex3f (-1, 1,  1);
		glEnd();

		// bottom
		if (textures) Skybox[4].Bind();
		glBegin(GL_QUADS);
			glTexCoord2f (aa, aa); glVertex3f (-1, -1,  1);
			glTexCoord2f (bb, aa); glVertex3f ( 1, -1,  1);
			glTexCoord2f (bb, bb); glVertex3f ( 1, -1, -1);
			glTexCoord2f (aa, bb); glVertex3f (-1, -1, -1);
		glEnd();

		// back
		if (textures) Skybox[5].Bind();
		glBegin(GL_QUADS);
			glTexCoord2f (aa, aa); glVertex3f ( 1, -1, 1);
			glTexCoord2f (bb, aa); glVertex3f (-1, -1, 1);
			glTexCoord2f (bb, bb); glVertex3f (-1,  1, 1);
			glTexCoord2f (aa, bb); glVertex3f ( 1,  1, 1);
		glEnd();
	}

	if (!textures) glEnable (GL_TEXTURE_2D);

	glDepthMask (GL_TRUE);
}

void CEnvironment::DrawFog () {
	if (!fog.is_on) return;

    TPlane bottom_plane, top_plane;
    TVector3 left, right, vpoint;
    TVector3 topleft, topright;
    TVector3 bottomleft, bottomright;

	// the clipping planes are calculated by view frustum (view.cpp)
	TPlane leftclip = get_left_env_clip_plane ();
	TPlane rightclip = get_right_env_clip_plane ();
	TPlane farclip = get_far_env_clip_plane ();
	TPlane bottomclip = get_bottom_env_clip_plane ();

	// --------------- calculate the planes ---------------------------

    float slope = tan (ANGLES_TO_RADIANS (Course.GetCourseAngle()));
//	TPlane left_edge_plane = MakePlane (1.0, 0.0, 0.0, 0.0);
//	TPlane right_edge_plane = MakePlane (-1.0, 0.0, 0.0, Course.width);

    bottom_plane.nml = TVector3(0.0, 1, -slope);
    float height = Course.GetBaseHeight (0);
    bottom_plane.d = -height * bottom_plane.nml.y;

    top_plane.nml = bottom_plane.nml;
    height = Course.GetMaxHeight (0) + fog.height;
    top_plane.d = -height * top_plane.nml.y;


    if (!IntersectPlanes (bottom_plane, farclip, leftclip,  &left)) return;
    if (!IntersectPlanes (bottom_plane, farclip, rightclip, &right)) return;
    if (!IntersectPlanes (top_plane,    farclip, leftclip,  &topleft)) return;
    if (!IntersectPlanes (top_plane,    farclip, rightclip, &topright)) return;
    if (!IntersectPlanes (bottomclip,   farclip, leftclip,  &bottomleft)) return;
    if (!IntersectPlanes (bottomclip,   farclip, rightclip, &bottomright)) return;

	TVector3 leftvec  = SubtractVectors (topleft, left);
    TVector3 rightvec = SubtractVectors (topright, right);

	// --------------- draw the fog plane -----------------------------

	ScopedRenderMode rm(FOG_PLANE);
    glEnable (GL_FOG);

	// only the alpha channel is used
	float bottom_dens[4]     = {0, 0, 0, 1.0};
	float top_dens[4]        = {0, 0, 0, 0.9};
	float leftright_dens[4]  = {0, 0, 0, 0.3};
	float top_bottom_dens[4] = {0, 0, 0, 0.0};

    glBegin (GL_QUAD_STRIP);
	    glColor4fv (bottom_dens);
	    glVertex3f (bottomleft.x, bottomleft.y, bottomleft.z);
    	glVertex3f (bottomright.x, bottomright.y, bottomright.z);
	    glVertex3f (left.x, left.y, left.z);
    	glVertex3f (right.x, right.y, right.z);

	    glColor4fv (top_dens);
    	glVertex3f (topleft.x, topleft.y, topleft.z);
    	glVertex3f (topright.x, topright.y, topright.z);

	    glColor4fv (leftright_dens);
    	vpoint = AddVectors (topleft, leftvec);
	    glVertex3f (vpoint.x, vpoint.y, vpoint.z);
    	vpoint = AddVectors (topright, rightvec);
	    glVertex3f (vpoint.x, vpoint.y, vpoint.z);

	    glColor4fv (top_bottom_dens);
	    vpoint = AddVectors (topleft, ScaleVector (3.0, leftvec));
    	glVertex3f (vpoint.x, vpoint.y, vpoint.z);
	    vpoint = AddVectors (topright, ScaleVector (3.0, rightvec));
    	glVertex3f (vpoint.x, vpoint.y, vpoint.z);
    glEnd();
}


bool CEnvironment::LoadEnvironment (size_t loc, size_t light) {
	if (loc >= locs.size()) loc = 0;
	if (light >= 4) light = 0;
	// remember: with (example) 3 locations and 4 lights there
	// are 12 different environments
	size_t env_id = loc * 100 + light;
	static size_t prev_env_id = 9999;

	if (env_id == prev_env_id) {//EnvID) {
		return false;
	}
	prev_env_id = env_id; // jdt: remember previous environment

	// Set directory. The dir is used several times.
	EnvDir = GetDir (loc, light);

	// Load skybox. If the sky can't be loaded for any reason, the
	// texture id's are set to 0 and the sky will not be drawn.
	// There is no error handlung, you see the result on the screen.
	ResetSkybox ();
	LoadSkybox ();

	// Load light conditions.
	ResetFog ();
	ResetLight ();
	LoadLight ();
	return true;
}

size_t CEnvironment::GetEnvIdx (const string& tag) const {
	return EnvIndex.at(tag);
}

size_t CEnvironment::GetLightIdx (const string& tag) const {
	return LightIndex.at(tag);
}
