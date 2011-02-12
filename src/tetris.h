#ifndef TETRIS_H
#define TETRIS_H

namespace tetris
{
  class cBoard;

  class cBoardCollection
  {
  public:
    typedef std::list<cBoard*>::iterator iterator;

    void OnScoreTetris(const cBoard& rhs);
    void OnScoreOtherThanTetris(const cBoard& rhs, size_t lines);
    void OnPieceHitsGround(const cBoard& rhs) { _OnPieceHitsGround(rhs); }
    void OnGameOver(const cBoard& rhs) { _OnGameOver(rhs); }
    
    std::list<cBoard*> board;

  private:
    void _AddLinesToEveryOtherBoard(const cBoard& rhs, size_t lines);

    virtual void _OnScoreOtherThanTetris(const cBoard& rhs) = 0;
    virtual void _OnScoreTetris(const cBoard& rhs) = 0;
    virtual void _OnPieceHitsGround(const cBoard& rhs) = 0;
    virtual void _OnGameOver(const cBoard& rhs) = 0;
  };

	class cPiece
	{
	public:
		explicit cPiece();
		explicit cPiece(size_t width, size_t height);
		
    cPiece(const cPiece& rhs);

		cPiece& operator=(const cPiece& rhs);

		void SetWidth(size_t width);
		void SetHeight(size_t height);
		void FlipVertically();

		size_t GetWidth() const { return width; }
		size_t GetHeight() const { return height; }

		void SetBlock(size_t x, size_t y, int colour);
		int GetBlock(size_t x, size_t y) const;

		cPiece GetRotatedCounterClockWise() const;
		cPiece GetRotatedClockWise() const;

		void RemoveLine(size_t row);
		void Clear();

    void ShiftUpOneRow();

	private:
		void _Resize(size_t n);

		std::vector<int> block;
			
		size_t width;
		size_t height;
	};

	enum STATE
	{
		STATE_PLAYING = 0,
		STATE_FINISHED,
	};

	class cBoard
	{
	public:
		explicit cBoard(cBoardCollection& collection);
    ~cBoard();

		void StartGame(breathe::sampletime_t currentTime);
		void Update(breathe::sampletime_t currentTime);

    void CopySettingsFrom(const cBoard& rhs);
		void SetWidth(size_t width);
		void SetHeight(size_t height);
		
		bool IsPlaying() const { return (state == STATE_PLAYING); }
		bool IsFinished() const { return (state == STATE_FINISHED); }
		
    int GetScore() const { return score; }
		int GetLevel() const { return level; }

    void SetPositionBoardX(float x) { position_board_x = x; }
    void SetPositionBoardY(float y) { position_board_y = y; }
    float GetPositionBoardX() const { return position_board_x; }
    float GetPositionBoardY() const { return position_board_y; }

    void SetPositionNextX(float x) { position_next_x = x; }
    void SetPositionNextY(float y) { position_next_y = y; }
    float GetPositionNextX() const { return position_next_x; }
    float GetPositionNextY() const { return position_next_y; }

		size_t GetWidth() const { return board.GetWidth(); }
		size_t GetHeight() const { return board.GetHeight(); }
		size_t GetWidestPiece() const { return widest_piece; }

		breathe::math::cColour GetColour(size_t i) const { assert(i < possible_colours.size()); return possible_colours[i]; }
		size_t GetColourFromName(const std::string& colour);

		void SetBlock(size_t x, size_t y, int colour);
		int GetBlock(size_t x, size_t y) const;

		size_t GetCurrentPieceX() const { return current_x; }
		size_t GetCurrentPieceY() const { return current_y; }

		const cPiece& GetBoard() const { return board; }
		const cPiece& GetCurrentPiece() const { return current_piece; }
		const cPiece& GetNextPiece() const { return next_piece; }

		size_t GetColours() const { return possible_colours.size(); }
		
		void AddPossibleColour(const std::string& name, const breathe::math::cColour& colour);
		void AddPossiblePiece(const cPiece& piece);
    void AddRandomLineAddEnd();
		
		void PieceGenerate();

		void PieceMoveLeft();
		void PieceMoveRight();
		void PieceRotateCounterClockWise();
		void PieceRotateClockWise();
		void PieceDropOneRow();
		void PieceDropToGround();

#ifdef BUILD_DEBUG
		// For printing out as debug information
		const std::vector<cPiece>& GetPossiblePieces() const { return possible_pieces; }
#endif
    
	private:
    void _AddPieceToScore();
		void _AddRowsToScore(size_t rows);
		void _CheckForCompleteLines();
		void _RemoveLine(size_t row);
		
		bool _IsCompleteLine(size_t row) const;

		bool _IsCollided(const cPiece& rhs, size_t position_x, size_t position_y) const;
		
		void _AddPieceToBoardCheckAndGenerate();
		void _AddPieceToBoard();

		std::vector<std::string> possible_colour_names;
		std::vector<breathe::math::cColour> possible_colours;
		std::vector<cPiece> possible_pieces;
		size_t widest_piece;
		
		cPiece board;
		cPiece current_piece;
		cPiece next_piece;

		size_t current_x;
		size_t current_y;

    float position_board_x;
    float position_board_y;
    float position_next_x;
    float position_next_y;

		STATE state;
		uint32_t score;
		size_t level;
		size_t rows_this_level;
		size_t consecutive_tetris;
		
		breathe::sampletime_t lastUpdatedTime;

    cBoardCollection& parentCollection;

    cBoard();
		NO_COPY(cBoard);
	};
}

#endif //TETRIS_H
