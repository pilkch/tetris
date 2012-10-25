#ifndef TETRIS_STATES_H
#define TETRIS_STATES_H

// Breathe headers
#include <breathe/gui/cManager.h>
#include <breathe/gui/cRenderer.h>

// Tetris headers
#include "application.h"
#include "tetris.h"

class cApplication;


// ** cBoardRepresentation

class cBoardRepresentation
{
public:
  cBoardRepresentation(tetris::cBoard& board, const spitfire::string_t& sName);

  tetris::cBoard& board;
  spitfire::string_t sName;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBoardTriangles;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPieceTriangles;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectNextPieceTriangles;

  bool bIsInputPieceMoveLeft;
  bool bIsInputPieceMoveRight;
  bool bIsInputPieceRotateCounterClockWise;
  bool bIsInputPieceRotateClockWise;
  bool bIsInputPieceDropOneRow;
  bool bIsInputPieceDropToGround;

  spitfire::sampletime_t lastKeyLeft;
  spitfire::sampletime_t lastKeyRight;
};


// ** cState

class cState : public breathe::gui::cWidgetEventListener
{
public:
  explicit cState(cApplication& application);
  virtual ~cState();

  void Pause() { _OnPause(); }
  void Resume() { _OnResume(); }

  void OnWindowEvent(const opengl::cWindowEvent& event) { _OnWindowEvent(event); }
  void OnMouseEvent(const opengl::cMouseEvent& event) { _OnMouseEvent(event); }
  void OnKeyboardEvent(const opengl::cKeyboardEvent& event) { _OnKeyboardEvent(event); }

  void Update(const spitfire::math::cTimeStep& timeStep) { _Update(timeStep); }
  void UpdateInput(const spitfire::math::cTimeStep& timeStep) { _UpdateInput(timeStep); }
  void Render(const spitfire::math::cTimeStep& timeStep) { _Render(timeStep); }

protected:
  breathe::gui::cStaticText* AddStaticText(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width);
  breathe::gui::cRetroButton* AddRetroButton(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width);
  breathe::gui::cRetroInput* AddRetroInput(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width);
  breathe::gui::cRetroInputUpDown* AddRetroInputUpDown(breathe::gui::id_t id, int min, int max, int value, float x, float y, float width);

  cApplication& application;
  cSettings& settings;

  opengl::cSystem& system;
  opengl::cWindow* pWindow;
  opengl::cContext* pContext;
  opengl::cFont* pFont;

  breathe::audio::cManager* pAudioManager;

  breathe::gui::cManager* pGuiManager;
  breathe::gui::cRenderer* pGuiRenderer;

  breathe::gui::cLayer* pLayer;

  bool bIsWireframe;

private:
  virtual void _OnEnter() {}
  virtual void _OnExit() {}
  virtual void _OnPause();
  virtual void _OnResume();

  virtual void _OnWindowEvent(const opengl::cWindowEvent& event) {}
  virtual void _OnMouseEvent(const opengl::cMouseEvent& event);
  virtual void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  virtual void _Update(const spitfire::math::cTimeStep& timeStep) {}
  virtual void _UpdateInput(const spitfire::math::cTimeStep& timeStep) {}
  virtual void _Render(const spitfire::math::cTimeStep& timeStep) {}


  virtual void _OnStateMouseEvent(const opengl::cMouseEvent& event) {}
  virtual void _OnStateKeyboardEvent(const opengl::cKeyboardEvent& event) {}

  virtual void _OnWidgetEvent(const breathe::gui::cWidgetEvent& event) {}
};


// ** States

class cStateMenu : public cState
{
public:
  explicit cStateMenu(cApplication& application);

private:
  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _Render(const spitfire::math::cTimeStep& timeStep);

  void _OnStateKeyboardEvent(const opengl::cKeyboardEvent& event);

  void _OnWidgetEvent(const breathe::gui::cWidgetEvent& event);

  struct OPTION {
    static const int NEW_GAME = 1;
    static const int HIGH_SCORES = 2;
    //static const int PREFERENCES = 3;
    static const int QUIT = 3;
  };

  bool bIsKeyReturn;
};

class cStateNewGame : public cState
{
public:
  explicit cStateNewGame(cApplication& application);

private:
  void _OnStateKeyboardEvent(const opengl::cKeyboardEvent& event);

  void _OnWidgetEvent(const breathe::gui::cWidgetEvent& event);

  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _Render(const spitfire::math::cTimeStep& timeStep);

  struct OPTION {
    static const int NUMBER_OF_PLAYERS = 1;
    static const int NAME_PLAYER1 = 2;
    static const int NAME_PLAYER2 = 3;
    static const int START = 4;
    static const int BACK = 5;
  };

  breathe::gui::cRetroInputUpDown* pNumberOfPlayers;
  breathe::gui::cRetroInput* pPlayerName1;
  breathe::gui::cRetroInput* pPlayerName2;

  bool bIsKeyUp;
  bool bIsKeyDown;
  bool bIsKeyReturn;
};

class cStateHighScores : public cState
{
public:
  explicit cStateHighScores(cApplication& application);
  ~cStateHighScores();

private:
  void UpdateText();

  void _OnStateKeyboardEvent(const opengl::cKeyboardEvent& event);

  void _OnWidgetEvent(const breathe::gui::cWidgetEvent& event);

  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _UpdateInput(const spitfire::math::cTimeStep& timeStep);
  void _Render(const spitfire::math::cTimeStep& timeStep);

  struct OPTION {
    static const int BACK = 1;
  };

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectText;

  bool bIsDone;
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

  void _OnStateKeyboardEvent(const opengl::cKeyboardEvent& event);

  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _UpdateInput(const spitfire::math::cTimeStep& timeStep);
  void _Render(const spitfire::math::cTimeStep& timeStep);

  void _OnPieceMoved(const tetris::cBoard& board);
  void _OnPieceRotated(const tetris::cBoard& board);
  void _OnPieceChanged(const tetris::cBoard& board);
  void _OnPieceHitsGround(const tetris::cBoard& board);
  void _OnBoardChanged(const tetris::cBoard& board);
  void _OnGameScoreTetris(const tetris::cBoard& board, uint32_t uiScore);
  void _OnGameScoreOtherThanTetris(const tetris::cBoard& board, uint32_t uiScore);
  void _OnGameNewLevel(const tetris::cBoard& board, uint32_t uiLevel);
  void _OnGameOver(const tetris::cBoard& board);

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
