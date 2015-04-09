# [Extreme Tuxracer](http://sourceforge.net/projects/extremetuxracer/) VR 

The classic downhill skiing game now with Oculus Rift support!  Please have patience and read the following for running with proper settings and getting it to work acceptably (low latency).  WARNING: A decent gaming graphics card is necessary!  Integrated graphics hardware won't cut it.

## Install

### Linux:
* Please compile from source on linux:
* Dependencies: sdl2 sdl2-mixer sdl2-image ftgl freetype glew git gcc
* Download the oculus rift sdk 0.4.4 (0.5 not supported yet) and extract somewhere:
* https://developer.oculus.com/downloads/#version=pc-0.4.4-beta
```
git clone https://github.com/jdtaylor/tuxracer-vr.git
cd tuxracer-vr
export OVR_ROOT="/path/to/your/oculus/ovr_sdk_linux_0.4.4"
CXXFLAGS="-O3" ./configure --datadir=${PWD}/data
make
```
* Please make sure your oculusd or ovrd is running and issue ./etr to start.
* Oculus only support extended mode on Linux. 

### Windows:
* Donwload package [TODO]

### OSX:
* ... TODO

## Running:

