#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([etr], [0.6.3], [https://sourceforge.net/apps/phpbb/extremetuxracer])
AC_CONFIG_SRCDIR([src/gui.cpp])
AC_CONFIG_HEADERS([src/etr_config.h])

AM_INIT_AUTOMAKE([dist-xz dist-zip])

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_FT2

CXXFLAGS="$CXXFLAGS $FT2_CFLAGS"
LIBS="$LIBS $FT2_LIBS"

# Checks for header files.
AC_CHECK_HEADERS([float.h limits.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

#
case "$host_os" in
  *linux*)
     AC_DEFINE([OS_LINUX], [1], [Linux build]) ;;
  *mingw32*)
     AC_DEFINE([OS_WIN32_MINGW], [1], [MinGW build]) ;;
  *darwin*)
     AC_DEFINE([OS_MAC], [1], [Mac OS X build]) ;;
esac

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor getcwd memmove memset mkdir pow sqrt strchr strdup])

#AM_PATH_SDL([1.0.0])
AM_PATH_SDL2([2.0.0])
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

PKG_CHECK_MODULES([SDLIMAGE], [SDL2_image],
[
  CFLAGS="${CFLAGS} $SDLIMAGE_CFLAGS"
  LIBS="${LIBS} $SDLIMAGE_LIBS"
],
[AC_MSG_ERROR([SDL2_image not found])])

PKG_CHECK_MODULES([SDLMIXER], [SDL2_mixer],
[
  CFLAGS="${CFLAGS} $SDLMIXER_CFLAGS"
  LIBS="${LIBS} $SDLMIXER_LIBS"
],
[AC_MSG_ERROR([SDL2_mixer not found])])

AC_CHECK_LIB([GL], [glBegin], [],
[AC_MSG_ERROR([libGL not found])])

LIBS="$LIBS -lGL"

AC_CHECK_LIB([GLU], [gluBeginCurve], [],
[AC_MSG_ERROR([libGLU not found])])

LIBS="$LIBS -lGLU"

GLEW_CFLAGS=""
GLEW_LIBS="-lGLEW"
dnl Include a fallback path for GLEW for the moment while not all distros
dnl have picked up the .pc file.
PKG_CHECK_MODULES(GLEW, [glew], [], [
  AC_CHECK_HEADER([GL/glew.h],
  [],
  AC_MSG_ERROR([GLEW required]))
AC_CHECK_LIB([GLEW],
  [glewInit],
  [],
  AC_MSG_ERROR([GLEW required]))
])
CFLAGS="$CFLAGS $GLEW_CFLAGS"
LIBS="$LIBS $GLEW_LIBS"

PKG_CHECK_MODULES(FTGL, [ftgl],
[
  CFLAGS="${CFLAGS} $FTGL_CFLAGS"
  LIBS="${LIBS} $FTGL_LIBS"
],
[AC_MSG_ERROR([FTGL not found])])

# jdt: Change this to your Oculus SDK path:
LIB_OVR_DIR="${OVR_ROOT}/LibOVR"
CXXFLAGS="$CXXFLAGS -I${LIB_OVR_DIR}/Include -I${LIB_OVR_DIR}/Src"
LIBS="$LIBS -L${LIB_OVR_DIR}/Lib/Linux/Release/x86_64 -lXrandr -lXinerama -lovr -lX11 -lrt"
#LIBS="$LIBS -L${LIB_OVR_DIR}/Lib/Linux/Debug/x86_64 -lXrandr -lXinerama -lovr -lX11 -lrt"
# For sdk >= 0.5
#LIBS="$LIBS -L${LIB_OVR_DIR}/Lib/Linux/x86_64/Release -lXrandr -lXinerama -lOVR -lX11 -lrt -ldl"

CPPFLAGS="${CPPFLAGS} -DETR_DATA_DIR=\\\"$datadir\\\""

# Request c++11 compatibility and profiling info
CXXFLAGS="${CXXFLAGS} -std=c++11"

AC_CONFIG_FILES([
        Makefile
        build/Makefile
        build/Installer/Makefile
        data/Makefile
        data/char/Makefile
        data/char/beastie/Makefile
        data/char/boris/Makefile
        data/char/samuel/Makefile
        data/char/trixi/Makefile
        data/char/tux/Makefile
        data/courses/Makefile
        data/courses/bumpy_ride/Makefile
        data/courses/bunny_hill/Makefile
        data/courses/challenge_one/Makefile
        data/courses/chinese_wall/Makefile
        data/courses/chragis_gagiwaetter/Makefile
        data/courses/course/Makefile
        data/courses/downhill_fear/Makefile
        data/courses/explore_mountains/Makefile
        data/courses/frozen_lakes/Makefile
        data/courses/frozen_river/Makefile
        data/courses/hippo_run/Makefile
        data/courses/holygrail/Makefile
        data/courses/in_search_of_vodka/Makefile
        data/courses/keep_country_tidy/Makefile
        data/courses/milos_castle/Makefile
        data/courses/path_of_daggers/Makefile
        data/courses/penguins_cant_fly/Makefile
        data/courses/quiet_river/Makefile
        data/courses/secret_valleys/Makefile
        data/courses/this_means_something/Makefile
        data/courses/tux_at_home/Makefile
        data/courses/twisty_slope/Makefile
        data/courses/wild_mountains/Makefile
        data/env/Makefile
        data/env/etr/Makefile
        data/env/etr/cloudy/Makefile
        data/env/etr/evening/Makefile
        data/env/etr/night/Makefile
        data/env/etr/sunny/Makefile
        data/env/tuxracer/Makefile
        data/env/tuxracer/cloudy/Makefile
        data/env/tuxracer/evening/Makefile
        data/env/tuxracer/night/Makefile
        data/env/tuxracer/sunny/Makefile
        data/fonts/Makefile
        data/music/Makefile
        data/objects/Makefile
        data/players/Makefile
        data/sounds/Makefile
        data/terrains/Makefile
        data/textures/Makefile
        data/translations/Makefile
        doc/Makefile
        resources/Makefile
		src/Makefile
])
AC_OUTPUT
