# [Extreme Tuxracer](http://sourceforge.net/projects/extremetuxracer/) VR 

The classic downhill snow "skiing" game now with Oculus Rift support!  Please have patience and read the following to get it running acceptably (low latency).  WARNING: A decent gaming graphics card is necessary!  Integrated graphics hardware won't cut it.

## Install

### Linux:
* Please compile from source on linux:
* Dependencies: sdl2 sdl2-mixer sdl2-image ftgl freetype glew git gcc
* Download the oculus rift sdk 0.4.4 (0.5 not supported yet), extract somewhere, and compile it:
 * https://developer.oculus.com/downloads/#version=pc-0.4.4-beta
* Download and compile tuxracer vr:

 ```
git clone https://github.com/jdtaylor/tuxracer-vr.git
cd tuxracer-vr
export OVR_ROOT="/path/to/your/oculus/ovr_sdk_linux_0.4.4"
CXXFLAGS="-O3" ./configure --datadir=${PWD}/data
make
 ```

### Windows:
* ... TODO

### OSX:
* ... TODO

## Running:
* Please make sure your oculusd or ovrd is running and issue ./etr to start.
* Use the F2 key to toggle the window to the Rift and back.
* Don't rotate the rift.. keep it in its default portrait mode.
* On Linux, the Oculus SDK only supports extended mode.. whether using Xinerama or as separate displays via the DISPLAY environment variable.
 * I'm getting 75 fps with xinerama and the open source xf86-video-ati drivers
  * No mirroring which happens automatically on Windows with direct to rift mode.
 * I have to turn off vsync to get anything over half of 75 fps (37.5) for some reason.  
 * To turn off vsync append this option to the etr config file: [no_vsync] 1
* Sometimes if the rift is turned off and back on while Xorg is running, judder starts and I havn't found a way to make it go away without an X shutdown and restart w/ the Rift on.
* I've seen good results with the following in ~/.etr/options for the open source Radeon driver.
* Hit F2 once the game starts to move the window to the Rift.

```
[fullscreen] 0
[detail_level] 1
[forward_clip_distance] 80
[backward_clip_distance] 30
[tree_detail_distance] 30
[tux_sphere_divisions] 0
[tux_shadow_sphere_div] 0
[course_detail_level] 75
[full_skybox] 1
[no_vsync] 0
[no_prediction] 0
[no_timewarp] 1
[no_timewarp_spinwaits] 1
[no_hq_distortion] 0
[no_compute_shader] 1
```

