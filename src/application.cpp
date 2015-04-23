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

// OpenGL headers
#include <GL/GLee.h>
#include <GL/glu.h>

// Spitfire headers
#include <spitfire/spitfire.h>

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>
#include <spitfire/math/cColour.h>
#include <spitfire/math/letterbox.h>

#include <spitfire/util/log.h>

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

cApplication::cApplication(int argc, const char* const* argv) :
  breathe::util::cApplication(argc, argv),

  pFont(nullptr),

  pGuiManager(nullptr),
  pGuiRenderer(nullptr)
{
  settings.Load();
}

cApplication::~cApplication()
{
  settings.Save();
}

void cApplication::PlaySound(breathe::audio::cBufferRef pBuffer)
{
  breathe::audio::cSourceRef pSource = pAudioManager->CreateSourceAttachedToScreen(pBuffer);
  assert(pSource);

  pSource->Play();
}

bool cApplication::_Create()
{
  LOG<<"cApplication::_Create"<<std::endl;

  assert(pContext != nullptr);
  assert(pContext->IsValid());

  assert(pGuiManager == nullptr);
  assert(pGuiRenderer == nullptr);

  // Setup our gui
  pGuiManager = new breathe::gui::cManager;
  pGuiRenderer = new breathe::gui::cRenderer(*pGuiManager, *pContext);

  _LoadResources();

  // Push our first state
  PushState(new cStateMenu(*this));

  return true;
}

void cApplication::_Destroy()
{
  _DestroyResources();

  spitfire::SAFE_DELETE(pGuiRenderer);
  spitfire::SAFE_DELETE(pGuiManager);
}

bool cApplication::_LoadResources()
{
  LOG<<"cApplication::_LoadResources"<<std::endl;

  assert(pGuiManager != nullptr);
  assert(pGuiRenderer != nullptr);

  pFont = pContext->CreateFont(TEXT("data/fonts/pricedown.ttf"), 32, TEXT("data/shaders/font.vert"), TEXT("data/shaders/font.frag"));
  assert(pFont != nullptr);
  assert(pFont->IsValid());

  spitfire::math::cLetterBox letterBox(TETRIS_VIDEO_TARGET_WIDTH, TETRIS_VIDEO_TARGET_HEIGHT, pContext->GetWidth(), pContext->GetHeight());

  pGuiRenderer->LoadResources(letterBox.letterBoxedWidth, letterBox.letterBoxedHeight);

  // Load the resources of all the states
  std::list<breathe::util::cState*>::iterator iter = states.begin();
  const std::list<breathe::util::cState*>::iterator iterEnd = states.end();
  while (iter != iterEnd) {
    cState* pState = static_cast<cState*>(*iter);
    if (pState != nullptr) pState->LoadResources();

    iter++;
  }

  return true;
}

void cApplication::_DestroyResources()
{
  LOG<<"cApplication::_DestroyResources"<<std::endl;

  // Destroy the resources of all the states
  std::list<breathe::util::cState*>::iterator iter = states.begin();
  const std::list<breathe::util::cState*>::iterator iterEnd = states.end();
  while (iter != iterEnd) {
    cState* pState = static_cast<cState*>(*iter);
    if (pState != nullptr) pState->DestroyResources();

    iter++;
  }

  assert(pGuiManager != nullptr);
  assert(pGuiRenderer != nullptr);

  pGuiRenderer->DestroyResources();

  if (pFont != nullptr) {
    pContext->DestroyFont(pFont);
    pFont = nullptr;
  }
}


//pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_Paused").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_Paused_Instructions").c_str());

//pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_GameOver").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_GameOver_Instructions1").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.10f, 1.0f, breathe::LANG("L_GameOver_Instructions2").c_str());
