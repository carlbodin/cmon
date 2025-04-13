#include <pdh.h>
#include <string>
#include <vector>

#pragma comment(lib, "pdh.lib")

namespace cmon {

PDH_FMT_COUNTERVALUE counterVal;
MEMORYSTATUSEX memStatus;
constexpr double BYTE_TO_MB = 1.0 / (1024.0 * 1024.0);

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
      wchar_t counterPath[256];
      swprintf(counterPath, 256, L"\\Processor(%lu)\\%% Processor Time", i);
      PdhAddCounterW(query, counterPath, 0, &counter);
      counters.push_back(counter);
    }

    // Add counters for total CPU usage, frequency, and idle time
    PdhAddCounterW(query, L"\\Processor(_Total)\\% Processor Time", 0, &totalCounter);
    PdhAddCounterW(query, L"\\Processor Information(_Total)\\Processor Frequency", 0,
                   &cpuFrequencyCounter);
    PdhAddCounterW(query, L"\\Processor(_Total)\\% Idle Time", 0, &idleTimeCounter);
  }

  ~CpuUsageMonitor() { PdhCloseQuery(query); }

  void GetCpuUsage(std::vector<float> &cpuUsagePerCore, float &totalCpuUsage,
                   int &cpuFrequency, float &idleTime) {

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

  void GetMemoryUsage(float &memoryUsagePerc, float &swapUsagePerc, float &totalMemory,
                      float &usedMemory, float &totalSwap, float &usedSwap) {

    memStatus.dwLength = sizeof(memStatus);

    if (GlobalMemoryStatusEx(&memStatus)) {
      usedMemory = static_cast<float>(memStatus.ullTotalPhys - memStatus.ullAvailPhys) *
                   BYTE_TO_MB;
      totalMemory = static_cast<float>(memStatus.ullTotalPhys) * BYTE_TO_MB;
      memoryUsagePerc = (usedMemory / totalMemory) * 100.0;

      if (memStatus.ullTotalPageFile > memStatus.ullTotalPhys) {
        totalSwap =
            static_cast<float>(memStatus.ullTotalPageFile - memStatus.ullTotalPhys) *
            BYTE_TO_MB;
        usedSwap =
            static_cast<float>((memStatus.ullTotalPageFile - memStatus.ullTotalPhys) -
                               (memStatus.ullAvailPageFile - memStatus.ullAvailPhys)) *
            BYTE_TO_MB;
        swapUsagePerc = (usedSwap / totalSwap) * 100.0;
      } else {
        totalSwap = static_cast<float>(memStatus.ullTotalPageFile) * BYTE_TO_MB;
        usedSwap = static_cast<float>(memStatus.ullTotalPageFile -
                                      memStatus.ullAvailPageFile) *
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
};

} // namespace cmon