#include <iomanip> // Add this include for std::setprecision
#include <iostream>
#include <pdh.h>
#include <string>
#include <vector>
#include <windows.h>

#pragma comment(lib, "pdh.lib")

void GetCpuUsagePerCore(std::vector<double> &cpuUsagePerCore,
                        double &totalCpuUsage) {
  PDH_HQUERY query;
  std::vector<PDH_HCOUNTER> counters;
  PDH_HCOUNTER totalCounter;
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
    std::wstring counterPath =
        L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
    PdhAddCounterW(query, counterPath.c_str(), 0, &counter);
    counters.push_back(counter);
  }

  // Add a counter for total CPU usage
  PdhAddCounterW(query, L"\\Processor(_Total)\\% Processor Time", 0,
                 &totalCounter);

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

  // Close the query
  PdhCloseQuery(query);
}

int main() {
  while (true) {
    std::vector<double> cpuUsagePerCore;
    double totalCpuUsage = 0.0;

    GetCpuUsagePerCore(cpuUsagePerCore, totalCpuUsage);

    system("cls"); // Clear the console screen
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "===================" << "\n";
    std::cout << "|" << "CPU Usage: " << std::setw(4) << totalCpuUsage << "% |"
              << std::endl;
    std::cout << "|" << "-----------------|" << std::endl;
    std::cout << "|" << "Per Core:        |" << std::endl;
    for (size_t i = 0; i < cpuUsagePerCore.size(); ++i) {
      // Set fixed-point notation and 1 decimal place
      std::cout << "|" << std::setw(2) << i << ": " << std::setw(4)
                << cpuUsagePerCore[i] << "%        |" << std::endl;
    }
    std::cout << "===================" << "\n\n";
    std::cout << "Press Ctrl+C to exit." << std::endl;
    Sleep(750); // Update every second
  }
  return 0;
}