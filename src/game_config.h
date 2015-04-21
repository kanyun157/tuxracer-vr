/* --------------------------------------------------------------------
EXTREME TUXRACER

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

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "bh.h"

#define CONFIG_DIR ".etr"
#define PLAYER_FILE "players"

using namespace std;

struct TParam {
	// defined at runtime:
  //	string	prog_dir;
	string	config_dir;
    string	data_dir;
    string	common_course_dir;
    string	obj_dir;
    string	terr_dir;
    string	char_dir;
	string  env_dir2;
    string	tex_dir;
	string	sounds_dir;
	string  music_dir;
	string	screenshot_dir;
	string	font_dir;
	string  trans_dir;
	string  player_dir;
	string  configfile;

	// ------------------------------------
	// main config params:
	bool	fullscreen;
	size_t	res_type;
	int		perf_level;
	size_t	language;
    int		sound_volume;
    int		music_volume;

    int		forward_clip_distance;
    int		backward_clip_distance;
    int		fov;
    int		bpp_mode;
    int		tree_detail_distance;
    int		tux_sphere_divisions;
    int		tux_shadow_sphere_divisions;
    int		course_detail_level; // only for quadtree
	int		audio_freq;
	int		audio_buffer_size;

	int		use_papercut_font;
	bool	ice_cursor;
	bool	full_skybox;
	bool	no_skybox;
	bool	use_quad_scale;			// scaling type for menus

	string  menu_music;
	string  credits_music;
	string  config_music;

	// these params are not saved in options file
    bool	ui_snow;
    bool	display_fps;
	bool	show_hud;
	bool	attach_hud_to_face;

	bool    no_vsync;
	bool    no_prediction;
	bool    no_timewarp;
	bool    no_timewarp_spinwaits;
	bool    no_hq_distortion;
	bool    no_compute_shader;
	bool    no_restore;
	bool    console_dump;
	bool    use_fxaa;
	bool    quick_mode; // switches between event/practice
	float   ipd_multiplier;
	float   player_min_speed;
	float   player_frict_speed;
	float   player_height;
	float   camera_distance;
	float   camera_angle;
	float   fly_amount;
	float   quick_ipd_multiplier; // I needed two modes of play for 'event' achievements.
	float   quick_player_min_speed;
	float   quick_player_frict_speed; // speed at which friction kicks in.
	float   quick_camera_distance;
	float   quick_camera_angle;
	float   quick_fly_amount;
	float   event_ipd_multiplier; // slower params for gameplay.
	float   event_player_min_speed;
	float   event_player_frict_speed;
	float   event_camera_distance;
	float   event_camera_angle;
	float   event_fly_amount;
	bool    dire_straits_tux; // tux rendered w/ cubes.

    TViewMode view_mode;
};

void InitConfig (const char *arg0);
void SaveConfigFile ();

extern TParam param;

#endif
