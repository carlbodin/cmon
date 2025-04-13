#include "Cmon.cpp"
#include "CmonConsole.cpp"
#include "CmonCpuModelInfo.cpp"
#include "CmonPrint.cpp"

#include <iostream>

#pragma comment(lib, "psapi.lib") // Link the PSAPI library
#pragma comment(lib, "pdh.lib")

void ArgParse(int argc, char *argv[], bool &useBar) {
  useBar = false; // Default to false
  if (argc > 1) {
    std::string arg = argv[1];
    if (arg == "bar") {
      useBar = true;
    } else if (arg == "help") {
      PrintHelp();
      exit(0);
    } else {
      std::cerr << "Invalid option. Use 'help' for usage information.\n";
      exit(1);
    }
  }
}

int main(int argc, char *argv[]) {
  bool useBar;
  int width = 74, height = 20;
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  int cpuFrequency = 0;
  double idleTime = 0.0;
  double totalCpuUsagePerc = 0.0, memoryUsagePerc = 0.0, swapUsagePerc = 0.0;
  double totalMemory = 0.0, usedMemory = 0.0;
  double totalSwap = 0.0, usedSwap = 0.0;
  std::vector<double> cpuUsagePerCore;
  std::string processorName;

  system("cls");
  ArgParse(argc, argv, useBar);

  cmon::CpuUsageMonitor cpuMonitor;
  short numberOfCores = cpuMonitor.counters.size();

  SetupConsole(width, height, hConsole, numberOfCores, useBar);
  GetCpuInfoDetails(processorName);

  PrintStatic(useBar, processorName);
  while (true) {
    CheckExitEvent();
    cpuMonitor.GetCpuUsage(cpuUsagePerCore, totalCpuUsagePerc, cpuFrequency, idleTime);
    cmon::GetMemoryUsage(memoryUsagePerc, swapUsagePerc, totalMemory, usedMemory,
                         totalSwap, usedSwap);
    ClearConsole(hConsole, useBar);
    Update(useBar, processorName, totalCpuUsagePerc, memoryUsagePerc, swapUsagePerc,
           idleTime, cpuFrequency, totalMemory, usedMemory, totalSwap, usedSwap,
           cpuUsagePerCore, width);
  }
  return 0;
}