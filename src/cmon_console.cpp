#include <conio.h>
#include <iomanip> // Add this include for std::setprecision
#include <iostream>
#include <windows.h>

void hideCursor(HANDLE hConsole) {
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = FALSE; // Hide the cursor
  SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void showCursor() {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = TRUE; // Show the cursor
  SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void checkExitEvent() {
  // Check for exit events (Ctrl+C or close button)
  if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
    showCursor();
    system("cls");
    exit(0);
  }

  // Check if the Q or ESC key is pressed
  if (_kbhit()) {
    char key = _getch();
    if (key == 'q' || key == 'Q' || key == 27) { // 27 is the ESC key
      showCursor();
      system("cls");
      exit(0);
    }
  }
}

void clearConsole(HANDLE hConsole, bool useBar) {
  // Move cursor to top left corner instead of clearing the entire terminal for the new
  // loop information.
  COORD topLeft;
  if (useBar) {
    topLeft = {0, 1};
  } else {
    topLeft = {0, 2};
  }
  SetConsoleCursorPosition(hConsole, topLeft);
}

BOOL WINAPI consoleHandler(DWORD signal) {
  // Reset the screen before exiting
  if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
    showCursor();
    system("cls");
    exit(0);
  }
  return TRUE;
}

void getConsoleSize(HANDLE &hConsole, int &width, int &height) {

  // Get the console screen buffer info
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
    // csbi.srWindow.Left
    // csbi.srWindow.Right
    // csbi.srWindow.Top
    // csbi.srWindow.Bottom
    // csbi.dwSize.X
    // csbi.dwSize.Y
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  }
}

void setConsoleSize(HANDLE hConsole, short width, short height, short numberOfCores,
                    bool useBar) {
  // TODO: Currently hardcoded height and width variables.
  SMALL_RECT windowSize;
  COORD bufferSize;

  if (useBar) {
    windowSize = {0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(5 - 1)};
    bufferSize = {width, 5};
  } else {
    windowSize = {0, 0, static_cast<SHORT>(width - 1),
                  static_cast<SHORT>(9 + numberOfCores - 1)};
    bufferSize = {width, static_cast<SHORT>(9 + numberOfCores)};
  }

  SetConsoleScreenBufferSize(hConsole, bufferSize);
  SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
}

void setupConsole(int &width, int &height, HANDLE &hConsole, short numberOfCores,
                  bool useBar) {
  hideCursor(hConsole);
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCtrlHandler(consoleHandler, TRUE);
  setConsoleSize(hConsole, width, height, numberOfCores, useBar);
  getConsoleSize(hConsole, width, height);
  std::cout << std::fixed << std::setprecision(1);
}