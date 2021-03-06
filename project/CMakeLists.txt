# Set the minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# Needed for not globally unique names in executables
CMAKE_POLICY(SET CMP0002 OLD)

# Needed for strings in defines
CMAKE_POLICY(SET CMP0005 OLD)

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

# Set the project name
PROJECT(tetris)

# Debug build
SET(CMAKE_BUILD_TYPE Debug)

# Release build
#SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
#SET(CMAKE_C_FLAGS_RELEASE "-O3")
#ADD_DEFINITIONS("-DNDEBUG")

SET(CMAKE_SOURCE_DIR ./../../library/)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}cmake/")

SET(PROJECT_DIRECTORY "../")
SET(PROJECT_SRC "${PROJECT_DIRECTORY}src/")

SET(LIBRARY_INCLUDE "${CMAKE_SOURCE_DIR}include/")
SET(LIBRARY_SRC "${CMAKE_SOURCE_DIR}src/")

# Add definitions, compiler switches, etc.
ADD_DEFINITIONS("-Wall -W -Wextra -Werror -Wformat -Wformat-y2k -Winit-self -Wstack-protector -Wunknown-pragmas -Wundef -Wwrite-strings -Wno-unused-parameter -Wpointer-arith -Wno-switch -Woverloaded-virtual -Wno-stack-protector -Wmissing-include-dirs -Wuninitialized -Wc++0x-compat -std=c++0x -ffor-scope -O2")

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
  ADD_DEFINITIONS("-D__LINUX__")
ENDIF()

IF(WIN32)
  ADD_DEFINITIONS("-DUNICODE -D_UNICODE")
  ADD_DEFINITIONS("-DNOMINMAX")
ENDIF()

ADD_DEFINITIONS("-DSPITFIRE_APPLICATION_NAME=\"\\\"Tetris\\\"\"")
ADD_DEFINITIONS("-DSPITFIRE_APPLICATION_NAME_LWR=\"\\\"tetris\\\"\"")

ADD_DEFINITIONS("-DBUILD_APPLICATION_VERSION_STRING=\"\\\"0.1\\\"\"")

ADD_DEFINITIONS("-DBUILD_LIBOPENGLMM_OPENGL_VERSION=330")
ADD_DEFINITIONS("-DBUILD_LIBOPENGLMM_OPENGL_STRICT")
ADD_DEFINITIONS("-DBUILD_LIBOPENGLMM_WINDOW_SDL")
ADD_DEFINITIONS("-DBUILD_LIBOPENGLMM_FONT")
ADD_DEFINITIONS("-DBUILD_AUDIO_OPENAL -DBUILD_AUDIO_SDLMIXER")
ADD_DEFINITIONS("-DBUILD_SPITFIRE_UNITTEST")
ADD_DEFINITIONS("-DBUILD_XML_MATH")

INCLUDE_DIRECTORIES(${LIBRARY_INCLUDE})


# Files from library directory

MACRO(PREFIX_PATHS SOURCE_DIRECTORY SOURCE_FILES)
MESSAGE(STATUS "PREFIX_PATHS dir=${SOURCE_DIRECTORY}, files=${SOURCE_FILES}, ${ARGN}")
MESSAGE(a=${SOURCE_FILES})
  SET(OUTPUT_FILES "")
  SET(a ${OUTPUT_FILES})
  SET(OUTPUT_FILES ${a} "${SOURCE_DIRECTORY}${SOURCE_FILES}")
  FOREACH(SOURCE_FILE ${ARGN})
    MESSAGE(b=${SOURCE_FILE})
    SET(a ${OUTPUT_FILES})
    SET(OUTPUT_FILES ${a} "${SOURCE_DIRECTORY}${SOURCE_FILE}")
  ENDFOREACH()
MESSAGE(d=${OUTPUT_FILES})
ENDMACRO(PREFIX_PATHS)




SET(LIBRARY_LIBVOODOOMM_SOURCE_DIRECTORY libvoodoomm/)
SET(LIBRARY_LIBVOODOOMM_SOURCE_FILES
libvoodoomm.cpp cImage.cpp)

IF(WIN32)
  SET(LIBRARY_LIBVOODOOMM_SOURCE_FILES ${LIBRARY_LIBVOODOOMM_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_LIBVOODOOMM_SOURCE_DIRECTORY} ${LIBRARY_LIBVOODOOMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBVOODOOMM_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_LIBOPENGLMM_SOURCE_DIRECTORY libopenglmm/)
SET(LIBRARY_LIBOPENGLMM_SOURCE_FILES
libopenglmm.cpp cContext.cpp cFont.cpp cGeometry.cpp cShader.cpp cSystem.cpp cTexture.cpp cVertexBufferObject.cpp cWindow.cpp)

IF(WIN32)
  SET(LIBRARY_LIBOPENGLMM_SOURCE_FILES ${LIBRARY_LIBOPENGLMM_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_LIBOPENGLMM_SOURCE_DIRECTORY} ${LIBRARY_LIBOPENGLMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBOPENGLMM_SOURCE_FILES ${OUTPUT_FILES})


SET(LIBRARY_LIBTRASHMM_SOURCE_DIRECTORY libtrashmm/)
SET(LIBRARY_LIBTRASHMM_SOURCE_FILES
libtrashmm.cpp)

PREFIX_PATHS(${LIBRARY_LIBTRASHMM_SOURCE_DIRECTORY} ${LIBRARY_LIBTRASHMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBTRASHMM_SOURCE_FILES ${OUTPUT_FILES})


SET(LIBRARY_LIBXDGMM_SOURCE_DIRECTORY libxdgmm/)
SET(LIBRARY_LIBXDGMM_SOURCE_FILES
libxdgmm.cpp)

IF(WIN32)
  SET(LIBRARY_LIBXDGMM_SOURCE_FILES ${LIBRARY_LIBXDGMM_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_LIBXDGMM_SOURCE_DIRECTORY} ${LIBRARY_LIBXDGMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBXDGMM_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_SPITFIRE_SOURCE_DIRECTORY spitfire/)
SET(LIBRARY_SPITFIRE_SOURCE_FILES
math/cVec2.cpp math/cVec3.cpp math/cVec4.cpp math/cMat3.cpp math/cMat4.cpp math/cQuaternion.cpp math/math.cpp math/cPlane.cpp math/cColour.cpp math/geometry.cpp
storage/document.cpp storage/file.cpp storage/filesystem.cpp storage/xml.cpp
util/datetime.cpp util/string.cpp util/thread.cpp util/unittest.cpp)

IF(WIN32)
  SET(LIBRARY_SPITFIRE_SOURCE_FILES ${LIBRARY_SPITFIRE_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_SPITFIRE_SOURCE_DIRECTORY} ${LIBRARY_SPITFIRE_SOURCE_FILES})
SET(OUTPUT_LIBRARY_SPITFIRE_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_BREATHE_SOURCE_DIRECTORY breathe/)
SET(LIBRARY_BREATHE_SOURCE_FILES
audio/audio.cpp audio/audio_openal.cpp audio/audio_sdlmixer.cpp
gui/cManager.cpp gui/cRenderer.cpp
render/cTextureAtlas.cpp
util/cApplication.cpp)

IF(WIN32)
  SET(LIBRARY_BREATHE_SOURCE_FILES ${LIBRARY_BREATHE_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_BREATHE_SOURCE_DIRECTORY} ${LIBRARY_BREATHE_SOURCE_FILES})
SET(OUTPUT_LIBRARY_BREATHE_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_SOURCE_FILES ${OUTPUT_LIBRARY_SPITFIRE_SOURCE_FILES} ${OUTPUT_LIBRARY_BREATHE_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBVOODOOMM_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBOPENGLMM_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBTRASHMM_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBXDGMM_SOURCE_FILES}
GL/GLee.cpp
)
PREFIX_PATHS(${LIBRARY_SRC} ${LIBRARY_SOURCE_FILES})
SET(OUTPUT_LIBRARY_SOURCE_FILES ${OUTPUT_FILES})
#MESSAGE(final=${OUTPUT_LIBRARY_SOURCE_FILES})



SET(PROJECT_SOURCE_FILES
application.cpp main.cpp settings.cpp states.cpp tetris.cpp
)
PREFIX_PATHS(${PROJECT_SRC} ${PROJECT_SOURCE_FILES})
SET(OUTPUT_PROJECT_SOURCE_FILES ${OUTPUT_FILES})
#MESSAGE(final=${OUTPUT_PROJECT_SOURCE_FILES})



#list all source files here
ADD_EXECUTABLE(${PROJECT_NAME} ${OUTPUT_PROJECT_SOURCE_FILES} ${OUTPUT_LIBRARY_SOURCE_FILES})


FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})



SET(LIBRARIES
  ALUT
  OpenAL
  OpenGL
  SDL
  SDL_image
  SDL_mixer
  SDL_ttf
  XDG-basedir
)


# Some of the libraries have different names than their Find*.cmake name
SET(LIBRARIES_LINKED
  alut
  openal
  GL
  GLU
  SDL
  SDL_image
  SDL_mixer
  SDL_ttf
  freetype
  boost_iostreams
  boost_filesystem
  boost_locale
  boost_system
  xdg-basedir
)


FOREACH(LIBRARY_FILE ${LIBRARIES})
  Find_Package(${LIBRARY_FILE} REQUIRED)
ENDFOREACH(LIBRARY_FILE)

#need to link to some other libraries ? just add them here
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBRARIES_LINKED})

