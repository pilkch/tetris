#ifndef TETRIS_APPLICATION_H
#define TETRIS_APPLICATION_H

#include <stack>

class cTimeStep;
class cState;

// ** cApplication

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  friend class cState;

  cApplication();
  ~cApplication();

  bool Create();
  void Destroy();

  void Run();

  void PushStateSoon(cState* pState);
  void PopStateSoon();

  void PlaySound(breathe::audio::cBufferRef pBuffer);

private:
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

  typedef cState* cStateEvent;
  std::vector<cStateEvent> stateEvents;


  bool bIsDone;

  // TODO: Can these be removed?
  size_t width;
  size_t height;
  spitfire::math::cVec3 scale;

  // For controlling the rotation and zoom of the trackball
  // TODO: Can these be removed?
  spitfire::math::cQuaternion rotationX;
  spitfire::math::cQuaternion rotationZ;
  float fZoom;

  opengl::cSystem system;
  opengl::cWindow* pWindow;
  opengl::cContext* pContext;

public:
  // Text
  opengl::cFont* pFont;

  breathe::audio::cManager* pAudioManager;
};

#endif // TETRIS_APPLICATION_H
