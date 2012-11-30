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
#include "states.h"

// ** cLetterBox

cLetterBox::cLetterBox(size_t width, size_t height) :
  desiredWidth(0),
  desiredHeight(0),
  fDesiredRatio(0.0f),
  fRatio(0.0f),
  letterBoxedWidth(0),
  letterBoxedHeight(0)
{
  desiredWidth = 1920;
  desiredHeight = 1080;
  fDesiredRatio = float(desiredWidth) / float(desiredHeight);

  fRatio = float(width) / float(height);

  // Apply letter boxing
  letterBoxedWidth = width;
  letterBoxedHeight = height;

  if (fRatio < fDesiredRatio) {
    // Taller (4:3, 16:10 for example)
    letterBoxedHeight = width / fDesiredRatio;
  } else {
    // Wider
    letterBoxedWidth = height * fDesiredRatio;
  }

  // Round up to the next even number
  if ((letterBoxedWidth % 2) != 0) letterBoxedWidth++;
  if ((letterBoxedHeight % 2) != 0) letterBoxedHeight++;
}


// ** cApplication

cApplication::cApplication() :
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
  assert(pContext != nullptr);
  assert(pContext->IsValid());

  assert(pGuiManager == nullptr);
  assert(pGuiRenderer == nullptr);
  // Setup our gui
  pGuiManager = new breathe::gui::cManager;
  pGuiRenderer = new breathe::gui::cRenderer(*pGuiManager, system, *pContext);

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
  assert(pGuiManager != nullptr);
  assert(pGuiRenderer != nullptr);

  pFont = pContext->CreateFont(TEXT("data/fonts/pricedown.ttf"), 32, TEXT("data/shaders/font.vert"), TEXT("data/shaders/font.frag"));
  assert(pFont != nullptr);
  assert(pFont->IsValid());

  cLetterBox letterBox(pContext->GetWidth(), pContext->GetHeight());

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
