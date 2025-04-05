# CPU Monitor

A lightweight command line hardware monitor, developed on Windows in C++ for Windows 11.
Uses 28 MB of RAM and has a negligible CPU footprint.

```cmd
CPU Usage Per Core:
 0: 0.0%
 1: 2.3%
 2: 0.0%
 3: 0.0%
 4: 0.0%
 5: 5.3%
 6: 2.3%
 7: 0.0%
 8: 0.0%
 9: 0.0%
10: 0.0%
11: 0.0%
```

## Usage

Add to path in your user environment variables and run in `Command Prompt`.

```cmd
cmon
```

Exit with `Ctrl + C`.

## Development Setup

### Installation

`MSYS2`

`MinGW`

`g++` and `gdb`

Dependency `pdh.lib` already in win11 environment.

### Build

Build binary and link `pdh.lib` library, which contains the Performance Data Helper
(PDH) API functions.

```cmd
x86_64-w64-mingw32-g++ -Os -o build/cmon.exe cmon.cpp -lpdh
```

Run program in separate cmd instance.

```cmd
start build\cmon
```

Empty build folder.

```cmd
del build\*.exe
```

Deploy to bin folder that I have on path.

```cmd
copy build\cmon.exe C:\Users\carl\bin
```

Check size of files in build folder.

```cmd
size -d -t build/*
```
