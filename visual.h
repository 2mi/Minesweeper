#pragma once

#include <windows.h>
#include <vector>
#include "entry.h"

class ConsoleView
{
  void setConsoleAttribute();
  void fnMouseProc(COORD, DWORD, DWORD, DWORD);
  void fnKeyboardProc(BOOL, WORD, WORD, WORD, CHAR, DWORD);
  void paint(uint8_t, uint8_t, BlockType);
  inline COORD &coordMaptoPos(COORD &&);
  inline COORD &posMaptoCoord(COORD &&);
  inline bool isValidCoord(CONST COORD &);
  static inline friend COORD operator +(CONST COORD &, CONST COORD &);
  static inline std::pair<char, uint8_t> typeMap(BlockType);
  
  CONST HANDLE hInput, hOutput;
  uint8_t x_delim, y_delim;
  CONSOLE_FONT_INFOEX cfiConsoleCurrentFontEx, cfiOldConsoleCurrentFontEx;
  CONSOLE_SCREEN_BUFFER_INFOEX csbiOldConsoleScreenBufferInfoEx;
  UINT uiOldCP;
  //std::vector<uint8_t> blockPos;
  //std::vector<std::pair<CHAR_INFO, COORD>> flushVec;
  //unordered_map<uint8_t, char> typeMap { {}, };

  typedef enum : uint8_t { 
    COLOR_RED       = FOREGROUND_INTENSITY | FOREGROUND_RED,
    COLOR_GREEN     = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
    COLOR_BLUE      = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    COLOR_YELLOW    = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
    COLOR_PURPLE    = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
    COLOR_CYAN      = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
    COLOR_GRAY      = FOREGROUND_INTENSITY,
    COLOR_WHITE     = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    COLOR_HIGHWHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    COLOR_BLACK     = 0,
  } ConsoleFontColor;
public:
  ConsoleView();
  ~ConsoleView();
  void loop();
  void repaint();
  void resize(uint8_t col, uint8_t row, uint8_t numMines);
};
extern ConsoleView consoleView;