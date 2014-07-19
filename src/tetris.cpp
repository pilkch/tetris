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

#include <spitfire/algorithm/algorithm.h>
#include <spitfire/math/math.h>

#include <spitfire/util/timer.h>

#include "tetris.h"

namespace tetris
{
  // ** cGame

  cGame::cGame(cView& _view) :
    view(_view)
  {
  }

  void cGame::_AddRandomLinesToEveryOtherBoard(const cBoard& board, size_t lines)
  {
    // Add a random line to every other board that is still playing and not our board
    iterator iter = boards.begin();
    const iterator iterEnd = boards.end();
    cBoard* temp = nullptr;
    const cBoard* pBoard = &board;
    while (iter != iterEnd) {
      temp = *iter;
      if ((const_cast<const cBoard*>(temp) != pBoard) && (!temp->IsFinished())) {
        for (size_t i = 0; i < lines; i++) temp->AddRandomLineAddEnd();
      }
      iter++;
    }
  }

  void cGame::OnScoreTetris(const cBoard& board)
  {
    _AddRandomLinesToEveryOtherBoard(board, 4);

    view.OnGameScoreTetris(board, 4);
  }

  void cGame::OnScoreOtherThanTetris(const cBoard& board, size_t lines)
  {
    _AddRandomLinesToEveryOtherBoard(board, lines);

    view.OnGameScoreOtherThanTetris(board, lines);
  }

  void cGame::OnPieceRotated(const cBoard& board)
  {
    view.OnPieceRotated(board);
  }

  void cGame::OnPieceHitsGround(const cBoard& board)
  {
    view.OnPieceHitsGround(board);
  }

  void cGame::OnPieceChanged(const cBoard& board)
  {
    view.OnPieceChanged(board);
  }

  void cGame::OnBoardChanged(const cBoard& board)
  {
    view.OnBoardChanged(board);
  }

  void cGame::OnGameOver(const cBoard& board)
  {
    view.OnGameOver(board);
  }

  void cGame::StartGame(spitfire::durationms_t currentTime)
  {
    const size_t width = 10;
    const size_t height = 40;

    const spitfire::math::cColour colourRed(1.0f, 0.0f, 0.0f);
    const spitfire::math::cColour colourGreen(0.0, 1.0, 0.0);
    const spitfire::math::cColour colourBlue(0.0, 0.0, 1.0);
    const spitfire::math::cColour colourPink(1.0, 0.0, 0.5);
    const spitfire::math::cColour colourYellow(1.0, 1.0, 0.0);
    const spitfire::math::cColour colourLightBlue(0.5, 0.5, 1.0);
    const spitfire::math::cColour colourOrange(1.0, 0.5, 0.0);

    iterator iter = boards.begin();
    const iterator iterEnd = boards.end();
    while (iter != iterEnd) {
      cBoard* pBoard = *iter;
      pBoard->SetWidth(width);
      pBoard->SetHeight(height);

      pBoard->AddPossibleColour("red", colourRed);
      pBoard->AddPossibleColour("green", colourGreen);
      pBoard->AddPossibleColour("blue", colourBlue);
      pBoard->AddPossibleColour("pink", colourPink);
      pBoard->AddPossibleColour("yellow", colourYellow);
      pBoard->AddPossibleColour("light_blue", colourLightBlue);
      pBoard->AddPossibleColour("orange", colourOrange);


      //const size_t iBlank = 0;
      const size_t iRed = pBoard->GetColourFromName("red");
      const size_t iGreen = pBoard->GetColourFromName("green");
      const size_t iBlue = pBoard->GetColourFromName("blue");
      const size_t iPink = pBoard->GetColourFromName("pink");
      const size_t iYellow = pBoard->GetColourFromName("yellow");
      const size_t iLightBlue = pBoard->GetColourFromName("light_blue");
      const size_t iOrange = pBoard->GetColourFromName("orange");

      cPiece pieceLong;
      pieceLong.SetBlock(0, 0, int(iRed));
      pieceLong.SetBlock(0, 1, int(iRed));
      pieceLong.SetBlock(0, 2, int(iRed));
      pieceLong.SetBlock(0, 3, int(iRed));


      cPiece pieceSquare;
      pieceSquare.SetBlock(0, 0, int(iBlue)); pieceSquare.SetBlock(1, 0, int(iBlue));
      pieceSquare.SetBlock(0, 1, int(iBlue)); pieceSquare.SetBlock(1, 1, int(iBlue));


      cPiece pieceLReverse;
                                                  pieceLReverse.SetBlock(1, 0, int(iYellow));
                                                  pieceLReverse.SetBlock(1, 1, int(iYellow));
      pieceLReverse.SetBlock(0, 2, int(iYellow)); pieceLReverse.SetBlock(1, 2, int(iYellow));


      cPiece pieceL;
      pieceL.SetBlock(0, 0, int(iPink));
      pieceL.SetBlock(0, 1, int(iPink));
      pieceL.SetBlock(0, 2, int(iPink)); pieceL.SetBlock(1, 2, int(iPink));


      cPiece piecePyramid;
                                                piecePyramid.SetBlock(1, 0, int(iGreen));
      piecePyramid.SetBlock(0, 1, int(iGreen)); piecePyramid.SetBlock(1, 1, int(iGreen)); piecePyramid.SetBlock(2, 1, int(iGreen));


      cPiece pieceS;
                                              pieceS.SetBlock(1, 0, int(iLightBlue)); pieceS.SetBlock(2, 0, int(iLightBlue));
      pieceS.SetBlock(0, 1, int(iLightBlue)); pieceS.SetBlock(1, 1, int(iLightBlue));


      cPiece pieceZ;
      pieceZ.SetBlock(0, 0, int(iOrange)); pieceZ.SetBlock(1, 0, int(iOrange));
                                           pieceZ.SetBlock(1, 1, int(iOrange)); pieceZ.SetBlock(2, 1, int(iOrange));


      pBoard->AddPossiblePiece(pieceLong);
      pBoard->AddPossiblePiece(pieceSquare);
      pBoard->AddPossiblePiece(pieceLReverse);
      pBoard->AddPossiblePiece(pieceL);
      pBoard->AddPossiblePiece(piecePyramid);
      pBoard->AddPossiblePiece(pieceS);
      pBoard->AddPossiblePiece(pieceZ);

      pBoard->StartGame(currentTime);

      iter++;
    }
  }

  void cGame::Update(spitfire::durationms_t currentTime)
  {
    iterator iter = boards.begin();
    const iterator iterEnd = boards.end();
    while (iter != iterEnd) {
      cBoard* pBoard = *iter;
      pBoard->Update(currentTime);

      iter++;
    }
  }


  // ** cPiece

  cPiece::cPiece() :
    width(1),
    height(1)
  {
    blocks.resize(width * height, 0);
  }

  cPiece::cPiece(size_t _width, size_t _height) :
    width(_width),
    height(_height)
  {
    if (width < 1) width = 1;
    if (height < 1) height = 1;

    blocks.resize(width * height, 0);
  }

  cPiece::cPiece(const cPiece& rhs)
  {
    *this = rhs;
  }

  void cPiece::_Resize(size_t _width, size_t _height)
  {
    if (_width < width) _width = width;
    if (_height < height) _height = height;

    std::vector<int> temp;
    temp.resize(_width * _height, 0);

    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++) {
        temp[(y * _width) + x] = blocks[(y * width) + x];
      }
    }

    blocks = temp;

    width = _width;
    height = _height;

    assert((width * height) == blocks.size());
  }

  void cPiece::SetWidth(size_t _width)
  {
    _width++;
    if (_width < width) return;

    _Resize(_width, height);

    assert(blocks.size() >= (width * height));
  }

  void cPiece::SetHeight(size_t _height)
  {
    _height++;
    if (_height < height) return;

    _Resize(width, _height);

    assert(blocks.size() >= (width * height));
  }

  int cPiece::GetBlock(size_t x, size_t y) const
  {
    assert(((y * width) + x) < blocks.size());
    return blocks[(y * width) + x];
  }

  void cPiece::SetBlock(size_t x, size_t y, int colour)
  {
    std::cout<<"cPiece::SetBlock "<<x<<","<<y<<" "<<colour<<" blocks size "<<blocks.size()<<std::endl;
    _Resize(x + 1, y + 1);

    std::cout<<"cPiece::SetBlock blocks size "<<blocks.size()<<std::endl;
    assert(((y * width) + x) < blocks.size());
    blocks[(y * width) + x] = colour;

    // Make sure that our block has now been set to the correct colour
    assert(GetBlock(x, y) == colour);
  }

  void cPiece::RemoveLine(size_t row)
  {
    assert(row <= height);
    std::copy(blocks.begin() + (row + 1) * width, blocks.end(), blocks.begin() + row * width);
    size_t i = width * (height-1);
    size_t n = width * height;
    for (; i < n; i++) blocks[i] = 0;
  }

  // For adding a random line at the bottom of the board
  void cPiece::ShiftUpOneRow()
  {
    size_t i = 0;
    size_t n = width * height;

    std::vector<int> temp;
    temp.resize(n);

    // Set all of the first line to 0
    for (i = 0; i < width; i++) temp[i] = 0;

    // Copy the rest of the piece to temp
    std::copy(blocks.begin(), blocks.end() - width, temp.begin() + width);

    // Copy the whole thing back again
    blocks = temp;

    assert((temp.size() == blocks.size()) && (blocks.size() == n));
  }

  void cPiece::FlipVertically()
  {
    // Make a temp copy of the whole array so that we don't tread on our original data
    std::vector<int> temp = blocks;

    // Now copy back the data in the correct order
    size_t y2 = height - 1;
    for (size_t y1 = 0; y1 < height; y1++, y2--) {
      for (size_t x1 = 0, x2 = 0; x1 < width; x1++, x2++) {
        blocks[y1 * width + x1] = temp[y2 * width + x2];
      }
    }
  }

  cPiece cPiece::GetRotatedCounterClockWise() const
  {
    std::cout<<"cPiece::GetRotatedCounterClockWise"<<std::endl;
    cPiece result;

    // Set our width
    result.width = height;
    result.height = width;

    // Add enough blocks
    size_t n = width * height;
    result.blocks.resize(n, 0);

    // Now copy our blocks over to the result piece, but rotated
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++) {
        result.blocks[x * height + (height - 1 - y)] = blocks[(y * width) + x];
      }
    }

    return result;
  }

  cPiece cPiece::GetRotatedClockWise() const
  {
    std::cout<<"cPiece::GetRotatedClockWise"<<std::endl;
    cPiece result;

    // Set our width
    result.width = height;
    result.height = width;

    // Add enough blocks
    size_t n = width * height;
    result.blocks.resize(n, 0);

    // Now copy our blocks over to the result piece, but rotated
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++) {
        result.blocks[((width - 1 - x) * height) + y] = blocks[(y * width) + x];
      }
    }

    return result;
  }

  cPiece& cPiece::operator=(const cPiece& rhs)
  {
    blocks = rhs.blocks;
    width = rhs.width;
    height = rhs.height;

    return *this;
  }

  void cPiece::Clear()
  {
    blocks.resize(width * height, 0);
  }


  // ** cBoard

  cBoard::cBoard(cGame& _game) :
    game(_game),

    widest_piece(0),

    current_x(0),
    current_y(0),

    state(STATE_FINISHED),

    rows_this_level(0)
  {
    AddPossibleColour("", spitfire::math::cColour());
  }

  cBoard::~cBoard()
  {
    possible_colours.clear();
    possible_pieces.Clear();
  }

  void cBoard::CopySettingsFrom(const cBoard& rhs)
  {
    board = rhs.board;

    possible_pieces = rhs.possible_pieces;
    possible_colour_names = rhs.possible_colour_names;
    possible_colours = rhs.possible_colours;

    widest_piece = rhs.widest_piece;
    state = rhs.state;
  }

  void cBoard::StartGame(spitfire::durationms_t currentTime)
  {
    lastUpdatedTime = currentTime;

    state = STATE_PLAYING;
    score = 0;
    level = 1;
    rows_this_level = 0;

    // Clear the board
    board.Clear();

    if (board.GetWidth() == 0) std::cout<<"cBoard::StartGame Width not defined"<<std::endl;
    if ((board.GetHeight()>>1) == 0) std::cout<<"cBoard::StartGame Height not defined"<<std::endl;
    if (GetColours() == 0) std::cout<<"cBoard::StartGame Not enough colours defined"<<std::endl;

    assert(board.GetWidth() != 0);
    assert((board.GetHeight()>>1) != 0);
    assert(GetColours() != 0);


    // Add some random blocks to make it interesting at the start
    for (size_t i = 0; i < 60; i++) {
      board.SetBlock(spitfire::math::random(int(board.GetWidth())), spitfire::math::random(int(board.GetHeight()>>1)), spitfire::math::random(int(GetColours())));
    }

    PieceGenerate(currentTime);
    PieceGenerate(currentTime);
  }

  void cBoard::Update(spitfire::durationms_t currentTime)
  {
    // If we have wait a sufficient amount of time, then do an update
    if ((currentTime - lastUpdatedTime) > (1500 / level)) {
      lastUpdatedTime = currentTime;

      if (state != STATE_FINISHED) PieceDropOneRow(currentTime);
    }
  }

  void cBoard::_AddPieceToScore()
  {
  }

  void cBoard::_AddRowsToScore(size_t rows)
  {
    // Increase the player's score
    score += (rows * rows) * 100;

    // If the player has had 8 rows this level increment the level
    rows_this_level += rows;
    if (rows_this_level > 8) {
      rows_this_level = 0;
      level++;
    }

    if (rows > 3) game.OnScoreTetris(*this);
    else game.OnScoreOtherThanTetris(*this, rows);
  }

  bool cBoard::_IsCompleteLine(size_t row) const
  {
    assert(row <= board.GetHeight());

    size_t column = 0;
    for (column = 0; column < board.GetWidth(); column++)
      if (board.GetBlock(column, row) == 0) return false;

    return true;
  }

  void cBoard::_RemoveLine(size_t row)
  {
    assert(row <= board.GetHeight());
    board.RemoveLine(row);
  }

  void cBoard::_CheckForCompleteLines()
  {
    size_t row = 0;
    size_t consecutive = 0;
    for (row = 0; row < board.GetHeight(); row++) {
      consecutive = 0;
      while (_IsCompleteLine(row)) {
        _RemoveLine(row);
        consecutive++;
      }

      // If we actually have one or more complete rows then add them to our score
      if (consecutive > 0) _AddRowsToScore(consecutive);
    }
  }

  void cBoard::_AddPieceToBoard()
  {
    assert(current_x <= board.GetWidth() - current_piece.GetWidth());

    size_t y1 = 0;
    size_t x1 = 0;
    size_t x2 = 0;
    size_t y2 = 0;
    size_t width = current_piece.GetWidth();
    size_t height = current_piece.GetHeight();
    int colour = 0;
    for (y1 = 0, y2 = current_y - height; (y1 < height) && (y2 < board.GetHeight()); y1++, y2++) {
      for (x1 = 0, x2 = current_x; x1 < width; x1++, x2++) {
        colour = current_piece.GetBlock(x1, y1);
        if (colour != 0) board.SetBlock(x2, y2, colour);
      }
    }
  }

  void cBoard::_AddPieceToBoardCheckAndGenerate(spitfire::durationms_t currentTime)
  {
    _AddPieceToBoard();

    _AddPieceToScore();

    _CheckForCompleteLines();

    // Now generate our new piece so that we always have a usable piece
    PieceGenerate(currentTime);

    game.OnPieceHitsGround(*this);
    game.OnBoardChanged(*this);
  }

  bool cBoard::_IsCollided(const cPiece& rhs, size_t position_x, size_t position_y) const
  {
    if (position_x > board.GetWidth() - rhs.GetWidth()) return true;

    size_t y1 = 0;
    size_t x1 = 0;
    size_t x2 = 0;
    size_t y2 = 0;
    size_t width = rhs.GetWidth();
    size_t height = rhs.GetHeight();
    for (y1 = 0, y2 = position_y - height; y1 < height; y1++, y2++) {
      for (x1 = 0, x2 = position_x; x1 < width; x1++, x2++) {
        if (y2 < board.GetHeight() && (rhs.GetBlock(x1, y1) != 0) && (board.GetBlock(x2, y2) != 0)) return true;
      }
    }

    return false;
  }

  void cBoard::SetWidth(size_t _width)
  {
    board.SetWidth(_width);
  }

  void cBoard::SetHeight(size_t _height)
  {
    board.SetHeight(_height);
  }

  void cBoard::AddPossibleColour(const std::string& name, const spitfire::math::cColour& colour)
  {
    possible_colour_names.push_back(name);
    possible_colours.push_back(colour);
  }

  void cBoard::AddPossiblePiece(const cPiece& piece)
  {
    possible_pieces.AddItem(piece);
    widest_piece = std::max(std::max(widest_piece, piece.GetWidth()), piece.GetHeight());
  }

  void cBoard::SetBlock(size_t x, size_t y, int colour)
  {
    board.SetBlock(x, y, colour);
  }

  int cBoard::GetBlock(size_t x, size_t y) const
  {
    return board.GetBlock(x, y);
  }

  size_t cBoard::GetColourFromName(const std::string& name)
  {
    size_t i = 0;
    size_t n = possible_colour_names.size();

    while (i < n) {
      if (name == possible_colour_names[i]) return i;

      i++;
    }

    return 0;
  }


  void cBoard::PieceGenerate(spitfire::durationms_t currentTime)
  {
    current_piece = next_piece;

    next_piece = possible_pieces.GetRandomItem();
    std::cout<<"cBoard::PieceGenerate Adding piece which is "<<next_piece.GetWidth()<<" by "<<next_piece.GetHeight()<<std::endl;

    current_x = (board.GetWidth()>>1) - (current_piece.GetWidth()>>1);
    current_y = board.GetHeight() + current_piece.GetHeight();
    while (current_y > board.GetHeight()) {
      if (_IsCollided(current_piece, current_x, current_y - 1)) {
        // Add as much of the piece as possible to the board
        _AddPieceToBoard();

        // Tell the game that this board has finished
        game.OnGameOver(*this);
        state = STATE_FINISHED;

        // Clear the piece
        current_piece.Clear();

        break;
      }

      current_y--;
    };

    lastUpdatedTime = currentTime;

    game.OnPieceChanged(*this);
  }

  template <class T>
  inline T GetListElement(std::list<T>& elements, size_t n)
  {
    typedef typename std::list<T>::iterator iterator;

    iterator iter = elements.begin();
    size_t count = elements.size();
    if (1 == count) return *iter;

    iterator iterEnd = elements.end();
    size_t i = 0;
    while (iter != iterEnd) {
      if (i == n) break;
      i++;
      iter++;
    }

    T temp = *iter;
    elements.erase(iter);
    assert(elements.size() == count - 1);

    return temp;
  }

  void cBoard::AddRandomLineAddEnd()
  {
    board.ShiftUpOneRow();

    size_t width = board.GetWidth();
    //size_t height = board.GetHeight();
    size_t i = 0;
    size_t n = 0;
    size_t possible_colours_n = possible_colours.size();
    std::list<int> possible_blocks;

    n = (width>>2) + 1;
    for (i = 0; i < n; i ++) possible_blocks.push_back(0);

    n = width;
    for (; i < n; i++) possible_blocks.push_back(spitfire::math::random(int(possible_colours_n)));

    size_t colour = 0;
    n = width - 1;
    //size_t startingCount = possible_blocks.size();
    for (i = 0; i < n; i++) {
      size_t count = possible_blocks.size();
      colour = GetListElement(possible_blocks, spitfire::math::random(int(count)));
      assert(colour < possible_colours.size());
      board.SetBlock(i, 0, int(colour));
    }

    colour = GetListElement(possible_blocks, 0);
    assert(colour < possible_colours.size());
    board.SetBlock(i, 0, int(colour));

    game.OnBoardChanged(*this);
  }



  // *** Input

  void cBoard::PieceMoveLeft()
  {
    if (state != STATE_PLAYING) return;

    if (current_x > 0) current_x--;
    if (_IsCollided(current_piece, current_x, current_y)) current_x++;
  }

  void cBoard::PieceMoveRight()
  {
    if (state != STATE_PLAYING) return;

    current_x = std::min(current_x + 1, board.GetWidth() - current_piece.GetWidth());
    if (_IsCollided(current_piece, current_x, current_y)) current_x--;
  }

  void cBoard::PieceRotateCounterClockWise()
  {
    if (state != STATE_PLAYING) return;

    cPiece rotated = current_piece.GetRotatedCounterClockWise();
    if (_IsCollided(rotated, current_x, current_y)) {
      std::cout<<"cBoard::PieceRotateCounterClockWise Rotated piece would collide, returning"<<std::endl;
      return;
    }

    current_piece = rotated;

    game.OnPieceRotated(*this);
  }

  void cBoard::PieceRotateClockWise()
  {
    if (state != STATE_PLAYING) return;

    cPiece rotated = current_piece.GetRotatedClockWise();
    if (_IsCollided(rotated, current_x, current_y)) {
      std::cout<<"cBoard::PieceRotateClockWise Rotated piece would collide, returning"<<std::endl;
      return;
    }

    current_piece = rotated;

    game.OnPieceRotated(*this);
  }

  void cBoard::PieceDropOneRow(spitfire::durationms_t currentTime)
  {
    if (state != STATE_PLAYING) return;

    if ((int(current_y) - int(current_piece.GetHeight())) <= 0) {
      current_y = current_piece.GetHeight();
      _AddPieceToBoardCheckAndGenerate(currentTime);
      return;
    }

    current_y--;

    if (_IsCollided(current_piece, current_x, current_y)) {
      current_y++;
      _AddPieceToBoardCheckAndGenerate(currentTime);
      return;
    }
  }

  void cBoard::PieceDropToGround(spitfire::durationms_t currentTime)
  {
    if (state != STATE_PLAYING) return;

    do {
      if ((int(current_y) - int(current_piece.GetHeight())) <= 0) {
        current_y = current_piece.GetHeight();
        _AddPieceToBoardCheckAndGenerate(currentTime);
        return;
      }

      current_y--;

      if (_IsCollided(current_piece, current_x, current_y)) {
        current_y++;
        _AddPieceToBoardCheckAndGenerate(currentTime);
        return;
      }
    } while (true);
  }
}

