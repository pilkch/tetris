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
#include <SDL2/SDL_image.h>

// libopenglmm headers
#include <libopenglmm/cGeometry.h>

// Spitfire headers
#include <spitfire/spitfire.h>

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>
#include <spitfire/math/cColour.h>

#include <spitfire/storage/filesystem.h>
#include <spitfire/storage/xml.h>

// Tetris headers
#include "application.h"
#include "states.h"

// ** cState

cState::cState(cApplication& _application) :
  breathe::util::cState(_application),
  application(_application),
  settings(application.settings),
  pFont(application.pFont),
  pAudioManager(application.pAudioManager),
  pGuiManager(application.pGuiManager),
  pGuiRenderer(application.pGuiRenderer),
  pLayer(nullptr),
  bIsWireframe(false),
  pVertexBufferObjectLetterBoxedRectangle(nullptr),
  pFrameBufferObjectLetterBoxedRectangle(nullptr),
  pShaderLetterBoxedRectangle(nullptr)
{
  breathe::gui::cLayer* pRoot = static_cast<breathe::gui::cLayer*>(pGuiManager->GetRoot());

  pLayer = new breathe::gui::cLayer;
  pRoot->AddChild(pLayer);
}

cState::~cState()
{
  if (pVertexBufferObjectLetterBoxedRectangle != nullptr) DestroyVertexBufferObjectLetterBoxedRectangle();
  if (pFrameBufferObjectLetterBoxedRectangle != nullptr) DestroyFrameBufferObjectLetterBoxedRectangle();
  if (pShaderLetterBoxedRectangle != nullptr) DestroyShaderLetterBoxedRectangle();

  if (pLayer != nullptr) {
    breathe::gui::cWidget* pRoot = pGuiManager->GetRoot();
    ASSERT(pRoot != nullptr);
    pRoot->RemoveChildAndDestroy(pLayer);
  }
}

void cState::_OnPause()
{
  if (pLayer != nullptr) pLayer->SetVisible(false);
}

void cState::_OnResume()
{
  if (pLayer != nullptr) {
    pLayer->SetVisible(true);

    pLayer->SetFocusToFirstChild();
  }
}

void cState::_OnKeyboardEvent(const breathe::gui::cKeyboardEvent& event)
{
  bool bIsHandled = false;
  if (event.IsKeyDown()) bIsHandled = pGuiManager->InjectEventKeyboardDown(event.GetKeyCode());
  else bIsHandled = pGuiManager->InjectEventKeyboardUp(event.GetKeyCode());

  if (!bIsHandled) _OnStateKeyboardEvent(event);
}

void cState::_OnMouseEvent(const breathe::gui::cMouseEvent& event)
{
  const float x = event.GetX() / pContext->GetWidth();
  const float y = event.GetY() / pContext->GetHeight();

  bool bIsHandled = false;
  if (event.IsButtonDown()) bIsHandled = pGuiManager->InjectEventMouseDown(event.GetButton(), x, y);
  else if (event.IsButtonUp()) bIsHandled = pGuiManager->InjectEventMouseUp(event.GetButton(), x, y);
  else bIsHandled = pGuiManager->InjectEventMouseMove(event.GetButton(), x, y);

  if (!bIsHandled) _OnStateMouseEvent(event);
}

  void cState::_OnJoystickEvent(const breathe::util::cJoystickEvent& event)
  {
    bool bIsHandled = false;
    if (event.IsButtonDown()) bIsHandled = pGuiManager->InjectEventJoystickEvent(event);

    if (!bIsHandled) _OnStateJoystickEvent(event);
  }

breathe::gui::cStaticText* cState::AddStaticText(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width)
{
  breathe::gui::cStaticText* pStaticText = new breathe::gui::cStaticText;
  pStaticText->SetId(id);
  pStaticText->SetCaption(sText);
  pStaticText->SetRelativePosition(spitfire::math::cVec2(x, y));
  pStaticText->SetWidth(width);
  pStaticText->SetHeight(pGuiManager->GetStaticTextHeight());
  pLayer->AddChild(pStaticText);

  return pStaticText;
}

breathe::gui::cRetroButton* cState::AddRetroButton(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width)
{
  breathe::gui::cRetroButton* pRetroButton = new breathe::gui::cRetroButton;
  pRetroButton->SetEventListener(*this);
  pRetroButton->SetId(id);
  pRetroButton->SetCaption(sText);
  pRetroButton->SetRelativePosition(spitfire::math::cVec2(x, y));
  pRetroButton->SetWidth(width);
  pRetroButton->SetHeight(pGuiManager->GetStaticTextHeight());
  pLayer->AddChild(pRetroButton);

  return pRetroButton;
}

breathe::gui::cRetroInput* cState::AddRetroInput(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width)
{
  breathe::gui::cRetroInput* pRetroInput = new breathe::gui::cRetroInput;
  pRetroInput->SetEventListener(*this);
  pRetroInput->SetId(id);
  pRetroInput->SetCaption(sText);
  pRetroInput->SetRelativePosition(spitfire::math::cVec2(x, y));
  pRetroInput->SetWidth(width);
  pRetroInput->SetHeight(pGuiManager->GetStaticTextHeight());
  pLayer->AddChild(pRetroInput);

  return pRetroInput;
}

breathe::gui::cRetroInputUpDown* cState::AddRetroInputUpDown(breathe::gui::id_t id, int min, int max, int value, float x, float y, float width)
{
  breathe::gui::cRetroInputUpDown* pRetroInputUpDown = new breathe::gui::cRetroInputUpDown;
  pRetroInputUpDown->SetEventListener(*this);
  pRetroInputUpDown->SetId(id);
  pRetroInputUpDown->SetRange(min, max);
  pRetroInputUpDown->SetValue(value, false);
  pRetroInputUpDown->SetRelativePosition(spitfire::math::cVec2(x, y));
  pRetroInputUpDown->SetWidth(width);
  pRetroInputUpDown->SetHeight(pGuiManager->GetStaticTextHeight());
  pLayer->AddChild(pRetroInputUpDown);

  return pRetroInputUpDown;
}

breathe::gui::cRetroColourPicker* cState::AddRetroColourPicker(breathe::gui::id_t id, float x, float y, float width)
{
  breathe::gui::cRetroColourPicker* pRetroColourPicker = new breathe::gui::cRetroColourPicker;
  pRetroColourPicker->SetEventListener(*this);
  pRetroColourPicker->SetId(id);
  pRetroColourPicker->SetRelativePosition(spitfire::math::cVec2(x, y));
  pRetroColourPicker->SetWidth(width);
  pRetroColourPicker->SetHeight(pGuiManager->GetStaticTextHeight());
  pLayer->AddChild(pRetroColourPicker);

  return pRetroColourPicker;
}

void cState::CreateVertexBufferObjectLetterBoxedRectangle(size_t width, size_t height)
{
  ASSERT(pVertexBufferObjectLetterBoxedRectangle == nullptr);

  pVertexBufferObjectLetterBoxedRectangle = pContext->CreateStaticVertexBufferObject();

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v2_c4_t2 builder(*pGeometryDataPtr);


  cLetterBox letterBox(width, height);

  const float fWidth = float(letterBox.letterBoxedWidth);
  const float fHeight = float(letterBox.letterBoxedHeight);

  // Texture coordinates
  // NOTE: The v coordinates have been swapped, the code looks correct but with normal v coordinates the gui is rendered upside down
  const float fU = 0.0f;
  const float fV = float(letterBox.letterBoxedHeight);
  const float fU2 = float(letterBox.letterBoxedWidth);
  const float fV2 = 0.0f;

  const float x = 0.0f;
  const float y = 0.0f;

  const spitfire::math::cColour colour(1.0f, 1.0f, 1.0f, 1.0f);

  // Front facing triangles
  builder.PushBack(spitfire::math::cVec2(x, y + fHeight), colour, spitfire::math::cVec2(fU, fV2));
  builder.PushBack(spitfire::math::cVec2(x + fWidth, y + fHeight), colour, spitfire::math::cVec2(fU2, fV2));
  builder.PushBack(spitfire::math::cVec2(x + fWidth, y), colour, spitfire::math::cVec2(fU2, fV));
  builder.PushBack(spitfire::math::cVec2(x + fWidth, y), colour, spitfire::math::cVec2(fU2, fV));
  builder.PushBack(spitfire::math::cVec2(x, y), colour, spitfire::math::cVec2(fU, fV));
  builder.PushBack(spitfire::math::cVec2(x, y + fHeight), colour, spitfire::math::cVec2(fU, fV2));

  pVertexBufferObjectLetterBoxedRectangle->SetData(pGeometryDataPtr);

  pVertexBufferObjectLetterBoxedRectangle->Compile2D(system);
}

void cState::DestroyVertexBufferObjectLetterBoxedRectangle()
{
  if (pVertexBufferObjectLetterBoxedRectangle != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pVertexBufferObjectLetterBoxedRectangle);
    pVertexBufferObjectLetterBoxedRectangle = nullptr;
  }
}

void cState::CreateFrameBufferObjectLetterBoxedRectangle(size_t width, size_t height)
{
  ASSERT(pFrameBufferObjectLetterBoxedRectangle == nullptr);

  cLetterBox letterBox(width, height);

  pFrameBufferObjectLetterBoxedRectangle = pContext->CreateTextureFrameBufferObjectNoMipMaps(letterBox.letterBoxedWidth, letterBox.letterBoxedHeight, opengl::PIXELFORMAT::R8G8B8A8);
}

void cState::DestroyFrameBufferObjectLetterBoxedRectangle()
{
  if (pFrameBufferObjectLetterBoxedRectangle != nullptr) {
    pContext->DestroyTextureFrameBufferObject(pFrameBufferObjectLetterBoxedRectangle);
    pFrameBufferObjectLetterBoxedRectangle = nullptr;
  }
}

void cState::CreateShaderLetterBoxedRectangle()
{
  ASSERT(pShaderLetterBoxedRectangle == nullptr);

  pShaderLetterBoxedRectangle = pContext->CreateShader(TEXT("data/shaders/passthroughwithcolour.vert"), TEXT("data/shaders/passthroughwithcolourrect.frag"));
}

void cState::DestroyShaderLetterBoxedRectangle()
{
  if (pShaderLetterBoxedRectangle != nullptr) {
    pContext->DestroyShader(pShaderLetterBoxedRectangle);
    pShaderLetterBoxedRectangle = nullptr;
  }
}

void cState::LoadResources()
{
  const size_t width = pContext->GetWidth();
  const size_t height = pContext->GetHeight();
  CreateFrameBufferObjectLetterBoxedRectangle(width, height);
  CreateShaderLetterBoxedRectangle();
  CreateVertexBufferObjectLetterBoxedRectangle(width, height);
}

void cState::DestroyResources()
{
  DestroyFrameBufferObjectLetterBoxedRectangle();
  DestroyShaderLetterBoxedRectangle();
  DestroyVertexBufferObjectLetterBoxedRectangle();
}

void cState::_Render(const spitfire::math::cTimeStep& timeStep)
{
  const size_t width = pContext->GetWidth();
  const size_t height = pContext->GetHeight();

  cLetterBox letterBox(width, height);

  if ((width == letterBox.desiredWidth) || (height == letterBox.desiredHeight)) {
    // Render the scene
    const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
    pContext->SetClearColour(clearColour);

    pContext->BeginRenderToScreen();

      if (bIsWireframe) pContext->EnableWireframe();

      _RenderToTexture(timeStep);

    pContext->EndRenderToScreen(*pWindow);
  } else {
    // Render the scene to a texture and draw the texture to the screen letter boxed

    if (pVertexBufferObjectLetterBoxedRectangle == nullptr) CreateVertexBufferObjectLetterBoxedRectangle(width, height);
    if (pFrameBufferObjectLetterBoxedRectangle == nullptr) CreateFrameBufferObjectLetterBoxedRectangle(width, height);
    if (pShaderLetterBoxedRectangle == nullptr) CreateShaderLetterBoxedRectangle();

    // Render the scene to the texture
    {
      const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pFrameBufferObjectLetterBoxedRectangle);

        if (bIsWireframe) pContext->EnableWireframe();

        _RenderToTexture(timeStep);

      pContext->EndRenderToTexture(*pFrameBufferObjectLetterBoxedRectangle);
    }

    // Render the texture to the screen
    {
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToScreen();

        //if (bIsWireframe) pContext->EnableWireframe();

        pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

          {
            // Set the position of the layer
            spitfire::math::cMat4 matModelView2D;
            if (letterBox.fRatio < letterBox.fDesiredRatio) matModelView2D.SetTranslation(0.0f, float((height - letterBox.letterBoxedHeight) / 2), 0.0f);
            else matModelView2D.SetTranslation(float((width - letterBox.letterBoxedWidth) / 2), 0.0f, 0.0f);

            pContext->EnableBlending();

            pContext->BindTexture(0, *pFrameBufferObjectLetterBoxedRectangle);

            pContext->BindShader(*pShaderLetterBoxedRectangle);

            pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_DIMENSIONS_AND_ASPECT_RATIO, matModelView2D);

            pContext->BindStaticVertexBufferObject2D(*pVertexBufferObjectLetterBoxedRectangle);
            pContext->DrawStaticVertexBufferObjectTriangles2D(*pVertexBufferObjectLetterBoxedRectangle);
            pContext->UnBindStaticVertexBufferObject2D(*pVertexBufferObjectLetterBoxedRectangle);

            pContext->UnBindShader(*pShaderLetterBoxedRectangle);

            pContext->UnBindTexture(0, *pFrameBufferObjectLetterBoxedRectangle);

            pContext->DisableBlending();
          }

        pContext->EndRenderMode2D();

      pContext->EndRenderToScreen(*pWindow);
    }
  }
}


// ** cBoardRepresentation

cBoardRepresentation::cBoardRepresentation(tetris::cBoard& _board, const spitfire::string_t& _sName) :
  board(_board),
  sName(_sName),

  pStaticVertexBufferObjectBoardTriangles(nullptr),
  pStaticVertexBufferObjectPieceTriangles(nullptr),
  pStaticVertexBufferObjectNextPieceTriangles(nullptr),

  bIsInputPieceMoveLeft(false),
  bIsInputPieceMoveRight(false),
  bIsInputPieceRotateCounterClockWise(false),
  bIsInputPieceRotateClockWise(false),
  bIsInputPieceDropOneRow(false),
  bIsInputPieceDropToGround(false),

  lastKeyLeft(0),
  lastKeyRight(0)
{
}


// ** cHighScoresTable

class cHighScoresTable
{
public:
  explicit cHighScoresTable(cSettings& settings);

  bool Load();
  bool Save();

  size_t GetEntryCount() const;
  const cHighScoresTableEntry& GetEntry(size_t index) const;
  bool IsScoreGoodEnough(int score) const;
  bool SubmitEntry(const spitfire::string_t& sName, int score);

private:
  void Clear();
  void Sort();

  cSettings& settings;

  static const size_t nMaxEntries = 10;

  std::vector<cHighScoresTableEntry> entries;
};

cHighScoresTable::cHighScoresTable(cSettings& _settings) :
  settings(_settings)
{
}

void cHighScoresTable::Clear()
{
  entries.clear();
}

void cHighScoresTable::Sort()
{
  // Sort the entries
  std::sort(entries.begin(), entries.end(), cHighScoresTableEntry::ScoreCompare);
}

bool cHighScoresTable::Load()
{
  Clear();

  entries = settings.GetHighScores();

  Sort();

  return true;
}

bool cHighScoresTable::Save()
{
  settings.SetHighScores(entries);

  return true;
}

size_t cHighScoresTable::GetEntryCount() const
{
  return entries.size();
}

const cHighScoresTableEntry& cHighScoresTable::GetEntry(size_t index) const
{
  assert(index < entries.size());
  return entries[index];
}

bool cHighScoresTable::IsScoreGoodEnough(int score) const
{
  const size_t n = entries.size();
  if (n < nMaxEntries) return true;

  // If the score better than the last score than it is not good enough to get in
  return (score > entries[nMaxEntries - 1].score);
}

bool cHighScoresTable::SubmitEntry(const spitfire::string_t& sName, int score)
{
  // If we don't have enough scores yet then add a new entry
  if (GetEntryCount() < nMaxEntries) {
    cHighScoresTableEntry entry;
    entry.sName = sName;
    entry.score = score;
    entries.push_back(entry);
    return true;
  }

  if (IsScoreGoodEnough(score)) {
    // Set the last entry to our new entry
    entries[nMaxEntries - 1].sName = sName;
    entries[nMaxEntries - 1].score = score;

    // Sort the entries
    Sort();
    return true;
  }

  return false;
}


spitfire::math::cSpring<spitfire::math::cVec2> spring;


// ** cStateMenu

cStateMenu::cStateMenu(cApplication& application) :
  cState(application),
  bIsKeyReturn(false)
{
  std::cout<<"cStateMenu::cStateMenu"<<std::endl;

  const size_t ids[] = {
    OPTION::NEW_GAME,
    OPTION::HIGH_SCORES,
    //OPTION::PREFERENCES,
    OPTION::QUIT,
  };
  const spitfire::string_t options[] = {
    TEXT("New Game"),
    TEXT("High Scores"),
    //TEXT("Preferences"),
    TEXT("Quit")
  };

  const float x = 0.04f;
  float y = 0.2f;

  const size_t n = countof(options);
  for (size_t i = 0; i < n; i++) {
    // Create the text for this option
    AddRetroButton(ids[i], options[i], x, y, 0.15f);

    y += pGuiManager->GetStaticTextHeight() + 0.007f;
  }

  pLayer->SetFocusToNextChild();


  breathe::gui::cWindow* pWindow = new breathe::gui::cWindow;
  pWindow->SetId(101);
  pWindow->SetCaption(TEXT("Caption"));
  pWindow->SetRelativePosition(spitfire::math::cVec2(0.1f, 0.15f));
  pWindow->SetWidth(0.05f + (2.0f * (0.1f + 0.05f)));
  pWindow->SetHeight(0.05f + (2.0f * (0.1f + 0.05f)));
  pLayer->AddChild(pWindow);

  //pWindow->SetVisible(false);

  breathe::gui::cStaticText* pStaticText = new breathe::gui::cStaticText;
  pStaticText->SetId(102);
  pStaticText->SetCaption(TEXT("StaticText"));
  pStaticText->SetRelativePosition(spitfire::math::cVec2(0.03f, 0.05f));
  pStaticText->SetWidth(0.15f);
  pStaticText->SetHeight(pGuiManager->GetStaticTextHeight());
  pWindow->AddChild(pStaticText);

  breathe::gui::cButton* pButton = new breathe::gui::cButton;
  pButton->SetId(103);
  pButton->SetCaption(TEXT("Button"));
  pButton->SetRelativePosition(spitfire::math::cVec2(pStaticText->GetX() + pStaticText->GetWidth() + 0.05f, 0.05f));
  pButton->SetWidth(0.15f);
  pButton->SetHeight(pGuiManager->GetButtonHeight());
  pWindow->AddChild(pButton);

  breathe::gui::cInput* pInput = new breathe::gui::cInput;
  pInput->SetId(104);
  pInput->SetCaption(TEXT("Input"));
  pInput->SetRelativePosition(spitfire::math::cVec2(pStaticText->GetX(), pStaticText->GetY() + pStaticText->GetHeight() + 0.05f));
  pInput->SetWidth(0.15f);
  pInput->SetHeight(pGuiManager->GetInputHeight());
  pWindow->AddChild(pInput);

  breathe::gui::cSlider* pSlider = new breathe::gui::cSlider;
  pSlider->SetId(105);
  pSlider->SetCaption(TEXT("Slider"));
  pSlider->SetRelativePosition(spitfire::math::cVec2(pStaticText->GetX() + pStaticText->GetWidth() + 0.05f, pStaticText->GetY() + pStaticText->GetHeight() + 0.05f));
  pSlider->SetWidth(0.15f);
  pSlider->SetHeight(0.1f);
  pWindow->AddChild(pSlider);
}

void cStateMenu::_Update(const spitfire::math::cTimeStep& timeStep)
{
  // Update the hud offset to shake the gui
  spring.Update(timeStep);

  pGuiManager->SetHUDOffset(spring.GetPosition());

  pGuiRenderer->Update();
}


void cStateMenu::_OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event)
{
  if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case breathe::gui::KEY::NUMBER_1: {
        std::cout<<"cStateMenu::_OnStateKeyboardEvent 1 up"<<std::endl;
        bIsWireframe = !bIsWireframe;
        break;
      }
      case breathe::gui::KEY::NUMBER_2: {
        std::cout<<"cStateMenu::_OnStateKeyboardEvent 2 up"<<std::endl;
        spring.SetPosition(spitfire::math::cVec2(0.0f, -0.05f));
        spring.SetVelocity(spitfire::math::cVec2(0.0f, -0.00001f));
        break;
      }
    }
  }
}

breathe::gui::EVENT_RESULT cStateMenu::_OnWidgetEvent(const breathe::gui::cWidgetEvent& event)
{
  std::cout<<"cStateMenu::_OnWidgetEvent"<<std::endl;

  if (event.IsPressed()) {
    switch (event.GetWidget()->GetId()) {
      case OPTION::NEW_GAME: {
        // Push our game state
        application.PushStateSoon(new cStateNewGame(application));
        break;
      }
      case OPTION::HIGH_SCORES: {
        // Push our high scores state
        application.PushStateSoon(new cStateHighScores(application));
        break;
      }
      //case OPTION::PREFERENCES: {
        //  // TODO: Add preferences for example tile set clasic or new
        //  break;
        //}
      case OPTION::QUIT: {
        // Pop our menu state
        application.PopStateSoon();
        break;
      }
    }
  }

  return breathe::gui::EVENT_RESULT::NOT_HANDLED_PERCOLATE;
}

void cStateMenu::_RenderToTexture(const spitfire::math::cTimeStep& timeStep)
{
  // Render the scene
  const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
  pContext->SetClearColour(clearColour);

  {
    if (pGuiRenderer != nullptr) {
      pGuiRenderer->SetWireFrame(bIsWireframe);
      pGuiRenderer->Render();
    }
  }
}


// ** cStateNewGame

cStateNewGame::cStateNewGame(cApplication& application) :
  cState(application),
  pNumberOfPlayers(nullptr),
  pPlayerName1(nullptr),
  pPlayerName2(nullptr),
  bIsKeyUp(false),
  bIsKeyDown(false),
  bIsKeyReturn(false),
  previousColour1(0),
  previousColour2(1)
{
  const float fSpacerVertical = 0.007f;
  const float fSpacerHorizontal = 0.007f;

  const float x = 0.04f;
  float y = 0.2f;
  const float width = 0.4f;

  AddStaticText(0, TEXT("Number of Players:"), x, y, width);
  pNumberOfPlayers = AddRetroInputUpDown(OPTION::NUMBER_OF_PLAYERS, 1, 2, settings.GetNumberOfPlayers(), x + width + fSpacerHorizontal, y, width);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;

  AddStaticText(0, TEXT("Player 1"), x, y, width);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
  AddStaticText(0, TEXT("Name:"), x, y, width);
  pPlayerName1 = AddRetroInput(OPTION::NAME_PLAYER1, settings.GetPlayerName(0), x + width + fSpacerHorizontal, y, width);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
  AddStaticText(0, TEXT("Colour:"), x, y, width);
  pPlayerColour1 = AddRetroColourPicker(OPTION::COLOUR_PLAYER1, x + width + fSpacerHorizontal, y, width);
  AddColours(pPlayerColour1);
  const spitfire::string_t sPlayerColour1 = settings.GetPlayerColour(0);
  previousColour1 = 0;
  if (sPlayerColour1 == TEXT("Red")) previousColour1 = 0;
  else if (sPlayerColour1 == TEXT("Blue")) previousColour1 = 1;
  else if (sPlayerColour1 == TEXT("Green")) previousColour1 = 2;
  else if (sPlayerColour1 == TEXT("Yellow")) previousColour1 = 3;
  pPlayerColour1->SetSelectedColour(previousColour1, false);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;

  AddStaticText(0, TEXT("Player 2"), x, y, width);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
  AddStaticText(0, TEXT("Name:"), x, y, width);
  pPlayerName2 = AddRetroInput(OPTION::NAME_PLAYER2, settings.GetPlayerName(1), x + width + fSpacerHorizontal, y, width);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
  AddStaticText(0, TEXT("Colour:"), x, y, width);
  pPlayerColour2 = AddRetroColourPicker(OPTION::COLOUR_PLAYER2, x + width + fSpacerHorizontal, y, width);
  AddColours(pPlayerColour2);
  const spitfire::string_t sPlayerColour2 = settings.GetPlayerColour(1);
  previousColour2 = 1;
  if (sPlayerColour2 == TEXT("Red")) previousColour2 = 0;
  else if (sPlayerColour2 == TEXT("Blue")) previousColour2 = 1;
  else if (sPlayerColour2 == TEXT("Green")) previousColour2 = 2;
  else if (sPlayerColour2 == TEXT("Yellow")) previousColour2 = 3;
  pPlayerColour2->SetSelectedColour(previousColour2, false);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;

  AddRetroButton(OPTION::START, TEXT("Start Game"), x, y, width);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
  AddRetroButton(OPTION::BACK, TEXT("Back"), x, y, width);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
}

void cStateNewGame::AddColours(breathe::gui::cRetroColourPicker* pColourPicker)
{
  pColourPicker->AddColour(TEXT("Red"), spitfire::math::cColour(1.0f, 0.0f, 0.0f));
  pColourPicker->AddColour(TEXT("Blue"), spitfire::math::cColour(0.0f, 0.0f, 1.0f));
  pColourPicker->AddColour(TEXT("Green"), spitfire::math::cColour(0.0f, 1.0f, 0.0f));
  pColourPicker->AddColour(TEXT("Yellow"), spitfire::math::cColour(1.0f, 1.0f, 0.0f));
}

void cStateNewGame::_OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event)
{
  if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case breathe::gui::KEY::UP: {
        std::cout<<"cStateNewGame::_OnStateKeyboardEvent Up"<<std::endl;
        bIsKeyUp = true;
        break;
      }
      case breathe::gui::KEY::DOWN: {
        std::cout<<"cStateNewGame::_OnStateKeyboardEvent Down"<<std::endl;
        bIsKeyDown = true;
        break;
      }
      case breathe::gui::KEY::RETURN: {
        std::cout<<"cStateNewGame::_OnStateKeyboardEvent Return"<<std::endl;
        bIsKeyReturn = true;
        break;
      }
    }
  }
}

breathe::gui::EVENT_RESULT cStateNewGame::_OnWidgetEvent(const breathe::gui::cWidgetEvent& event)
{
  std::cout<<"cStateNewGame::_OnWidgetEvent"<<std::endl;

  switch (event.GetWidget()->GetId()) {
    case OPTION::COLOUR_PLAYER1:
    case OPTION::COLOUR_PLAYER2: {
      if (event.IsChanged()) {
        // If the same colour has been selected for both players then veto the event
        if ((pPlayerColour1 != nullptr) && (pPlayerColour2 != nullptr)) {
          if (pPlayerColour1->GetSelectedColour() == pPlayerColour2->GetSelectedColour()) {
            if (event.GetWidget()->GetId() == OPTION::COLOUR_PLAYER1) {
              if (previousColour1 < pPlayerColour1->GetSelectedColour()) {
                // Going up
                size_t newColour = pPlayerColour1->GetSelectedColour() + 1;
                if (newColour < pPlayerColour1->GetNumberOfColours()) {
                  // Skip a colour
                  pPlayerColour1->SetSelectedColour(newColour, false);
                } else {
                  // Revert to the previous colour
                  pPlayerColour1->SetSelectedColour(previousColour1, false);
                }
              } else {
                // Going down
                if (pPlayerColour1->GetSelectedColour() == 0) {
                  // Revert to the previous colour
                  pPlayerColour1->SetSelectedColour(previousColour1, false);
                } else {
                  // Skip a colour
                  pPlayerColour1->SetSelectedColour(pPlayerColour1->GetSelectedColour() - 1, false);
                }
              }
            } else {
              if (previousColour2 < pPlayerColour2->GetSelectedColour()) {
                // Going up
                size_t newColour = pPlayerColour2->GetSelectedColour() + 1;
                if (newColour < pPlayerColour2->GetNumberOfColours()) {
                  // Skip a colour
                  pPlayerColour2->SetSelectedColour(newColour, false);
                } else {
                  // Revert to the previous colour
                  pPlayerColour2->SetSelectedColour(previousColour2, false);
                }
              } else {
                // Going down
                if (pPlayerColour2->GetSelectedColour() == 0) {
                  // Revert to the previous colour
                  pPlayerColour2->SetSelectedColour(previousColour2, false);
                } else {
                  // Skip a colour
                  pPlayerColour2->SetSelectedColour(pPlayerColour2->GetSelectedColour() - 1, false);
                }
              }
            }
          }

          // Update our previous colours
          previousColour1 = pPlayerColour1->GetSelectedColour();
          previousColour2 = pPlayerColour2->GetSelectedColour();
        }
      }

      break;
    }
    case OPTION::START: {
      if (event.IsPressed()) {
        // Update our settings
        settings.SetNumberOfPlayers(pNumberOfPlayers->GetValue());
        settings.SetPlayerName(0, pPlayerName1->GetCaption());
        settings.SetPlayerColour(0, pPlayerColour1->GetColourName(pPlayerColour1->GetSelectedColour()));
        settings.SetPlayerName(1, pPlayerName2->GetCaption());
        settings.SetPlayerColour(1, pPlayerColour2->GetColourName(pPlayerColour2->GetSelectedColour()));
        settings.Save();

        // Pop our current state
        application.PopStateSoon();
        // Push our game state
        application.PushStateSoon(new cStateGame(application));
      }
      break;
    }
    case OPTION::BACK: {
      if (event.IsPressed()) {
        // Pop our menu state
        application.PopStateSoon();
      }
      break;
    }
  }

  return breathe::gui::EVENT_RESULT::NOT_HANDLED_PERCOLATE;
}

void cStateNewGame::_Update(const spitfire::math::cTimeStep& timeStep)
{
  // Update the hud offset to shake the gui
  spring.Update(timeStep);

  pGuiManager->SetHUDOffset(spring.GetPosition());

  pGuiRenderer->Update();
}

void cStateNewGame::_RenderToTexture(const spitfire::math::cTimeStep& timeStep)
{
  // Render the scene
  const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
  pContext->SetClearColour(clearColour);

  {
    if (pGuiRenderer != nullptr) {
      pGuiRenderer->SetWireFrame(bIsWireframe);
      pGuiRenderer->Render();
    }
  }
}


// ** cStateHighScores

cStateHighScores::cStateHighScores(cApplication& application) :
  cState(application),
  bIsDone(false)
{
  const float fSpacerVertical = 0.007f;

  const float x = 0.04f;
  float y = 0.2f;
  const float width = 0.4f;

  const spitfire::math::cColour white(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);

  breathe::gui::cStaticText* pStaticText = nullptr;


  // Header
  pStaticText = AddStaticText(0, TEXT("High Scores"), x, y, width);
  pStaticText->SetTextColour(red);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
  y += 0.05f;

  pStaticText = AddStaticText(0, TEXT("Name"), x, y, width);
  pStaticText->SetTextColour(red);
  pStaticText = AddStaticText(0, TEXT("Score"), x + 0.2f, y, width);
  pStaticText->SetTextColour(red);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;


  // Scores
  cHighScoresTable table(settings);
  table.Load();

  const size_t n = table.GetEntryCount();
  for (size_t i = 0; i < n; i++) {
    const cHighScoresTableEntry& entry = table.GetEntry(i);
    spitfire::ostringstream_t o;
    o<<entry.score;
    pStaticText = AddStaticText(0, entry.sName, x, y, width);
    pStaticText->SetTextColour(white);
    pStaticText = AddStaticText(0, o.str(), x + 0.2f, y, width);
    pStaticText->SetTextColour(white);
    y += pGuiManager->GetStaticTextHeight();
  }

  y += fSpacerVertical;

  AddRetroButton(OPTION::BACK, TEXT("Back"), x, y, width);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
}

void cStateHighScores::_OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event)
{
  if (event.IsKeyDown()) {
    switch (event.GetKeyCode()) {
      case breathe::gui::KEY::ESCAPE:
      case breathe::gui::KEY::RETURN: {
        bIsDone = true;
        break;
      }
    }
  }
}

breathe::gui::EVENT_RESULT cStateHighScores::_OnWidgetEvent(const breathe::gui::cWidgetEvent& event)
{
  std::cout<<"cStateHighScores::_OnWidgetEvent"<<std::endl;

  if (event.IsPressed()) {
    switch (event.GetWidget()->GetId()) {
      case OPTION::BACK: {
        // Pop our menu state
        application.PopStateSoon();
        break;
      }
    }
  }

  return breathe::gui::EVENT_RESULT::NOT_HANDLED_PERCOLATE;
}

void cStateHighScores::_Update(const spitfire::math::cTimeStep& timeStep)
{
  // Update the hud offset to shake the gui
  spring.Update(timeStep);

  pGuiManager->SetHUDOffset(spring.GetPosition());

  pGuiRenderer->Update();
}

void cStateHighScores::_UpdateInput(const spitfire::math::cTimeStep& timeStep)
{
  if (bIsDone) {
    bIsDone = false;

    // Pop our high scores state
    application.PopStateSoon();
  }
}

void cStateHighScores::_RenderToTexture(const spitfire::math::cTimeStep& timeStep)
{
  // Render the scene
  const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
  pContext->SetClearColour(clearColour);

  {
    if (pGuiRenderer != nullptr) {
      pGuiRenderer->SetWireFrame(bIsWireframe);
      pGuiRenderer->Render();
    }
  }
}


// ** cStatePauseMenu

cStatePauseMenu::cStatePauseMenu(cApplication& application, cStateGame& _parentState) :
  cState(application),
  parentState(_parentState),
  bIsKeyReturn(false)
{
  std::cout<<"cStatePauseMenu::cStatePauseMenu"<<std::endl;
  
  const float fSpacerVertical = 0.007f;

  const float x = 0.04f;
  float y = 0.2f;
  const float width = 0.4f;

  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);

  breathe::gui::cStaticText* pStaticText = nullptr;

  // Header
  pStaticText = AddStaticText(0, TEXT("Paused"), x, y, width);
  pStaticText->SetTextColour(red);
  y += pGuiManager->GetStaticTextHeight() + fSpacerVertical;
  y += 0.05f;


  const size_t ids[] = {
    OPTION::RESUME_GAME,
    OPTION::END_GAME,
  };
  const spitfire::string_t options[] = {
    TEXT("Resum Game"),
    TEXT("End Game")
  };

  const size_t n = countof(options);
  for (size_t i = 0; i < n; i++) {
    // Create the text for this option
    AddRetroButton(ids[i], options[i], x, y, 0.15f);

    y += pGuiManager->GetStaticTextHeight() + 0.007f;
  }

  pLayer->SetFocusToNextChild();
}

void cStatePauseMenu::_Update(const spitfire::math::cTimeStep& timeStep)
{
  // Update the hud offset to shake the gui
  spring.Update(timeStep);

  pGuiManager->SetHUDOffset(spring.GetPosition());

  pGuiRenderer->Update();
}

void cStatePauseMenu::_OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event)
{
}

breathe::gui::EVENT_RESULT cStatePauseMenu::_OnWidgetEvent(const breathe::gui::cWidgetEvent& event)
{
  std::cout<<"cStatePauseMenu::_OnWidgetEvent"<<std::endl;

  if (event.IsPressed()) {
    switch (event.GetWidget()->GetId()) {
      case OPTION::RESUME_GAME: {
        // Pop our menu state
        application.PopStateSoon();
        break;
      }
      case OPTION::END_GAME: {
        // Pop our menu state
        application.PopStateSoon();
        parentState.SetQuitSoon();
        break;
      }
    }
  }

  return breathe::gui::EVENT_RESULT::NOT_HANDLED_PERCOLATE;
}

void cStatePauseMenu::_RenderToTexture(const spitfire::math::cTimeStep& timeStep)
{
  // Render the scene
  const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
  pContext->SetClearColour(clearColour);

  {
    if (pGuiRenderer != nullptr) {
      pGuiRenderer->SetWireFrame(bIsWireframe);
      pGuiRenderer->Render();
    }
  }
}


// ** cStateGame

cStateGame::cStateGame(cApplication& application) :
  cState(application),

  pTextureBlock(nullptr),

  pShaderBlock(nullptr),

  game(*this),

  bPauseSoon(false),
  bQuitSoon(false)
{
  pTextureBlock = pContext->CreateTexture(TEXT("data/textures/block.png"));

  pShaderBlock = pContext->CreateShader(TEXT("data/shaders/passthroughwithcolour.vert"), TEXT("data/shaders/passthroughwithcolour.frag"));

  const spitfire::durationms_t currentTime = SDL_GetTicks();

  spitfire::math::SetRandomSeed(currentTime);

  game.boards.push_back(new tetris::cBoard(game));
  if (settings.GetNumberOfPlayers() != 1) game.boards.push_back(new tetris::cBoard(game));

  game.StartGame(currentTime);

  //const tetris::cBoard& board = *(game.boards[0]);
  //width = board.GetWidth();
  //height = board.GetHeight();

  //scale.Set(0.2f, 0.2f, 10.0f);

  for (size_t i = 0; i < game.boards.size(); i++) {
    tetris::cBoard& board = *(game.boards[i]);

    cBoardRepresentation* pBoardRepresentation = new cBoardRepresentation(board, settings.GetPlayerName(i));

    pBoardRepresentation->pStaticVertexBufferObjectBoardTriangles = pContext->CreateStaticVertexBufferObject();
    UpdateBoardVBO(pBoardRepresentation->pStaticVertexBufferObjectBoardTriangles, board);

    pBoardRepresentation->pStaticVertexBufferObjectPieceTriangles = pContext->CreateStaticVertexBufferObject();
    UpdatePieceVBO(pBoardRepresentation->pStaticVertexBufferObjectPieceTriangles, board, board.GetCurrentPiece());

    pBoardRepresentation->pStaticVertexBufferObjectNextPieceTriangles = pContext->CreateStaticVertexBufferObject();
    UpdatePieceVBO(pBoardRepresentation->pStaticVertexBufferObjectNextPieceTriangles, board, board.GetNextPiece());

    boardRepresentations.push_back(pBoardRepresentation);
  }


  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);
  const spitfire::math::cColour green(0.0f, 1.0f, 0.0f);
  const spitfire::math::cColour blue(0.0f, 0.0f, 1.0f);
  const spitfire::math::cColour yellow(1.0f, 1.0f, 0.0f);

  // Text
  const float x = 0.02f;
  float y = 0.2f;
  const float width = 0.4f;

  for (size_t i = 0; i < game.boards.size(); i++) {
    const spitfire::string_t sColour = settings.GetPlayerColour(i);
    spitfire::math::cColour colour = red;
    if (sColour == TEXT("Green")) colour = green;
    else if (sColour == TEXT("Blue")) colour = blue;
    else if (sColour == TEXT("Yellow")) colour = yellow;

    // Create the text for this board
    breathe::gui::cStaticText* pName = AddStaticText(0, settings.GetPlayerName(i), x, y, width);
    pName->SetTextColour(colour);
    y += pGuiManager->GetStaticTextHeight();

    spitfire::ostringstream_t o;

    pLevelText[i] = AddStaticText(0, TEXT("Level 1"), x, y, width);
    pLevelText[i]->SetTextColour(colour);
    y += pGuiManager->GetStaticTextHeight();
    pScoreText[i] = AddStaticText(0, TEXT("Score 0"), x, y, width);
    pScoreText[i]->SetTextColour(colour);
    y += pGuiManager->GetStaticTextHeight();

    y += 0.05f;
  }

  UpdateText();


  // Load our sounds
  pAudioBufferPieceHitsGround = pAudioManager->CreateBuffer(TEXT("data/audio/piece_hits_ground.wav"));
  assert(pAudioBufferPieceHitsGround != nullptr);
  pAudioBufferScoreTetris = pAudioManager->CreateBuffer(TEXT("data/audio/score_tetris.wav"));
  assert(pAudioBufferScoreTetris != nullptr);
  pAudioBufferScoreOtherThanTetris = pAudioManager->CreateBuffer(TEXT("data/audio/score_other.wav"));
  assert(pAudioBufferScoreOtherThanTetris != nullptr);
  pAudioBufferGameOver = pAudioManager->CreateBuffer(TEXT("data/audio/game_over.wav"));
  assert(pAudioBufferGameOver != nullptr);
}

cStateGame::~cStateGame()
{
  std::cout<<"cStateGame::~cStateGame"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    cBoardRepresentation* pBoardRepresentation = boardRepresentations[i];
    if (pBoardRepresentation->pStaticVertexBufferObjectNextPieceTriangles != nullptr) {
      pContext->DestroyStaticVertexBufferObject(pBoardRepresentation->pStaticVertexBufferObjectNextPieceTriangles);
      pBoardRepresentation->pStaticVertexBufferObjectNextPieceTriangles = nullptr;
    }
    if (pBoardRepresentation->pStaticVertexBufferObjectPieceTriangles != nullptr) {
      pContext->DestroyStaticVertexBufferObject(pBoardRepresentation->pStaticVertexBufferObjectPieceTriangles);
      pBoardRepresentation->pStaticVertexBufferObjectPieceTriangles = nullptr;
    }
    if (pBoardRepresentation->pStaticVertexBufferObjectBoardTriangles != nullptr) {
      pContext->DestroyStaticVertexBufferObject(pBoardRepresentation->pStaticVertexBufferObjectBoardTriangles);
      pBoardRepresentation->pStaticVertexBufferObjectBoardTriangles = nullptr;
    }
    spitfire::SAFE_DELETE(pBoardRepresentation);
  }

  boardRepresentations.clear();

  game.boards.clear();

  if (pShaderBlock != nullptr) {
    pContext->DestroyShader(pShaderBlock);
    pShaderBlock = nullptr;
  }

  if (pTextureBlock != nullptr) {
    pContext->DestroyTexture(pTextureBlock);
    pTextureBlock = nullptr;
  }


  std::cout<<"cStateGame::~cStateGame returning"<<std::endl;
}

void cStateGame::SetQuitSoon()
{
  bQuitSoon = true;
}

void cStateGame::UpdateText()
{
  for (size_t i = 0; i < game.boards.size(); i++) {
    tetris::cBoard& board = *(game.boards[i]);

    spitfire::ostringstream_t o;

    const uint32_t uiLevel = board.GetLevel();
    o<<TEXT("Level ");
    o<<uiLevel;
    pLevelText[i]->SetCaption(o.str());
    o.str(TEXT(""));

    const uint32_t uiScore = board.GetScore();
    o<<TEXT("Score ");
    o<<uiScore;
    pScoreText[i]->SetCaption(o.str());
    o.str(TEXT(""));
  }
}

void cStateGame::UpdateBoardVBO(breathe::render::cVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board)
{
  assert(pStaticVertexBufferObject != nullptr);

  //if (boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles != nullptr) {
  //  pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles);
  //  boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles = nullptr;
  //}
  //pBoardRepresentation->pStaticVertexBufferObjectBoardTriangles = pContext->CreateStaticVertexBufferObject();

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v2_c4_t2 builder(*pGeometryDataPtr);

  const spitfire::math::cVec2 scale(0.015f, 0.015f);

  const float fBlockAndLightmapU = 0.0f;
  const float fBlockAndLightmapV = 0.0f;
  const float fBlockAndLightmapU2 = 1.0f;
  const float fBlockAndLightmapV2 = 1.0f;

  const size_t width = board.GetWidth();
  const size_t height = board.GetHeight();
  for (size_t _y = 0; _y < height; _y++) {
    for (size_t x = 0; x < width; x++) {
      // We want to add the blocks in upside down order
      const size_t y = (height - 1) - _y;

      int c = board.GetBlock(x, _y);
      {
        const spitfire::math::cColour colour(board.GetColour(c));

        // Front facing triangles
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y + 1)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV2));
        builder.PushBack(scale * spitfire::math::cVec2(float(x + 1), float(y + 1)), colour, spitfire::math::cVec2(fBlockAndLightmapU2, fBlockAndLightmapV2));
        builder.PushBack(scale * spitfire::math::cVec2(float(x + 1), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU2, fBlockAndLightmapV));
        builder.PushBack(scale * spitfire::math::cVec2(float(x + 1), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU2, fBlockAndLightmapV));
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV));
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y + 1)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV2));
      }
    }
  }

  if (pGeometryDataPtr->nVertexCount != 0) {
    pStaticVertexBufferObject->SetData(pGeometryDataPtr);

    pStaticVertexBufferObject->Compile2D(system);
  }
}

void cStateGame::UpdatePieceVBO(breathe::render::cVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board, const tetris::cPiece& piece)
{
  std::cout<<"cStateGame::UpdatePieceVBO"<<std::endl;
  assert(pStaticVertexBufferObject != nullptr);

  //if (boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles != nullptr) {
  //  pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles);
  //  boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles = nullptr;
  //}
  //pBoardRepresentation->pStaticVertexBufferObjectBoardTriangles = pContext->CreateStaticVertexBufferObject();

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v2_c4_t2 builder(*pGeometryDataPtr);

  const spitfire::math::cVec2 scale(0.015f, 0.015f);

  const float fBlockAndLightmapU = 0.0f;
  const float fBlockAndLightmapV = 0.0f;
  const float fBlockAndLightmapU2 = 1.0f;
  const float fBlockAndLightmapV2 = 1.0f;

  const size_t width = piece.GetWidth();
  const size_t height = piece.GetHeight();
  for (size_t _y = 0; _y < height; _y++) {
    for (size_t x = 0; x < width; x++) {
      // We want to add the blocks in upside down order
      const size_t y = (height - 1) - _y;

      int c = piece.GetBlock(x, _y);
      if (c != 0) {
        const spitfire::math::cColour colour(board.GetColour(c));

        // Front facing quad
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y + 1)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV2));
        builder.PushBack(scale * spitfire::math::cVec2(float(x + 1), float(y + 1)), colour, spitfire::math::cVec2(fBlockAndLightmapU2, fBlockAndLightmapV2));
        builder.PushBack(scale * spitfire::math::cVec2(float(x + 1), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU2, fBlockAndLightmapV));
        builder.PushBack(scale * spitfire::math::cVec2(float(x + 1), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU2, fBlockAndLightmapV));
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV));
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y + 1)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV2));
      }
    }
  }

  if (pGeometryDataPtr->nVertexCount != 0) {
    pStaticVertexBufferObject->SetData(pGeometryDataPtr);

    pStaticVertexBufferObject->Compile2D(system);
  }
}

void cStateGame::_OnPieceMoved(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnPieceMoved"<<std::endl;
  //... update piece position
}

void cStateGame::_OnPieceRotated(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnPieceRotated"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    if (&boardRepresentations[i]->board == &board) {
      if (boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles);
        boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles = pContext->CreateStaticVertexBufferObject();
      UpdatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles, board, board.GetCurrentPiece());
    }
  }
}

void cStateGame::_OnPieceChanged(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnPieceChanged"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    if (&boardRepresentations[i]->board == &board) {
      if (boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles);
        boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles = pContext->CreateStaticVertexBufferObject();
      UpdatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectPieceTriangles, board, board.GetCurrentPiece());

      if (boardRepresentations[i]->pStaticVertexBufferObjectNextPieceTriangles != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectNextPieceTriangles);
        boardRepresentations[i]->pStaticVertexBufferObjectNextPieceTriangles = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectNextPieceTriangles = pContext->CreateStaticVertexBufferObject();
      UpdatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectNextPieceTriangles, board, board.GetNextPiece());
    }
  }
}

void cStateGame::_OnPieceHitsGround(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnPieceHitsGround"<<std::endl;
  application.PlaySound(pAudioBufferPieceHitsGround);

  // Shake the gui
  spring.SetPosition(spitfire::math::cVec2(0.0f, -0.02f));
  spring.SetVelocity(spitfire::math::cVec2(0.0f, -0.00001f));
}

void cStateGame::_OnBoardChanged(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnBoardChanged"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    if (&boardRepresentations[i]->board == &board) {
      if (boardRepresentations[i]->pStaticVertexBufferObjectBoardTriangles != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectBoardTriangles);
        boardRepresentations[i]->pStaticVertexBufferObjectBoardTriangles = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectBoardTriangles = pContext->CreateStaticVertexBufferObject();
      UpdateBoardVBO(boardRepresentations[i]->pStaticVertexBufferObjectBoardTriangles, board);
    }
  }
}

void cStateGame::_OnGameScoreTetris(const tetris::cBoard& board, uint32_t uiScore)
{
  std::cout<<"cStateGame::_OnGameScoreTetris"<<std::endl;
  application.PlaySound(pAudioBufferScoreTetris);
  UpdateText();

  // Shake the gui
  spring.SetPosition(spitfire::math::cVec2(0.0f, -0.05f));
  spring.SetVelocity(spitfire::math::cVec2(0.0f, -0.00001f));
}

void cStateGame::_OnGameScoreOtherThanTetris(const tetris::cBoard& board, uint32_t uiScore)
{
  std::cout<<"cStateGame::_OnGameScoreOtherThanTetris"<<std::endl;
  application.PlaySound(pAudioBufferScoreOtherThanTetris);
  UpdateText();
}

void cStateGame::_OnGameNewLevel(const tetris::cBoard& board, uint32_t uiLevel)
{
  std::cout<<"cStateGame::_OnGameNewLevel"<<std::endl;
  //... show new level message
}

void cStateGame::_OnGameOver(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnGameOver"<<std::endl;
  application.PlaySound(pAudioBufferGameOver);
  //... show game over screen, stop game

  cHighScoresTable table(settings);
  table.Load();

  if (table.IsScoreGoodEnough(board.GetScore())) {
    const size_t n = boardRepresentations.size();
    for (size_t i = 0; i < n; i++) {
      if (&boardRepresentations[i]->board == &board) {
        table.SubmitEntry(boardRepresentations[i]->sName, board.GetScore());
        table.Save();
      }
    }
  }
}

void cStateGame::_OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event)
{
  std::cout<<"cStateGame::_OnStateKeyboardEvent"<<std::endl;

  if (event.IsKeyDown()) {
    std::cout<<"cStateGame::_OnStateKeyboardEvent Key down"<<std::endl;
    switch (event.GetKeyCode()) {
      case breathe::gui::KEY::ESCAPE: {
        std::cout<<"cStateGame::_OnStateKeyboardEvent Escape down"<<std::endl;
        bPauseSoon = true;
        break;
      }
    }
  } else if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case breathe::gui::KEY::NUMBER_1: {
        std::cout<<"cStateGame::_OnStateKeyboardEvent 1 up"<<std::endl;
        bIsWireframe = !bIsWireframe;
        break;
      }
    }
  }
}

  void cStateGame::_OnStateJoystickEvent(const breathe::util::cJoystickEvent& event)
  {
    const size_t index = event.GetIndex();
    const size_t n = boardRepresentations.size();
    if (index > n) return;

    // Convert the joystick button up events to keyboard events
    if (event.IsButtonUp()) {
      cBoardRepresentation* pBoardRepresentation = boardRepresentations[index];

      switch (event.GetButton()) {
        case breathe::util::GAMECONTROLLER_BUTTON::DPAD_LEFT: {
          pBoardRepresentation->bIsInputPieceMoveLeft = true;
          break;
        }
        case breathe::util::GAMECONTROLLER_BUTTON::DPAD_RIGHT: {
          pBoardRepresentation->bIsInputPieceMoveRight = true;
          break;
        }
        case breathe::util::GAMECONTROLLER_BUTTON::DPAD_UP: {
          pBoardRepresentation->bIsInputPieceRotateCounterClockWise = true;
          break;
        }
        case breathe::util::GAMECONTROLLER_BUTTON::DPAD_DOWN: {
          pBoardRepresentation->bIsInputPieceDropOneRow = true;
          break;
        }

        case breathe::util::GAMECONTROLLER_BUTTON::X: {
          pBoardRepresentation->bIsInputPieceRotateCounterClockWise = true;
          break;
        }
        case breathe::util::GAMECONTROLLER_BUTTON::B: {
          pBoardRepresentation->bIsInputPieceRotateClockWise = true;
          break;
        }

        case breathe::util::GAMECONTROLLER_BUTTON::A: {
          pBoardRepresentation->bIsInputPieceDropToGround = true;
          break;
        }

        case breathe::util::GAMECONTROLLER_BUTTON::START: {
          bPauseSoon = true;
          break;
        }
      }
    }
  }

void cStateGame::_Update(const spitfire::math::cTimeStep& timeStep)
{
  // For each board we need to send the inputs to the board
  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    cBoardRepresentation* pBoardRepresentation = boardRepresentations[i];
    tetris::cBoard& board = pBoardRepresentation->board;

    if (pBoardRepresentation->bIsInputPieceRotateCounterClockWise) {
      board.PieceRotateCounterClockWise();
      pBoardRepresentation->bIsInputPieceRotateCounterClockWise = false;
    }
    if (pBoardRepresentation->bIsInputPieceRotateClockWise) {
      board.PieceRotateClockWise();
      pBoardRepresentation->bIsInputPieceRotateClockWise = false;
    }
    if (pBoardRepresentation->bIsInputPieceDropOneRow) {
      board.PieceDropOneRow(timeStep.GetCurrentTimeMS());
      pBoardRepresentation->bIsInputPieceDropOneRow = false;
    }
    if (pBoardRepresentation->bIsInputPieceDropToGround) {
      board.PieceDropToGround(timeStep.GetCurrentTimeMS());
      pBoardRepresentation->bIsInputPieceDropToGround = false;
    }
    if (pBoardRepresentation->bIsInputPieceMoveLeft) {
      board.PieceMoveLeft();
      pBoardRepresentation->bIsInputPieceMoveLeft = false;
    }
    if (pBoardRepresentation->bIsInputPieceMoveRight) {
      board.PieceMoveRight();
      pBoardRepresentation->bIsInputPieceMoveRight = false;
    }
  }

  if (bPauseSoon) {
    // Push our game state
    application.PushStateSoon(new cStatePauseMenu(application, *this));
    bPauseSoon = false;
  }
  if (bQuitSoon) {
    // Pop our menu state
    application.PopStateSoon();
  }

  game.Update(timeStep.GetCurrentTimeMS());

  // Update the hud offset to shake the gui
  spring.Update(timeStep);

  pGuiManager->SetHUDOffset(spring.GetPosition());

  pGuiRenderer->Update();
}

void cStateGame::_UpdateInput(const spitfire::math::cTimeStep& timeStep)
{
  assert(pWindow != nullptr);

  if (boardRepresentations.size() == 1) {
    // Single player
    cBoardRepresentation* pBoardRepresentation = boardRepresentations[0];

    // Player 1
    if (pWindow->IsKeyUp(breathe::gui::KEY::BACKSLASH)) {
      //std::cout<<"cStateGame::UpdateInput BACKSLASH up"<<std::endl;
      pBoardRepresentation->bIsInputPieceRotateCounterClockWise = true;
    }
    if (pWindow->IsKeyUp(breathe::gui::KEY::UP)) {
      //std::cout<<"cStateGame::UpdateInput UP up"<<std::endl;
      pBoardRepresentation->bIsInputPieceRotateClockWise = true;
    }
    if (pWindow->IsKeyHeld(breathe::gui::KEY::DOWN)) {
      //std::cout<<"cStateGame::UpdateInput DOWN held"<<std::endl;
      pBoardRepresentation->bIsInputPieceDropOneRow = true;
    }
    if (pWindow->IsKeyUp(breathe::gui::KEY::SPACE)) {
      //std::cout<<"cStateGame::UpdateInput SPACE up"<<std::endl;
      pBoardRepresentation->bIsInputPieceDropToGround = true;
    }
    if (pWindow->IsKeyHeld(breathe::gui::KEY::LEFT)) {
      //std::cout<<"cStateGame::UpdateInput LEFT Held"<<std::endl;
      if ((timeStep.GetCurrentTimeMS() - pBoardRepresentation->lastKeyLeft) > 50) {
        pBoardRepresentation->bIsInputPieceMoveLeft = true;
        pBoardRepresentation->lastKeyLeft = timeStep.GetCurrentTimeMS();
      }
    }
    if (pWindow->IsKeyHeld(breathe::gui::KEY::RIGHT)) {
      //std::cout<<"cStateGame::UpdateInput RIGHT Held"<<std::endl;
      if ((timeStep.GetCurrentTimeMS() - pBoardRepresentation->lastKeyRight) > 50) {
        pBoardRepresentation->bIsInputPieceMoveRight = true;
        pBoardRepresentation->lastKeyRight = timeStep.GetCurrentTimeMS();
      }
    }
  } else {
    {
      // Player 1
      cBoardRepresentation* pBoardRepresentation = boardRepresentations[0];

      if (pWindow->IsKeyUp(breathe::gui::KEY::Q)) {
        //std::cout<<"cStateGame::UpdateInput Q up"<<std::endl;
        pBoardRepresentation->bIsInputPieceRotateCounterClockWise = true;
      }
      if (pWindow->IsKeyUp(breathe::gui::KEY::W)) {
        //std::cout<<"cStateGame::UpdateInput W up"<<std::endl;
        pBoardRepresentation->bIsInputPieceRotateClockWise = true;
      }
      if (pWindow->IsKeyHeld(breathe::gui::KEY::S)) {
        //std::cout<<"cStateGame::UpdateInput S Held"<<std::endl;
        pBoardRepresentation->bIsInputPieceDropOneRow = true;
      }
      if (pWindow->IsKeyUp(breathe::gui::KEY::F)) {
        //std::cout<<"cStateGame::UpdateInput F up"<<std::endl;
        pBoardRepresentation->bIsInputPieceDropToGround = true;
      }
      if (pWindow->IsKeyHeld(breathe::gui::KEY::A)) {
        //std::cout<<"cStateGame::UpdateInput A Held"<<std::endl;
        if ((timeStep.GetCurrentTimeMS() - pBoardRepresentation->lastKeyLeft) > 50) {
          pBoardRepresentation->bIsInputPieceMoveLeft = true;
          pBoardRepresentation->lastKeyLeft = timeStep.GetCurrentTimeMS();
        }
      }
      if (pWindow->IsKeyHeld(breathe::gui::KEY::D)) {
        //std::cout<<"cStateGame::UpdateInput D Held"<<std::endl;
        if ((timeStep.GetCurrentTimeMS() - pBoardRepresentation->lastKeyRight) > 50) {
          pBoardRepresentation->bIsInputPieceMoveRight = true;
          pBoardRepresentation->lastKeyRight = timeStep.GetCurrentTimeMS();
        }
      }
    }

    {
      // Player 2
      cBoardRepresentation* pBoardRepresentation = boardRepresentations[1];

      if (pWindow->IsKeyUp(breathe::gui::KEY::BACKSLASH)) {
        //std::cout<<"cStateGame::UpdateInput BACKSLASH up"<<std::endl;
        pBoardRepresentation->bIsInputPieceRotateCounterClockWise = true;
      }
      if (pWindow->IsKeyUp(breathe::gui::KEY::UP)) {
        //std::cout<<"cStateGame::UpdateInput UP up"<<std::endl;
        pBoardRepresentation->bIsInputPieceRotateClockWise = true;
      }
      if (pWindow->IsKeyHeld(breathe::gui::KEY::DOWN)) {
        //std::cout<<"cStateGame::UpdateInput DOWN Held"<<std::endl;
        pBoardRepresentation->bIsInputPieceDropOneRow = true;
      }
      if (pWindow->IsKeyUp(breathe::gui::KEY::SPACE)) {
        //std::cout<<"cStateGame::UpdateInput SPACE up"<<std::endl;
        pBoardRepresentation->bIsInputPieceDropToGround = true;
      }
      if (pWindow->IsKeyHeld(breathe::gui::KEY::LEFT)) {
        //std::cout<<"cStateGame::UpdateInput LEFT Held"<<std::endl;
        if ((timeStep.GetCurrentTimeMS() - pBoardRepresentation->lastKeyLeft) > 50) {
          pBoardRepresentation->bIsInputPieceMoveLeft = true;
          pBoardRepresentation->lastKeyLeft = timeStep.GetCurrentTimeMS();
        }
      }
      if (pWindow->IsKeyHeld(breathe::gui::KEY::RIGHT)) {
        //std::cout<<"cStateGame::UpdateInput RIGHT Held"<<std::endl;
        if ((timeStep.GetCurrentTimeMS() - pBoardRepresentation->lastKeyRight) > 50) {
          pBoardRepresentation->bIsInputPieceMoveRight = true;
          pBoardRepresentation->lastKeyRight = timeStep.GetCurrentTimeMS();
        }
      }
    }
  }
}

void cStateGame::_RenderToTexture(const spitfire::math::cTimeStep& timeStep)
{
  // Render the scene
  const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
  pContext->SetClearColour(clearColour);

  if (bIsWireframe) pContext->EnableWireframe();

  {
    pContext->BeginRenderMode2D(breathe::render::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_ASPECT_RATIO);

    // Draw the boards
    {
      assert(!game.boards.empty());

      float x = 0.5f;
      const float y = 0.1f;

      const size_t n = boardRepresentations.size();
      for (size_t i = 0; i < n; i++) {
        cBoardRepresentation* pBoardRepresentation = boardRepresentations[i];
        //const tetris::cBoard& board = pBoardRepresentation->board;

        breathe::render::cVertexBufferObject* pStaticVertexBufferObjectBoardTriangles = pBoardRepresentation->pStaticVertexBufferObjectBoardTriangles;
        if ((pStaticVertexBufferObjectBoardTriangles != nullptr) && pStaticVertexBufferObjectBoardTriangles->IsCompiled()) {
          spitfire::math::cMat4 matModelView2D;
          matModelView2D.SetTranslation(x, y, 0.0f);

          pContext->BindTexture(0, *pTextureBlock);

          pContext->BindShader(*pShaderBlock);

          pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(breathe::render::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_ASPECT_RATIO, matModelView2D);

          pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectBoardTriangles);
          pContext->DrawStaticVertexBufferObjectTriangles2D(*pStaticVertexBufferObjectBoardTriangles);
          pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectBoardTriangles);

          pContext->UnBindShader(*pShaderBlock);

          pContext->UnBindTexture(0, *pTextureBlock);
        }

        breathe::render::cVertexBufferObject* pStaticVertexBufferObjectPieceTriangles = pBoardRepresentation->pStaticVertexBufferObjectPieceTriangles;
        if (pBoardRepresentation->board.IsPlaying() && (pStaticVertexBufferObjectPieceTriangles != nullptr) && pStaticVertexBufferObjectPieceTriangles->IsCompiled()) {
          spitfire::math::cMat4 matModelView2D;
          matModelView2D.SetTranslation(x + (0.015f * float(pBoardRepresentation->board.GetCurrentPieceX())), y + (0.015f * (float(pBoardRepresentation->board.GetHeight()) - float(pBoardRepresentation->board.GetCurrentPieceY()))), 0.0f);

          pContext->BindTexture(0, *pTextureBlock);

          pContext->BindShader(*pShaderBlock);

          pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(breathe::render::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_ASPECT_RATIO, matModelView2D);

          pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectPieceTriangles);
          pContext->DrawStaticVertexBufferObjectTriangles2D(*pStaticVertexBufferObjectPieceTriangles);
          pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectPieceTriangles);

          pContext->UnBindShader(*pShaderBlock);

          pContext->UnBindTexture(0, *pTextureBlock);
        }

        breathe::render::cVertexBufferObject* pStaticVertexBufferObjectNextPieceTriangles = pBoardRepresentation->pStaticVertexBufferObjectNextPieceTriangles;
        if ((pStaticVertexBufferObjectPieceTriangles != nullptr) && pStaticVertexBufferObjectNextPieceTriangles->IsCompiled()) {
          spitfire::math::cMat4 matModelView2D;
          matModelView2D.SetTranslation(x + (0.015f * float(pBoardRepresentation->board.GetWidth())) + (0.015f * 3.0f), y + (0.015f * (0.5f * float(pBoardRepresentation->board.GetHeight()))), 0.0f);

          pContext->BindTexture(0, *pTextureBlock);

          pContext->BindShader(*pShaderBlock);

          pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(breathe::render::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_ASPECT_RATIO, matModelView2D);

          pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectNextPieceTriangles);
          pContext->DrawStaticVertexBufferObjectTriangles2D(*pStaticVertexBufferObjectNextPieceTriangles);
          pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectNextPieceTriangles);

          pContext->UnBindShader(*pShaderBlock);

          pContext->UnBindTexture(0, *pTextureBlock);
        }

        x += 0.4f;
      }
    }

    pContext->EndRenderMode2D();

    if (pGuiRenderer != nullptr) {
      pGuiRenderer->SetWireFrame(bIsWireframe);
      pGuiRenderer->Render();
    }
  }
}


//pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_Paused").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_Paused_Instructions").c_str());

//pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_GameOver").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_GameOver_Instructions1").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.10f, 1.0f, breathe::LANG("L_GameOver_Instructions2").c_str());
