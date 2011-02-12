#ifndef CGAME_H
#define CGAME_H

class cGame : public breathe::cApp, public tetris::cBoardCollection
{
public:
	cGame(int argc, char** argv);
	~cGame();
  
  typedef std::list<tetris::cBoard*>::iterator board_iterator;
		
	bool LoadScene();
	bool InitScene();
	bool DestroyScene();

	void FullscreenSwitch();

	bool Execute(const std::string& sCommand);

  breathe::render::cFont* GetFont() { return pFont; }
  std::vector<SDL_Joystick*>& GetJoysticks() { return vJoystick; }

  void SetPlayers(size_t players);

private:
	void _OnMouseEvent(int button, int state, int x, int y);
	
  void AddPlayer();
  void RemovePlayer(size_t i);

  void StartGame();
	void LoadConfiguration();
	void LoadPiece(breathe::xml::cNode::iterator iter);
	

  // *** From tetris::cBoardCollection
  void _OnScoreTetris(const tetris::cBoard& rhs);
  void _OnScoreOtherThanTetris(const tetris::cBoard& rhs);
  void _OnPieceHitsGround(const tetris::cBoard& rhs);
  void _OnGameOver(const tetris::cBoard& rhs);


  class cTetrisStatePlaying : public breathe::cApp::cAppState
  {
  public:
    cTetrisStatePlaying(cGame& _game) : game(_game) {}

  private:
    void _OnResume(int iResult);

    void _Update(breathe::sampletime_t currentTime);
	  void _UpdateInput(breathe::sampletime_t currentTime);
	  void _RenderScreenSpace(breathe::sampletime_t currentTime);
    
	  void RenderTetrisPiece(const tetris::cBoard& board, const tetris::cPiece& piece, float x, float y);

    static const float block_w;
    static const float block_h;

    cGame& game;
  };

  class cTetrisStatePaused : public breathe::cApp::cAppState
  {
  public:
    cTetrisStatePaused(cGame& _game) : game(_game) {}

  private:
	  void _UpdateInput(breathe::sampletime_t currentTime);
	  void _RenderScreenSpace(breathe::sampletime_t currentTime);

    cGame& game;
  };

  class cTetrisStateFinished : public breathe::cApp::cAppState
  {
  public:
    cTetrisStateFinished(cGame& _game) : game(_game) {}

  private:
	  void _UpdateInput(breathe::sampletime_t currentTime);
	  void _RenderScreenSpace(breathe::sampletime_t currentTime);

    cGame& game;
  };
};


#endif //CGAME_H
