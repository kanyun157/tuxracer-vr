# [Extreme Tuxracer](http://sourceforge.net/projects/extremetuxracer/) VR 

The classic downhill winter game now with Oculus Rift support!  Please have patience and read the following to get it running acceptably (low latency).  WARNING: A decent gaming graphics card is necessary!  Integrated graphics hardware, eg. most laptops, won't be fast enough to create the proper feeling of presence in VR applications.

## Installation

### Linux:
* Please compile from source on linux (for now):
* Dependencies: sdl2 sdl2-mixer sdl2-image ftgl freetype glew git gcc
* Download the oculus rift sdk 0.4.4 (0.5 not supported yet), extract somewhere, and compile it:
 * https://developer.oculus.com/downloads/#version=pc-0.4.4-beta
* Download and compile tuxracer vr:

 ```
git clone https://github.com/jdtaylor/tuxracer-vr.git
cd tuxracer-vr
export OVR_ROOT="/path/to/your/oculus/ovr_sdk_linux_0.4.4"
# TODO: test w/ -Ofast -fomit-frame-pointer -march=native
CXXFLAGS="-O3" ./configure --datadir=${PWD}/data
make 
 ```

### Windows:
* ... TODO

### OSX:
* ... TODO

## Running:
* Don't rotate the rift if using extended mode. Keep it in its default portrait mode.
* On Linux, the Oculus SDK only supports extended mode.. whether using Xinerama or as separate displays via the DISPLAY environment variable.
* Please make sure your oculusd or ovrd service is running and issue ./etr from the build directory.
* If in extended mode, use the F9 key to toggle between the Rift and back.  Direct-to-rift should "Just Work" (no need for F9 key) and mirror to both displays.
 * I'm getting 75 fps with xinerama and the open source xf86-video-ati drivers
 * If you get poor <75 fps performance, try disabling vsync in the configuration file.
  * To disable vsync, in the etr/options file change '[no_vsync] 0' to '[no_vsync] 1'
* Sometimes if the rift is turned off and back on while Xorg is running, judder starts and I havn't found a way to make it go away without an X shutdown and restart w/ the Rift on.
* I've seen good results with the following in ~/.etr/options for the open source Radeon driver.

```
[fullscreen] 0
[detail_level] 1
[forward_clip_distance] 80
[backward_clip_distance] 30
[tree_detail_distance] 30
[tux_sphere_divisions] 1
[tux_shadow_sphere_div] 1
[course_detail_level] 75
[full_skybox] 1
[no_vsync] 0
[no_prediction] 0
[no_timewarp] 1
[no_timewarp_spinwaits] 1
[no_hq_distortion] 0
[no_compute_shader] 1
```

## Need Help!
* Translations are behind.  Spanish, etc.  See data/translations for language files.

## Contact:
* IRC: I'm 'metric' on #etuxracer (Freenode)
* Github: http://github.com/jdtaylor
* Email: james.d.taylor@gmail.com
* My public key can be found at http://jdtaylor.org

## Donations:
* If you like what you see and would like to support more Free Software for VR; please send bitcoin: 12ptLNTGD16itaG9mXQxYRaExwXr3aVyFd

