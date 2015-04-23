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
#include <breathe/util/joystick.h>

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

  breathe::render::cVertexBufferObject vertexBufferObjectBoardTriangles;
  breathe::render::cVertexBufferObject vertexBufferObjectPieceTriangles;
  breathe::render::cVertexBufferObject vertexBufferObjectNextPieceTriangles;

  bool bIsInputPieceMoveLeft;
  bool bIsInputPieceMoveRight;
  bool bIsInputPieceRotateCounterClockWise;
  bool bIsInputPieceRotateClockWise;
  bool bIsInputPieceDropOneRow;
  bool bIsInputPieceDropToGround;

  spitfire::durationms_t lastKeyLeft;
  spitfire::durationms_t lastKeyRight;
};


// ** cState

class cState : public breathe::util::cState
{
public:
  explicit cState(cApplication& application);
  virtual ~cState();

  void LoadResources();
  void DestroyResources();

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
  virtual void _OnEnter() override {}
  virtual void _OnExit() override {}
  virtual void _OnPause() override;
  virtual void _OnResume() override;

  virtual void _OnWindowEvent(const breathe::gui::cWindowEvent& event) override {}
  virtual void _OnMouseEvent(const breathe::gui::cMouseEvent& event) override;
  virtual void _OnKeyboardEvent(const breathe::gui::cKeyboardEvent& event) override;
  virtual void _OnJoystickEvent(const breathe::util::cJoystickEvent& event) override;

  virtual void _Update(const spitfire::math::cTimeStep& timeStep) override {}
  virtual void _UpdateInput(const spitfire::math::cTimeStep& timeStep) override {}
  virtual void _Render(const spitfire::math::cTimeStep& timeStep) override;

  virtual void _RenderToTexture(const spitfire::math::cTimeStep& timeStep) {}


  virtual void _OnStateMouseEvent(const breathe::gui::cMouseEvent& event) override {}
  virtual void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event) override {}
  virtual void _OnStateJoystickEvent(const breathe::util::cJoystickEvent& event) override {}

  virtual breathe::gui::EVENT_RESULT _OnWidgetEvent(const breathe::gui::cWidgetEvent& event) override { return breathe::gui::EVENT_RESULT::NOT_HANDLED_PERCOLATE; }

  void CreateVertexBufferObjectLetterBoxedRectangle(size_t width, size_t height);
  void DestroyVertexBufferObjectLetterBoxedRectangle();

  void CreateFrameBufferObjectLetterBoxedRectangle(size_t width, size_t height);
  void DestroyFrameBufferObjectLetterBoxedRectangle();

  void CreateShaderLetterBoxedRectangle();
  void DestroyShaderLetterBoxedRectangle();

  breathe::render::cVertexBufferObject vertexBufferObjectLetterBoxedRectangle;
  breathe::render::cTextureFrameBufferObject* pFrameBufferObjectLetterBoxedRectangle;
  breathe::render::cShader* pShaderLetterBoxedRectangle;
};


// ** States

class cStateMenu : public cState
{
public:
  explicit cStateMenu(cApplication& application);

private:
  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _RenderToTexture(const spitfire::math::cTimeStep& timeStep);

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
  void _RenderToTexture(const spitfire::math::cTimeStep& timeStep);

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

private:
  void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event);

  breathe::gui::EVENT_RESULT _OnWidgetEvent(const breathe::gui::cWidgetEvent& event);

  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _UpdateInput(const spitfire::math::cTimeStep& timeStep);
  void _RenderToTexture(const spitfire::math::cTimeStep& timeStep);

  struct OPTION {
    static const int BACK = 1;
  };

  bool bIsDone;
};

class cStateGame;

class cStatePauseMenu : public cState
{
public:
  cStatePauseMenu(cApplication& application, cStateGame& parentState);

private:
  void _Update(const spitfire::math::cTimeStep& timeStep);
  void _RenderToTexture(const spitfire::math::cTimeStep& timeStep);

  void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event);

  breathe::gui::EVENT_RESULT _OnWidgetEvent(const breathe::gui::cWidgetEvent& event);

  cStateGame& parentState;

  struct OPTION {
    static const int RESUME_GAME = 1;
    static const int END_GAME = 3;
  };

  bool bIsKeyReturn;
};

class cStateGame : public cState, public tetris::cView
{
public:
  explicit cStateGame(cApplication& application);
  ~cStateGame();

  void SetQuitSoon(); // Called by the pause menu when the game should quit

private:
  void UpdateText();

  void UpdateBoardVBO(breathe::render::cVertexBufferObject& vertexBufferObject, const tetris::cBoard& board);
  void UpdatePieceVBO(breathe::render::cVertexBufferObject& vertexBufferObject, const tetris::cBoard& board, const tetris::cPiece& piece);

  virtual void _OnStateKeyboardEvent(const breathe::gui::cKeyboardEvent& event) override;
  virtual void _OnStateJoystickEvent(const breathe::util::cJoystickEvent& event) override;

  virtual void _Update(const spitfire::math::cTimeStep& timeStep) override;
  virtual void _UpdateInput(const spitfire::math::cTimeStep& timeStep) override;
  virtual void _RenderToTexture(const spitfire::math::cTimeStep& timeStep) override;

  virtual void _OnPieceMoved(const tetris::cBoard& board) override;
  virtual void _OnPieceRotated(const tetris::cBoard& board) override;
  virtual void _OnPieceChanged(const tetris::cBoard& board) override;
  virtual void _OnPieceHitsGround(const tetris::cBoard& board) override;
  virtual void _OnBoardChanged(const tetris::cBoard& board) override;
  virtual void _OnGameScoreTetris(const tetris::cBoard& board, size_t uiScore) override;
  virtual void _OnGameScoreOtherThanTetris(const tetris::cBoard& board, size_t uiScore) override;
  virtual void _OnGameNewLevel(const tetris::cBoard& board, size_t uiLevel) override;
  virtual void _OnGameOver(const tetris::cBoard& board) override;

  breathe::gui::cStaticText* pLevelText[4];
  breathe::gui::cStaticText* pScoreText[4];

  breathe::render::cTexture* pTextureBlock;

  breathe::render::cShader* pShaderBlock;
  breathe::audio::cBufferRef pAudioBufferPieceHitsGround;
  breathe::audio::cBufferRef pAudioBufferScoreTetris;
  breathe::audio::cBufferRef pAudioBufferScoreOtherThanTetris;
  breathe::audio::cBufferRef pAudioBufferGameOver;

  std::vector<cBoardRepresentation*> boardRepresentations;

  tetris::cGame game;

  bool bPauseSoon;
  bool bQuitSoon;
};

#endif // TETRIS_STATES_H
