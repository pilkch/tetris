#ifndef TETRIS_H
#define TETRIS_H

// Spitfire headers
#include <spitfire/spitfire.h>

#include <spitfire/algorithm/algorithm.h>
#include <spitfire/math/cColour.h>

namespace tetris
{
  class cBoard;
  class cView;

  class cGame
  {
  public:
    cGame(cView& view);

    typedef std::vector<cBoard*>::iterator iterator;

    void OnScoreTetris(const cBoard& rhs);
    void OnScoreOtherThanTetris(const cBoard& rhs, size_t lines);
    void OnPieceRotated(const cBoard& rhs);
    void OnPieceHitsGround(const cBoard& rhs);
    void OnPieceChanged(const cBoard& board);
    void OnBoardChanged(const cBoard& board);
    void OnGameOver(const cBoard& rhs);

    std::vector<cBoard*> boards;

    void StartGame(spitfire::sampletime_t currentTime);
    void Update(spitfire::sampletime_t currentTime);

  private:
    void _AddRandomLinesToEveryOtherBoard(const cBoard& rhs, size_t lines);

    cView& view;
  };

  class cPiece
  {
  public:
    explicit cPiece();
    explicit cPiece(size_t width, size_t height);
    
    cPiece(const cPiece& rhs);

    cPiece& operator=(const cPiece& rhs);

    size_t GetWidth() const { return width; }
    void SetWidth(size_t width);
    size_t GetHeight() const { return height; }
    void SetHeight(size_t height);

    int GetBlock(size_t x, size_t y) const;
    void SetBlock(size_t x, size_t y, int colour);

    cPiece GetRotatedCounterClockWise() const;
    cPiece GetRotatedClockWise() const;

    void RemoveLine(size_t row);
    void Clear();

    void ShiftUpOneRow();
    void FlipVertically();

  private:
    void _Resize(size_t width, size_t height);

    std::vector<int> blocks;
      
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
    explicit cBoard(cGame& game);
    ~cBoard();

    void StartGame(spitfire::sampletime_t currentTime);
    void Update(spitfire::sampletime_t currentTime);

    void CopySettingsFrom(const cBoard& rhs);
    void SetWidth(size_t width);
    void SetHeight(size_t height);

    bool IsPlaying() const { return (state == STATE_PLAYING); }
    bool IsFinished() const { return (state == STATE_FINISHED); }

    int GetScore() const { return score; }
    int GetLevel() const { return level; }

    size_t GetWidth() const { return board.GetWidth(); }
    size_t GetHeight() const { return board.GetHeight(); }
    size_t GetWidestPiece() const { return widest_piece; }

    spitfire::math::cColour GetColour(size_t i) const { assert(i < possible_colours.size()); return possible_colours[i]; }
    size_t GetColourFromName(const std::string& colour);

    void SetBlock(size_t x, size_t y, int colour);
    int GetBlock(size_t x, size_t y) const;

    size_t GetCurrentPieceX() const { return current_x; }
    size_t GetCurrentPieceY() const { return current_y; }

    const cPiece& GetBoard() const { return board; }
    const cPiece& GetCurrentPiece() const { return current_piece; }
    const cPiece& GetNextPiece() const { return next_piece; }

    size_t GetColours() const { return possible_colours.size(); }

    void AddPossibleColour(const std::string& name, const spitfire::math::cColour& colour);
    void AddPossiblePiece(const cPiece& piece);
    void AddRandomLineAddEnd();

    void PieceGenerate(spitfire::sampletime_t currentTime);

    void PieceMoveLeft();
    void PieceMoveRight();
    void PieceRotateCounterClockWise();
    void PieceRotateClockWise();
    void PieceDropOneRow(spitfire::sampletime_t currentTime);
    void PieceDropToGround(spitfire::sampletime_t currentTime);

#define BUILD_DEBUG
#ifdef BUILD_DEBUG
    // For printing out as debug information
    const std::list<cPiece>& GetPossiblePieces() const { return possible_pieces.GetPossibleItems(); }
#endif

  private:
    void _AddPieceToScore();
    void _AddRowsToScore(size_t rows);
    void _CheckForCompleteLines();
    void _RemoveLine(size_t row);

    bool _IsCompleteLine(size_t row) const;

    bool _IsCollided(const cPiece& rhs, size_t position_x, size_t position_y) const;

    void _AddPieceToBoardCheckAndGenerate(spitfire::sampletime_t currentTime);
    void _AddPieceToBoard();

    cGame& game;

    std::vector<std::string> possible_colour_names;
    std::vector<spitfire::math::cColour> possible_colours;
    spitfire::cRandomBucket<cPiece> possible_pieces;
    size_t widest_piece;

    cPiece board;
    cPiece current_piece;
    cPiece next_piece;

    size_t current_x;
    size_t current_y;

    STATE state;
    uint32_t score;
    size_t level;
    size_t rows_this_level;
    size_t consecutive_tetris;
    
    spitfire::sampletime_t lastUpdatedTime;

    cBoard();
    NO_COPY(cBoard);
  };


  class cView
  {
  public:
    virtual ~cView() {}

    void OnPieceMoved(const cBoard& board) { _OnPieceMoved(board); }
    void OnPieceRotated(const cBoard& board) { _OnPieceRotated(board); }
    void OnPieceChanged(const cBoard& board) { _OnPieceChanged(board); }
    void OnPieceHitsGround(const cBoard& board) { _OnPieceHitsGround(board); }
    void OnBoardChanged(const cBoard& board) { _OnBoardChanged(board); }
    void OnGameScoreTetris(const cBoard& board, uint32_t uiScore) { _OnGameScoreTetris(board, uiScore); }
    void OnGameScoreOtherThanTetris(const cBoard& board, uint32_t uiScore) { _OnGameScoreOtherThanTetris(board, uiScore); }
    void OnGameNewLevel(const cBoard& board, uint32_t uiLevel) { _OnGameNewLevel(board, uiLevel); }
    void OnGameOver(const cBoard& board) { _OnGameOver(board); }

  private:
    virtual void _OnPieceMoved(const cBoard& board) = 0;
    virtual void _OnPieceRotated(const cBoard& board) = 0;
    virtual void _OnPieceChanged(const cBoard& board) = 0;
    virtual void _OnPieceHitsGround(const cBoard& board) = 0;
    virtual void _OnBoardChanged(const cBoard& board) = 0;
    virtual void _OnGameScoreTetris(const cBoard& board, uint32_t uiScore) = 0;
    virtual void _OnGameScoreOtherThanTetris(const cBoard& board, uint32_t uiScore) = 0;
    virtual void _OnGameNewLevel(const cBoard& board, uint32_t uiLevel) = 0;
    virtual void _OnGameOver(const cBoard& board) = 0;
  };
}

#endif //TETRIS_H
