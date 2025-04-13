#include <iomanip>
#include <iostream>
#include <vector>

void PrintHelp() {
  std::cout << "Usage: cmon [bar|help]\n\n";
  std::cout << "A lightweight CPU monitoring program in terminal for Windows.\n\n";
  std::cout << "Options:\n";
  std::cout << "  bar   : Display info in bar graphs.\n";
  std::cout << "  help  : Display this help message.\n";
}

void PrintStatic(bool useBar, std::string &processorName) {
  if (useBar) {
    std::cout << processorName << std::endl;
  } else {
    std::cout << "|         CPU          |         Memory        |          Swap       "
                 "   |\n";
    std::cout << "|======================|=======================|====================="
                 "===|\n";
  }
}

const void PrintBar(const std::string &name, const double usagePercent,
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

void UpdateContentBar(const double &totalCpuUsagePerc, const double &memoryUsagePerc,
                      const double &swapUsagePerc, const int &width) {
  PrintBar(" CPU", totalCpuUsagePerc, width - 1);
  PrintBar(" Mem", memoryUsagePerc, width - 1);
  PrintBar("Swap", swapUsagePerc, width - 1);
}

void UpdateContent(std::string &processorName, const double &totalCpuUsagePerc,
                   const double &memoryUsagePerc, const double &swapUsagePerc,
                   const double &idleTime, const int &cpuFrequency,
                   const double &totalMemory, const double &usedMemory,
                   const double &totalSwap, const double &usedSwap,
                   const std::vector<double> &cpuUsagePerCore) {
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
  std::cout << "|----------------------|-----------------------|---------------------"
               "---|\n";
  std::cout << "| Per Core:            | " << processorName << "\n";
  for (size_t i = 0; i < cpuUsagePerCore.size(); ++i) {
    std::cout << "| " << std::setw(2) << i + 1 << ": " << std::setw(6)
              << cpuUsagePerCore[i] << "%          |\n";
  }
  std::cout << "|----------------------|" << std::endl;
}

void Update(bool useBar, std::string &processorName, const double &totalCpuUsagePerc,
            const double &memoryUsagePerc, const double &swapUsagePerc,
            const double &idleTime, const int &cpuFrequency, const double &totalMemory,
            const double &usedMemory, const double &totalSwap, const double &usedSwap,
            const std::vector<double> &cpuUsagePerCore, int width) {
  // Update the console with the new information.
  if (useBar) {
    UpdateContentBar(totalCpuUsagePerc, memoryUsagePerc, swapUsagePerc, width);
  } else {
    UpdateContent(processorName, totalCpuUsagePerc, memoryUsagePerc, swapUsagePerc,
                  idleTime, cpuFrequency, totalMemory, usedMemory, totalSwap, usedSwap,
                  cpuUsagePerCore);
  }
}