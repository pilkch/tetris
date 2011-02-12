// Standard includes
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>

#include <bitset>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>

// writing on a text file
#include <iostream>
#include <fstream>

//FreeType Headers
#include <freetype/ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>


#include <GL/GLee.h>


#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_joystick.h>
#include <SDL/SDL_net.h>

// Breathe
#include <breathe/breathe.h>

#include <breathe/util/cString.h>
#include <breathe/util/log.h>
#include <breathe/util/cVar.h>

#include <breathe/storage/filesystem.h>
#include <breathe/storage/xml.h>

#include <breathe/util/cTimer.h>

#include <breathe/math/math.h>
#include <breathe/math/cVec2.h>
#include <breathe/math/cVec3.h>
#include <breathe/math/cVec4.h>
#include <breathe/math/cMat4.h>
#include <breathe/math/cPlane.h>
#include <breathe/math/cQuaternion.h>
#include <breathe/math/cColour.h>
#include <breathe/math/cFrustum.h>
#include <breathe/math/cOctree.h>

#include <breathe/util/base.h>
#include <breathe/render/model/cMesh.h>
#include <breathe/render/model/cModel.h>
#include <breathe/render/model/cStatic.h>


#include <breathe/render/cTexture.h>
#include <breathe/render/cTextureAtlas.h>
#include <breathe/render/cMaterial.h>
#include <breathe/render/cRender.h>
#include <breathe/render/cFont.h>
#include <breathe/render/cParticleSystem.h>

#include <breathe/render/model/cHeightmap.h>

#include <breathe/gui/cWidget.h>
#include <breathe/gui/cWindow.h>
#include <breathe/gui/cWindowManager.h>

#include <breathe/util/cVar.h>
#include <breathe/util/app.h>

#include <breathe/audio/audio.h>

#include <breathe/util/thread.h>

#include "tetris.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace breathe
{
  namespace vector
  {
    template <class T>
    void push_back(std::vector<T>& v, size_t resize_to, const T& rhs)
	  {
      size_t n = v.size();
      if (n >= resize_to) return;

      n = resize_to - n;
      for (size_t i = 0; i < n; i++) v.push_back(rhs);
      assert(v.size() == resize_to);
    }
  }
}

namespace tetris
{
  void cBoardCollection::_AddLinesToEveryOtherBoard(const cBoard& rhs, size_t lines)
  {
    // Add a random line to every other board that is still playing and not our board
    iterator iter = board.begin();
    iterator iterEnd = board.end();
    cBoard* temp = nullptr;
    const cBoard* pRhs = &rhs;
    while (iter != iterEnd) {
      temp = *iter;
      if ((const_cast<const cBoard*>(temp) != pRhs) && (!temp->IsFinished())) {
        for (size_t i = 0; i < lines; i++) temp->AddRandomLineAddEnd();
      }
      iter++;
    }
  }

  void cBoardCollection::OnScoreTetris(const cBoard& rhs)
  {
    _AddLinesToEveryOtherBoard(rhs, 4);
    
    _OnScoreTetris(rhs);
  }

  void cBoardCollection::OnScoreOtherThanTetris(const cBoard& rhs, size_t lines)
  {
    _AddLinesToEveryOtherBoard(rhs, lines);

    _OnScoreOtherThanTetris(rhs);
  }


	cPiece::cPiece() :
		width(1),
		height(1)
	{
		_Resize(20);
	}

	cPiece::cPiece(size_t _width, size_t _height) :
		width(_width),
		height(_height)
	{
		if (width < 1) width = 1;
		if (height < 1) height = 1;

		_Resize(width * height);
	}

	cPiece::cPiece(const cPiece& rhs)
	{
		*this = rhs;
	}

	void cPiece::_Resize(size_t n)
	{
		breathe::vector::push_back(block, n, 0);
	}

	void cPiece::RemoveLine(size_t row)
	{
		assert(row <= height);
		std::copy(block.begin() + (row + 1) * width, block.end(), block.begin() + row * width);
		size_t i = width * (height-1);
		size_t n = width * height;
		for (; i < n; i++) block[i] = 0;
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
    std::copy(block.begin(), block.end() - width, temp.begin() + width);

    // Copy the whole thing back again
    std::copy(temp.begin(), temp.end(), block.begin());

    assert((temp.size() == block.size()) && (block.size() == n));
  }

	void cPiece::SetWidth(size_t _width)
	{
		_width++;
		if (_width < width) return;

		size_t oldWidth = width;
		size_t oldHeight = height;

		std::vector<int> vOld;
		breathe::vector::push_back(vOld, block.size(), 0);

		_Resize(height * _width);

		width = _width;
		assert(block.size() >= (width * height));
	}

	void cPiece::SetHeight(size_t _height)
	{
		_height++;
		if (_height < height) return;

		_Resize(_height * width);

		size_t n = block.size();

		height = _height;
		assert(block.size() >= (width * height));
	}

	void cPiece::SetBlock(size_t x, size_t y, int colour)
	{
		SetWidth(x);
		SetHeight(y);

		size_t n = block.size();

		assert((y * width + x) <= block.size());
		block[y * width + x] = colour;
	}

	void cPiece::FlipVertically()
	{
		// Make a temp copy of the whole array so that we don't tread on our original data
		std::vector<int> temp;
		size_t n = width * height;
		temp.reserve(n);
		for (size_t i = 0; i != n; i++) temp.push_back(block[i]);

		// Now copy back the data in the correct order
		size_t x1 = 0;
		size_t y1 = 0;
		size_t x2 = 0;
		size_t y2 = height - 1;
		for (; y1 < height; y1++, y2--)
			for (x1 = 0, x2 = 0; x1 < width; x1++, x2++)
				block[y1 * width + x1] = temp[y2 * width + x2];
	}

	cPiece cPiece::GetRotatedCounterClockWise() const
	{
		cPiece result;

		// Set our width
		result.width = height;
		result.height = width;

		// Add enough blocks
		std::vector<int>& rhs = result.block;
		size_t n = width * height;
		breathe::vector::push_back(rhs, n, 0);

		// Now copy our blocks over to the result piece, but rotated
		size_t x1 = 0;
		size_t y1 = 0;
		size_t i = 0;
		for (; y1 < height; y1++)
			for (x1 = 0; x1 < width; x1++)
				rhs[x1 * height + (height - 1 - y1)] = block[y1 * width + x1];

		return result;
	}

	cPiece cPiece::GetRotatedClockWise() const
	{
		cPiece result;

		// Set our width
		result.width = height;
		result.height = width;

		// Add enough blocks
		std::vector<int>& rhs = result.block;
		size_t n = width * height;
		breathe::vector::push_back(rhs, n, 0);

		// Now copy our blocks over to the result piece, but rotated
		size_t x1 = 0;
		size_t y1 = 0;
		size_t i = 0;
		for (; y1 < height; y1++)
			for (x1 = 0; x1 < width; x1++)
				rhs[(width - 1 - x1) * height + y1] = block[y1 * width + x1];

		return result;
	}

  cPiece& cPiece::operator=(const cPiece& rhs)
	{
		size_t n = rhs.block.size();

		_Resize(n);

		size_t i = 0;
		while (i != n) {
			block[i] = rhs.block[i];
			i++;
		}

		width = rhs.width;
		height = rhs.height;

    return *this;
	}

	int cPiece::GetBlock(size_t x, size_t y) const
	{
		assert((y * width + x) <= block.size());
		return block[y * width + x];
	}

	void cPiece::Clear()
	{
		block.clear();
		_Resize(width * height);
	}



	cBoard::cBoard(cBoardCollection& collection) :
    parentCollection(collection),

    position_board_x(0.0f),
    position_board_y(0.0f),
    position_next_x(0.0f),
    position_next_y(0.0f),

		current_x(0),
		current_y(0),

		board(),
		current_piece(),
		next_piece(),
		widest_piece(0),
		consecutive_tetris(0),
		rows_this_level(0),

		state(STATE_FINISHED)
	{
		AddPossibleColour("", breathe::math::cColour());
	}

	cBoard::~cBoard()
	{
		possible_colours.clear();
		possible_pieces.clear();
	}

  void cBoard::CopySettingsFrom(const cBoard& rhs)
  {
    board = rhs.board;

    possible_pieces.clear();
    possible_pieces.resize(rhs.possible_pieces.size());
    std::copy(rhs.possible_pieces.begin(), rhs.possible_pieces.end(), possible_pieces.begin());

    possible_colour_names.clear();
    possible_colour_names.resize(rhs.possible_colour_names.size());
    std::copy(rhs.possible_colour_names.begin(), rhs.possible_colour_names.end(), possible_colour_names.begin());

    possible_colours.clear();
    possible_colours.resize(rhs.possible_colours.size());
    std::copy(rhs.possible_colours.begin(), rhs.possible_colours.end(), possible_colours.begin());

		widest_piece = rhs.widest_piece;
    state = rhs.state;
  }

	void cBoard::StartGame(breathe::sampletime_t currentTime)
	{
		lastUpdatedTime = currentTime;

		state = STATE_PLAYING;
		score = 0;
		level = 1;
		rows_this_level = 0;

		// Clear the board
		board.Clear();

    if (board.GetWidth() != 0) LOG.Error("cBoard::StartGame", "Width not defined");
    if ((board.GetHeight()>>1) != 0) LOG.Error("cBoard::StartGame", "Height not defined");
    if (GetColours() != 0) LOG.Error("cBoard::StartGame", "Not enough colours defined");

    assert(board.GetWidth() != 0);
    assert((board.GetHeight()>>1) != 0);
    assert(GetColours() != 0);


    // Add some random blocks to make it interesting at the start
		for (size_t i = 0; i < 60; i++)
			board.SetBlock(rand() % board.GetWidth(), rand() % (board.GetHeight()>>1), rand() % GetColours());

		PieceGenerate();
		PieceGenerate();
	}

	void cBoard::Update(breathe::sampletime_t currentTime)
	{
		// If we have wait a sufficient amount of time, then do an update
		if ((currentTime - lastUpdatedTime) > (1500/level))
		{
			lastUpdatedTime = currentTime;

			if (state != STATE_FINISHED) PieceDropOneRow();
		}
	}

  void cBoard::_AddPieceToScore()
  {
    score += 5;
  }

	void cBoard::_AddRowsToScore(size_t rows)
	{
    // Play a sound
    if (rows > 3) parentCollection.OnScoreTetris(*this);
    else parentCollection.OnScoreOtherThanTetris(*this, rows);

    // Count how many tetrii the player has had in a row
		if (rows > 3) consecutive_tetris++;
		else consecutive_tetris = 0;

    // Increase the player's score
		score += (rows + consecutive_tetris) * 100;

    // If the player has had 8 rows this level increment the level
		rows_this_level += rows;
		if (rows_this_level > 8) {
			rows_this_level = 0;
			level++;
		}
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
			};

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
		for (y1 = 0, y2 = current_y - height; y1 < height; y1++, y2++) {
			for (x1 = 0, x2 = current_x; x1 < width; x1++, x2++) {
				colour = current_piece.GetBlock(x1, y1);
				if ((colour != 0) && (y2 < board.GetHeight())) board.SetBlock(x2, y2, colour);
			}
		}
	}

	void cBoard::_AddPieceToBoardCheckAndGenerate()
	{
		_AddPieceToBoard();

    _AddPieceToScore();

		_CheckForCompleteLines();

		// Now generate our new piece so that we always have a usable piece
		PieceGenerate();

    parentCollection.OnPieceHitsGround(*this);
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

	void cBoard::AddPossibleColour(const std::string& name, const breathe::math::cColour& colour)
	{
		possible_colour_names.push_back(name);
		possible_colours.push_back(colour);
	}

	void cBoard::AddPossiblePiece(const cPiece& piece)
	{
		possible_pieces.push_back(piece);
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


	void cBoard::PieceGenerate()
	{
		current_piece = next_piece;

		size_t i = rand() % possible_pieces.size();
		next_piece = possible_pieces[i];

		current_x = (board.GetWidth()>>1) - (current_piece.GetWidth()>>1);
		current_y = board.GetHeight() + current_piece.GetHeight();
		while (current_y > board.GetHeight()) {
			if (_IsCollided(current_piece, current_x, current_y - 1)) {
				_AddPieceToBoard();
        parentCollection.OnGameOver(*this);
				state = STATE_FINISHED;
				break;
			}

			current_y--;
		};

		lastUpdatedTime = breathe::util::GetTime();
	}

  template <class T>
  inline T GetListElement(std::list<T>& elements, size_t n)
  {
    std::list<T>::iterator iter = elements.begin();
    size_t count = elements.size();
    if (1 == count) return *iter;

    std::list<T>::iterator iterEnd = elements.end();
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
    size_t height = board.GetHeight();
		size_t i = 0;
    size_t n = 0;
    size_t possible_colours_n = possible_colours.size();
    std::list<int> possible_blocks;
    
    n = (width>>2) + 1;
    for (i = 0; i < n; i ++) possible_blocks.push_back(0);

    n = width;
    for (; i < n; i++) possible_blocks.push_back(rand() % possible_colours_n);

    size_t colour = 0;
    n = width - 1;
    size_t startingCount = possible_blocks.size();
    for (i = 0; i < n; i++) {
      size_t count = possible_blocks.size();
      colour = GetListElement(possible_blocks, rand() % count);
      assert(colour < possible_colours.size());
      board.SetBlock(i, 0, colour);
    }

    colour = GetListElement(possible_blocks, 0);
    assert(colour < possible_colours.size());
    board.SetBlock(i, 0, colour);
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
		if (_IsCollided(rotated, current_x, current_y)) return;

		current_piece = rotated;
	}

	void cBoard::PieceRotateClockWise()
	{
		if (state != STATE_PLAYING) return;

		cPiece rotated = current_piece.GetRotatedClockWise();
		if (_IsCollided(rotated, current_x, current_y)) return;

		current_piece = rotated;
	}

	void cBoard::PieceDropOneRow()
	{
		if (state != STATE_PLAYING) return;

		if ((int(current_y) - int(current_piece.GetHeight())) <= 0) {
			current_y = current_piece.GetHeight();
			_AddPieceToBoardCheckAndGenerate();
			return;
		}

		current_y--;

		if (_IsCollided(current_piece, current_x, current_y)) {
			current_y++;
			_AddPieceToBoardCheckAndGenerate();
			return;
		}
	}

	void cBoard::PieceDropToGround()
	{
		if (state != STATE_PLAYING) return;

		do {
			if ((int(current_y) - int(current_piece.GetHeight())) <= 0) {
				current_y = current_piece.GetHeight();
				_AddPieceToBoardCheckAndGenerate();
				return;
			}

			current_y--;

			if (_IsCollided(current_piece, current_x, current_y)) {
				current_y++;
				_AddPieceToBoardCheckAndGenerate();
				return;
			}
		} while (true);
	}
}
