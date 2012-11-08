#ifndef TETRIS_STATES_H
#define TETRIS_STATES_H

// Breathe headers
#include <breathe/gui/cManager.h>
#include <breathe/gui/cRenderer.h>
#include <breathe/render/cContext.h>
#include <breathe/render/cFont.h>
#include <breathe/render/cSystem.h>
#include <breathe/render/cVertexBufferObject.h>
#include <breathe/render/cWindow.h>
#include <breathe/util/cApplication.h>

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

  breathe::render::cVertexBufferObject* pStaticVertexBufferObjectBoardTriangles;
  breathe::render::cVertexBufferObject* pStaticVertexBufferObjectPieceTriangles;
  breathe::render::cVertexBufferObject* pStaticVertexBufferObjectNextPieceTriangles;

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

class cState : public breathe::util::cState
{
public:
  explicit cState(cApplication& application);
  virtual ~cState();

protected:
  breathe::gui::cStaticText* AddStaticText(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width);
  breathe::gui::cRetroButton* AddRetroButton(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width);
  breathe::gui::cRetroInput* AddRetroInput(breathe::gui::id_t id, const spitfire::string_t& sText, float x, float y, float width);
  breathe::gui::cRetroInputUpDown* AddRetroInputUpDown(breathe::gui::id_t id, int min, int max, int value, float x, float y, float width);
  breathe::gui::cRetroColourPicker* AddRetroColourPicker(breathe::gui::id_t id, float x, float y, float width);

  cApplication& application;

  cSettings& settings;

  breathe::render::cFont* pFont;

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

  virtual void _OnWindowEvent(const breathe::gui::cWindowEvent& event) {}
  virtual void _OnMouseEvent(const breathe::gui::cMouseEvent& event);
  virtual void _OnKeyboardEvent(const breathe::gui::cKeyboardEvent& event);

  virtual void _Update(const spitfire::math::cTimeStep& timeStep) {}
  virtual void _UpdateInput(const spitfire::math::cTimeStep& timeStep) {}
  virtual void _Render(const spitfire::math::cTimeStep& timeStep) {}


  virtual void _OnStateMouseEvent(const breathe::gui::cMouseEvent& event) {}
  virtual void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event) {}

  virtual breathe::gui::EVENT_RESULT _OnWidgetEvent(const breathe::gui::cWidgetEvent& event) { return breathe::gui::EVENT_RESULT::NOT_HANDLED_PERCOLATE; }
};


// ** States

class cStateMenu : public cState
{
public:
  explicit cStateMenu(cApplication& application);

private:
  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _Render(const spitfire::math::cTimeStep& timeStep);

  void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event);

  breathe::gui::EVENT_RESULT _OnWidgetEvent(const breathe::gui::cWidgetEvent& event);

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
  void AddColours(breathe::gui::cRetroColourPicker* pColourPicker);

  void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event);

  breathe::gui::EVENT_RESULT _OnWidgetEvent(const breathe::gui::cWidgetEvent& event);

  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _Render(const spitfire::math::cTimeStep& timeStep);

  struct OPTION {
    static const size_t NUMBER_OF_PLAYERS = 1;
    static const size_t NAME_PLAYER1 = 2;
    static const size_t COLOUR_PLAYER1 = 3;
    static const size_t NAME_PLAYER2 = 4;
    static const size_t COLOUR_PLAYER2 = 5;
    static const size_t START = 6;
    static const size_t BACK = 7;
  };

  breathe::gui::cRetroInputUpDown* pNumberOfPlayers;
  breathe::gui::cRetroInput* pPlayerName1;
  breathe::gui::cRetroInput* pPlayerName2;
  breathe::gui::cRetroColourPicker* pPlayerColour1;
  breathe::gui::cRetroColourPicker* pPlayerColour2;

  bool bIsKeyUp;
  bool bIsKeyDown;
  bool bIsKeyReturn;

  size_t previousColour1;
  size_t previousColour2;
};

class cStateHighScores : public cState
{
public:
  explicit cStateHighScores(cApplication& application);
  ~cStateHighScores();

private:
  void UpdateText();

  void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event);

  breathe::gui::EVENT_RESULT _OnWidgetEvent(const breathe::gui::cWidgetEvent& event);

  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _UpdateInput(const spitfire::math::cTimeStep& timeStep);
  void _Render(const spitfire::math::cTimeStep& timeStep);

  struct OPTION {
    static const int BACK = 1;
  };

  breathe::render::cVertexBufferObject* pStaticVertexBufferObjectText;

  bool bIsDone;
};


class cStateGame : public cState, public tetris::cView
{
public:
  explicit cStateGame(cApplication& application);
  ~cStateGame();

private:
  void UpdateText();

  void UpdateBoardVBO(breathe::render::cVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board);
  void UpdatePieceVBO(breathe::render::cVertexBufferObject* pStaticVertexBufferObject, const tetris::cBoard& board, const tetris::cPiece& piece);

  void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event);

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

  breathe::render::cVertexBufferObject* pStaticVertexBufferObjectText;

  breathe::render::cTexture* pTextureBlock;

  breathe::render::cShader* pShaderBlock;
  breathe::audio::cBufferRef pAudioBufferPieceHitsGround;
  breathe::audio::cBufferRef pAudioBufferScoreTetris;
  breathe::audio::cBufferRef pAudioBufferScoreOtherThanTetris;
  breathe::audio::cBufferRef pAudioBufferGameOver;

  std::vector<cBoardRepresentation*> boardRepresentations;

  tetris::cGame game;
};

#endif // TETRIS_STATES_H
