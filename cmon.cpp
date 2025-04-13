#include <conio.h> // For _kbhit() and _getch()
#include <cstdlib>
#include <iostream>
#include <pdh.h>
#include <psapi.h> // For GetProcessMemoryInfo
#include <string>
#include <vector>
#include <wincrypt.h>
#include <winscard.h>

#pragma comment(lib, "psapi.lib") // Link the PSAPI library
#pragma comment(lib, "pdh.lib")

namespace cmon {

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
    memoryUsagePerc = 0.0;
    swapUsagePerc = 0.0;
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
}; // namespace cmon