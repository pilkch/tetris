#ifndef TETRIS_APPLICATION_H
#define TETRIS_APPLICATION_H

// Standard headers
#include <stack>

// Spitfire headers
#include <spitfire/math/math.h>

// Breathe headers
#include <breathe/audio/audio.h>

#include <breathe/gui/cManager.h>
#include <breathe/gui/cRenderer.h>

#include <breathe/util/cApplication.h>

// Tetris headers
#include "settings.h"

// ** A simple class for calculating letter box dimensions

class cLetterBox
{
public:
  cLetterBox(size_t width, size_t height);

  size_t desiredWidth;
  size_t desiredHeight;
  float fDesiredRatio;

  float fRatio;

  size_t letterBoxedWidth;
  size_t letterBoxedHeight;
};


class cState;

// ** cApplication

class cApplication : public breathe::util::cApplication
{
public:
  friend class cState;

  cApplication(int argc, const char* const* argv);
  ~cApplication();

  void PlaySound(breathe::audio::cBufferRef pBuffer);

protected:
  cSettings settings;

private:
  virtual override bool _Create();
  virtual override void _Destroy();

  virtual override bool _LoadResources();
  virtual override void _DestroyResources();

  // Text
  opengl::cFont* pFont;

  // Gui
  breathe::gui::cManager* pGuiManager;
  breathe::gui::cRenderer* pGuiRenderer;
};

#endif // TETRIS_APPLICATION_H
