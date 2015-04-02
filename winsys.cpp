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

#include "winsys.h"
#include "ogl.h"
#include "audio.h"
#include "game_ctrl.h"
#include "font.h"
#include "score.h"
#include "textures.h"
#include "spx.h"
#include "course.h"
#include "states.h"
#include "SDL2/SDL_syswm.h"
#include <iostream>

#define USE_JOYSTICK true

TVector2 cursor_pos(0, 0);

CWinsys Winsys;

CWinsys::CWinsys ()
	: auto_resolution(800, 600)
{
	sdlWindow = NULL;

	joystick = NULL;
	numJoysticks = 0;
	joystick_active = false;

	resolutions[0] = TScreenRes(0, 0);
	resolutions[1] = TScreenRes(800, 600);
	resolutions[2] = TScreenRes(1024, 768);
	resolutions[3] = TScreenRes(1152, 864);
	resolutions[4] = TScreenRes(1280, 960);
	resolutions[5] = TScreenRes(1280, 1024);
	resolutions[6] = TScreenRes(1360, 768);
	resolutions[7] = TScreenRes(1400, 1050);
	resolutions[8] = TScreenRes(1440, 900);
	resolutions[9] = TScreenRes(1680, 1050);
	resolutions[10] = TScreenRes(1920, 1080); // rift dk2

	frame_index = 0;
}

const TScreenRes& CWinsys::GetResolution (size_t idx) const {
	if (idx == 0 || idx >= NUM_RESOLUTIONS) return auto_resolution;
	return resolutions[idx];
}

string CWinsys::GetResName (size_t idx) const {
	if (idx >= NUM_RESOLUTIONS) return "800 x 600";
	if (idx == 0) return ("auto");
	string line = Int_StrN (resolutions[idx].width);
	line += " x " + Int_StrN (resolutions[idx].height);
	return line;
}

double CWinsys::CalcScreenScale () const {
	if (resolution.height < 768) return 0.78;
	else if (resolution.height == 768) return 1.0;
	else return (resolution.height / 768);
}

void CWinsys::SetupVideoMode (const TScreenRes& res) {
    resolution = res;
	scale = CalcScreenScale ();
	if (param.use_quad_scale) scale = sqrt (scale);

	printf("resizing SDL window to res: %dx%d\n", resolution.width, resolution.height);
    SDL_SetWindowSize(sdlWindow, resolution.width, resolution.height);
    SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    Reshape (resolution.width, resolution.height); // does nothing
}

void CWinsys::SetupVideoMode (size_t idx) {
	SetupVideoMode (GetResolution(idx));
}

void CWinsys::SetupVideoMode (int width, int height) {
	SetupVideoMode (TScreenRes(width, height));
}

void CWinsys::InitJoystick () {
    if (SDL_InitSubSystem (SDL_INIT_JOYSTICK) < 0) {
		Message ("Could not initialize SDL_joystick: %s", SDL_GetError());
		return;
	}
	numJoysticks = SDL_NumJoysticks ();
	if (numJoysticks < 1) {
		joystick = NULL;
		return;
	}
	SDL_JoystickEventState (SDL_ENABLE);
	joystick = SDL_JoystickOpen (0);	// first stick with number 0
    if (joystick == NULL) {
		Message ("Cannot open joystick %s", SDL_GetError ());
		return;
    }
	joystick_active = true;
}

void CWinsys::OvrConfigureRendering()
{
	// enable position and rotation tracking
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
	// retrieve the optimal render target resolution for each eye
	// jdt: I tried reducing pixelsPerDisplayPixel from 1.0 for performance but 
	// we seem to be CPU bound on the quadtree and actual trees.. setting back to 1
	eyeres[0] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0);
	eyeres[1] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0);

	// Set etr fov to that of the Oculus Rift
	float fovTan0 = max(hmd->DefaultEyeFov[0].LeftTan, hmd->DefaultEyeFov[0].RightTan);
	float fovTan1 = max(hmd->DefaultEyeFov[1].LeftTan, hmd->DefaultEyeFov[1].RightTan);
	param.fov = floor(abs(2.0f * atan(max(fovTan0, fovTan1)) * (180.0f / M_PI)));
	//jdt TODO param.fov -= 10; // account for overlap in fov of both eyes.  fudge..
	// I'm seeing 95 degrees here.. doesn't look good to me.
	printf("Detected HMD FOV of %d.  Overriding etr fov with it.\n", param.fov);

	// and create a single render target texture to encompass both eyes 
	// jdt: fb_width, etc declared in ogl.h
	fb_width = eyeres[0].w + eyeres[1].w;
	fb_height = eyeres[0].h > eyeres[1].h ? eyeres[0].h : eyeres[1].h;
	printf("fb_width: %d\n", fb_width);
	printf("fb_height: %d\n", fb_height);

	fbo = fb_tex = fb_depth = 0;
	UpdateRenderTarget(fb_width, fb_height);

	printf("fb_tex_width: %d\n", fb_tex_width); // firmware major: 2
	printf("fb_tex_height: %d\n", fb_tex_height); // minor: 12
	printf("fb_tex: %d\n", fb_tex);

	// fill in the ovrGLTexture structures that describe our render target texture
	for(int i=0; i<2; i++) {
		fb_ovr_tex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
		fb_ovr_tex[i].OGL.Header.TextureSize.w = fb_tex_width;
		fb_ovr_tex[i].OGL.Header.TextureSize.h = fb_tex_height;
		// this next field is the only one that differs between the two eyes
		fb_ovr_tex[i].OGL.Header.RenderViewport.Pos.x = i == 0 ? 0 : fb_width / 2.0;
		fb_ovr_tex[i].OGL.Header.RenderViewport.Pos.y = 0;
		fb_ovr_tex[i].OGL.Header.RenderViewport.Size.w = fb_width / 2.0;
		fb_ovr_tex[i].OGL.Header.RenderViewport.Size.h = fb_height;
		fb_ovr_tex[i].OGL.TexId = fb_tex;	// both eyes will use the same texture id 
	}

	// fill in the ovrGLConfig structure needed by the SDK to draw our stereo pair
	// to the actual HMD display (SDK-distortion mode)
	memset(&glcfg, 0, sizeof glcfg);
	glcfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	glcfg.OGL.Header.BackBufferSize.w = hmd->Resolution.w;
	glcfg.OGL.Header.BackBufferSize.h = hmd->Resolution.h;
	glcfg.OGL.Header.Multisample = 1;

#ifdef WIN32
	glcfg.OGL.Window = GetActiveWindow();
	glcfg.OGL.DC = wglGetCurrentDC();
#else
	glcfg.OGL.Disp = glXGetCurrentDisplay();
#endif

	if(hmd->HmdCaps & ovrHmdCap_ExtendDesktop) {
		printf("Running in \"extended desktop\" mode\n");
	} else {
		// to sucessfully draw to the HMD display in "direct-hmd" mode, we have to
		// call ovrHmd_AttachToWindow
		// XXX: this doesn't work properly yet due to bugs in the oculus 0.4.1 sdk/driver
		//
#ifdef WIN32
		ovrHmd_AttachToWindow(hmd, glcfg.OGL.Window, 0, 0);
#else
		ovrHmd_AttachToWindow(hmd, (void*)glXGetCurrentDrawable(), 0, 0); // 0.4.4
#endif
		printf("Running in \"direct-hmd\" mode.  Or maybe oculusd isn't running..?\n");
#ifndef WIN32
		printf("RUN oculusd FIRST!! Take this out if Oculus fixes direct mode on Linux\n");
		SDL_Quit();
#endif
	}

	// enable low-persistence display and dynamic prediction for latency compensation
	hmd_caps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
	hmd_caps |= ovrHmdCap_NoVSync; // This.. for whatever reason "solves" the low 37.5 fps problem.
	// well.. now I don't have a 37.5 fps problem after updating mesa,xorg-server and restarting X...
	ovrHmd_SetEnabledCaps(hmd, hmd_caps);

	printf("New HMD capabilities set.\n");

	// configure SDK-rendering and enable chromatic abberation correction, vignetting, and
	// timewrap, which shifts the image before drawing to counter any latency between the call
	// to ovrHmd_GetEyePose and ovrHmd_EndFrame.
	//
	
	distort_caps = ovrDistortionCap_Overdrive;
#if OVR_MAJOR_VERSION < 5
	distort_caps |= ovrDistortionCap_Chromatic;
#endif
	distort_caps |= ovrDistortionCap_Vignette;
	distort_caps |= ovrDistortionCap_TimeWarp;
#ifdef WIN32
	//distort_caps |= ovrDistortionCap_ComputeShader; // #ifdef'd out in the sdk for linux
#endif
#if OVR_MAJOR_VERSION < 5
	distort_caps |= ovrDistortionCap_ProfileNoTimewarpSpinWaits;
#endif
	distort_caps |= ovrDistortionCap_HqDistortion;
	if(!ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc)) {
		fprintf(stderr, "failed to configure renderer for Oculus SDK\n");
	}

	ovrHmd_DismissHSWDisplay(hmd);
}

void CWinsys::Init () {

	// jdt: oculus init needs better home
	ovr_Initialize();

	Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | SDL_INIT_TIMER;
    if (SDL_Init (sdl_flags) < 0) Message ("Could not initialize SDL");

	// requiring anything higher than OpenGL 3.0 causes deprecation of 
	// GL_LIGHTING GL_LIGHT0 GL_NORMALIZE, etc.. need replacements.
    // also deprecates immediate mode, which would be a complete overhaul.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	#if defined (USE_STENCIL_BUFFER)
	    SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, 8);
	#endif

	resolution = GetResolution (param.res_type);
	Uint32 window_width = resolution.width;
	Uint32 window_height = resolution.height;
	Uint32 window_flags = SDL_WINDOW_OPENGL;
	if (param.fullscreen) {
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		window_width = window_height = 0; // don't switch display mode.
	}

	sdlWindow = SDL_CreateWindow(WINDOW_TITLE, 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		window_width, window_height, window_flags);
	if (sdlWindow == NULL) {
		Message("Failed to create window: ", SDL_GetError());
		SDL_Quit();
	}

	// Create an opengl context instead of an sdl renderer.
	glContext = SDL_GL_CreateContext(sdlWindow);
	if (!glContext) {
		Message ("Couldn't initialize OpenGL context: ",  SDL_GetError());
		SDL_Quit();
	}
	SDL_GL_MakeCurrent(sdlWindow, glContext); // jdt: probably not necessary

	// Initialize opengl extension wrangling lib for Frame Buffer Object support (rift)
	glewExperimental = GL_TRUE; // jdt: probably not necessary
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		Message ("Failed to initialize GLEW library: ", (char*)glewGetErrorString(err));
		SDL_Quit();
	}
	Message ("Status: Using GLEW: ", (char*)glewGetString(GLEW_VERSION));
	printf("Setting up video mode with res: %ux%u\n", resolution.width, resolution.height);

	if (!(hmd = ovrHmd_Create(0))) {
		Message ("No Oculus Rift device found.  Creating fake DK2.");
		if(!(hmd = ovrHmd_CreateDebug(ovrHmd_DK2))) {
    		Message("failed to create virtual debug HMD");
			SDL_Quit();
		}
	}
	printf("initialized HMD: %s - %s\n", hmd->Manufacturer, hmd->ProductName);
	printf("\tdisplay resolution: %dx%d\n", hmd->Resolution.w, hmd->Resolution.h);
	printf("\tdisplay position: %d,%d\n", hmd->WindowsPos.x, hmd->WindowsPos.y);

	SetupVideoMode (hmd->Resolution.w, hmd->Resolution.h);

	OvrConfigureRendering();

	KeyRepeat (false);
	if (USE_JOYSTICK) InitJoystick ();
//	SDL_EnableUNICODE (1);
}

void CWinsys::KeyRepeat (bool repeat) {
	// jdt
	//if (repeat)
	//	SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	//else SDL_EnableKeyRepeat (0, 0);
}

void CWinsys::SetFonttype () {
	if (param.use_papercut_font > 0) {
		FT.SetFont ("pc20");
	} else {
		FT.SetFont ("bold");
	}
}

void CWinsys::CloseJoystick () {
	if (joystick_active) SDL_JoystickClose (joystick);
}

void CWinsys::Quit () {
	CloseJoystick ();
	Score.SaveHighScore ();
	SaveMessages ();
	Audio.Close ();		// frees music and sound as well
	FT.Clear ();
	if (g_game.argument < 1) Players.SavePlayers ();
	ovrHmd_Destroy(hmd);
	ovr_Shutdown();
	SDL_Quit ();
}

void CWinsys::Terminate () {
	Quit();
	exit(0);
}

void CWinsys::PrintJoystickInfo () const {
	if (joystick_active == false) {
		Message ("No joystick found");
		return;
	}
	PrintStr ("");
	PrintStr (SDL_JoystickName (0));
	int num_buttons = SDL_JoystickNumButtons (joystick);
	cout << "Joystick has " << num_buttons << " button" << (num_buttons == 1 ? "" : "s") << '\n';
	int num_axes = SDL_JoystickNumAxes (joystick);
	cout << "Joystick has " << num_axes << " ax" << (num_axes == 1 ? "i" : "e") << "s\n\n";
}

void CWinsys::ToggleHmdFullscreen()
{
	static int fullscr, prev_x, prev_y;
	fullscr = !fullscr;

	if(fullscr) {
		//
		// going fullscreen on the rift. save current window position, and move it
		// to the rift's part of the desktop before going fullscreen
		//
		SDL_GetWindowPosition(sdlWindow, &prev_x, &prev_y);
		printf("Going fullscreen to Rift:\n");
		printf("\tprev window position: %d,%d\n", prev_x, prev_y);
		printf("\thmd window position: %d,%d\n", hmd->WindowsPos.x, hmd->WindowsPos.y);
		SDL_SetWindowPosition(sdlWindow, hmd->WindowsPos.x, hmd->WindowsPos.y);
		SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);

#ifdef OVR_OS_LINUX
		// on linux for now we have to deal with screen rotation during rendering. The docs are promoting
		// not rotating the DK2 screen globally
		//
		glcfg.OGL.Header.BackBufferSize.w = hmd->Resolution.h; // >= 0.4.4
		glcfg.OGL.Header.BackBufferSize.h = hmd->Resolution.w;
		printf("\tSwapping window resolution to: %dx%d\n", hmd->Resolution.h, hmd->Resolution.w);

		distort_caps |= ovrDistortionCap_LinuxDevFullscreen;
		ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc);
#endif
	} else {
		// return to windowed mode and move the window back to its original position
		SDL_SetWindowFullscreen(sdlWindow, 0);
		SDL_SetWindowPosition(sdlWindow, prev_x, prev_y);

#ifdef OVR_OS_LINUX
		glcfg.OGL.Header.BackBufferSize = hmd->Resolution;

		distort_caps &= ~ovrDistortionCap_LinuxDevFullscreen;
		ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc);
#endif
	}
}

const int maxFrames = 50;
static int numFrames = 0;
static float averagefps = 0;
static float sumTime = 0;

void dump_fps()
{
    if (numFrames >= maxFrames) {
		averagefps = 1 / sumTime * maxFrames;
		numFrames = 0;
		sumTime = 0;
		printf("FPS: %.3f\n", averagefps);
	} else {
		sumTime += g_game.time_step;
		numFrames++;
	}
}

static TVector3 lookAtPrevPos[2];
static TVector3 lookAtPos[2];
static GLfloat lookAtDepth[2];
static GLfloat lookAtRgb[2][3];

void LookAtSelection(ovrEyeType eye)
{
	int idx = eye == ovrEye_Left ? 0 : 1;

	SetupDisplay (eye);
	SetupGuiDisplay ();

	GLdouble modelview[16];
	GLdouble projection[16];
	GLint viewport[4];

	glGetDoublev (GL_PROJECTION_MATRIX, projection);
	glGetDoublev (GL_MODELVIEW_MATRIX, modelview);
	glGetIntegerv (GL_VIEWPORT, viewport);

	// viewport is that of the framebuffer size.. not 1920x1080
	TVector3 center(viewport[2]/2.f + viewport[0], viewport[3]/2.f + viewport[1], 0);

	GLfloat depth;
	glReadPixels((int)center.x, (int)center.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth); 
	lookAtDepth[idx] = depth;

	/*
	GLfloat rgb[3];
	glReadPixels((int)center.x, (int)center.y, 1, 1, GL_RGB, GL_FLOAT, &rgb); 
	lookAtRgb[eye][0] = rgb[0];
	lookAtRgb[eye][1] = rgb[1];
	lookAtRgb[eye][2] = rgb[2];
	*/

	GLdouble fx, fy, fz; 
	if (depth == 1.0 ||  
		gluUnProject (center.x, center.y, (GLdouble)depth, modelview, projection, viewport, &fx, &fy, &fz) != GL_TRUE)
	{
		fx = fy = fz = 0.0;
	}

	lookAtPrevPos[idx] = lookAtPos[idx];
	lookAtPos[idx] = TVector3(fx, fy, fz);
}


void CWinsys::RenderFrame(State *current)
{
    ovrHmd_BeginFrame(hmd, frame_index);

    ovrVector3f eye_view_offsets[2] = { eye_rdesc[0].HmdToEyeViewOffset,
        eye_rdesc[1].HmdToEyeViewOffset };
    ovrHmd_GetEyePoses(hmd, frame_index, eye_view_offsets, eyePose, &trackingState);
    frame_index++;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Set modelview to "cyclops" mode for non-opengl geometry culling (UpdateCourse).
    // jdt: we just use the left eye for now. (might want to average values for both)
    SetupDisplay (ovrEye_Left);

    glNewList(stereo_gl_list, GL_COMPILE);
    current->Loop(g_game.time_step);
    glEndList();

    ClearDisplay(); // was.. ClearRenderContext ();

    for (int i = 0; i < 2; ++i)
    {
        ovrEyeType eye = hmd->EyeRenderOrder[i];

        if (eye == ovrEye_Left) {
            glViewport(0, 0, fb_width/2, fb_height);
        } else {
            glViewport(fb_width/2, 0, fb_width/2, fb_height);
        }

        SetupDisplay (eye);

        glCallList(stereo_gl_list);

        LookAtSelection (eye);
    }

	// jdt: todo only do if in GUI state.
	// jdt: fx and fy should now be the x,y location of the widget we are looking at.
	float eps = 100.f;
	if (abs(lookAtPos[0].x) > 0 && abs(lookAtPos[0].y) > 0) {
		if (abs(lookAtPos[0].x - lookAtPos[1].x) < eps && abs(lookAtPos[0].y - lookAtPos[1].y) < eps) {
			glColor4f (1.0, 0.0, 0.0, 1.0);
			glRectd (lookAtPos[0].x - 5, lookAtPos[0].y - 5, lookAtPos[0].x + 5, lookAtPos[0].y + 5);
			current->Motion(lookAtPos[0].x - lookAtPrevPos[0].x, lookAtPos[0].y - lookAtPrevPos[0].y);
		}
	}


    // after drawing both eyes into the texture render target, revert to drawing directly to the
    // display, and we call ovrHmd_EndFrame, to let the Oculus SDK draw both images properly
    // compensated for lens distortion and chromatic abberation onto the HMD screen.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ovrHmd_EndFrame(hmd, eyePose, &fb_ovr_tex[0].Texture);



    dump_fps();
}



