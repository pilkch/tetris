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

// Breathe headers
#include <breathe/audio/audio.h>

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
#include "states.h"

// ** cApplication

cApplication::cApplication() :
  bIsDone(false),

  width(0),
  height(0),

  fZoom(0.0f),

  pWindow(nullptr),
  pContext(nullptr),

  pFont(nullptr),

  pAudioManager(nullptr)
{
  settings.Load();
}

cApplication::~cApplication()
{
  Destroy();
  settings.Save();
}

void cApplication::PlaySound(breathe::audio::cBufferRef pBuffer)
{
  breathe::audio::cSourceRef pSource = pAudioManager->CreateSourceAttachedToScreen(pBuffer);
  assert(pSource);

  pSource->Play();
}

bool cApplication::Create()
{
  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  opengl::cResolution resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 1024) || (resolution.height < 768) || (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8)) {
    std::cout<<"Current screen resolution is not adequate "<<resolution.width<<"x"<<resolution.height<<std::endl;
    return false;
  }

  // Set our required resolution
  resolution.width = 1024;
  resolution.height = 768;
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;

  pWindow = system.CreateWindow(TEXT("tetris"), resolution, false);
  if (pWindow == nullptr) {
    std::cout<<"Window could not be created"<<std::endl;
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    std::cout<<"Context could not be created"<<std::endl;
    return false;
  }


  pFont = pContext->CreateFont(TEXT("data/fonts/pricedown.ttf"), 32, TEXT("data/shaders/font.vert"), TEXT("data/shaders/font.frag"));


  pAudioManager = breathe::audio::Create(breathe::audio::DRIVER::SDLMIXER);


  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  // Push our first state
  PushState(new cStateMenu(*this));

  return true;
}

void cApplication::Destroy()
{
  if (pFont != nullptr) {
    pContext->DestroyFont(pFont);
    pFont = nullptr;
  }


  pContext = nullptr;

  if (pWindow != nullptr) {
    system.DestroyWindow(pWindow);
    pWindow = nullptr;
  }


  if (pAudioManager != nullptr) {
    breathe::audio::Destroy(pAudioManager);
    pAudioManager = nullptr;
  }
}

void cApplication::_OnWindowEvent(const opengl::cWindowEvent& event)
{
   std::cout<<"cApplication::_OnWindowEvent"<<std::endl;

   if (event.IsQuit()) {
      std::cout<<"cApplication::_OnWindowEvent Quiting"<<std::endl;
      bIsDone = true;
   }
}

void cApplication::_OnMouseEvent(const opengl::cMouseEvent& event)
{
  cState* pState = GetState();
  if (pState != nullptr) pState->OnMouseEvent(event);
}

void cApplication::_OnKeyboardEvent(const opengl::cKeyboardEvent& event)
{
  cState* pState = GetState();
  if (pState != nullptr) pState->OnKeyboardEvent(event);
}

void cApplication::PushStateSoon(cState* pState)
{
  cStateEvent event(pState);
  stateEvents.push_back(event);
}

void cApplication::PopStateSoon()
{
  cStateEvent event(nullptr); // A null state event means "Pop the current state"
  stateEvents.push_back(event);
}

void cApplication::PushState(cState* pState)
{
  cState* pOldState = GetState();
  if (pOldState != nullptr) pOldState->Pause();

  states.push(pState);
}

void cApplication::PopState()
{
  cState* pOldState = GetState();
  if (pOldState != nullptr) {
    // Delete and remove the old state
    delete pOldState;
    states.pop();

    // Prepare the previous state
    cState* pCurrentState = GetState();
    if (pCurrentState != nullptr) pCurrentState->Resume();
  }
}

cState* cApplication::GetState()
{
  cState* pState = nullptr;
  if (!states.empty()) pState = states.top();
  return pState;
}

void cApplication::ProcessStateEvents()
{
  const size_t n = stateEvents.size();
  for (size_t i = 0; i < n; i++) {
    cState* pState = stateEvents[i];
    if (pState != nullptr) PushState(pState);
    else PopState(); // A null state event means "Pop the current state"
  }

  stateEvents.clear();
}

void cApplication::Run()
{
  assert(pContext != nullptr);
  assert(pContext->IsValid());

  // Text
  assert(pFont != nullptr);
  assert(pFont->IsValid());

  const spitfire::math::cColour sunColour(0.2, 0.2, 0.0);

  // Setup materials
  const spitfire::math::cColour ambient(sunColour);
  pContext->SetMaterialAmbientColour(ambient);
  const spitfire::math::cColour diffuse(0.8, 0.1, 0.0);
  pContext->SetMaterialDiffuseColour(diffuse);
  const spitfire::math::cColour specular(1.0, 0.3, 0.3);
  pContext->SetMaterialSpecularColour(specular);
  const float fShininess = 50.0f;
  pContext->SetMaterialShininess(fShininess);


  // Setup lighting
  // NOTE: No lighting to show that the lightmap is working
  //pContext->EnableLighting();
  //pContext->EnableLight(0);
  //const spitfire::math::cVec3 lightPosition(5.0f, 5.0f, 10.0f);
  //pContext->SetLightPosition(0, lightPosition);
  //const spitfire::math::cColour lightAmbient(sunColour);
  //pContext->SetLightAmbientColour(0, lightAmbient);
  //const spitfire::math::cColour lightDiffuse(1.0, 1.0, 1.0);
  //pContext->SetLightDiffuseColour(0, lightDiffuse);
  //const spitfire::math::cColour lightSpecular(1.0f, 1.0f, 1.0f);
  //pContext->SetLightSpecularColour(0, lightSpecular);


  uint32_t T0 = 0;
  uint32_t Frames = 0;

  uint32_t currentTime = SDL_GetTicks();
  uint32_t lastTime = SDL_GetTicks();

  uint32_t lastUpdateTime = SDL_GetTicks();

  while (!states.empty()) {
    ProcessStateEvents();
    if (states.empty()) break;

    // Update window events
    pWindow->UpdateEvents();

    // Update state
    lastTime = currentTime;
    currentTime = SDL_GetTicks();
    {
      const cTimeStep timeStep(currentTime, currentTime - lastTime);
      cState* pState = GetState();
      assert(pState != nullptr);
      pState->UpdateInput(timeStep);
    }

    // Perform an Update, these are locked at 30 fps
    if ((currentTime - lastUpdateTime) > 33) {
      const cTimeStep timeStep(currentTime, 33);
      cState* pState = GetState();
      assert(pState != nullptr);
      pState->Update(timeStep);
      lastUpdateTime = currentTime;
    }

    // Update audio
    const spitfire::math::cVec3 listenerPosition;
    const spitfire::math::cVec3 listenerTarget;
    const spitfire::math::cVec3 listenerUp(0.0f, 0.0f, 1.0f);
    pAudioManager->Update(currentTime, listenerPosition, listenerTarget, listenerUp);

    // Render a frame
    {
      const cTimeStep timeStep(currentTime, currentTime - lastTime);
      cState* pState = GetState();
      assert(pState != nullptr);
      pState->Render(timeStep);
    }

    // Gather our frames per second
    Frames++;
    {
      const uint32_t t = SDL_GetTicks();
      // TODO: Try changing this to 1000
      if (t - T0 >= 5000) {
        const float seconds = (t - T0) / 1000.0;
        const float fps = Frames / seconds;
        std::cout<<Frames<<" frames in "<<seconds<<" seconds = "<<fps<<" FPS"<<std::endl;
        T0 = t;
        Frames = 0;
      }
    }
  };
}



//pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_Paused").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_Paused_Instructions").c_str());

//pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_GameOver").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_GameOver_Instructions1").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.10f, 1.0f, breathe::LANG("L_GameOver_Instructions2").c_str());
