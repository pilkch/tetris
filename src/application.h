#ifndef TETRIS_APPLICATION_H
#define TETRIS_APPLICATION_H

// Standard headers
#include <stack>

// Breathe headers
#include <breathe/audio/audio.h>

#include <breathe/gui/cManager.h>
#include <breathe/gui/cRenderer.h>

#include <breathe/util/cApplication.h>

// Tetris headers
#include "settings.h"


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
  virtual bool _Create() override;
  virtual void _Destroy() override;

  virtual bool _LoadResources() override;
  virtual void _DestroyResources() override;

  // Text
  opengl::cFont* pFont;

  // Gui
  breathe::gui::cManager* pGuiManager;
  breathe::gui::cRenderer* pGuiRenderer;
};

#endif // TETRIS_APPLICATION_H
