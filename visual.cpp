#include <iostream>
#include <sstream>
#include <windows.h>
#include "visual.h"
#include "main.h"

using namespace std;

#pragma warning (disable: 4533 4996)

#define TEST_MOUSEPOS 0
#define PRESSED  0
#define RELEASED 1
#define UNION_KEY(key, st) ((key) << 4 | (st))
#define AMONG_KEY(key, st1, st2) UNION_KEY(key, (st1) | (st2))

decltype(auto) col = minesweeper.getCol(), row = minesweeper.getRow();
//decltype(auto) board = minesweeper.getBoard();

void ConsoleView::fnMouseProc(
    COORD dwMousePosition,
    DWORD dwButtonState,
    [[maybe_unused]] DWORD dwControlKeyState,
    DWORD dwEventFlags
    )
{
    if (0 != dwEventFlags && DOUBLE_CLICK != dwEventFlags) {
        goto __leave_;
    }

    static DWORD dwlastButtonState = 0;
    DWORD keyCode = dwButtonState | dwlastButtonState, keyState = !static_cast<bool>((dwButtonState ^ dwlastButtonState) & dwButtonState);
    dwlastButtonState = dwButtonState;
#if !TEST_MOUSEPOS
    if (GameStatus::None == (minesweeper.gameStatus & 0xf0)) {
        goto __leave_;
    }
#endif
    COORD dwOldMousePosition = dwMousePosition;
    coordMaptoPos(std::move(dwMousePosition));

    switch(AMONG_KEY(keyCode, keyState, dwEventFlags))
    {
        case UNION_KEY(RIGHTMOST_BUTTON_PRESSED, PRESSED):
            minesweeper.setFlag(
                static_cast<uint8_t>(dwMousePosition.Y), 
                static_cast<uint8_t>(dwMousePosition.X),
                std::bind(&ConsoleView::paint, this, placeholders::_1, placeholders::_2, placeholders::_3) //
            );
            break;
         case UNION_KEY(FROM_LEFT_1ST_BUTTON_PRESSED, RELEASED):
            minesweeper.click(
                static_cast<uint8_t>(dwMousePosition.Y), 
                static_cast<uint8_t>(dwMousePosition.X), 
                std::bind(&ConsoleView::paint, this, placeholders::_1, placeholders::_2, placeholders::_3)
            );
            break;
        case UNION_KEY(FROM_LEFT_1ST_BUTTON_PRESSED, DOUBLE_CLICK):
            minesweeper.doubleClick(
                static_cast<uint8_t>(dwMousePosition.Y), 
                static_cast<uint8_t>(dwMousePosition.X), 
                std::bind(&ConsoleView::paint, this, placeholders::_1, placeholders::_2, placeholders::_3)
            );
        //__click_outer_:
            break;
        default:
            goto __leave_;
    }

    if (isValidCoord(dwOldMousePosition)) {
        SetConsoleCursorPosition(hOutput, dwOldMousePosition); //?
    }
__leave_:
    return;
}

void ConsoleView::fnKeyboardProc(
    BOOL  bKeyDown,
    [[maybe_unused]] WORD  wRepeatCount,
    WORD  wVirtualKeyCode,
    [[maybe_unused]] WORD  wVirtualScanCode,
    [[maybe_unused]] CHAR  AsciiChar,
    [[maybe_unused]] DWORD dwControlKeyState
    )
{
    if (false == bKeyDown || GameStatus::None == (minesweeper.gameStatus & 0xf0)) {
        goto __leave_;
    }

    bool isCommit = false;
    COORD curOffset {0};

    switch(wVirtualKeyCode)
    {
        case VK_UP:
            --curOffset.Y;
            break;
        case VK_DOWN:
            ++curOffset.Y;
            break;
        case VK_LEFT:
            --curOffset.X;
            break;
        case VK_RIGHT:
            ++curOffset.X;
            break;
        case VK_RETURN: //wRepeatCount
            isCommit = true;
            break;
        case VK_SPACE:
            break;
        default:
            goto __leave_;
    }

    CONSOLE_SCREEN_BUFFER_INFO cursorInfo;
    GetConsoleScreenBufferInfo(hOutput, &cursorInfo);
    cursorInfo.dwCursorPosition = cursorInfo.dwCursorPosition + posMaptoCoord(std::move(curOffset));

    if (isCommit) {
        coordMaptoPos(std::move(cursorInfo.dwCursorPosition));
        minesweeper.click(
            static_cast<uint8_t>(cursorInfo.dwCursorPosition.Y),
            static_cast<uint8_t>(cursorInfo.dwCursorPosition.X),
            std::bind(&ConsoleView::paint, this, placeholders::_1, placeholders::_2, placeholders::_3)
        );
    } else if (isValidCoord(cursorInfo.dwCursorPosition)) {
        SetConsoleCursorPosition(hOutput, cursorInfo.dwCursorPosition);
    }
__leave_:
    return;
}

COORD &ConsoleView::coordMaptoPos(COORD &&coord)
{
    coord.Y /= (y_delim + 1);
    coord.X /= (x_delim + 1);
    return coord;
}

COORD &ConsoleView::posMaptoCoord(COORD &&coord)
{
    coord.Y *= (y_delim + 1);
    coord.X *= (x_delim + 1);
    return coord;
}

bool ConsoleView::isValidCoord(CONST COORD &curCoord) {
    return curCoord.X >=0 && curCoord.X < row * (x_delim + 1) && curCoord.Y >=0 && curCoord.Y < col * (y_delim + 1);
}

COORD operator +(CONST COORD &_left, CONST COORD &_right)
{
    return {_left.X + _right.X, _left.Y + _right.Y};
}

void ConsoleView::setConsoleAttribute()
{
    bool bRes = SetConsoleMode (
        hInput,
        ENABLE_ECHO_INPUT | ENABLE_INSERT_MODE | ENABLE_LINE_INPUT | ENABLE_MOUSE_INPUT | 
        ENABLE_PROCESSED_OUTPUT | ENABLE_LVB_GRID_WORLDWIDE
    );

    if (!bRes) {
        std::cerr << "[warning] mouse input have to be disabled!" << std::endl;
    }
    
    COORD dwBufferSize = posMaptoCoord({row, col}) + COORD{1, 2};
    auto fontWidth = dwBufferSize.X * cfiConsoleCurrentFontEx.dwFontSize.X, fontHeigh = dwBufferSize.Y * cfiConsoleCurrentFontEx.dwFontSize.Y;
    dwBufferSize.X += static_cast<SHORT>(max(fontHeigh - fontWidth, 0) / cfiConsoleCurrentFontEx.dwFontSize.X);
    dwBufferSize.Y += static_cast<SHORT>(max(fontWidth - fontHeigh, 0) / cfiConsoleCurrentFontEx.dwFontSize.Y);
    
    stringstream ss;
    CONSOLE_SCREEN_BUFFER_INFOEX csbiConsoleScreenBufferInfoEx =  {
        .cbSize = sizeof(csbiConsoleScreenBufferInfoEx),
        .dwSize = dwBufferSize,
        .dwCursorPosition {0, 0},
        .wAttributes = COLOR_WHITE,
        .srWindow { 0, 0, dwBufferSize.X, dwBufferSize.Y },
        .dwMaximumWindowSize = dwBufferSize,
        .wPopupAttributes = COLOR_WHITE,
        .bFullscreenSupported = true,
        .ColorTable { 0x000000, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080, 0xffffff, 0x808080, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff,0xffffff }
    };
    SetConsoleScreenBufferInfoEx(hOutput, &csbiConsoleScreenBufferInfoEx);
    SetConsoleTitleA(static_cast<stringstream&>(ss << (int)col << 'x' << (int)row << '=' << (int)minesweeper.getNumMines()).str().c_str());
}

ConsoleView::ConsoleView()
 : x_delim(0), 
 y_delim(0), 
 hInput(GetStdHandle(STD_INPUT_HANDLE)),
 hOutput(GetStdHandle(STD_OUTPUT_HANDLE))
{
    uiOldCP = GetConsoleOutputCP();
    cfiConsoleCurrentFontEx.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    csbiOldConsoleScreenBufferInfoEx.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
    GetCurrentConsoleFontEx(hOutput, false, &cfiOldConsoleCurrentFontEx);
    GetConsoleScreenBufferInfoEx(hOutput, &csbiOldConsoleScreenBufferInfoEx);

    cfiConsoleCurrentFontEx = {
        .cbSize = sizeof(cfiConsoleCurrentFontEx),
        .nFont = 0,
        .dwFontSize {13, 20},
        .FontFamily = FF_DONTCARE,
        .FontWeight = FW_BOLD,
        .FaceName = L"Lucida Console"
    };
    SetConsoleOutputCP(437);
    SetCurrentConsoleFontEx(hOutput, false, &cfiConsoleCurrentFontEx);

    //setConsoleAttribute();
}

ConsoleView::~ConsoleView()
{
    system("cls");
    SetConsoleScreenBufferInfoEx(hOutput, &csbiOldConsoleScreenBufferInfoEx);
    SetCurrentConsoleFontEx(hOutput, false, &cfiOldConsoleCurrentFontEx);
    SetConsoleOutputCP(uiOldCP);
}

pair<char, uint8_t> ConsoleView::typeMap(BlockType type)
{
    pair<char, uint8_t> res;
    [[maybe_unused]]auto &[ch, clr] = res;

    if ((type & 0xf0) == BlockType::Flag) {
        res = {'#', COLOR_GREEN};
    } else if ((type & 0xf0) == BlockType::Mystery) {
        res = {'?', COLOR_GRAY};
    } else if (type == BlockType::Explode) {
        res = {'X', COLOR_RED}; 
    } else if (type == BlockType::Blank) {
        res = {' ', COLOR_BLACK};
    } else if (type < (BlockType::Digital + 8)) {
        res = {static_cast<char>('0' | type), static_cast<uint8_t>(COLOR_BLUE + (type - 1) / 2)};
    } else if (true) { // (type & 0x0f) == BlockType::Normal
        res = {'E', COLOR_WHITE};
    }

    return res;
}

void ConsoleView::paint(uint8_t i, uint8_t j, BlockType type)
{
    COORD crdPos = posMaptoCoord({j, i});
    SMALL_RECT srcWriteRect { crdPos.X, crdPos.Y, crdPos.X + 1, crdPos.Y + 1};
    CHAR_INFO chi;
    std::tie(chi.Char.AsciiChar, chi.Attributes) = typeMap(type);
    WriteConsoleOutput(hOutput, &chi, {1, 1}, {0, 0}, &srcWriteRect);
}

void ConsoleView::repaint()
{
    system("cls");
    setConsoleAttribute();

    string flushBuf;

    for (int y = col ; y != 0; --y)
    {
        for (int x = row ; x != 0 ; --x)
        {
            flushBuf.push_back('E');
            flushBuf.append(x_delim, ' ');
        }
        flushBuf.append(1 + y_delim, '\n');
    }

    SetConsoleCursorPosition(hOutput, {0, 0});
    WriteConsole(hOutput, flushBuf.data(), static_cast<DWORD>(flushBuf.size()), nullptr, nullptr);
}

void ConsoleView::resize(uint8_t _col, uint8_t _row, uint8_t _numMines)
{
    new(&minesweeper) Minesweeper(_col, _row, _numMines);
    //GetCurrentConsoleFont(hOutput, false, &cfiConsoleCurrentFontEx);
    auto fontWidth = cfiConsoleCurrentFontEx.dwFontSize.X * _row, fontHeigh = cfiConsoleCurrentFontEx.dwFontSize.Y * _col;
    x_delim = static_cast<uint8_t>(max(fontHeigh - fontWidth, 0) / (fontWidth - cfiConsoleCurrentFontEx.dwFontSize.X));
    y_delim = static_cast<uint8_t>(max(fontWidth - fontHeigh, 0) / (fontHeigh - cfiConsoleCurrentFontEx.dwFontSize.Y));
    repaint();
}

void ConsoleView::loop()
{
    SetConsoleCursorPosition(hOutput, COORD{0, 0});

    INPUT_RECORD in;
    for (DWORD dwNumEvents ; ReadConsoleInput(hInput, &in, 1, &dwNumEvents) ; )
    {
        if (MOUSE_EVENT == in.EventType) {
            fnMouseProc(in.Event.MouseEvent.dwMousePosition, in.Event.MouseEvent.dwButtonState, in.Event.MouseEvent.dwControlKeyState, in.Event.MouseEvent.dwEventFlags);
        } else if (KEY_EVENT == in.EventType) {
            fnKeyboardProc(in.Event.KeyEvent.bKeyDown, in.Event.KeyEvent.wRepeatCount, in.Event.KeyEvent.wVirtualKeyCode, in.Event.KeyEvent.wVirtualScanCode, in.Event.KeyEvent.uChar.AsciiChar, in.Event.KeyEvent.dwControlKeyState);
        } else {
            goto __continue_;
        }

        if (GameStatus::None == (minesweeper.gameStatus & 0xf0)) {
            break;
        }
    __continue_:;
    }

    COORD endPos = {0, col};
    SetConsoleCursorPosition(hOutput, posMaptoCoord(std::move(endPos)) + COORD{0, 1});
    minesweeper.expose(std::bind(&ConsoleView::paint, this, placeholders::_1, placeholders::_2, placeholders::_3));
    MessageBoxA(
        GetConsoleWindow(), 
        GameStatus::Won == (minesweeper.gameStatus & 0x0f) ?
             "You Won! " : 
             "You Lost. " , 
        "Result", 
        MB_OK | MB_ICONINFORMATION
    );
    cout << "Continue [Y/N]?" << endl;
}
ConsoleView consoleView;