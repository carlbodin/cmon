# CPU Monitor

A lightweight command line hardware monitor, developed on Windows in C++ for Windows 11.
Uses 10 MB of RAM and has a tiny CPU footprint.

```plaintext
|         CPU          |         Memory        |          Swap          |
|======================|=======================|========================|
|  CPU Usage:    13.3% |   Percent:     42.7%  |    Percent:     61.9%  |
|  Idle Time:    85.2% |  Used Mem:   6965 MB  |  Used Swap:   1426 MB  |
|   CPU Freq: 3600 MHz | Total Mem:  16307 MB  | Total Swap:   2304 MB  |
|----------------------|-----------------------|------------------------|
| Per Core:            |
|  1:   23.3%          |
|  2:   17.1%          |
|  3:   25.4%          |
|  4:   15.0%          |
|  5:   10.9%          |
|  6:   21.3%          |
|  7:    4.7%          |
|  8:    8.8%          |
|  9:    8.8%          |
| 10:    4.7%          |
| 11:   10.9%          |
| 12:    8.8%          |
|----------------------|                            Exit: Ctrl + C
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
x86_64-w64-mingw32-g++ -static -static-libgcc -static-libstdc++ -Ofast -o build/cmon.exe cmon.cpp cmon.res -lpdh
```

Embed the manifest asking for admin privileges using `mt` in the Windows SDK. Make sure
to add its path to the user's Path environment variable:
`C:\Program Files (x86)\Windows Kits\10\bin\<version>\x64\mt.exe`. Then you can run the
following.

```cmd
mt.exe -manifest cmon.manifest -outputresource:build\cmon.exe;1
```

To add an icon, save it as .ico in the project. Since I use MinGW, I need to compile it
to `.res` using `windres` first. Then, add the `cmon.res` input file to the build
command.

```cmd
windres resources/cmon.rc -O coff -o cmon.res
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
copy /Y build\cmon.exe C:\Users\carl\bin
```

Check size of files in build folder.

```cmd
size -d -t build/*
```
