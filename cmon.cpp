#include <iomanip> // Add this include for std::setprecision
#include <iostream>
#include <pdh.h>
#include <psapi.h> // For GetProcessMemoryInfo
#include <string>
#include <vector>
#include <windows.h>

#pragma comment(lib, "psapi.lib") // Link the PSAPI library
#pragma comment(lib, "pdh.lib")

constexpr double B_TO_MB = 9.5367431640625e-7; // 1 / (1024 * 1024)

void SetConsoleSize(int width, int height) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsole == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to get console handle." << std::endl;
    return; // Exit the function if getting the console handle fails
  }

  // Set the console window size
  SMALL_RECT windowSize;
  windowSize.Left = 0;
  windowSize.Top = 0;
  windowSize.Right = width - 1;
  windowSize.Bottom = height - 1;
  if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
    DWORD error = GetLastError();
    std::cerr << "Failed to set console window size. Error: " << error << std::endl;
  }
  // Ensure the buffer size is at least as large as the window size
  COORD bufferSize;
  bufferSize.X = width;
  bufferSize.Y = height;
  if (!SetConsoleScreenBufferSize(hConsole, bufferSize)) {
    DWORD error = GetLastError();
    std::cerr << "Failed to set screen buffer size. Error: " << error << std::endl;
    return; // Exit the function if setting the buffer size fails
  }
}

void GetCpuUsagePerCore(std::vector<double> &cpuUsagePerCore, double &totalCpuUsage, int &cpuFrequency, double &idleTime) {
  PDH_HQUERY query;
  std::vector<PDH_HCOUNTER> counters;
  PDH_HCOUNTER cpuFrequencyCounter, idleTimeCounter, totalCounter;
  PDH_FMT_COUNTERVALUE counterVal;

  // Open the query
  PdhOpenQuery(NULL, 0, &query);

  // Get the number of logical processors
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  DWORD numCores = sysInfo.dwNumberOfProcessors;

  // Add counters for each core
  for (DWORD i = 0; i < numCores; ++i) {
    PDH_HCOUNTER counter;
    std::wstring counterPath = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
    PdhAddCounterW(query, counterPath.c_str(), 0, &counter);
    counters.push_back(counter);
  }

  // Add a counter for total CPU usage
  PdhAddCounterW(query, L"\\Processor(_Total)\\% Processor Time", 0, &totalCounter);
  PdhAddCounterW(query, L"\\Processor Information(_Total)\\Processor Frequency", 0, &cpuFrequencyCounter);
  PdhAddCounterW(query, L"\\Processor(_Total)\\% Idle Time", 0, &idleTimeCounter);

  // Collect data
  PdhCollectQueryData(query);
  Sleep(750);
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

  // Close the query
  PdhCloseQuery(query);
}

void GetMemoryUsage(double &memoryUsagePerc, double &swapUsagePerc, SIZE_T &totalMemory, SIZE_T &usedMemory, SIZE_T &totalSwap, SIZE_T &usedSwap) {
  MEMORYSTATUSEX memStatus;
  memStatus.dwLength = sizeof(memStatus);

  if (GlobalMemoryStatusEx(&memStatus)) {
    // Calculate memory usage as a percentage
    usedMemory = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
    memoryUsagePerc = (double)usedMemory / memStatus.ullTotalPhys * 100.0;

    // Calculate swap usage as a percentage
    usedSwap = (memStatus.ullTotalPageFile - memStatus.ullTotalPhys) - (memStatus.ullAvailPageFile - memStatus.ullAvailPhys);
    swapUsagePerc = (double)usedSwap / (memStatus.ullTotalPageFile - memStatus.ullTotalPhys) * 100.0;

    // Get total and available sizes in MB
    totalMemory = memStatus.ullTotalPhys * B_TO_MB;
    usedMemory = usedMemory * B_TO_MB;
    totalSwap = (memStatus.ullTotalPageFile - memStatus.ullTotalPhys) * B_TO_MB;
    usedSwap = usedSwap * B_TO_MB;
  } else {
    // If the function fails, set usage to -1 and sizes to 0
    memoryUsagePerc = -1.0;
    swapUsagePerc = -1.0;
    totalMemory = 0;
    usedMemory = 0;
    totalSwap = 0;
    usedSwap = 0;
    std::cerr << "Failed to retrieve memory status. Error: " << GetLastError() << std::endl;
  }
}

void GetProcessResourceUsage(SIZE_T &processMemoryUsage) {
  // Get the current process handle
  HANDLE hProcess = GetCurrentProcess();

  // Get memory usage
  PROCESS_MEMORY_COUNTERS_EX memCounters;
  if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *)&memCounters, sizeof(memCounters))) {
    processMemoryUsage = memCounters.WorkingSetSize * B_TO_MB; // Convert to MB
  } else {
    processMemoryUsage = 0; // Set to 0 if the call fails
    std::cerr << "Failed to retrieve process memory info. Error: " << GetLastError() << std::endl;
  }
}

int main() {
  std::vector<double> cpuUsagePerCore;
  int cpuFrequency = 0.0;
  double idleTime = 0.0;
  double totalCpuUsage = 0.0;
  GetCpuUsagePerCore(cpuUsagePerCore, totalCpuUsage, cpuFrequency, idleTime);

  // This only works when run as admin.
  SetConsoleSize(74, 10 + cpuUsagePerCore.size());

  while (true) {

    // Get CPU usage
    GetCpuUsagePerCore(cpuUsagePerCore, totalCpuUsage, cpuFrequency, idleTime);

    // Get memory and swap usage
    double memoryUsagePerc = 0.0, swapUsagePerc = 0.0;
    SIZE_T totalMemory = 0, usedMemory = 0, totalSwap = 0, usedSwap = 0;
    GetMemoryUsage(memoryUsagePerc, swapUsagePerc, totalMemory, usedMemory, totalSwap, usedSwap);

    // Process resource usage
    // SIZE_T processMemoryUsage = 0;
    // GetProcessResourceUsage(processMemoryUsage);
    // std::cout << "| Process Memory: " << std::setw(6) << processMemoryUsage << " MB  |\n\n" << std::endl;

    system("cls"); // Clear the console screen
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "|         CPU          |         Memory        |          Swap          |" << "\n";
    std::cout << "|======================|=======================|========================|" << "\n";
    std::cout << "|  CPU Usage:   " << std::setw(5) << totalCpuUsage << "% |   Percent:    " << std::setw(5) << memoryUsagePerc
              << "%  |    Percent:    " << std::setw(5) << swapUsagePerc << "%  |" << std::endl;
    std::cout << "|  Idle Time:  " << std::setw(6) << idleTime << "% |  Used Mem: " << std::setw(6) << usedMemory << " MB  "
              << "|  Used Swap: " << std::setw(6) << usedSwap << " MB  |" << std::endl;
    std::cout << "|   CPU Freq: " << std::setw(4) << cpuFrequency << " MHz | Total Mem: " << std::setw(6) << totalMemory << " MB  "
              << "| Total Swap: " << std::setw(6) << totalSwap << " MB  |" << std::endl;
    std::cout << "|----------------------|-----------------------|------------------------|" << std::endl;
    std::cout << "| Per Core:            |" << std::endl;
    for (size_t i = 0; i < cpuUsagePerCore.size(); ++i) {
      std::cout << "| " << std::setw(2) << i + 1 << ": " << std::setw(6) << cpuUsagePerCore[i] << "%          |" << std::endl;
    }
    std::cout << "|----------------------|                            Exit: Ctrl + C" << std::endl;
    Sleep(750); // Update every 750ms
  }
  return 0;
}