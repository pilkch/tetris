#ifndef TETRIS_SETTINGS_H
#define TETRIS_SETTINGS_H

#include <spitfire/storage/document.h>

// ** cHighScoresTableEntry

class cHighScoresTableEntry
{
public:
  bool IsValid() const { return !sName.empty(); }

  static bool ScoreCompare(const cHighScoresTableEntry& lhs, const cHighScoresTableEntry& rhs);

  spitfire::string_t sName;
  size_t score;
};

inline bool cHighScoresTableEntry::ScoreCompare(const cHighScoresTableEntry& lhs, const cHighScoresTableEntry& rhs)
{
  return (lhs.score > rhs.score);
}


// ** cSettings

class cSettings
{
public:
  void Load();
  void Save();

  size_t GetNumberOfPlayers() const;
  void SetNumberOfPlayers(size_t nPlayers);

  spitfire::string_t GetPlayerName(size_t i) const;
  void SetPlayerName(size_t i, const spitfire::string_t& sName);
  spitfire::string_t GetPlayerColour(size_t i) const;
  void SetPlayerColour(size_t i, const spitfire::string_t& colour);

  std::vector<cHighScoresTableEntry> GetHighScores() const;
  void SetHighScores(const std::vector<cHighScoresTableEntry>& entries);

private:
  template <class T>
  T GetXMLValue(const spitfire::string_t& sSection, const spitfire::string_t& sItem, const spitfire::string_t& sAttribute, const T& valueDefault) const;
  template <class T>
  void SetXMLValue(const spitfire::string_t& sSection, const spitfire::string_t& sItem, const spitfire::string_t& sAttribute, const T& value);

  spitfire::document::cDocument document;
};

#endif // TETRIS_SETTINGS_H
