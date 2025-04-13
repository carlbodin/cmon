#include "cmon.cpp"
#include "cmon_console.cpp"
#include "cmon_cpu_model_info.cpp"
#include "cmon_print.cpp"

#include <iostream>

void argParse(int argc, char *argv[], bool &useBar) {
  if (argc > 1) {
    std::string arg = argv[1];
    if (arg == "bar") {
      useBar = true;
    } else if (arg == "help") {
      printHelp();
      exit(0);
    } else {
      std::cerr << "Invalid option. Use 'help' for usage information.\n";
      exit(1);
    }
  }
}

int main(int argc, char *argv[]) {
  bool useBar = false;
  int width = 74, height = 20;
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  int cpuFrequency = 0;
  float totalCpuUsagePerc = 0.0, memoryUsagePerc = 0.0, swapUsagePerc = 0.0;
  float totalMemory = 0.0, usedMemory = 0.0, idleTime = 0.0;
  float totalSwap = 0.0, usedSwap = 0.0;
  std::vector<float> cpuUsagePerCore;
  std::string processorName;

  system("cls");
  argParse(argc, argv, useBar);

  cmon::CpuUsageMonitor cpuMonitor;
  short numberOfCores = cpuMonitor.counters.size();

  setupConsole(width, height, hConsole, numberOfCores, useBar);
  getCpuInfoDetails(processorName);

  printStatic(useBar, processorName);
  while (true) {
    cpuMonitor.getCpuUsage(cpuUsagePerCore, totalCpuUsagePerc, cpuFrequency, idleTime);
    cpuMonitor.getMemoryUsage(memoryUsagePerc, swapUsagePerc, totalMemory, usedMemory,
                              totalSwap, usedSwap);
    clearConsole(hConsole, useBar);
    update(useBar, processorName, totalCpuUsagePerc, memoryUsagePerc, swapUsagePerc,
           idleTime, cpuFrequency, totalMemory, usedMemory, totalSwap, usedSwap,
           cpuUsagePerCore, width);
    checkExitEvent();
  }
  return 0;
}