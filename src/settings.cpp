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

#include <spitfire/util/log.h>

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
  spitfire::util::cProcessInterfaceVoid interface;
  spitfire::util::PROCESS_RESULT result = reader.ReadFromFile(interface, document, sFilename);
  if (result != spitfire::util::PROCESS_RESULT::COMPLETE) {
    LOG<<"cSettings::Load \""<<sFilename<<"\" not found"<<std::endl;
    return;
  }
}

void cSettings::Save()
{
  spitfire::xml::writer writer;

  const spitfire::string_t sDirectory = spitfire::filesystem::GetThisApplicationSettingsDirectory();
  LOG<<"cSettings::Save Creating directory \""<<sDirectory<<"\""<<std::endl;
  spitfire::filesystem::CreateDirectory(sDirectory);
  const spitfire::string_t sFilename = sDirectory + TEXT("config.xml");
  LOG<<"cSettings::Save Creating file \""<<sFilename<<"\""<<std::endl;
  if (!writer.WriteToFile(document, sFilename)) {
    LOG<<"cSettings::Save Error saving to file \""<<sFilename<<"\""<<std::endl;
    return;
  }
}

template <class T>
T cSettings::GetXMLValue(const spitfire::string_t& sSection, const spitfire::string_t& sItem, const spitfire::string_t& sAttribute, const T& valueDefault) const
{
  T value = valueDefault;

  spitfire::document::cNode::const_iterator iterConfig(document);
  if (!iterConfig.IsValid()) return value;

  {
    spitfire::document::cNode::const_iterator iter(iterConfig);

    iter.FindChild(spitfire::string::ToUTF8(sSection));
    if (iter.IsValid()) {
      iter.FindChild(spitfire::string::ToUTF8(sItem));
      if (iter.IsValid()) {
        iter.GetAttribute(spitfire::string::ToUTF8(sAttribute), value);
        //LOG<<"cSettings::GetXMLValue Item \""<<sItem<<"\" found "<<spitfire::string::ToString(value)<<std::endl;
      }
    }
  }

  return value;
}

template <class T>
void cSettings::SetXMLValue(const spitfire::string_t& sSection, const spitfire::string_t& sItem, const spitfire::string_t& sAttribute, const T& value)
{
  // Get or create the config element
  spitfire::document::cNode::iterator iterConfig(document);
  if (!iterConfig.IsValid()) {
    spitfire::document::element* configElement = document.CreateElement("config");
    document.AppendChild(configElement);
    iterConfig = document;
    assert(iterConfig.IsValid());
  }

  // Get or create the config element
  if (!iterConfig.IsValid()) {
    spitfire::document::element* configElement = document.CreateElement("config");
    document.AppendChild(configElement);
    iterConfig = document;
    assert(iterConfig.IsValid());
    iterConfig.FindChild("config");
    assert(iterConfig.IsValid());
  }

  // Get or create the section element
  spitfire::document::cNode::iterator iterSection(iterConfig);
  iterSection.FindChild(spitfire::string::ToUTF8(sSection));
  if (!iterSection.IsValid()) {
    spitfire::document::element* sectionElement = document.CreateElement(spitfire::string::ToUTF8(sSection));
    spitfire::document::element* configElement = iterConfig.Get();
    configElement->AppendChild(sectionElement);
    iterSection = iterConfig;
    assert(iterSection.IsValid());
    iterSection.FindChild(spitfire::string::ToUTF8(sSection));
    assert(iterSection.IsValid());
  }

  // Get or create the item element
  spitfire::document::cNode::iterator iterItem(iterSection);
  iterItem.FindChild(spitfire::string::ToUTF8(sItem));
  if (!iterItem.IsValid()) {
    spitfire::document::element* itemElement = document.CreateElement(spitfire::string::ToUTF8(sItem));
    spitfire::document::element* sectionElement = iterSection.Get();
    sectionElement->AppendChild(itemElement);
    iterItem = iterSection;
    assert(iterItem.IsValid());
    iterItem.FindChild(spitfire::string::ToUTF8(sItem));
    assert(iterItem.IsValid());
  }

  spitfire::document::element* itemElement = iterItem.Get();

  // Create and append the item element
  itemElement->SetAttribute(spitfire::string::ToUTF8(sAttribute), value);
}

size_t cSettings::GetNumberOfPlayers() const
{
  return GetXMLValue(TEXT("settings"), TEXT("numberOfPlayers"), TEXT("value"), 1);
}

void cSettings::SetNumberOfPlayers(size_t nPlayers)
{
  SetXMLValue(TEXT("settings"), TEXT("numberOfPlayers"), TEXT("value"), nPlayers);
}

spitfire::string_t cSettings::GetPlayerName(size_t i) const
{
  spitfire::ostringstream_t o;
  o<<"player";
  o<<i;
  const spitfire::string_t sItem = o.str();

  // Clear the stream
  o.clear();
  o.str(TEXT(""));
  o<<"Player ";
  o<<i;
  const spitfire::string_t sDefaultValue = o.str();

  return GetXMLValue(TEXT("players"), sItem, TEXT("name"), sDefaultValue);
}

void cSettings::SetPlayerName(size_t i, const spitfire::string_t& sName)
{
  spitfire::ostringstream_t o;
  o<<"player";
  o<<i;
  const spitfire::string_t sItem = o.str();

  SetXMLValue(TEXT("players"), sItem, TEXT("name"), sName);
}

spitfire::string_t cSettings::GetPlayerColour(size_t i) const
{
  spitfire::ostringstream_t o;
  o<<"player";
  o<<i;
  const spitfire::string_t sItem = o.str();

  const spitfire::string_t defaultColours[] = {
    TEXT("Red"),
    TEXT("Blue"),
    TEXT("Yellow"),
    TEXT("Green")
  };
  assert(i < 4);
  spitfire::string_t defaultValue = defaultColours[i];

  return GetXMLValue(TEXT("settings"), sItem, TEXT("colour"), defaultValue);
}

void cSettings::SetPlayerColour(size_t i, const spitfire::string_t& colour)
{
  spitfire::ostringstream_t o;
  o<<"player";
  o<<i;
  const spitfire::string_t sItem = o.str();

  SetXMLValue(TEXT("settings"), sItem, TEXT("colour"), colour);
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
      while (iter.IsValid()) {
        cHighScoresTableEntry entry;
        if (iter.GetAttribute("name", entry.sName)) {
          iter.GetAttribute("score", entry.score);

          entries.push_back(entry);
        }

        iter.Next("entry");
      };
    }
  }

  return entries;
}

void cSettings::SetHighScores(const std::vector<cHighScoresTableEntry>& entries)
{
  // Get or create the config element
  spitfire::document::cNode::iterator iterConfig(document);

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
  spitfire::document::cNode::iterator iterHighscores(iterConfig.GetChild("highscores"));
  if (!iterHighscores.IsValid()) {
    spitfire::document::element* highscoresElement = document.CreateElement("highscores");
    spitfire::document::element* configElement = iterConfig.Get();
    configElement->AppendChild(highscoresElement);
    iterHighscores = iterConfig.GetChild("highscores");
    assert(iterHighscores.IsValid());
  }

  spitfire::document::element* highscoresElement = iterHighscores.Get();
  highscoresElement->RemoveChildren();

  // Entries
  const size_t n = entries.size();
  for (size_t i = 0; i < n; i++) {
    spitfire::document::element* entryElement = document.CreateElement("entry");
    highscoresElement->AppendChild(entryElement);

    entryElement->SetAttribute("name", entries[i].sName);
    const uint64_t score = entries[i].score;
    entryElement->SetAttribute("score", score);
  }
}
