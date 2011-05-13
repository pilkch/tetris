#ifndef TETRIS_APPLICATION_H
#define TETRIS_APPLICATION_H

// Tetris headers
#include "tetris.h"

// ** cBoardRepresentation

class cBoardRepresentation
{
public:
  explicit cBoardRepresentation(tetris::cBoard& board);

  tetris::cBoard& board;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBoardQuads;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPieceQuads;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectNextPieceQuads;

  bool bIsInputPieceMoveLeft;
  bool bIsInputPieceMoveRight;
  bool bIsInputPieceRotateCounterClockWise;
  bool bIsInputPieceRotateClockWise;
  bool bIsInputPieceDropOneRow;
  bool bIsInputPieceDropToGround;
};


// ** cApplication

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener, public tetris::cView
{
public:
   cApplication();
   ~cApplication();

   bool Create();
   void Destroy();

   void Run();

private:
  void UpdateText();
  void UpdateBoardVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board);
  void UpdatePieceVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board, const tetris::cPiece& piece);

  void PlaySound(breathe::audio::cBufferRef pBuffer);

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

  void UpdateInput(spitfire::sampletime_t currentTime);
  void Update(spitfire::sampletime_t currentTime);

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

  // Text
  opengl::cFont* pFont;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectText;

  opengl::cTexture* pTextureBlock;

  opengl::cShader* pShaderBlock;

  breathe::audio::cManager* pAudioManager;

  breathe::audio::cBufferRef pAudioBufferPieceHitsGround;
  breathe::audio::cBufferRef pAudioBufferScoreTetris;
  breathe::audio::cBufferRef pAudioBufferScoreOtherThanTetris;
  breathe::audio::cBufferRef pAudioBufferGameOver;

  std::vector<cBoardRepresentation*> boardRepresentations;

  tetris::cGame game;
};

#endif // TETRIS_APPLICATION_H
