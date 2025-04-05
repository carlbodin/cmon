#include <iomanip> // Add this include for std::setprecision
#include <iostream>
#include <pdh.h>
#include <string>
#include <vector>
#include <windows.h>

#pragma comment(lib, "pdh.lib")

void GetCpuUsagePerCore(std::vector<double> &cpuUsagePerCore) {
  PDH_HQUERY query;
  std::vector<PDH_HCOUNTER> counters;
  PDH_FMT_COUNTERVALUE counterVal;

  // Use 0 instead of NULL for DWORD_PTR arguments
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

  PdhCollectQueryData(query);
  Sleep(500);
  PdhCollectQueryData(query);

  cpuUsagePerCore.clear();
  for (const auto &counter : counters) {
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterVal);
    cpuUsagePerCore.push_back(counterVal.doubleValue);
  }

  PdhCloseQuery(query);
}

int main() {
  while (true) {
    std::vector<double> cpuUsagePerCore;
    GetCpuUsagePerCore(cpuUsagePerCore);

    system("cls"); // Clear the console screen
    std::cout << "CPU Usage Per Core:" << std::endl;
    for (size_t i = 0; i < cpuUsagePerCore.size(); ++i) {
      // Set fixed-point notation and 1 decimal place
      std::cout << std::fixed << std::setprecision(1);
      std::cout << std::setw(2) << i << ": " << cpuUsagePerCore[i] << "%"
                << std::endl;
    }

    Sleep(500); // Update every second
  }
  return 0;
}