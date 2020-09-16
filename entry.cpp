#include <iostream>
#include <sstream>
#include "entry.h"
#include "visual.h"
#include "main.h"

#pragma comment(lib, "User32.lib")

using namespace std;

#undef max

void setBoardSize(const char *buf)
{
    char delim;
    stringstream ss;
    if (buf) {
        ss.str(buf);
    } else {
        ss.set_rdbuf(cin.rdbuf());
    }
    int col, row, numMines;
    ss >> col >> delim >> row >> delim >> numMines;
    if (col <= 32 && row <= 32 && numMines < col * row) {
        consoleView.resize(static_cast<uint8_t>(col), static_cast<uint8_t>(row), static_cast<uint8_t>(numMines));
    } else {
        std::cerr << "[wrong] incorrect board size!" << std::endl;
    }
}

int main (int argv, TCHAR* args[])
{
    setBoardSize(argv - 1 ? reinterpret_cast<CHAR **>(args)[1] : "9x9=10");

    for (int cmd = 0 ; cmd != EOF ; ) 
    {
        consoleView.loop();

        cmd = toupper(cin.get());
        if (cmd == 'S') {
            for ( ; cin.peek() != '\n' || (minesweeper.gameStatus & 0xf0) != GameStatus::Processing ; ) {
                setBoardSize(nullptr);
                cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
            }
        } else if (cmd == 'Y' || cmd == '\n') {
             consoleView.resize(minesweeper.getCol(), minesweeper.getRow(), minesweeper.getNumMines());
        } else if (cmd == 'R') {
            consoleView.repaint();
            minesweeper.restore();
        } else {
            cmd = EOF;
        }

        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
    }

    cout << "system ret: " << GetLastError() << endl;
    cin.get();

    return 0;
}