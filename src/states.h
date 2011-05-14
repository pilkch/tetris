#ifndef TETRIS_STATES_H
#define TETRIS_STATES_H

// Tetris headers
#include "application.h"
#include "tetris.h"

class cApplication;

// ** cTimeStep

class cTimeStep
{
public:
  cTimeStep(spitfire::sampletime_t currentTimeMS, float fStepMS);

  spitfire::sampletime_t GetCurrentTimeMS() const { return currentTimeMS; }
  float GetStepMS() const { return fStepMS; }

private:
  spitfire::sampletime_t currentTimeMS;
  float fStepMS;
};

inline cTimeStep::cTimeStep(spitfire::sampletime_t _currentTimeMS, float _fStepMS) :
  currentTimeMS(_currentTimeMS),
  fStepMS(_fStepMS)
{
}


// ** cState

class cState
{
public:
  explicit cState(cApplication& application);
  virtual ~cState() {}

  void Pause() { _OnPause(); }
  void Resume() { _OnResume(); }

  void OnWindowEvent(const opengl::cWindowEvent& event) { _OnWindowEvent(event); }
  void OnMouseEvent(const opengl::cMouseEvent& event) { _OnMouseEvent(event); }
  void OnKeyboardEvent(const opengl::cKeyboardEvent& event) { _OnKeyboardEvent(event); }

  void Update(const cTimeStep& timeStep) { _Update(timeStep); }
  void UpdateInput(const cTimeStep& timeStep) { _UpdateInput(timeStep); }
  void Render(const cTimeStep& timeStep) { _Render(timeStep); }

protected:
  cApplication& application;

  opengl::cSystem& system;
  opengl::cWindow* pWindow;
  opengl::cContext* pContext;
  opengl::cFont* pFont;

  breathe::audio::cManager* pAudioManager;

private:
  virtual void _OnEnter() {}
  virtual void _OnExit() {}
  virtual void _OnPause() {}
  virtual void _OnResume() {}

  virtual void _OnWindowEvent(const opengl::cWindowEvent& event) {}
  virtual void _OnMouseEvent(const opengl::cMouseEvent& event) {}
  virtual void _OnKeyboardEvent(const opengl::cKeyboardEvent& event) {}

  virtual void _Update(const cTimeStep& timeStep) {}
  virtual void _UpdateInput(const cTimeStep& timeStep) {}
  virtual void _Render(const cTimeStep& timeStep) {}
};


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





class cStateMenu : public cState
{
public:
  explicit cStateMenu(cApplication& application);
};

class cStateGame : public cState, public tetris::cView
{
public:
  explicit cStateGame(cApplication& application);
  ~cStateGame();

private:
  void UpdateText();
  void UpdateBoardVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board);
  void UpdatePieceVBO(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board, const tetris::cPiece& piece);

  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  void _Update(const cTimeStep& timeStep);
  void _UpdateInput(const cTimeStep& timeStep);
  void _Render(const cTimeStep& timeStep);

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

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectText;

  opengl::cTexture* pTextureBlock;

  opengl::cShader* pShaderBlock;
  breathe::audio::cBufferRef pAudioBufferPieceHitsGround;
  breathe::audio::cBufferRef pAudioBufferScoreTetris;
  breathe::audio::cBufferRef pAudioBufferScoreOtherThanTetris;
  breathe::audio::cBufferRef pAudioBufferGameOver;

  std::vector<cBoardRepresentation*> boardRepresentations;

  tetris::cGame game;
};

#endif // TETRIS_STATES_H
