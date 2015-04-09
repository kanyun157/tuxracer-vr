# Extreme Tuxracer VR

Patches for Oculus Rift support!  Please read the following carefully for installing and running with proper settings to it to work acceptably.  WARNING: A decent gaming graphics card is necessary!  Integrated graphics hardware won't cut it.

## Install

### Linux:
* Please compile from source on linux:
* Dependencies: sdl2 sdl2-mixer sdl2-image ftgl freetype glew git g++
* Download the oculus rift sdk 0.4.4 (0.5 not supported yet) and extract somewhere:
* https://developer.oculus.com/downloads/#version=pc-0.4.4-beta
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

```python
   from etr import vr
   def func():
```

* See (http://sourceforge.net/projects/extremetuxracer/)


