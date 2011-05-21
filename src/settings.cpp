// Standard headers
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

// Boost headers
#include <boost/shared_ptr.hpp>

// Spitfire headers
#include <spitfire/spitfire.h>

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>
#include <spitfire/math/cColour.h>

#include <spitfire/storage/filesystem.h>
#include <spitfire/storage/xml.h>

// Breathe headers
#include <breathe/audio/audio.h>

// libopenglmm headers
#include <libopenglmm/libopenglmm.h>
#include <libopenglmm/cContext.h>
#include <libopenglmm/cFont.h>
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Tetris headers
#include "settings.h"

void cSettings::Load()
{
  spitfire::xml::reader reader;

  const spitfire::string_t sFilename = spitfire::filesystem::GetThisApplicationSettingsDirectory() + TEXT("config.xml");
  if (!reader.ReadFromFile(document, sFilename)) {
    std::cout<<"cSettings::Load \""<<spitfire::string::ToUTF8(sFilename)<<"\" not found"<<std::endl;
    return;
  }
}

void cSettings::Save()
{
  spitfire::xml::writer writer;

  const spitfire::string_t sFilename = spitfire::filesystem::GetThisApplicationSettingsDirectory() + TEXT("config.xml");
  if (!writer.WriteToFile(document, sFilename)) {
    std::cout<<"cSettings::Save Error saving to file \""<<spitfire::string::ToUTF8(sFilename)<<"\""<<std::endl;
    return;
  }
}

size_t cSettings::GetNumberOfPlayers() const
{
  size_t nPlayers = 1;

  std::vector<cHighScoresTableEntry> entries;

  spitfire::document::cNode::const_iterator iterConfig(document);
  if (!iterConfig.IsValid()) return nPlayers;

  iterConfig.FindChild("config");
  if (!iterConfig.IsValid()) return nPlayers;

  {
    spitfire::document::cNode::const_iterator iter(iterConfig);

    iter.FindChild("players");
    if (iter.IsValid()) {
      spitfire::string_t sNumberOfPlayers;
      if (iter.GetAttribute("numberOfPlayers", sNumberOfPlayers)) {
        std::cout<<"cSettings::SetHighScores Number of players found "<<spitfire::string::ToUTF8(sNumberOfPlayers)<<std::endl;
        nPlayers = spitfire::string::ToUnsignedInt(sNumberOfPlayers);
      }
    }
  }

  return nPlayers;
}

void cSettings::SetNumberOfPlayers(size_t nPlayers)
{
  ...
}

spitfire::string_t cSettings::GetPlayerName(size_t i) const
{
  spitfire::ostringstream_t o;
  o<<"Player ";
  o<<i;
  spitfire::string_t sPlayerName = o.str();

  ...

  return sPlayerName;
}

void cSettings::SetPlayerName(size_t i, const spitfire::string_t& sName)
{
  ...
}

spitfire::math::cColour cSettings::GetPlayerColour(size_t i) const
{
  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);
  const spitfire::math::cColour blue(0.0f, 0.0f, 1.0f);
  const spitfire::math::cColour yellow(1.0f, 1.0f, 0.0f);
  const spitfire::math::cColour green(0.0f, 1.0f, 0.0f);
  const spitfire::math::cColour defaultColours[] = {
    red,
    blue,
    yellow,
    green
  };
  assert(i < 4);
  spitfire::math::cColour colour = defaultColours[i];

  ...

  return colour;
}

void cSettings::SetPlayerColour(size_t i, const spitfire::math::cColour& colour)
{
  ... find the node in document and set the colour
}

std::vector<cHighScoresTableEntry> cSettings::GetHighScores() const
{
  std::vector<cHighScoresTableEntry> entries;

  spitfire::document::cNode::const_iterator iterConfig(document);
  if (!iterConfig.IsValid()) return entries;

  iterConfig.FindChild("config");
  if (!iterConfig.IsValid()) return entries;

  // Load high scores
  {
    spitfire::document::cNode::const_iterator iter(iterConfig);

    iter.FindChild("highscores");
    if (iter.IsValid()) {
      iter.FindChild("entry");
      if (iter.IsValid()) {
        while (iter.IsValid()) {
          spitfire::string_t sName;
          if (iter.GetAttribute("name", sName)) {
            std::cout<<"cSettings::SetHighScores Adding High Score "<<spitfire::string::ToUTF8(sName)<<std::endl;
            cHighScoresTableEntry entry;

            entry.sName = spitfire::string::ToString_t(sName);
            iter.GetAttribute("score", entry.score);

            entries.push_back(entry);
          }

          iter.Next("entry");
        };
      }
    }
  }

  return entries;
}

void cSettings::SetHighScores(const std::vector<cHighScoresTableEntry>& entries)
{
  // Get or create the document
  spitfire::document::cNode::iterator iterConfig(document);
  if (!iterConfig.IsValid()) {
    spitfire::document::element* configElement = document.CreateElement("config");
    document.AppendChild(configElement);
    iterConfig = document;
    assert(iterConfig.IsValid());
  }

  // Get or create the config element
  iterConfig.FindChild("config");
  if (!iterConfig.IsValid()) {
    spitfire::document::element* configElement = document.CreateElement("config");
    document.AppendChild(configElement);
    iterConfig = document;
    assert(iterConfig.IsValid());
    iterConfig.FindChild("config");
    assert(iterConfig.IsValid());
  }

  // Get or create the highscores element
  spitfire::document::cNode::iterator iterHighscores(iterConfig);
  iterHighscores.FindChild("highscores");
  if (!iterHighscores.IsValid()) {
    spitfire::document::element* highscoresElement = document.CreateElement("highscores");
    spitfire::document::element* configElement = iterConfig.Get();
    configElement->AppendChild(highscoresElement);
    iterHighscores = iterConfig;
    assert(iterHighscores.IsValid());
    iterHighscores.FindChild("highscores");
    assert(iterHighscores.IsValid());
  }

  spitfire::document::element* highscoresElement = iterHighscores.Get();
  highscoresElement->Clear();

  // Entries
  const size_t n = entries.size();
  for (size_t i = 0; i < n; i++) {
    spitfire::document::element* entryElement = document.CreateElement("entry");
    highscoresElement->AppendChild(entryElement);

    entryElement->AddAttribute("name", entries[i].sName);
    const uint64_t score = entries[i].score;
    entryElement->AddAttribute("score", score);
  }
}
