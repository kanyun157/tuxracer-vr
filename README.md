# [Extreme Tuxracer](http://sourceforge.net/projects/extremetuxracer/) VR 

The classic downhill skiing game now with Oculus Rift support!  Please have patience and read the following for running with proper settings and getting it to work acceptably (low latency).  WARNING: A decent gaming graphics card is necessary!  Integrated graphics hardware won't cut it.

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
* Donwload package [TODO]

### OSX:
* ... TODO

## Running:
* Please make sure your oculusd or ovrd is running and issue ./etr to start.
* Use the F2 key to toggle the window to the Rift and back.
* On Linux, the Oculus SDK only supports extended mode.. whether using Xinerama or as separate displays via the DISPLAY environment variable.
 * I'm getting 75 fps with xinerama and the open source xf86-video-ati drivers
  * No mirroring which happens automatically on Windows with direct to rift mode.
 * I have to turn off vsync to get anything over half of 75 fps (37.5) for some reason.  
 * To turn off vsync append this option to the etr config file: [no_vsync] 1


