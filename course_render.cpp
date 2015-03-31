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

#include <list>
#include <algorithm>

#include "textures.h"
#include "course_render.h"
#include "course.h"
#include "ogl.h"
#include "quadtree.h"
#include "particles.h"
#include "env.h"
#include "game_ctrl.h"
#include "physics.h"
#include "etr_types.h"

#define TEX_SCALE 6
static const bool clip_course = true;

void setup_course_tex_gen () {
    static GLfloat xplane[4] = {1.0 / TEX_SCALE, 0.0, 0.0, 0.0 };
    static GLfloat zplane[4] = {0.0, 0.0, 1.0 / TEX_SCALE, 0.0 };
    glTexGenfv (GL_S, GL_OBJECT_PLANE, xplane);
    glTexGenfv (GL_T, GL_OBJECT_PLANE, zplane);
}

// --------------------------------------------------------------------
//							render course
// --------------------------------------------------------------------
void RenderCourse () {
	ScopedRenderMode rm(COURSE);
    setup_course_tex_gen ();
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    set_material (colWhite, colBlack, 1.0);
    const CControl *ctrl = Players.GetCtrl (g_game.player_id);
    UpdateQuadtree (ctrl->viewpos, param.course_detail_level);
    RenderQuadtree ();
}

// --------------------------------------------------------------------
//				DrawTrees
// --------------------------------------------------------------------
void DrawTrees() {
    size_t			tree_type = -1;
    size_t			item_type = -1;
	TObjectType*	object_types = &Course.ObjTypes[0];
	const CControl*	ctrl = Players.GetCtrl (g_game.player_id);

	ScopedRenderMode rm(TREES);
    double fwd_clip_limit = param.forward_clip_distance;
    double bwd_clip_limit = param.backward_clip_distance;

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    set_material (colWhite, colBlack, 1.0);

//	-------------- trees ------------------------
    TCollidable* treeLocs = &Course.CollArr[0];
    size_t numTrees = Course.CollArr.size();

	static list<TCollidable*> treeCache;
	static unsigned int treesDirtyCount = 0; // how many frames since cache of trees was updated.
	static string cachedName = Course.GetCurrCourse()->name;
	if (treesDirtyCount++ % 75 == 0 || !treeCache.size() || cachedName != Course.GetCurrCourse()->name) {
		// update local list of trees every second or so (@75fps)
		treeCache.clear();
		int i = 0;
		while(i < numTrees && treeLocs[i].pt.z > ctrl->viewpos.z + bwd_clip_limit) i++;
		while(i < numTrees && treeLocs[i].pt.z > ctrl->viewpos.z - fwd_clip_limit) {
			treeCache.push_back(&treeLocs[i]);
			i++;
		}
	}

	vector<TCollidable*> trees;
	if (treeCache.size()) {
		// jdt: height isn't quite right here.. should be [0, height], not [-height, height].
		TCollidable *modelTree = *treeCache.begin();
		TVector3 radius(modelTree->diam / 2.0, modelTree->diam / 2.0, modelTree->height);

		for (list<TCollidable*>::iterator i = treeCache.begin(); i != treeCache.end(); ++i) {
			if (clip_aabb_to_view_frustum ((*i)->pt - radius, (*i)->pt + radius) != NotVisible)
				trees.push_back(*i);
		}
	}

	// sort by textures to reduce binds
	std::sort(trees.begin(), trees.end(), [](const TCollidable *a, const TCollidable *b)
			{ return a->tree_type < b->tree_type; });

	for (int i = 0; i < trees.size(); ++i) {
		TCollidable &tree = *trees[i];
		double dx = ctrl->viewpos.x - tree.pt.x;
		double dz = ctrl->viewpos.z - tree.pt.z;
		double distsqr = dx * dx + dz * dz;

		if (tree.tree_type != tree_type) {
		    tree_type = tree.tree_type;
			object_types[tree_type].texture->Bind();
		}

        glPushMatrix();
        glTranslatef (tree.pt.x, tree.pt.y, tree.pt.z);
		if (param.perf_level > 1) glRotatef (1, 0, 1, 0);

        GLfloat treeRadius = tree.diam / 2.0;
        GLfloat treeHeight = tree.height;

		//TVector3 normal(0, 0, 1);
		//glNormal3f (normal.x, normal.y, normal.z);
		// slower but better method of setting the normals
		TVector3 normal = SubtractVectors (ctrl->viewpos, tree.pt);
		NormVector (normal);
		glNormal3f (normal.x, normal.y, normal.z);

		glBegin (GL_QUADS);
			glTexCoord2f (0.0, 0.0);
    	    glVertex3f (-treeRadius, 0.0, 0.0);
    	    glTexCoord2f (1.0, 0.0);
    	    glVertex3f (treeRadius, 0.0, 0.0);
    	    glTexCoord2f (1.0, 1.0);
    	    glVertex3f (treeRadius, treeHeight, 0.0);
    	    glTexCoord2f (0.0, 1.0);
    	    glVertex3f (-treeRadius, treeHeight, 0.0);

			if (/*!clip_course || */distsqr < param.tree_detail_distance * param.tree_detail_distance)
			{
			    glTexCoord2f  (0., 0.);
			    glVertex3f  (0.0, 0.0, -treeRadius);
			    glTexCoord2f  (1., 0.);
			    glVertex3f  (0.0, 0.0, treeRadius);
			    glTexCoord2f  (1., 1.);
			    glVertex3f  (0.0, treeHeight, treeRadius);
			    glTexCoord2f  (0., 1.);
			    glVertex3f  (0.0, treeHeight, -treeRadius);
			} 

		glEnd();

        glPopMatrix();
	}

//  items -----------------------------
	TItem* itemLocs = &Course.NocollArr[0];
	size_t numItems = Course.NocollArr.size();

	vector<TItem*> items;

	if (numItems) {
		TVector3 radius(itemLocs[0].diam / 2.0, itemLocs[0].diam / 2.0, itemLocs[0].height);

		//for (int i = 0; i < numItems; i++) {
		int i = 0;
		while(i < numItems && itemLocs[i].pt.z > ctrl->viewpos.z + bwd_clip_limit) i++;
		while(i < numItems && itemLocs[i].pt.z > ctrl->viewpos.z - fwd_clip_limit) {
			if (itemLocs[i].collectable && itemLocs[i].drawable)
			if (clip_aabb_to_view_frustum (itemLocs[i].pt - radius, itemLocs[i].pt + radius) != NotVisible)
				items.push_back(&itemLocs[i]);
			i++;
		}
	}

	std::sort(items.begin(), items.end(), [](const TItem *a, const TItem *b)
			{ return a->item_type < b->item_type; });

    for (int i = 0; i<items.size(); i++) {
		TItem &item = *items[i];

		if (item.item_type != item_type) {
		    item_type = item.item_type;
			object_types[item_type].texture->Bind();
		}

		glPushMatrix();
		    glTranslatef (item.pt.x, item.pt.y,  item.pt.z);
		    GLfloat itemRadius = item.diam / 2;
		    GLfloat itemHeight = item.height;

			TVector3 normal;

		    if (object_types[item_type].use_normal) {
				normal = object_types[item_type].normal;
		    } else {
//				normal = MakeVector (0, 0, 1);
				normal = SubtractVectors (ctrl->viewpos, item.pt);
		    }
		    NormVector (normal); // jdt: wasn't normalizing before glNormal3f all the time.
		    glNormal3f (normal.x, normal.y, normal.z);
		    normal.y = 0.0;

			// orig 0.6 immediate mode extremely slow:
		    glBegin (GL_QUADS);
				glTexCoord2f (0., 0.);
				glVertex3f (-itemRadius*normal.z, 0.0,  itemRadius*normal.x);
				glTexCoord2f (1., 0.);
				glVertex3f (itemRadius*normal.z, 0.0, -itemRadius*normal.x);
				glTexCoord2f (1., 1.);
				glVertex3f (itemRadius*normal.z, itemHeight, -itemRadius*normal.x);
				glTexCoord2f (0., 1.);
				glVertex3f (-itemRadius*normal.z, itemHeight, itemRadius*normal.x);
	    	glEnd();
        glPopMatrix();
    }
}
