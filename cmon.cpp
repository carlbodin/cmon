#include <conio.h> // For _kbhit() and _getch()
#include <cstdlib>
#include <iomanip> // Add this include for std::setprecision
#include <iostream>
#include <pdh.h>
#include <psapi.h> // For GetProcessMemoryInfo
#include <string>
#include <vector>
#include <windows.h>
#include <winscard.h>

#pragma comment(lib, "psapi.lib") // Link the PSAPI library
#pragma comment(lib, "pdh.lib")

void ClearConsole(HANDLE hConsole) {
  // Move cursor to top left corner instead of clearing the entire terminal for the new
  // loop information.

  COORD topLeft = {0, 0};
  SetConsoleCursorPosition(hConsole, topLeft);
}

void SetConsoleSize(HANDLE hConsole, short width, short height) {
  SMALL_RECT windowSize = {0, 0, static_cast<SHORT>(width - 1),
                           static_cast<SHORT>(height - 1)};
  COORD bufferSize = {width, height};

  SetConsoleScreenBufferSize(hConsole, bufferSize);
  SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
}

class CpuUsageMonitor {
public:
  PDH_HQUERY query;
  std::vector<PDH_HCOUNTER> counters;
  PDH_HCOUNTER totalCounter, cpuFrequencyCounter, idleTimeCounter;

  CpuUsageMonitor() {
    PdhOpenQuery(NULL, 0, &query);

    // Get the number of logical processors
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD numCores = sysInfo.dwNumberOfProcessors;

    // Add counters for each core
    for (DWORD i = 0; i < numCores; ++i) {
      PDH_HCOUNTER counter;
      std::wstring counterPath =
          L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
      PdhAddCounterW(query, counterPath.c_str(), 0, &counter);
      counters.push_back(counter);
    }

    // Add counters for total CPU usage, frequency, and idle time
    PdhAddCounterW(query, L"\\Processor(_Total)\\% Processor Time", 0, &totalCounter);
    PdhAddCounterW(query, L"\\Processor Information(_Total)\\Processor Frequency", 0,
                   &cpuFrequencyCounter);
    PdhAddCounterW(query, L"\\Processor(_Total)\\% Idle Time", 0, &idleTimeCounter);
  }

  ~CpuUsageMonitor() { PdhCloseQuery(query); }

  void GetCpuUsage(std::vector<double> &cpuUsagePerCore, double &totalCpuUsage,
                   int &cpuFrequency, double &idleTime) {
    PDH_FMT_COUNTERVALUE counterVal;

    // Collect data
    PdhCollectQueryData(query);
    Sleep(1000);
    PdhCollectQueryData(query);

    // Get per-core CPU usage
    cpuUsagePerCore.clear();
    for (const auto &counter : counters) {
      PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterVal);
      cpuUsagePerCore.push_back(counterVal.doubleValue);
    }

    // Get total CPU usage
    PdhGetFormattedCounterValue(totalCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
    totalCpuUsage = counterVal.doubleValue;

    // Get CPU frequency
    PdhGetFormattedCounterValue(cpuFrequencyCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
    cpuFrequency = (int)counterVal.doubleValue;

    // Get idle time
    PdhGetFormattedCounterValue(idleTimeCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
    idleTime = counterVal.doubleValue;
  }
};

void GetMemoryUsage(double &memoryUsagePerc, double &swapUsagePerc, double &totalMemory,
                    double &usedMemory, double &totalSwap, double &usedSwap) {
  constexpr double BYTE_TO_MB = 1.0 / (1024.0 * 1024.0);
  MEMORYSTATUSEX memStatus;
  memStatus.dwLength = sizeof(memStatus);

  if (GlobalMemoryStatusEx(&memStatus)) {
    usedMemory = static_cast<double>(memStatus.ullTotalPhys - memStatus.ullAvailPhys) *
                 BYTE_TO_MB;
    totalMemory = static_cast<double>(memStatus.ullTotalPhys) * BYTE_TO_MB;
    memoryUsagePerc = (usedMemory / totalMemory) * 100.0;

    if (memStatus.ullTotalPageFile > memStatus.ullTotalPhys) {
      totalSwap =
          static_cast<double>(memStatus.ullTotalPageFile - memStatus.ullTotalPhys) *
          BYTE_TO_MB;
      usedSwap =
          static_cast<double>((memStatus.ullTotalPageFile - memStatus.ullTotalPhys) -
                              (memStatus.ullAvailPageFile - memStatus.ullAvailPhys)) *
          BYTE_TO_MB;
      swapUsagePerc = (usedSwap / totalSwap) * 100.0;
    } else {
      totalSwap = static_cast<double>(memStatus.ullTotalPageFile) * BYTE_TO_MB;
      usedSwap =
          static_cast<double>(memStatus.ullTotalPageFile - memStatus.ullAvailPageFile) *
          BYTE_TO_MB;
      swapUsagePerc = (usedSwap / totalSwap) * 100.0;
    }
  } else {
    memoryUsagePerc = -1.0;
    swapUsagePerc = -1.0;
    totalMemory = 0.0;
    usedMemory = 0.0;
    totalSwap = 0.0;
    usedSwap = 0.0;
  }
  if (swapUsagePerc < 0.0 || swapUsagePerc > 100.0) {
    swapUsagePerc = 0.0;
    usedSwap = 0.0;
    totalSwap = 0.0;
  }
}

void GetProcessResourceUsage(SIZE_T &processMemoryUsage) {
  // Get the current process handle
  HANDLE hProcess = GetCurrentProcess();

  // Get memory usage
  PROCESS_MEMORY_COUNTERS_EX memCounters;
  if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *)&memCounters,
                           sizeof(memCounters))) {
    processMemoryUsage = memCounters.WorkingSetSize /= (1024 * 1024); // Convert to MB
  } else {
    processMemoryUsage = 0; // Set to 0 if the call fails
    std::cerr << "Failed to retrieve process memory info. Error: " << GetLastError()
              << std::endl;
  }
}

void hideCursor() {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
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

BOOL WINAPI consoleHandler(DWORD signal) {
  // Reset the screen before exiting
  if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
    system("cls");
    showCursor();
    exit(0);
  }
  return TRUE;
}

const void printBar(const std::string &name, const double usagePercent,
                    const int &lineWidth) {
  // Bar width is line width minus name, spaces, and brackets.
  int barWidth = lineWidth - name.length() - 1 - 2;
  int filled = static_cast<int>((usagePercent / 100.0f) * barWidth);
  int usagePercentInt = static_cast<int>(usagePercent);

  std::cout << name;
  std::cout << " [";
  for (int i = 0; i < barWidth; ++i) {
    if (i < filled)
      std::cout << "█";
    else
      std::cout << " ";
  }

  // Move cursor back and print the percentage.
  std::cout << "\b\b\b\b" << std::setw(3) << usagePercentInt << "%";
  std::cout << "]\n";
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

int main(int argc, char *argv[]) {
  bool useBar = argc > 1 && std::string(argv[1]) == "bar" ? true : false;
  if (argc > 1 && std::string(argv[1]) == "help") {
    std::cout << "Usage: cmon [bar|help]\n\n";
    std::cout << "A lightweight CPU monitoring program in terminal for Windows.\n\n";
    std::cout << "Options:\n";
    std::cout << "  bar   : Display info in bar graphs.\n";
    std::cout << "  help  : Display this help message.\n";
    return 0;
  }
  if (argc > 1 && std::string(argv[1]) != "bar") {
    std::cerr << "Invalid option. Use 'help' for usage information.\n";
    return 1;
  }

  CpuUsageMonitor cpuMonitor;

  int width = 74, height = 20;
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  hideCursor();
  system("cls");
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCtrlHandler(consoleHandler, TRUE);
  SetConsoleSize(hConsole, width, height);
  getConsoleSize(hConsole, width, height);

  std::vector<double> cpuUsagePerCore;
  int cpuFrequency = 0;
  double idleTime = 0.0, totalCpuUsagePerc = 0.0, memoryUsagePerc = 0.0,
         swapUsagePerc = 0.0, totalMemory = 0.0, usedMemory = 0.0, totalSwap = 0.0,
         usedSwap = 0.0;

  std::cout << std::fixed << std::setprecision(1);

  while (true) {
    // Check if the Q or ESC key is pressed
    if (_kbhit()) {
      char key = _getch();
      if (key == 'q' || key == 'Q' || key == 27) { // 27 is the ESC key
        system("cls");
        break;
      }
    }

    cpuMonitor.GetCpuUsage(cpuUsagePerCore, totalCpuUsagePerc, cpuFrequency, idleTime);
    GetMemoryUsage(memoryUsagePerc, swapUsagePerc, totalMemory, usedMemory, totalSwap,
                   usedSwap);

    ClearConsole(hConsole);
    if (useBar) {
      printBar(" CPU", totalCpuUsagePerc, width - 1);
      printBar(" Mem", memoryUsagePerc, width - 1);
      printBar("Swap", swapUsagePerc, width - 1);
    } else {
      std::cout
          << "|         CPU          |         Memory        |          Swap       "
             "   |\n";
      std::cout
          << "|======================|=======================|====================="
             "===|\n";
      std::cout << "|  CPU Usage:   " << std::setw(5) << totalCpuUsagePerc
                << "% |   Percent:    " << std::setw(5) << memoryUsagePerc
                << "%  |    Percent:    " << std::setw(5) << swapUsagePerc << "%  |\n";
      std::cout << "|  Idle Time:  " << std::setw(6) << idleTime
                << "% |  Used Mem: " << std::setw(6) << static_cast<int>(usedMemory)
                << " MB  "
                << "|  Used Swap: " << std::setw(6) << static_cast<int>(usedSwap)
                << " MB  |\n";
      std::cout << "|   CPU Freq: " << std::setw(4) << cpuFrequency
                << " MHz | Total Mem: " << std::setw(6) << static_cast<int>(totalMemory)
                << " MB  "
                << "| Total Swap: " << std::setw(6) << static_cast<int>(totalSwap)
                << " MB  |\n";
      std::cout
          << "|----------------------|-----------------------|---------------------"
             "---|\n";
      std::cout << "| Per Core:            |\n";
      for (size_t i = 0; i < cpuUsagePerCore.size(); ++i) {
        std::cout << "| " << std::setw(2) << i + 1 << ": " << std::setw(6)
                  << cpuUsagePerCore[i] << "%          |\n";
      }
      std::cout
          << "|----------------------|                            Exit: Ctrl + C\n";
    }
  }

  return 0;
}