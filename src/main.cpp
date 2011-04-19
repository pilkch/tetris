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
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Tetris headers
#include "tetris.h"


class cBoardRepresentation
{
public:
  explicit cBoardRepresentation(tetris::cBoard& board);

  tetris::cBoard& board;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBoardQuads;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPieceQuads;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectNextPieceQuads;
};

cBoardRepresentation::cBoardRepresentation(tetris::cBoard& _board) :
  board(_board),
  pStaticVertexBufferObjectBoardQuads(nullptr),
  pStaticVertexBufferObjectPieceQuads(nullptr),
  pStaticVertexBufferObjectNextPieceQuads(nullptr)
{
}

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener, public tetris::cView
{
public:
   cApplication();
   ~cApplication();

   bool Create();
   void Destroy();

   void Run();

private:
   void CreateBoardVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board);
   void CreatePieceVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board, const tetris::cPiece& piece);

   void _OnWindowEvent(const opengl::cWindowEvent& event);
   void _OnMouseEvent(const opengl::cMouseEvent& event);
   void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

   void _OnPieceMoved(const tetris::cBoard& board);
   void _OnPieceRotated(const tetris::cBoard& board);
   void _OnPieceChanged(const tetris::cBoard& board);
   void _OnPieceHitsGround(const tetris::cBoard& board);
   void _OnBoardChanged(const tetris::cBoard& board);
   void _OnGameScoreTetris(const tetris::cBoard& board, uint32_t uiScore);
   void _OnGameScoreOtherThanTetris(const tetris::cBoard& board, uint32_t uiScore);
   void _OnGameNewLevel(const tetris::cBoard& board, uint32_t uiLevel);
   void _OnGameOver(const tetris::cBoard& board);

   bool bIsWireframe;
   bool bIsDone;

   size_t width;
   size_t height;
   spitfire::math::cVec3 scale;

   // For controlling the rotation and zoom of the trackball
   spitfire::math::cQuaternion rotationX;
   spitfire::math::cQuaternion rotationZ;
   float fZoom;

   opengl::cSystem system;

   opengl::cWindow* pWindow;

   opengl::cContext* pContext;

   opengl::cTexture* pTextureBlock;

   opengl::cShader* pShaderBlock;

   std::vector<cBoardRepresentation*> boardRepresentations;

  tetris::cGame game;
};

cApplication::cApplication() :
  bIsWireframe(false),
  bIsDone(false),

  width(0),
  height(0),

  fZoom(0.0f),

  pWindow(nullptr),
  pContext(nullptr),

  pTextureBlock(nullptr),

  pShaderBlock(nullptr),

  game(*this)
{
}

cApplication::~cApplication()
{
   Destroy();
}

void cApplication::CreateBoardVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board)
{
  assert(pStaticVertexBufferObject != nullptr);

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

      /*if (board.GetBlock(x, y) != 0)*/ {
        const spitfire::math::cColour colour(board.GetColour(board.GetBlock(x, _y)));

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

void cApplication::CreatePieceVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board, const tetris::cPiece& piece)
{
  assert(pStaticVertexBufferObject != nullptr);

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

  pTextureBlock = pContext->CreateTexture(TEXT("data/textures/block.png"));

  pShaderBlock = pContext->CreateShader(TEXT("data/shaders/passthroughwithcolour.vert"), TEXT("data/shaders/passthroughwithcolour.frag"));
  pShaderBlock->bTexUnit0 = true;

  const spitfire::sampletime_t currentTime = SDL_GetTicks();

  spitfire::math::SetRandomSeed(currentTime);

  game.boards.push_back(new tetris::cBoard(game));
  game.boards.push_back(new tetris::cBoard(game));

  const tetris::cBoard& board = *(game.boards.front());

  game.StartGame(currentTime);

  width = board.GetWidth();
  height = board.GetHeight();

  scale.Set(0.2f, 0.2f, 10.0f);

  for (size_t i = 0; i < game.boards.size(); i++) {
    tetris::cBoard& board = *(game.boards[i]);
    cBoardRepresentation* pBoardRepresentation = new cBoardRepresentation(board);
    pBoardRepresentation->pStaticVertexBufferObjectBoardQuads = pContext->CreateStaticVertexBufferObject();
    CreateBoardVBO(pBoardRepresentation->pStaticVertexBufferObjectBoardQuads, board);

    pBoardRepresentation->pStaticVertexBufferObjectPieceQuads = pContext->CreateStaticVertexBufferObject();
    CreatePieceVBO(pBoardRepresentation->pStaticVertexBufferObjectPieceQuads, board, board.GetCurrentPiece());

    pBoardRepresentation->pStaticVertexBufferObjectNextPieceQuads = pContext->CreateStaticVertexBufferObject();
    CreatePieceVBO(pBoardRepresentation->pStaticVertexBufferObjectNextPieceQuads, board, board.GetNextPiece());

    boardRepresentations.push_back(pBoardRepresentation);
  }

  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
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

  pContext = nullptr;

  if (pWindow != nullptr) {
    system.DestroyWindow(pWindow);
    pWindow = nullptr;
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
   /*// These a little too numerous to log every single one
   //std::cout<<"cApplication::_OnMouseEvent"<<std::endl;

   if (event.IsMouseMove()) {
      std::cout<<"cApplication::_OnMouseEvent Mouse move"<<std::endl;

      if (fabs(event.GetX() - (pWindow->GetWidth() * 0.5f)) > 0.5f) {
         const spitfire::math::cVec3 axisIncrementZ(0.0f, 0.0f, 1.0f);
         spitfire::math::cQuaternion rotationIncrementZ;
         rotationIncrementZ.SetFromAxisAngle(axisIncrementZ, -0.001f * (event.GetX() - (pWindow->GetWidth() * 0.5f)));
         rotationZ *= rotationIncrementZ;
      }

      if (fabs(event.GetY() - (pWindow->GetHeight() * 0.5f)) > 1.5f) {
         const spitfire::math::cVec3 axisIncrementX(1.0f, 0.0f, 0.0f);
         spitfire::math::cQuaternion rotationIncrementX;
         rotationIncrementX.SetFromAxisAngle(axisIncrementX, -0.0005f * (event.GetY() - (pWindow->GetHeight() * 0.5f)));
         rotationX *= rotationIncrementX;
      }

      spitfire::math::cVec3 newAxis = rotationZ.GetAxis();
      float fNewAngleDegrees = rotationZ.GetAngleDegrees();
      //std::cout<<"cApplication::_OnMouseEvent z newAxis={ "<<newAxis.x<<", "<<newAxis.y<<", "<<newAxis.z<<" } angle="<<fNewAngleDegrees<<std::endl;

      newAxis = rotationX.GetAxis();
      fNewAngleDegrees = rotationX.GetAngleDegrees();
      //std::cout<<"cApplication::_OnMouseEvent x newAxis={ "<<newAxis.x<<", "<<newAxis.y<<", "<<newAxis.z<<" } angle="<<fNewAngleDegrees<<std::endl;
   } else if (event.IsButtonDown()) {
      const float fZoomIncrement = 5.0f;

      switch (event.GetButton()) {
         case SDL_BUTTON_WHEELUP: {
            std::cout<<"cApplication::_OnMouseEvent Wheel up"<<std::endl;
            fZoom -= fZoomIncrement;
            break;
         }
         case SDL_BUTTON_WHEELDOWN: {
            std::cout<<"cApplication::_OnMouseEvent Wheel down"<<std::endl;
            fZoom += fZoomIncrement;
            break;
         }
      };
   }*/
}

void cApplication::_OnKeyboardEvent(const opengl::cKeyboardEvent& event)
{
  std::cout<<"cApplication::_OnKeyboardEvent"<<std::endl;

  tetris::cBoard& board = *(game.boards.front());
  const spitfire::sampletime_t currentTime = SDL_GetTicks();

  if (event.IsKeyDown()) {
    switch (event.GetKeyCode()) {
      case SDLK_ESCAPE: {
        std::cout<<"cApplication::_OnKeyboardEvent Escape key pressed, quiting"<<std::endl;
        bIsDone = true;
        break;
      }
    }
  } else if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case SDLK_w: {
        std::cout<<"cApplication::_OnKeyboardEvent w up"<<std::endl;
        bIsWireframe = !bIsWireframe;
        break;
      }
      case SDLK_UP: {
        std::cout<<"cApplication::_OnKeyboardEvent UP up"<<std::endl;
        board.PieceRotateClockWise();
        break;
      }
      case SDLK_DOWN: {
        std::cout<<"cApplication::_OnKeyboardEvent DOWN up"<<std::endl;
        board.PieceDropOneRow(currentTime);
        break;
      }
      case SDLK_SPACE: {
        std::cout<<"cApplication::_OnKeyboardEvent SPACE up"<<std::endl;
        board.PieceDropToGround(currentTime);
        break;
      }
      case SDLK_LEFT: {
        std::cout<<"cApplication::_OnKeyboardEvent LEFT up"<<std::endl;
        board.PieceMoveLeft();
        break;
      }
      case SDLK_RIGHT: {
        std::cout<<"cApplication::_OnKeyboardEvent RIGHT up"<<std::endl;
        board.PieceMoveRight();
        break;
      }

      //board.PieceRotateCounterClockWise();
    }
  }
}

void cApplication::_OnPieceMoved(const tetris::cBoard& board)
{
  std::cout<<"cApplication::_OnPieceMoved"<<std::endl;
  //... update piece position
}

void cApplication::_OnPieceRotated(const tetris::cBoard& board)
{
  std::cout<<"cApplication::_OnPieceRotated"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    if (&boardRepresentations[i]->board == &board) {
      if (boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads);
        boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = pContext->CreateStaticVertexBufferObject();
      CreatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads, board, board.GetCurrentPiece());
    }
  }
}

void cApplication::_OnPieceChanged(const tetris::cBoard& board)
{
  std::cout<<"cApplication::_OnPieceChanged"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    if (&boardRepresentations[i]->board == &board) {
      if (boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads);
        boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads = pContext->CreateStaticVertexBufferObject();
      CreatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectNextPieceQuads, board, board.GetNextPiece());

      if (boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads);
        boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads = pContext->CreateStaticVertexBufferObject();
      CreatePieceVBO(boardRepresentations[i]->pStaticVertexBufferObjectPieceQuads, board, board.GetCurrentPiece());
    }
  }
}

void cApplication::_OnPieceHitsGround(const tetris::cBoard& board)
{
  std::cout<<"cApplication::_OnPieceHitsGround"<<std::endl;
  //play(spitfire::filesystem::FindFile(TEXT("audio/piece_hits_ground.wav")));
}

void cApplication::_OnBoardChanged(const tetris::cBoard& board)
{
  std::cout<<"cApplication::_OnBoardChanged"<<std::endl;

  const size_t n = boardRepresentations.size();
  for (size_t i = 0; i < n; i++) {
    if (&boardRepresentations[i]->board == &board) {
      if (boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads != nullptr) {
        pContext->DestroyStaticVertexBufferObject(boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads);
        boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads = nullptr;
      }
      boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads = pContext->CreateStaticVertexBufferObject();
      CreateBoardVBO(boardRepresentations[i]->pStaticVertexBufferObjectBoardQuads, board);
    }
  }
}

void cApplication::_OnGameScoreTetris(const tetris::cBoard& board, uint32_t uiScore)
{
  std::cout<<"cApplication::_OnGameScoreTetris"<<std::endl;
  //play(spitfire::filesystem::FindFile(TEXT("audio/score_tetris.wav")));
  //... update score text
}

void cApplication::_OnGameScoreOtherThanTetris(const tetris::cBoard& board, uint32_t uiScore)
{
  std::cout<<"cApplication::_OnGameScoreOtherThanTetris"<<std::endl;
  //play(spitfire::filesystem::FindFile(TEXT("audio/score_other.wav")));
  //... update score text
}

void cApplication::_OnGameNewLevel(const tetris::cBoard& board, uint32_t uiLevel)
{
  std::cout<<"cApplication::_OnGameNewLevel"<<std::endl;
  //... show new level message
}

void cApplication::_OnGameOver(const tetris::cBoard& board)
{
  std::cout<<"cApplication::_OnGameOver"<<std::endl;
  //play(spitfire::filesystem::FindFile(TEXT("audio/game_over.wav")));
  //... show game over screen, stop game
}

void cApplication::Run()
{
   assert(pContext != nullptr);
   assert(pContext->IsValid());
   assert(pTextureBlock != nullptr);
   assert(pTextureBlock->IsValid());
   assert(pShaderBlock != nullptr);
   assert(pShaderBlock->IsCompiledProgram());

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


   // Center the camera at the middle of the board
   spitfire::math::cMat4 matTranslation;
   matTranslation.SetTranslation(-0.5f * scale.x * float(width), -0.5f * scale.y * float(height), 0.0f);

   // Set the defaults for the orbiting camera
   {
      const spitfire::math::cVec3 axisZ(0.0f, 0.0f, 1.0f);
      rotationZ.SetFromAxisAngleDegrees(axisZ, 0.0f);

      const spitfire::math::cVec3 axisX(1.0f, 0.0f, 0.0f);
      rotationX.SetFromAxisAngleDegrees(axisX, -20.0f);
   }
   fZoom = 60.0f;


   spitfire::math::cMat4 matRotation;

   uint32_t T0 = 0;
   uint32_t Frames = 0;

   uint32_t previousTime = SDL_GetTicks();
   uint32_t currentTime = SDL_GetTicks();

   // Setup mouse
   pWindow->ShowCursor(false);
   pWindow->WarpCursorToMiddleOfScreen();

   while (!bIsDone) {
      // Update window events
      pWindow->UpdateEvents();

      // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
      pWindow->WarpCursorToMiddleOfScreen();

      // Update state
      previousTime = currentTime;
      currentTime = SDL_GetTicks();

      game.Update(currentTime);

      matRotation.SetRotation(rotationZ * rotationX);


      // Render the scene
      const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRendering();

      if (bIsWireframe) pContext->EnableWireframe();

      const spitfire::math::cVec3 offset = matRotation.GetRotatedVec3(spitfire::math::cVec3(0.0f, -fZoom, 0.0f));
      const spitfire::math::cVec3 up = matRotation.GetRotatedVec3(spitfire::math::v3Up);

      const spitfire::math::cVec3 target(0.0f, 0.0f, 0.0f);
      const spitfire::math::cVec3 eye(target + offset);
      spitfire::math::cMat4 matModelView;
      matModelView.LookAt(eye, target, up);

      {
        pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

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

        pContext->EndRenderMode2D();
      }

      pContext->EndRendering();

      // Gather our frames per second
      Frames++;
      {
         uint32_t t = SDL_GetTicks();
         if (t - T0 >= 5000) {
            float seconds = (t - T0) / 1000.0;
            float fps = Frames / seconds;
            std::cout<<Frames<<" frames in "<<seconds<<" seconds = "<<fps<<" FPS"<<std::endl;
            T0 = t;
            Frames = 0;
         }
      }
   };

   pWindow->ShowCursor(true);
}

int main(int argc, char** argv)
{
   bool bIsSuccess = true;

   //pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_Paused").c_str());
   //pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_Paused_Instructions").c_str());

   //pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_GameOver").c_str());
   //pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_GameOver_Instructions1").c_str());
   //pFont->PrintCenteredHorizontally(x, y + 0.10f, 1.0f, breathe::LANG("L_GameOver_Instructions2").c_str());

   cApplication application;

   bIsSuccess = application.Create();
   if (bIsSuccess) application.Run();

   application.Destroy();

   return bIsSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}

