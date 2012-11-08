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

// Tetris headers
#include "settings.h"

class cState;

// ** cApplication

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  friend class cState;

  cApplication();
  ~cApplication();

  void Run();

  void PushStateSoon(cState* pState);
  void PopStateSoon();

  void PlaySound(breathe::audio::cBufferRef pBuffer);

private:
  bool Create();
  void Destroy();

  void MainLoop();

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  // State management
  const cState* GetState() const;
  cState* GetState();
  void PushState(cState* pState);
  void PopState();

  // State event management (Transitioning between states)
  void ProcessStateEvents();


  std::stack<cState*> states;

  // State change event queue
  typedef cState* cStateEvent;
  std::vector<cState*> stateEvents;

  cSettings settings;

  bool bIsDone;

  opengl::cSystem system;
  opengl::cWindow* pWindow;
  opengl::cContext* pContext;

  breathe::audio::cManager* pAudioManager;

  // Text
  opengl::cFont* pFont;

  // Gui
  breathe::gui::cManager* pGuiManager;
  breathe::gui::cRenderer* pGuiRenderer;
};

#endif // TETRIS_APPLICATION_H
