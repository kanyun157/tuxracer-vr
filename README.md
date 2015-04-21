# [Extreme Tuxracer](http://sourceforge.net/projects/extremetuxracer/) VR 

The classic downhill winter game now with Oculus Rift support!  Please have patience and read the following to get it running acceptably (low latency).  WARNING: A decent gaming graphics card is necessary!  Integrated graphics hardware, eg. most laptops, won't be fast enough to create the proper feeling of presence in VR applications.

## Installation

### Windows:
* [TuxRacerVR_v0.62.zip](https://github.com/jdtaylor/tuxracer-vr/releases/download/v0.62/TuxRacerVR_v0.62.zip)
* Make sure the Oculus runtime is installed and running in either extended or direct-to-rift mode.
* Download, unzip, and run exe from the extracted directory.
* If in extended mode, use the F9 key to toggle between the Rift and back.  Direct-to-rift should "Just Work" (no need for F9 key) and mirror to both displays.
* See config/options.txt for performance tweaks if needed (see below).

### Linux:
* Please compile from source on linux (for now):
* Dependencies: sdl2 sdl2-mixer sdl2-image ftgl freetype glew git gcc
* Download the oculus rift sdk 0.4.4 (0.5 not supported yet), extract somewhere, and compile it:
 * https://developer.oculus.com/downloads/#version=pc-0.4.4-beta
* Download and compile tuxracer vr:

 ```
git clone https://github.com/jdtaylor/tuxracer-vr.git
cd tuxracer-vr
export OVR_ROOT="/path/to/your/compiled/oculus/ovr_sdk_linux_0.4.4"
export CXXFLAGS="-Ofast -fomit-frame-pointer -march=native" 
./configure --datadir=${PWD}/data
make 
 ```

* Don't rotate the rift if using extended mode. Keep it in its default portrait mode.
* On Linux, the Oculus SDK only supports extended mode.. whether using Xinerama or as separate displays via the DISPLAY environment variable.
* Please make sure your oculusd or ovrd service is running and issue src/etr from the root build directory.
* Use the F9 key to toggle to the rift and back.
 * If you get poor <75 fps performance, try disabling vsync in the configuration file.
  * To disable vsync, in the ~/.etr/options file change '[no_vsync] 0' to '[no_vsync] 1'
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
[quick_ipd_multiplier] 16
[quick_player_frict_speed] 240
[quick_camera_distance] 9
[quick_camera_angle] -26
[quick_fly_amount] 50
[event_ipd_multiplier] 2
[event_player_frict_speed] 30
[event_camera_distance] 3
[event_camera_angle] -9
[event_fly_amount] 1
[dire_straits_tux] 1
```

### OSX:
* It should compile.. I havn't tried yet. 

## Gameplay
Controls are all based on head movements (hopefully).
* Look at gui components and wait for a second to select them.
* Lean left/right to control Tux's direction down the course.
* Look down and pop up to get more air before a jump.
* Lean back to brake and slow down.

## Keyboard Settings
* Arrow keys can be used to adjust the camera position behind Tux:
 * Up/Down changes distance behind tux.
 * Right/Left changes the angle above and behind.
* 'H' toggles the heads up display. 
* 'A' attaches the HUD to your face instead of floating in the world.
* 'F' toggles display of fps (requires HUD to be enabled).
* '1,2,3' flips between preset camera positions.
* F3-F8 toggles display of elements in the course.
* ESC quits the level.

## Quick Race
This mode enables a toy-like feeling by increasing the eye's interpupilary distance (IPD) which makes the course seem like a different size.  This mode isn't used for the official events because it affects the speed and other gameplay dynamics which would make the events too difficult without changing their layout.  
* The details for these can be tweaked at the bottom of the options file.

## Things Needing Attention
* I've only been testing with open source radeon drivers.  Please let me know if other cards/drivers work or not.
* Optimization is needed for higher detail levels, track marks, ice env maps, snow.
* Skybox doesn't show up w/ fog.. and I can't render it up close because of stereo depth.
* The snow particle system for GUI and racing needs a lot of attention to work w/ VR.
* Fails to start with: 'Error: [Context] Unable to obtain x11 visual from context'
 * Occurs when using the xf86-video-ati open source radeon driver. See the following for a patch to the Oculus SDK:
 * https://forums.oculus.com/viewtopic.php?t=16664#p252973
* Translations are behind.  Spanish, etc.  See data/translations for language files.

## Contact:
* IRC: I'm metric on #etuxracer (Freenode)
* Github: http://github.com/jdtaylor
* Email: j.douglas.taylor@gmail.com
* My public key can be found at http://jdtaylor.org

## Donations:
* If you like what you see and would like to support more Free Software for VR; please send bitcoin: 12ptLNTGD16itaG9mXQxYRaExwXr3aVyFd

