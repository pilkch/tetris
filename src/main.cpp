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
