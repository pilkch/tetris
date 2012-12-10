// Standard headers
#include <cassert>
#include <cmath>
#include <cstring>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>
#include <list>

// Boost headers
#include <boost/shared_ptr.hpp>

// OpenGL headers
#include <GL/GLee.h>
#include <GL/glu.h>

// SDL headers
#include <SDL/SDL_image.h>

// Spitfire headers
#include <spitfire/spitfire.h>

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>
#include <spitfire/math/cColour.h>

// libopenglmm headers
#include <libopenglmm/libopenglmm.h>
#include <libopenglmm/cContext.h>
#include <libopenglmm/cFont.h>
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Tetris headers
#include "application.h"

int main(int argc, char** argv)
{
  #if defined(BUILD_DEBUG) && defined(PLATFORM_LINUX_OR_UNIX)
  // KDevelop only shows output sent to cerr, we redirect cout to cerr here so that it will show up
  // Back up cout's streambuf
  std::streambuf* backup = std::cout.rdbuf();

  // Assign cerr's streambuf to cout
  std::cout.rdbuf(std::cerr.rdbuf());
  #endif

  bool bIsSuccess = true;

  {
    cApplication application;

    bIsSuccess = application.Run();
  }

  #if defined(BUILD_DEBUG) && defined(PLATFORM_LINUX_OR_UNIX)
  // Restore the original cout streambuf
  std::cout.rdbuf(backup);
  #endif

  return bIsSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}
