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

#include <spitfire/storage/filesystem.h>
#include <spitfire/storage/xml.h>

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

// ** cState

cState::cState(cApplication& _application) :
  application(_application),
  system(application.system),
  pWindow(application.pWindow),
  pContext(application.pContext),
  pFont(application.pFont),
  pAudioManager(application.pAudioManager)
{
}


// ** cBoardRepresentation

cBoardRepresentation::cBoardRepresentation(tetris::cBoard& _board, const spitfire::string_t& _sName) :
  board(_board),
  sName(_sName),

  pStaticVertexBufferObjectBoardQuads(nullptr),
  pStaticVertexBufferObjectPieceQuads(nullptr),
  pStaticVertexBufferObjectNextPieceQuads(nullptr),

  bIsInputPieceMoveLeft(false),
  bIsInputPieceMoveRight(false),
  bIsInputPieceRotateCounterClockWise(false),
  bIsInputPieceRotateClockWise(false),
  bIsInputPieceDropOneRow(false),
  bIsInputPieceDropToGround(false)
{
}


// ** cHighScoresTableEntry

class cHighScoresTableEntry
{
public:
  bool IsValid() const { return !sName.empty(); }

  static bool ScoreCompare(const cHighScoresTableEntry& lhs, const cHighScoresTableEntry& rhs);

  spitfire::string_t sName;
  int score;
};

bool cHighScoresTableEntry::ScoreCompare(const cHighScoresTableEntry& lhs, const cHighScoresTableEntry& rhs)
{
  return (lhs.score > rhs.score);
}

// ** cHighScoresTable

class cHighScoresTable
{
public:
  bool Load();
  bool Save();

  size_t GetEntryCount() const;
  const cHighScoresTableEntry& GetEntry(size_t index) const;
  bool IsScoreGoodEnough(int score) const;
  bool SubmitEntry(const spitfire::string_t& sName, int score);

private:
  void Clear();

  static const size_t nMaxEntries = 10;

  std::vector<cHighScoresTableEntry> entries;
};

void cHighScoresTable::Clear()
{
  entries.clear();
}

bool cHighScoresTable::Load()
{
  Clear();

  spitfire::xml::reader reader;

  spitfire::xml::document doc;
  const spitfire::string_t sFilename = spitfire::filesystem::GetThisApplicationSettingsDirectory() + TEXT("config.xml");
  if (!reader.ReadFromFile(doc, sFilename)) {
    std::cout<<"cHighScoresTable::Load \""<<spitfire::string::ToUTF8(sFilename)<<"\" not found"<<std::endl;
    return false;
  }


  spitfire::xml::cNode::iterator iterConfig(doc);
  if (!iterConfig.IsValid()) return false;

  iterConfig.FindChild("config");
  if (!iterConfig.IsValid()) return false;

  // Load high scores
  {
    spitfire::xml::cNode::iterator iter(iterConfig);

    iter.FindChild("highscores");
    if (iter.IsValid()) {
      iter.FindChild("entry");
      if (iter.IsValid()) {
        while (iter.IsValid()) {
          spitfire::string_t sName;
          if (iter.GetAttribute("name", sName)) {
            std::cout<<"Adding High Score "<<spitfire::string::ToUTF8(sName)<<std::endl;
            cHighScoresTableEntry entry;

            entry.sName = spitfire::string::ToString_t(sName);
            iter.GetAttribute("score", entry.score);

            entries.push_back(entry);
          }

          iter.Next("entry");
        };
      }
    }
  }

  return true;
}

bool cHighScoresTable::Save()
{
  spitfire::xml::document doc;

  spitfire::xml::element* configElement = doc.CreateElement("config");
  doc.AppendChild(configElement);

  if (!entries.empty()) {
    spitfire::xml::element* highscoresElement = doc.CreateElement("highscores");
    configElement->AppendChild(highscoresElement);

    // Entries
    const size_t n = entries.size();
    for (size_t i = 0; i < n; i++) {
      spitfire::xml::element* entryElement = doc.CreateElement("entry");
      highscoresElement->AppendChild(entryElement);

      entryElement->AddAttribute("name", entries[i].sName);
      const uint64_t score = entries[i].score;
      entryElement->AddAttribute("score", score);
    }
  }

  // Now actually write the xml to filename
  spitfire::xml::writer writer;

  const spitfire::string_t sFolder = spitfire::filesystem::GetThisApplicationSettingsDirectory();
  spitfire::filesystem::CreateDirectory(sFolder);
  const spitfire::string_t sFilename = sFolder + TEXT("config.xml");
  std::cout<<"cHighScoresTable::Save "<<spitfire::string::ToUTF8(sFilename)<<std::endl;
  return writer.WriteToFile(doc, sFilename);
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
    // Overwrite the last entry with our new entry
    cHighScoresTableEntry entry;
    entry.sName = sName;
    entry.score = score;
    entries[nMaxEntries - 1] = entry;

    // Sort the entries
    std::sort(entries.begin(), entries.end(), cHighScoresTableEntry::ScoreCompare);
  }

  return false;
}


// ** cStateMenu

cStateMenu::cStateMenu(cApplication& application) :
  cState(application),
  pStaticVertexBufferObjectText(nullptr),
  highlighted(OPTION::NEW_GAME),
  bIsKeyUp(false),
  bIsKeyDown(false),
  bIsKeyReturn(false)
{
  UpdateText();
}

cStateMenu::~cStateMenu()
{
  if (pStaticVertexBufferObjectText != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectText);
    pStaticVertexBufferObjectText = nullptr;
  }
}

void cStateMenu::UpdateText()
{
  assert(pFont != nullptr);

  if (pStaticVertexBufferObjectText != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectText);
    pStaticVertexBufferObjectText = nullptr;
  }

  pStaticVertexBufferObjectText = pContext->CreateStaticVertexBufferObject();

  std::vector<float> vertices;
  std::vector<float> colours;
  std::vector<float> textureCoordinates;

  opengl::cGeometryBuilder_v2_c4_t2 builder(vertices, colours, textureCoordinates);

  const spitfire::math::cColour white(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);

  const spitfire::string_t options[] = {
    TEXT("New Game"),
    TEXT("High Scores"),
    TEXT("Preferences"),
    TEXT("Quit")
  };

  const float x = 0.05f;
  float y = 0.2f;

  const size_t n = lengthof(options);
  for (size_t i = 0; i < n; i++) {
    const spitfire::math::cColour& colour = (int(i) == highlighted) ? red : white;

    // Create the text for this option
    pFont->PushBack(builder, options[i], colour, spitfire::math::cVec2(x, y));
    y += 0.05f;
  }

  pStaticVertexBufferObjectText->SetVertices(vertices);
  pStaticVertexBufferObjectText->SetColours(colours);
  pStaticVertexBufferObjectText->SetTextureCoordinates(textureCoordinates);

  pStaticVertexBufferObjectText->Compile2D(system);

  std::cout<<"cStateMenu::UpdateText vertices="<<vertices.size()<<", colours="<<colours.size()<<", textureCoordinates="<<textureCoordinates.size()<<std::endl;
}

void cStateMenu::_OnKeyboardEvent(const opengl::cKeyboardEvent& event)
{
  if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case opengl::KEY::UP: {
        std::cout<<"cStateMenu::_OnKeyboardEvent Up"<<std::endl;
        bIsKeyUp = true;
        break;
      }
      case opengl::KEY::DOWN: {
        std::cout<<"cStateMenu::_OnKeyboardEvent Down"<<std::endl;
        bIsKeyDown = true;
        break;
      }
      case opengl::KEY::RETURN: {
        std::cout<<"cStateMenu::_OnKeyboardEvent Return"<<std::endl;
        bIsKeyReturn = true;
        break;
      }
    }
  }
}

void cStateMenu::_UpdateInput(const cTimeStep& timeStep)
{
  if (bIsKeyUp) {
    bIsKeyUp = false;

    highlighted--;
    if (highlighted < OPTION::NEW_GAME) highlighted = OPTION::QUIT;

    UpdateText();
  } else if (bIsKeyDown) {
    bIsKeyDown = false;

    highlighted++;
    if (highlighted > OPTION::QUIT) highlighted = OPTION::NEW_GAME;

    UpdateText();
  } else if (bIsKeyReturn) {
    bIsKeyReturn = false;

    switch (highlighted) {
      case OPTION::NEW_GAME: {
        // Push our game state
        application.PushStateSoon(new cStateGame(application));
        break;
      }
      case OPTION::HIGH_SCORES: {
        // Push our high scores state
        application.PushStateSoon(new cStateHighScores(application));
        break;
      }
      case OPTION::PREFERENCES: {
        // TODO: Add preferences for example tile set clasic or new
        break;
      }
      case OPTION::QUIT: {
        // Pop our menu state
        application.PopStateSoon();
        break;
      }
    }
  }
}

void cStateMenu::_Render(const cTimeStep& timeStep)
{
  // Render the scene
  const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
  pContext->SetClearColour(clearColour);

  pContext->BeginRendering();

  {
    pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

    // Draw the text overlay
    {
      // Rendering the font in the middle of the screen
      spitfire::math::cMat4 matModelView;
      matModelView.SetTranslation(0.1f, 0.1f, 0.0f);

      //pContext->SetModelViewMatrix(matModelView);

      pContext->BindFont(*pFont);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectText);

      {
        //pContext->SetModelViewMatrix(matModelView);// * matTranslation * matRotation);

        pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectText);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectText);

      pContext->UnBindFont(*pFont);
    }

    pContext->EndRenderMode2D();
  }

  pContext->EndRendering();
}


// ** cStateHighScores

cStateHighScores::cStateHighScores(cApplication& application) :
  cState(application),
  pStaticVertexBufferObjectText(nullptr),
  bIsDone(false)
{
  UpdateText();
}

cStateHighScores::~cStateHighScores()
{
  if (pStaticVertexBufferObjectText != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectText);
    pStaticVertexBufferObjectText = nullptr;
  }
}

void cStateHighScores::UpdateText()
{
  assert(pFont != nullptr);

  if (pStaticVertexBufferObjectText != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectText);
    pStaticVertexBufferObjectText = nullptr;
  }

  pStaticVertexBufferObjectText = pContext->CreateStaticVertexBufferObject();

  std::vector<float> vertices;
  std::vector<float> colours;
  std::vector<float> textureCoordinates;

  opengl::cGeometryBuilder_v2_c4_t2 builder(vertices, colours, textureCoordinates);

  const spitfire::math::cColour white(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);

  const float x = 0.05f;
  float y = 0.2f;

  {
    // Create the text for this option
    pFont->PushBack(builder, TEXT("High Scores"), red, spitfire::math::cVec2(x, y));
    y += 0.05f;
    y += 0.05f;
    pFont->PushBack(builder, TEXT("Name"), red, spitfire::math::cVec2(x, y));
    pFont->PushBack(builder, TEXT("Score"), red, spitfire::math::cVec2(x + 0.2f, y));
    y += 0.05f;
  }

  cHighScoresTable table;
  table.Load();

  const size_t n = table.GetEntryCount();
  for (size_t i = 0; i < n; i++) {
    const cHighScoresTableEntry& entry = table.GetEntry(i);
    std::wostringstream o;
    o<<entry.score;
    pFont->PushBack(builder, entry.sName, white, spitfire::math::cVec2(x, y));
    pFont->PushBack(builder, o.str(), white, spitfire::math::cVec2(x + 0.2f, y));
    y += 0.03f;
  }

  pStaticVertexBufferObjectText->SetVertices(vertices);
  pStaticVertexBufferObjectText->SetColours(colours);
  pStaticVertexBufferObjectText->SetTextureCoordinates(textureCoordinates);

  pStaticVertexBufferObjectText->Compile2D(system);

  std::cout<<"cStateHighScores::UpdateText vertices="<<vertices.size()<<", colours="<<colours.size()<<", textureCoordinates="<<textureCoordinates.size()<<std::endl;
}

void cStateHighScores::_OnKeyboardEvent(const opengl::cKeyboardEvent& event)
{
  if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case opengl::KEY::ESCAPE:
      case opengl::KEY::RETURN: {
        bIsDone = true;
        break;
      }
    }
  }
}

void cStateHighScores::_UpdateInput(const cTimeStep& timeStep)
{
  if (bIsDone) {
    bIsDone = false;

    // Pop our high scores state
    application.PopStateSoon();
  }
}

void cStateHighScores::_Render(const cTimeStep& timeStep)
{
  // Render the scene
  const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
  pContext->SetClearColour(clearColour);

  pContext->BeginRendering();

  {
    pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

    // Draw the text overlay
    {
      // Rendering the font in the middle of the screen
      spitfire::math::cMat4 matModelView;
      matModelView.SetTranslation(0.1f, 0.1f, 0.0f);

      //pContext->SetModelViewMatrix(matModelView);

      pContext->BindFont(*pFont);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectText);

      {
        //pContext->SetModelViewMatrix(matModelView);// * matTranslation * matRotation);

        pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectText);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectText);

      pContext->UnBindFont(*pFont);
    }

    pContext->EndRenderMode2D();
  }

  pContext->EndRendering();
}


// ** cStateGame

cStateGame::cStateGame(cApplication& application) :
  cState(application),

  bIsWireframe(false),

  pStaticVertexBufferObjectText(nullptr),

  pTextureBlock(nullptr),

  pShaderBlock(nullptr),

  game(*this)
{
  pTextureBlock = pContext->CreateTexture(TEXT("data/textures/block.png"));

  pShaderBlock = pContext->CreateShader(TEXT("data/shaders/passthroughwithcolour.vert"), TEXT("data/shaders/passthroughwithcolour.frag"));
  pShaderBlock->bTexUnit0 = true;

  const spitfire::sampletime_t currentTime = SDL_GetTicks();

  spitfire::math::SetRandomSeed(currentTime);

  game.boards.push_back(new tetris::cBoard(game));
  game.boards.push_back(new tetris::cBoard(game));

  game.StartGame(currentTime);

  //const tetris::cBoard& board = *(game.boards[0]);
  //width = board.GetWidth();
  //height = board.GetHeight();

  //scale.Set(0.2f, 0.2f, 10.0f);

  for (size_t i = 0; i < game.boards.size(); i++) {
    tetris::cBoard& board = *(game.boards[i]);
    std::wostringstream o;
    o<<TEXT("Player");
    o<<(i + 1);
    cBoardRepresentation* pBoardRepresentation = new cBoardRepresentation(board, o.str());

    pBoardRepresentation->pStaticVertexBufferObjectBoardQuads = pContext->CreateStaticVertexBufferObject();
    UpdateBoardVBO(pBoardRepresentation->pStaticVertexBufferObjectBoardQuads, board);

    pBoardRepresentation->pStaticVertexBufferObjectPieceQuads = pContext->CreateStaticVertexBufferObject();
    UpdatePieceVBO(pBoardRepresentation->pStaticVertexBufferObjectPieceQuads, board, board.GetCurrentPiece());

    pBoardRepresentation->pStaticVertexBufferObjectNextPieceQuads = pContext->CreateStaticVertexBufferObject();
    UpdatePieceVBO(pBoardRepresentation->pStaticVertexBufferObjectNextPieceQuads, board, board.GetNextPiece());

    boardRepresentations.push_back(pBoardRepresentation);
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
    if (pBoardRepresentation->pStaticVertexBufferObjectNextPieceQuads != nullptr) {
      pContext->DestroyStaticVertexBufferObject(pBoardRepresentation->pStaticVertexBufferObjectNextPieceQuads);
      pBoardRepresentation->pStaticVertexBufferObjectNextPieceQuads = nullptr;
    }
    if (pBoardRepresentation->pStaticVertexBufferObjectPieceQuads != nullptr) {
      pContext->DestroyStaticVertexBufferObject(pBoardRepresentation->pStaticVertexBufferObjectPieceQuads);
      pBoardRepresentation->pStaticVertexBufferObjectPieceQuads = nullptr;
    }
    if (pBoardRepresentation->pStaticVertexBufferObjectBoardQuads != nullptr) {
      pContext->DestroyStaticVertexBufferObject(pBoardRepresentation->pStaticVertexBufferObjectBoardQuads);
      pBoardRepresentation->pStaticVertexBufferObjectBoardQuads = nullptr;
    }
    delete pBoardRepresentation;
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


  if (pStaticVertexBufferObjectText != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectText);
    pStaticVertexBufferObjectText = nullptr;
  }

  std::cout<<"cStateGame::~cStateGame returning"<<std::endl;
}

void cStateGame::UpdateText()
{
  assert(pFont != nullptr);

  if (pStaticVertexBufferObjectText != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectText);
    pStaticVertexBufferObjectText = nullptr;
  }

  pStaticVertexBufferObjectText = pContext->CreateStaticVertexBufferObject();

  std::vector<float> vertices;
  std::vector<float> colours;
  std::vector<float> textureCoordinates;

  opengl::cGeometryBuilder_v2_c4_t2 builder(vertices, colours, textureCoordinates);

  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);
  const spitfire::math::cColour green(0.0f, 1.0f, 0.0f);
  const spitfire::math::cColour blue(0.0f, 0.0f, 1.0f);
  const spitfire::math::cColour yellow(1.0f, 1.0f, 0.0f);
  const spitfire::math::cColour boardColours[] = {
    red,
    green,
    blue,
    yellow
  };

  float y = 0.2f;

  for (size_t i = 0; i < game.boards.size(); i++) {
    tetris::cBoard& board = *(game.boards[i]);

    const spitfire::math::cColour& colour = boardColours[i];

    // Create the text for this board
    std::wostringstream o;

    const uint32_t uiLevel = board.GetLevel();
    o<<TEXT("Level ");
    o<<uiLevel;
    pFont->PushBack(builder, o.str(), colour, spitfire::math::cVec2(0.0f, y));
    y += 0.05f;
    o.str(TEXT(""));

    const uint32_t uiScore = board.GetScore();
    o<<TEXT("Score ");
    o<<uiScore;
    pFont->PushBack(builder, o.str(), colour, spitfire::math::cVec2(0.0f, y));
    y += 0.05f;
    o.str(TEXT(""));
  }

  pStaticVertexBufferObjectText->SetVertices(vertices);
  pStaticVertexBufferObjectText->SetColours(colours);
  pStaticVertexBufferObjectText->SetTextureCoordinates(textureCoordinates);

  pStaticVertexBufferObjectText->Compile2D(system);

  std::cout<<"cStateGame::UpdateText vertices="<<vertices.size()<<", colours="<<colours.size()<<", textureCoordinates="<<textureCoordinates.size()<<std::endl;
}

void cStateGame::UpdateBoardVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board)
{
  assert(pStaticVertexBufferObject != nullptr);

  //if (boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads != nullptr) {
  //  pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads);
  //  boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = nullptr;
  //}
  //pBoardRepresentation->pStaticVertexBufferObjectBoardQuads = pContext->CreateStaticVertexBufferObject();

  std::vector<float> vertices;
  //std::vector<float> normals;
  std::vector<float> textureCoordinates;
  std::vector<float> colours;
  //std::vector<uint16_t> indices;

  opengl::cGeometryBuilder_v2_c4_t2 builder(vertices, colours, textureCoordinates);

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

        // Front facing quad
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y + 1)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV2));
        builder.PushBack(scale * spitfire::math::cVec2(float(x + 1), float(y + 1)), colour, spitfire::math::cVec2(fBlockAndLightmapU2, fBlockAndLightmapV2));
        builder.PushBack(scale * spitfire::math::cVec2(float(x + 1), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU2, fBlockAndLightmapV));
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV));
      }
    }
  }

  if (!vertices.empty()) {
    pStaticVertexBufferObject->SetVertices(vertices);
    //pStaticVertexBufferObject->SetNormals(normals);
    pStaticVertexBufferObject->SetTextureCoordinates(textureCoordinates);
    pStaticVertexBufferObject->SetColours(colours);
    //pStaticVertexBufferObject->SetIndices(indices);

    pStaticVertexBufferObject->Compile2D(system);
  }
}

void cStateGame::UpdatePieceVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board, const tetris::cPiece& piece)
{
  assert(pStaticVertexBufferObject != nullptr);

  //if (boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads != nullptr) {
  //  pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads);
  //  boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = nullptr;
  //}
  //pBoardRepresentation->pStaticVertexBufferObjectBoardQuads = pContext->CreateStaticVertexBufferObject();

  std::vector<float> vertices;
  //std::vector<float> normals;
  std::vector<float> textureCoordinates;
  std::vector<float> colours;
  //std::vector<uint16_t> indices;

  opengl::cGeometryBuilder_v2_c4_t2 builder(vertices, colours, textureCoordinates);

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
        builder.PushBack(scale * spitfire::math::cVec2(float(x), float(y)), colour, spitfire::math::cVec2(fBlockAndLightmapU, fBlockAndLightmapV));
      }
    }
  }

  if (!vertices.empty()) {
    pStaticVertexBufferObject->SetVertices(vertices);
    //pStaticVertexBufferObject->SetNormals(normals);
    pStaticVertexBufferObject->SetTextureCoordinates(textureCoordinates);
    pStaticVertexBufferObject->SetColours(colours);
    //pStaticVertexBufferObject->SetIndices(indices);

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
      if (boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads);
        boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = pContext->CreateStaticVertexBufferObject();
      UpdatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads, board, board.GetCurrentPiece());
    }
  }
}

void cStateGame::_OnPieceChanged(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnPieceChanged"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    if (&boardRepresentations[i]->board == &board) {
      if (boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads);
        boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads = pContext->CreateStaticVertexBufferObject();
      UpdatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads, board, board.GetNextPiece());

      if (boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads);
        boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = pContext->CreateStaticVertexBufferObject();
      UpdatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads, board, board.GetCurrentPiece());
    }
  }
}

void cStateGame::_OnPieceHitsGround(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnPieceHitsGround"<<std::endl;
  application.PlaySound(pAudioBufferPieceHitsGround);
}

void cStateGame::_OnBoardChanged(const tetris::cBoard& board)
{
  std::cout<<"cStateGame::_OnBoardChanged"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    if (&boardRepresentations[i]->board == &board) {
      if (boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads);
        boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads = pContext->CreateStaticVertexBufferObject();
      UpdateBoardVBO(boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads, board);
    }
  }
}

void cStateGame::_OnGameScoreTetris(const tetris::cBoard& board, uint32_t uiScore)
{
  std::cout<<"cStateGame::_OnGameScoreTetris"<<std::endl;
  application.PlaySound(pAudioBufferScoreTetris);
  UpdateText();
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

  cHighScoresTable table;
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

void cStateGame::_OnKeyboardEvent(const opengl::cKeyboardEvent& event)
{
  std::cout<<"cStateGame::_OnKeyboardEvent"<<std::endl;

  if (event.IsKeyDown()) {
    std::cout<<"cStateGame::_OnKeyboardEvent Key down"<<std::endl;
    switch (event.GetKeyCode()) {
      case opengl::KEY::ESCAPE: {
        std::cout<<"cStateGame::_OnKeyboardEvent Escape key pressed, quiting"<<std::endl;
        application.PopStateSoon();
        break;
      }
    }
  } else if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case opengl::KEY::NUMBER_1: {
        std::cout<<"cStateGame::_OnKeyboardEvent 1 up"<<std::endl;
        bIsWireframe = !bIsWireframe;
        break;
      }
    }
  }
}

void cStateGame::_Update(const cTimeStep& timeStep)
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

  game.Update(timeStep.GetCurrentTimeMS());
}

void cStateGame::_UpdateInput(const cTimeStep& timeStep)
{
  assert(pWindow != nullptr);

  if (boardRepresentations.size() == 1) {
    // Single player
    cBoardRepresentation* pBoardRepresentation = boardRepresentations[0];

    // Player 1
    if (pWindow->IsKeyUp(opengl::KEY::BACKSLASH)) {
      //std::cout<<"cStateGame::UpdateInput BACKSLASH up"<<std::endl;
      pBoardRepresentation->bIsInputPieceRotateCounterClockWise = true;
    }
    if (pWindow->IsKeyUp(opengl::KEY::UP)) {
      //std::cout<<"cStateGame::UpdateInput UP up"<<std::endl;
      pBoardRepresentation->bIsInputPieceRotateClockWise = true;
    }
    if (pWindow->IsKeyHeld(opengl::KEY::DOWN)) {
      //std::cout<<"cStateGame::UpdateInput DOWN held"<<std::endl;
      pBoardRepresentation->bIsInputPieceDropOneRow = true;
    }
    if (pWindow->IsKeyUp(opengl::KEY::SPACE)) {
      //std::cout<<"cStateGame::UpdateInput SPACE up"<<std::endl;
      pBoardRepresentation->bIsInputPieceDropToGround = true;
    }
    if (pWindow->IsKeyHeld(opengl::KEY::LEFT)) {
      //std::cout<<"cStateGame::UpdateInput LEFT Held"<<std::endl;
      pBoardRepresentation->bIsInputPieceMoveLeft = true;
    }
    if (pWindow->IsKeyHeld(opengl::KEY::RIGHT)) {
      //std::cout<<"cStateGame::UpdateInput RIGHT Held"<<std::endl;
      pBoardRepresentation->bIsInputPieceMoveRight = true;
    }
  } else {
    {
      // Player 1
      cBoardRepresentation* pBoardRepresentation = boardRepresentations[0];

      if (pWindow->IsKeyUp(opengl::KEY::Q)) {
        //std::cout<<"cStateGame::UpdateInput Q up"<<std::endl;
        pBoardRepresentation->bIsInputPieceRotateCounterClockWise = true;
      }
      if (pWindow->IsKeyUp(opengl::KEY::W)) {
        //std::cout<<"cStateGame::UpdateInput W up"<<std::endl;
        pBoardRepresentation->bIsInputPieceRotateClockWise = true;
      }
      if (pWindow->IsKeyHeld(opengl::KEY::S)) {
        //std::cout<<"cStateGame::UpdateInput S Held"<<std::endl;
        pBoardRepresentation->bIsInputPieceDropOneRow = true;
      }
      if (pWindow->IsKeyUp(opengl::KEY::F)) {
        //std::cout<<"cStateGame::UpdateInput F up"<<std::endl;
        pBoardRepresentation->bIsInputPieceDropToGround = true;
      }
      if (pWindow->IsKeyHeld(opengl::KEY::A)) {
        //std::cout<<"cStateGame::UpdateInput A Held"<<std::endl;
        pBoardRepresentation->bIsInputPieceMoveLeft = true;
      }
      if (pWindow->IsKeyHeld(opengl::KEY::D)) {
        //std::cout<<"cStateGame::UpdateInput D Held"<<std::endl;
        pBoardRepresentation->bIsInputPieceMoveRight = true;
      }
    }

    {
      // Player 2
      cBoardRepresentation* pBoardRepresentation = boardRepresentations[1];

      if (pWindow->IsKeyUp(opengl::KEY::BACKSLASH)) {
        //std::cout<<"cStateGame::UpdateInput BACKSLASH up"<<std::endl;
        pBoardRepresentation->bIsInputPieceRotateCounterClockWise = true;
      }
      if (pWindow->IsKeyUp(opengl::KEY::UP)) {
        //std::cout<<"cStateGame::UpdateInput UP up"<<std::endl;
        pBoardRepresentation->bIsInputPieceRotateClockWise = true;
      }
      if (pWindow->IsKeyHeld(opengl::KEY::DOWN)) {
        //std::cout<<"cStateGame::UpdateInput DOWN Held"<<std::endl;
        pBoardRepresentation->bIsInputPieceDropOneRow = true;
      }
      if (pWindow->IsKeyUp(opengl::KEY::SPACE)) {
        //std::cout<<"cStateGame::UpdateInput SPACE up"<<std::endl;
        pBoardRepresentation->bIsInputPieceDropToGround = true;
      }
      if (pWindow->IsKeyHeld(opengl::KEY::LEFT)) {
        //std::cout<<"cStateGame::UpdateInput LEFT Held"<<std::endl;
        pBoardRepresentation->bIsInputPieceMoveLeft = true;
      }
      if (pWindow->IsKeyHeld(opengl::KEY::RIGHT)) {
        //std::cout<<"cStateGame::UpdateInput RIGHT Held"<<std::endl;
        pBoardRepresentation->bIsInputPieceMoveRight = true;
      }
    }
  }
}

void cStateGame::_Render(const cTimeStep& timeStep)
{
  // Render the scene
  const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
  pContext->SetClearColour(clearColour);

  pContext->BeginRendering();

  if (bIsWireframe) pContext->EnableWireframe();

  {
    pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

    // Draw the boards
    {
      assert(!game.boards.empty());

      float x = 0.25f;
      const float y = 0.1f;

      const size_t n = boardRepresentations.size();
      for (size_t i = 0; i < n; i++) {
        cBoardRepresentation* pBoardRepresentation = boardRepresentations[i];
        const tetris::cBoard& board = pBoardRepresentation->board;

        opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBoardQuads = pBoardRepresentation->pStaticVertexBufferObjectBoardQuads;
        if (pStaticVertexBufferObjectBoardQuads->IsCompiled()) {
          spitfire::math::cMat4 matModelView2D;
          matModelView2D.SetTranslation(x, y, 0.0f);

          pContext->BindTexture(0, *pTextureBlock);

          pContext->BindShader(*pShaderBlock);

          pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectBoardQuads);
          pContext->SetModelViewMatrix(matModelView2D);
          pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectBoardQuads);
          pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectBoardQuads);

          pContext->UnBindShader(*pShaderBlock);

          pContext->UnBindTexture(0, *pTextureBlock);
        }

        opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPieceQuads = pBoardRepresentation->pStaticVertexBufferObjectPieceQuads;
        if (pStaticVertexBufferObjectPieceQuads->IsCompiled()) {
          spitfire::math::cMat4 matModelView2D;
          matModelView2D.SetTranslation(x + (0.015f * float(pBoardRepresentation->board.GetCurrentPieceX())), y + (0.015f * (float(pBoardRepresentation->board.GetHeight()) - float(board.GetCurrentPieceY()))), 0.0f);

          pContext->BindTexture(0, *pTextureBlock);

          pContext->BindShader(*pShaderBlock);

          pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectPieceQuads);
          pContext->SetModelViewMatrix(matModelView2D);
          pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectPieceQuads);
          pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectPieceQuads);

          pContext->UnBindShader(*pShaderBlock);

          pContext->UnBindTexture(0, *pTextureBlock);
        }

        opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectNextPieceQuads = pBoardRepresentation->pStaticVertexBufferObjectNextPieceQuads;
        if (pStaticVertexBufferObjectNextPieceQuads->IsCompiled()) {
          spitfire::math::cMat4 matModelView2D;
          matModelView2D.SetTranslation(x + (0.015f * float(pBoardRepresentation->board.GetWidth())) + (0.015f * 3.0f), y + (0.015f * (0.5f * float(pBoardRepresentation->board.GetHeight()))), 0.0f);

          pContext->BindTexture(0, *pTextureBlock);

          pContext->BindShader(*pShaderBlock);

          pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectNextPieceQuads);
          pContext->SetModelViewMatrix(matModelView2D);
          pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectNextPieceQuads);
          pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectNextPieceQuads);

          pContext->UnBindShader(*pShaderBlock);

          pContext->UnBindTexture(0, *pTextureBlock);
        }

        x += 0.4f;
      }
    }


    // Draw the text overlay
    {
      // Rendering the font in the middle of the screen
      spitfire::math::cMat4 matModelView;
      matModelView.SetTranslation(0.1f, 0.1f, 0.0f);

      //pContext->SetModelViewMatrix(matModelView);

      pContext->BindFont(*pFont);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectText);

      {
        //pContext->SetModelViewMatrix(matModelView);// * matTranslation * matRotation);

        pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectText);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectText);

      pContext->UnBindFont(*pFont);
    }

    pContext->EndRenderMode2D();
  }

  pContext->EndRendering();
}


//pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_Paused").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_Paused_Instructions").c_str());

//pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_GameOver").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_GameOver_Instructions1").c_str());
//pFont->PrintCenteredHorizontally(x, y + 0.10f, 1.0f, breathe::LANG("L_GameOver_Instructions2").c_str());
