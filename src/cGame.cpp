// Standard includes
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
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
#include <algorithm>

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
#include <breathe/util/lang.h>

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

#include <breathe/game/cPlayer.h>
#include <breathe/game/cPetrolBowser.h>

#include <breathe/util/cVar.h>
#include <breathe/util/app.h>

#include <breathe/audio/audio.h>

#include <breathe/util/thread.h>

#include <breathe/communication/network.h>

#include "tetris.h"
#include "cGame.h"

breathe::render::material::cMaterial* pMaterialAlphaBlend = nullptr;
breathe::render::cTexture* pTextureBlock = nullptr;
breathe::render::cParticleSystemBillboard* pFire = nullptr;

enum TETRIS_STATES {
  STATE_GAME_QUIT = 2,
  STATE_GAME_RESTART,
};

cGame::cGame(int argc, char** argv) :
	cApp(argc, argv)
{
  LOG.Newline("cGame");
  CONSOLE<<"cGame::cGame"<<std::endl;

	// Set up keys
	AddKeyNoRepeat(SDLK_r); // Reset
	AddKeyNoRepeat(SDLK_p); // Pause

	// Player 0
	AddKeyNoRepeat(SDLK_q);
	AddKeyNoRepeat(SDLK_e);
	AddKeyNoRepeat(SDLK_w);
	AddKeyNoRepeat(SDLK_UP);
	AddKeyNoRepeat(SDLK_SPACE);
	AddKeyNoRepeat(SDLK_z);

	AddKeyRepeat(SDLK_a);
	AddKeyRepeat(SDLK_s);
	AddKeyRepeat(SDLK_d);
  AddKeyNoRepeat(SDLK_UP);
	AddKeyRepeat(SDLK_DOWN);
	AddKeyRepeat(SDLK_LEFT);
	AddKeyRepeat(SDLK_RIGHT);

	// Player 1
  AddKeyNoRepeat(SDLK_KP0);
	AddKeyNoRepeat(SDLK_KP7);
	AddKeyNoRepeat(SDLK_KP8);
	AddKeyNoRepeat(SDLK_KP9);
	AddKeyRepeat(SDLK_KP4);
	AddKeyRepeat(SDLK_KP5);
	AddKeyRepeat(SDLK_KP6);
	AddKeyRepeat(SDLK_KP2);


#ifdef BUILD_DEBUG
	AddKeyNoRepeat(SDLK_y);
	AddKeyNoRepeat(SDLK_F6);
#endif

  AddPlayer();
  AddPlayer();
  CONSOLE<<"cGame::cGame returning"<<std::endl;
}

cGame::~cGame()
{
  while (!board.empty()) RemovePlayer(0);

	breathe::SAFE_DELETE(pTextureBlock);
	breathe::SAFE_DELETE(pFire);
}

void cGame::SetPlayers(size_t players)
{
  while (board.size() < players) AddPlayer();

  while (board.size() > players) RemovePlayer(board.size() - 1);
}

void cGame::AddPlayer()
{
  board.push_back(new tetris::cBoard(*this));
}

void cGame::RemovePlayer(size_t i)
{
  assert(i < board.size());
  board_iterator iter = board.begin();
  board_iterator iterEnd = board.end();
  size_t j = 0;
  while (iter != iterEnd) {
    if (j == i) {
      board.remove(*iter);
      return;
    }

    iter++;
    j++;
  }
}

bool cGame::DestroyScene()
{
	return breathe::GOOD;
}

void cGame::FullscreenSwitch()
{

}

void play(const breathe::string_t& sFilename)
{
	breathe::audio::cBuffer* pBuffer = breathe::audio::CreateBuffer(sFilename);
	assert(pBuffer);

	breathe::audio::cSource* pSource = breathe::audio::CreateSourceAttachedToScreen(pBuffer);
	assert(pSource);

	pSource->Play();
}

bool cGame::LoadScene()
{
	pRender->SetAtlasWidth(64, 256, pRender->iMaxTextureSize);

	pRender->AddTextureNotFoundTexture("texturenotfound.png");
	pRender->AddMaterialNotFoundMaterial("materialnotfound.png");

	pRender->BeginLoadingTextures();

#ifdef BUILD_DEBUG
	LOG.Newline();

	pRender->AddMaterial("cloud_billboard.mat");
#endif // BUILD_DEBUG

	pTextureBlock = pRender->AddTexture("block.png");

	LOG.Success("Game", "Init Fonts");
  pFont = new breathe::render::cFont(breathe::filesystem::FindFile(TEXT("fonts/pricedown.ttf")), 32);

	pFire = new breathe::render::cParticleSystemBillboard(100);
	pFire->SetMaterial(pRender->GetMaterial("common/materials/cloud_billboard.mat"));


	pMaterialAlphaBlend = pRender->AddMaterial("alphablend.mat");

	pRender->EndLoadingTextures();

	pRender->TransformModels();

	LOG.Success("Game", "LoadScene returning");
	return breathe::GOOD;
}


void cGame::LoadPiece(breathe::xml::cNode::iterator iter)
{
	tetris::cPiece piece;

	size_t x = 0;
	size_t y = 0;

	iter.FindChild("row");

	while (iter) {
		if ("row" == iter.GetName()) {
			breathe::xml::cNode::iterator iterBlock = iter;
			iterBlock.FindChild("block");

			x = 0;
			while (iterBlock) {
				std::string colour;
				iterBlock.GetAttribute("colour", colour);

				int iColour = board.front()->GetColourFromName(colour);

				piece.SetBlock(x, y, iColour);

				iterBlock++;
				x++;
			}

			y++;
		}

		iter++;
	}

	size_t iPieceWidth = piece.GetWidth();
	size_t iPieceHeight = piece.GetHeight();

	// Flip it because we have been putting in the y coordinate upside down, now that we know the height
	// we can flip it to be the correct way up
	piece.FlipVertically();

	board.front()->AddPossiblePiece(piece);
}

void cGame::LoadConfiguration()
{
	breathe::xml::cNode root("tetris.xml");

	breathe::xml::cNode::iterator iter(root);
	if (!iter) return;

	iter.FindChild("tetris");
	if (!iter) return;

	iter.FindChild("board");
	if (iter) {
		int width = 0;
		int height = 0;

		iter.GetAttribute("width", width);
		iter.GetAttribute("height", height);

		if (width != 0) board.front()->SetWidth(width);
		if (height != 0) board.front()->SetHeight(height);
	}

	iter = root;
	if (!iter) return;

	iter.FindChild("tetris");
	if (!iter) return;

	iter.FirstChild();

	while(iter)
	{
		if ("colour" == iter.GetName())
		{
			std::string name("UNKNOWN");
			iter.GetAttribute("name", name);

			breathe::math::cColour colour(1.0f, 1.0f, 1.0f, 1.0f);
			iter.GetAttribute("value", colour);

			board.front()->AddPossibleColour(name, colour);
		} else if ("piece" == iter.GetName()) {
			LoadPiece(iter);
		}

		iter++;
	};
}

bool cGame::InitScene()
{
	LOG.Success("Game", "InitScene");

	LoadConfiguration();

  StartGame();

  PushState(new cTetrisStatePlaying(*this));

	SDL_ShowCursor(SDL_DISABLE);
	SDL_WarpMouse(pRender->uiWidth>>1, pRender->uiHeight>>1);

	return breathe::GOOD;
}

void cGame::StartGame()
{
  const size_t n = board.size();

  // Copy the settings from the first board to each of the other boards
  board_iterator iter = board.begin();
  board_iterator iterEnd = board.end();

  tetris::cBoard& board_front = *(*iter);

  iter++;

  while (iter != iterEnd) {
    (*iter)->CopySettingsFrom(board_front);
    iter++;
  }

  // Finally start all the boards
  iter = board.begin();
  while (iter != iterEnd) {
    (*iter)->StartGame(breathe::util::GetTime());
    iter++;
  }
}

bool cGame::Execute(const std::string& sCommand)
{
	if ("reset" == sCommand)
	{
		// Reset
		return breathe::GOOD;
	}

	return breathe::BAD;
}



// *** From tetris::cBoardCollection

void cGame::_OnScoreTetris(const tetris::cBoard& rhs)
{
  //_OnScoreOtherThanTetris(rhs);
	play(breathe::filesystem::FindFile(TEXT("audio/score_tetris.wav")));
}

void cGame::_OnScoreOtherThanTetris(const tetris::cBoard& rhs)
{
	play(breathe::filesystem::FindFile(TEXT("audio/score_other.wav")));
}

void cGame::_OnPieceHitsGround(const tetris::cBoard& rhs)
{
	play(breathe::filesystem::FindFile(TEXT("audio/piece_hits_ground.wav")));
}

void cGame::_OnGameOver(const tetris::cBoard& rhs)
{
	play(breathe::filesystem::FindFile(TEXT("audio/game_over.wav")));
}


// *** cTetrisStatePlaying

const float cGame::cTetrisStatePlaying::block_w = 0.02f;
const float cGame::cTetrisStatePlaying::block_h = 0.02f;

void cGame::cTetrisStatePlaying::_OnResume(int iResult)
{
  switch (iResult) {
    case STATE_GAME_QUIT:
      game.PopStateSoon();
      break;

    case STATE_GAME_RESTART:
      game.StartGame();
      break;
  }
}

void cGame::cTetrisStatePlaying::_Update(breathe::sampletime_t currentTime)
{
  std::list<tetris::cBoard*>& board = game.board;

  size_t n = board.size();
  board_iterator iter = board.begin();
  switch (n)
  {
  case 2:
    (*iter)->SetPositionBoardX(0.1f);
    (*iter)->SetPositionBoardY(0.1f);
    (*iter)->SetPositionNextX(0.4f);
    (*iter)->SetPositionNextY(0.1f);
    iter++;
    (*iter)->SetPositionBoardX(0.65f);
    (*iter)->SetPositionBoardY(0.1f);
    (*iter)->SetPositionNextX(0.4f);
    (*iter)->SetPositionNextY(0.6f);
    break;
  default:
    (*iter)->SetPositionBoardX(0.3f);
    (*iter)->SetPositionBoardY(0.1f);
    (*iter)->SetPositionNextX(0.7f);
    (*iter)->SetPositionNextY(0.4f);
  };

  bool bAllFinished = true;
  iter = board.begin();
  board_iterator iterEnd = board.end();
  while (iter != iterEnd) {
    (*iter)->Update(currentTime);
    if (!(*iter)->IsFinished()) bAllFinished = false;
    iter++;
  }

	// Particle Systems
	pFire->position.Set(50.0f, 50.0f, 4.0f);

	pFire->Update(currentTime);

	// If we are in the console blink the cursor
	if (CONSOLE.IsVisible())
	{
		CONSOLE.uiCursorBlink++;

		if (CONSOLE.uiCursorBlink>50)
			CONSOLE.uiCursorBlink=0;
	}

  if (bAllFinished) game.PushStateSoon(new cTetrisStateFinished(game));
}

void cGame::cTetrisStatePlaying::_UpdateInput(breathe::sampletime_t currentTime)
{
	// *********************************************************************************************
	// Handle Mouse Input
	// *********************************************************************************************
  float x = (game.GetMouseX() - pRender->uiWidth*0.5f);
  float y = (game.GetMouseY() - pRender->uiHeight*0.5f);

	// *********************************************************************************************
	// Common Input
	// *********************************************************************************************

  if (game.IsKeyDown(SDLK_p)) {
    game.PushStateSoon(new cTetrisStatePaused(game));
    return;
  }

	if (game.IsKeyDown(SDLK_ESCAPE)) {
		LOG.Success("Update", "Escape Pressed: Quiting");
    game.PopStateSoon();
    return;
	}

  // Shared Keys
	if (game.IsKeyDown(SDLK_r)) game.StartGame();

	if (game.IsKeyDown(SDLK_y)) {
		play(breathe::filesystem::FindFile(TEXT("audio/weather/rain_loopable.wav")));
		play(breathe::filesystem::FindFile(TEXT("audio/weather/wind_loopable.wav")));
		play(breathe::filesystem::FindFile(TEXT("audio/weather/thunder.wav")));

		play(breathe::filesystem::FindFile(TEXT("audio/farm/hen_cluck.wav")));
		play(breathe::filesystem::FindFile(TEXT("audio/farm/horse_neigh.wav")));
		play(breathe::filesystem::FindFile(TEXT("audio/farm/lamb_baa.wav")));
	}


	// *********************************************************************************************
	// Player Input
	// *********************************************************************************************

  std::list<tetris::cBoard*>& board = game.board;
  board_iterator iter = board.begin();

  size_t n = board.size();

  if (1 == n) {
    // Player 0
		if (game.IsKeyDown(SDLK_a) || game.IsKeyDown(SDLK_LEFT)) (*iter)->PieceMoveLeft();
		if (game.IsKeyDown(SDLK_d) || game.IsKeyDown(SDLK_RIGHT)) (*iter)->PieceMoveRight();

		if (game.IsKeyDown(SDLK_q)) (*iter)->PieceRotateCounterClockWise();
		if (game.IsKeyDown(SDLK_e) || game.IsKeyDown(SDLK_w) || game.IsKeyDown(SDLK_UP)) (*iter)->PieceRotateClockWise();
		if (game.IsKeyDown(SDLK_s) || game.IsKeyDown(SDLK_DOWN)) (*iter)->PieceDropOneRow();

		if (game.IsKeyDown(SDLK_z) || game.IsKeyDown(SDLK_SPACE)) (*iter)->PieceDropToGround();
  } else {
    // Player 0
		if (game.IsKeyDown(SDLK_a)) (*iter)->PieceMoveLeft();
		if (game.IsKeyDown(SDLK_d)) (*iter)->PieceMoveRight();

		if (game.IsKeyDown(SDLK_q)) (*iter)->PieceRotateCounterClockWise();
		if (game.IsKeyDown(SDLK_e) || game.IsKeyDown(SDLK_w)) (*iter)->PieceRotateClockWise();
		if (game.IsKeyDown(SDLK_s)) (*iter)->PieceDropOneRow();

		if (game.IsKeyDown(SDLK_z)) (*iter)->PieceDropToGround();

    iter++;

    // Player 1
		if (game.IsKeyDown(SDLK_KP4) || game.IsKeyDown(SDLK_LEFT)) (*iter)->PieceMoveLeft();
		if (game.IsKeyDown(SDLK_KP6) || game.IsKeyDown(SDLK_RIGHT)) (*iter)->PieceMoveRight();

		if (game.IsKeyDown(SDLK_KP7)) (*iter)->PieceRotateCounterClockWise();
		if (game.IsKeyDown(SDLK_KP8) || game.IsKeyDown(SDLK_KP9) || game.IsKeyDown(SDLK_UP)) (*iter)->PieceRotateClockWise();
		if (game.IsKeyDown(SDLK_KP5) || game.IsKeyDown(SDLK_KP2) || game.IsKeyDown(SDLK_DOWN)) (*iter)->PieceDropOneRow();

		if (game.IsKeyDown(SDLK_KP0)) (*iter)->PieceDropToGround();
  }


	std::vector<SDL_Joystick*>& vJoystick = game.GetJoysticks();

	if (vJoystick.size())
	{
		SDL_JoystickUpdate();

		//Sint16 value;
		const Sint16 iMin = 1000;
		float fMax = 31768.0f; // 1000 taken off to tolerance

		unsigned int n = vJoystick.size();
		for (unsigned int j = 0; j < n; j++)
		{
			/*
			0 = left analog		left right
			1 = left analog		up down
			2 = right analog	left right
			3 = right analog	up down
			*/

			/*value = SDL_JoystickGetAxis (vJoystick[j], 0);
			if (value > iMin)
				pPlayer->fInputRight = (value - iMin) / fMax;
			else if (value < -iMin)
				pPlayer->fInputLeft = -(value + iMin) / fMax;

			value = SDL_JoystickGetAxis (vJoystick[j], 3);
			if (value > iMin)
				pPlayer->fInputDown = (value - iMin) / fMax;
			else if (value < -iMin)
				pPlayer->fInputUp = -(value + iMin) / fMax;*/
		}
	}
}

void cGame::cTetrisStatePlaying::_RenderScreenSpace(breathe::sampletime_t currentTime)
{
  std::list<tetris::cBoard*>& board = game.board;
  breathe::render::cFont* pFont = game.GetFont();

	glPushMatrix();
		glLoadIdentity();

    board_iterator iter = board.begin();
    board_iterator iterEnd = board.end();
    while (iter != iterEnd) {
      tetris::cBoard& board = *(*iter);
      float board_x = board.GetPositionBoardX();
      float board_y = board.GetPositionBoardY();
      float next_x = board.GetPositionNextX();
      float next_y = board.GetPositionNextY();

      // Draw black background
		  pRender->ClearMaterial();
		  pRender->SetColour(0.0f, 0.0f, 0.0f);
		  pRender->RenderScreenSpaceRectangle(board_x, board_y,
        block_w * board.GetWidth(), block_h * board.GetHeight());
		  pRender->RenderScreenSpaceRectangle(next_x, next_y,
			  block_w * board.GetWidestPiece(), block_h * board.GetWidestPiece());

		  // Draw blocks
		  pRender->SetMaterial(pMaterialAlphaBlend);
		  pRender->SetTexture0(pTextureBlock);
		  RenderTetrisPiece(board, board.GetBoard(), board_x, board_y);
		  RenderTetrisPiece(board, board.GetNextPiece(), next_x, next_y);

		  if (board.GetCurrentPieceY() <= board.GetHeight())
			  RenderTetrisPiece(board, board.GetCurrentPiece(),
				  board_x + block_w * float(board.GetCurrentPieceX()),
				  board_y + block_h * (float(board.GetHeight()) - float(board.GetCurrentPieceY())));

      iter++;
    }
	glPopMatrix();

	glPushMatrix();
		glLoadIdentity();
		pRender->SetColour(1.0f, 0.0f, 0.0f);

    iter = board.begin();
    while (iter != iterEnd) {
      tetris::cBoard& board = *(*iter);
      float x = board.GetPositionNextX();
      float y = board.GetPositionNextY() + 0.2f;

		  pFont->printf(x, y, "Score: %d", board.GetScore());
		  pFont->printf(x, y + 0.05f, "Level: %d", board.GetLevel());
      iter++;
    }
	glPopMatrix();
}

void cGame::cTetrisStatePlaying::RenderTetrisPiece(const tetris::cBoard& board, const tetris::cPiece& piece, float x, float y)
{
	if ((0 == piece.GetWidth()) || (0 == piece.GetHeight())) return;

	size_t iX = 0;
	size_t iY = 0;
	size_t iPieceWidth = piece.GetWidth();
	size_t iPieceHeight = piece.GetHeight();
	int current = 0;
	int last = -1;
	float piece_w = x + block_w * float(iPieceWidth);
	float piece_h = y + block_h * float(iPieceHeight);
	float piece_x = x;
	float piece_y = y;

	pRender->SetColour(board.GetColour(0));

	for (;y < piece_h, iY < iPieceHeight;y+=block_h, iY++)
	{
		for (x=piece_x, iX = 0;x < piece_w, iX < iPieceWidth;x+=block_w, iX++)
		{
			current = piece.GetBlock(iX, iPieceHeight - iY - 1);
			if (current == 0) continue;

			if (current != last) {
				pRender->SetColour(board.GetColour(current));
				last = current;
			}

			pRender->RenderScreenSpaceRectangle(x, y, block_w, block_h);
		}
	}
}

// *** cTetrisStatePaused

void cGame::cTetrisStatePaused::_UpdateInput(breathe::sampletime_t currentTime)
{
  if (game.IsKeyDown(SDLK_p) || game.IsKeyDown(SDLK_ESCAPE))
	{
		game.PopStateSoon();
    return;
	}
}

void cGame::cTetrisStatePaused::_RenderScreenSpace(breathe::sampletime_t currentTime)
{
  GetParent()->RenderScreenSpace(currentTime);

  breathe::render::cFont* pFont = game.GetFont();
	glPushMatrix();
		glLoadIdentity();
		pRender->SetColour(1.0f, 0.0f, 0.0f);

    float x = 0.5f;
    float y = 0.5f;

    pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_Paused").c_str());
    pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_Paused_Instructions").c_str());
	glPopMatrix();
}


// *** cTetrisStateFinished

void cGame::cTetrisStateFinished::_UpdateInput(breathe::sampletime_t currentTime)
{
  if (game.IsKeyDown(SDLK_r)) {
    SetResult(STATE_GAME_RESTART);
    game.PopStateSoon();
    return;
	}

  if (game.IsKeyDown(SDLK_ESCAPE)) {
    SetResult(STATE_GAME_QUIT);
		game.PopStateSoon();
  }
}

void cGame::cTetrisStateFinished::_RenderScreenSpace(breathe::sampletime_t currentTime)
{
  GetParent()->RenderScreenSpace(currentTime);

  breathe::render::cFont* pFont = game.GetFont();
	glPushMatrix();
		glLoadIdentity();
		pRender->SetColour(1.0f, 0.0f, 0.0f);

    float x = 0.5f;
    float y = 0.5f;

    pFont->PrintCenteredHorizontally(x, y, 1.0f, breathe::LANG("L_GameOver").c_str());
    pFont->PrintCenteredHorizontally(x, y + 0.05f, 1.0f, breathe::LANG("L_GameOver_Instructions1").c_str());
    pFont->PrintCenteredHorizontally(x, y + 0.10f, 1.0f, breathe::LANG("L_GameOver_Instructions2").c_str());
	glPopMatrix();
}
