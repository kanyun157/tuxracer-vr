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

#include "game_over.h"
#include "audio.h"
#include "ogl.h"
#include "view.h"
#include "course_render.h"
#include "env.h"
#include "hud.h"
#include "track_marks.h"
#include "particles.h"
#include "gui.h"
#include "font.h"
#include "spx.h"
#include "game_ctrl.h"
#include "translation.h"
#include "score.h"
#include "race_select.h"
#include "event.h"
#include "winsys.h"
#include "physics.h"
#include "loading.h"
#include "course.h"

CGameOver GameOver;

static CKeyframe *final_frame;
static int highscore_pos = 999;
static TTextButton* backButton;
static TTextButton* retryButton;
static TTextButton* continueButton;
static TTextField* hintText;

void QuitGameOver () {
	if (g_game.game_type == PRACTICING) {
		State::manager.RequestEnterState (RaceSelect);
	} else {
		State::manager.RequestEnterState (Event);
	}
}

void CGameOver::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	if (key == 13 || key == SDLK_ESCAPE) QuitGameOver ();
}

void CGameOver::Mouse (int button, int state, int x, int y) {
	if (state == 1) {
		TWidget* focussed = ClickGUI(x, y);
		if (focussed == backButton) {
			QuitGameOver ();
		} else if (focussed == retryButton) {
            CourseRenderReset ();
			State::manager.RequestEnterState (Loading);
		} else if (focussed == continueButton) {
            if (Event.isLastRace()) {
                State::manager.RequestEnterState (Event);
            } else {
                Event.UpdateCupRacing();
                Event.StartRace();
            }
		}
	}
}

void CGameOver::Motion (int x, int y) {
	MouseMoveGUI(x, y);
}

void DrawMessageFrame (float x, float y, float w, float h, int line,
		TColor backcol, TColor framecol, float transp) {

	float yy = Winsys.resolution.height - y - h;
	if (x < 0) 	x = (Winsys.resolution.width - w) / 2;

	glPushMatrix();
	glDisable (GL_TEXTURE_2D);

	glColor4f (framecol.r, framecol.g, framecol.b, transp);
	glTranslatef (x, yy, 0);
	glBegin (GL_QUADS);
	    glVertex2f (0, 0);
	    glVertex2f (w, 0);
	    glVertex2f (w, h);
	    glVertex2f (0, h);
	glEnd();

	glColor4f (backcol.r, backcol.g, backcol.b, transp);
	glBegin (GL_QUADS);
	    glVertex2f (0 + line, 0 + line);
	    glVertex2f (w - line, 0 + line);
	    glVertex2f (w - line, h - line);
	    glVertex2f (0 + line, h - line);
	glEnd();

	glEnable (GL_TEXTURE_2D);
    glPopMatrix();
}


void GameOverMessage (const CControl *ctrl) {
	int fwidth = 500;

	float leftframe = (Winsys.resolution.width - fwidth) / 2;
	float topframe = 80;

	const TColor& backcol = colWhite;
	static const TColor framecol(0.7, 0.7, 1, 1);

	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
	if (g_game.raceaborted) {
		DrawMessageFrame (leftframe, topframe, fwidth, 100, 4, backcol, framecol, 0.5);
		FT.SetColor (colDBlue);
		FT.DrawString (CENTER, topframe+30, Trans.Text(25));
	} else {
		DrawMessageFrame (leftframe, topframe, fwidth, 210, 4, backcol, framecol, 0.5);

		if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (14);
		if (g_game.race_result >= 0 || g_game.game_type != CUPRACING) FT.SetColor (colDBlue);
			else FT.SetColor (colDRed);

		string line = "Score:  ";
		FT.DrawString (leftframe+80, topframe+15, line);
		line = Int_StrN (g_game.score);
		line += "  pts";
		FT.DrawString (leftframe+240, topframe+15, line);

		line = "Herring:  ";
		FT.DrawString (leftframe+80, topframe+40, line);
		line = Int_StrN (g_game.herring);
		if (g_game.game_type == CUPRACING) {
			line += "  (";
			line += Int_StrN (g_game.herring_req.i);
			line += ")";
		}
		FT.DrawString (leftframe+240, topframe+40, line);

		line = "Time:  ";
		FT.DrawString (leftframe+80, topframe+65, line);
		line = Float_StrN (g_game.time, 2);
		line += "  s";
		if (g_game.game_type == CUPRACING) {
			line += "  (";
			line += Float_StrN (g_game.time_req.x, 2);
			line += ")";
		}
		FT.DrawString (leftframe+240, topframe+65, line);

		line = "Path length:  ";
		FT.DrawString (leftframe+80, topframe+90, line);
		line = Float_StrN (ctrl->way, 2);
		line += "  m";
		FT.DrawString (leftframe+240, topframe+90, line);

		line = "Average speed:  ";
		FT.DrawString (leftframe+80, topframe+115, line);
		line = Float_StrN (ctrl->way / g_game.time * 3.6, 2);
		line += "  km/h";
		FT.DrawString (leftframe+240, topframe+115, line);

		if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
		if (g_game.game_type == CUPRACING) {
			switch (g_game.race_result) {
				case -1: FT.DrawString (CENTER, topframe+150, Trans.Text(21)); break;
				case 0: FT.DrawString (CENTER, topframe+150, Trans.Text(22)); break;
				case 1: FT.DrawString (CENTER, topframe+150, Trans.Text(23)); break;
				case 2: FT.DrawString (CENTER, topframe+150,  Trans.Text(24)); break;
			}
		} else {
			if (highscore_pos < 5) {
				line = "Position ";
				line += Int_StrN (highscore_pos + 1);
				line += " in highscore list";
				FT.DrawString (CENTER, topframe+150, line);
			}
		}
	}
}

// =========================================================================
void CGameOver::Enter() {
	Sound.HaltAll ();
	ResetGUI ();
    CourseRenderReset ();

	if (!g_game.raceaborted) highscore_pos = Score.CalcRaceResult ();

	if (g_game.game_type == CUPRACING) {
		if (g_game.race_result >= 0) {
			Music.PlayTheme (g_game.theme_id, MUS_WONRACE);
		} else {
			Music.PlayTheme (g_game.theme_id, MUS_LOSTRACE);
		}
	} else {
		if (g_game.raceaborted) {
			Music.PlayTheme (g_game.theme_id, MUS_LOSTRACE);
		} else {
			Music.PlayTheme (g_game.theme_id, MUS_WONRACE);
		}
	}


	if (g_game.raceaborted || !g_game.use_keyframe) {
		final_frame = NULL;
	} else {
		if (g_game.game_type == CUPRACING) {
			if (g_game.race_result < 0) final_frame =
				Char.GetKeyframe (g_game.char_id, LOSTRACE);
				else final_frame = Char.GetKeyframe (g_game.char_id, WONRACE);
		} else final_frame = Char.GetKeyframe (g_game.char_id, FINISH);

		if (!g_game.raceaborted) {
			const CControl *ctrl = Players.GetCtrl (g_game.player_id);
			final_frame->Init (ctrl->cpos, -0.18);
		}
	}

	int top = AutoYPosN (30);
	int siz = FT.AutoSizeN (10);
	backButton = AddTextButton (Trans.Text(8), CENTER, top, siz);
	if (g_game.game_type != CUPRACING || g_game.race_result < 0)
    {
		string retryTxt;
		if (param.language == 0 || param.language == Trans.GetLangIdx("de_DE"))
			retryTxt = Trans.Text(84); // Retry / Race Again
		else
			retryTxt = Trans.Text(13); // Race!
		retryButton = AddTextButton (retryTxt, CENTER, top + 75, siz);
	}

    if (!g_game.raceaborted && g_game.game_type == CUPRACING) {
        if (g_game.race_result >= 0) {
            // quick next-level button for ppl in a hurry
            continueButton = AddTextButton(Trans.Text(9), CENTER, top + 150, siz);
        } else {
            string hintStr;
            // provide hints for failed runs
            if (g_game.time > g_game.time_req.x) {
                if (Course.GetCurrCourse()->name != "Bumpy Ride") {
                    hintStr = "Tip: Slide on ice for faster speeds.";
                } else {
                    hintStr = "Tip: Skip some herrings for quicker times.";
                }
            } else {
                hintStr = "Tip: Lean back to slow down and collect more herring.";
            }

            int frameWidth = FT.AutoSizeN (hintStr.size()) * Winsys.scale * 2;
            int frameHeight = 50 * Winsys.scale;
            hintText = AddTextField(hintStr, CENTER, top + 200, frameWidth, frameHeight);
        }
    }
}


void CGameOver::Loop(double time_step) {
	CControl *ctrl = Players.GetCtrl (g_game.player_id);
	int width, height;
	width = Winsys.resolution.width;
	height = Winsys.resolution.height;

	check_gl_error();

	Music.Update ();

	ClearRenderContext ();
	Env.SetupFog ();

	glPushMatrix();
	update_view (ctrl, 0);

	if (final_frame != NULL) final_frame->Update (time_step);

	SetupViewFrustum (ctrl);
	Env.DrawSkyboxRacing (ctrl->viewpos);
	Env.DrawFog ();
	Env.SetupLight ();

	RenderCourse ();
	DrawTrackmarks ();
	DrawTrees ();

	UpdateWind (time_step);
	UpdateSnow (time_step, ctrl);
	DrawSnow (ctrl);

	Char.Draw (g_game.char_id);

	glPopMatrix();
	ScopedRenderMode rm(GUI);

	if (final_frame == NULL || !final_frame->active) {
		SetupGuiDisplay ();
		GameOverMessage (ctrl);
		DrawGUI ();
	} else {
		if (param.show_hud) {
			SetupHudDisplay ();
			DrawHud (ctrl);
		}
	}

	Reshape (width, height);
	Winsys.SwapBuffers ();
}
