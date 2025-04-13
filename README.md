# cmon

A lightweight CPU monitoring program in terminal for Windows. Uses 4.9 MB of RAM, in
addition to the RAM of your terminal window, and has a negligable CPU footprint.

```plaintext
|         CPU          |         Memory        |          Swap          |
|======================|=======================|========================|
|  CPU Usage:     2.3% |   Percent:     39.4%  |    Percent:     71.6%  |
|  Idle Time:    98.1% |  Used Mem:   6426 MB  |  Used Swap:   1742 MB  |
|   CPU Freq: 3574 MHz | Total Mem:  16307 MB  | Total Swap:   2432 MB  |
|----------------------|-----------------------|------------------------|
| Per Core:            | AMD Ryzen 5 3600 6-Core Processor
|  1:    0.5%          |
|  2:    2.0%          |
|  3:    0.5%          |
|  4:    8.2%          |
|  5:    2.0%          |
|  6:    5.1%          |
|  7:    2.0%          |
|  8:    2.0%          |
|  9:    2.0%          |
| 10:    2.0%          |
| 11:    0.5%          |
| 12:    0.5%          |
|----------------------|
```

Run with argument `bar` to see this instead.

```plaintext
AMD Ryzen 5 3600 6-Core Processor
 CPU [██                                                              3%]
 Mem [████████████████████████████                                   43%]
Swap [███████████████████████████████████████████████████            77%]
```

Run with argument `help` to see program instructions.

```plaintext
Usage: cmon [bar|help]

A lightweight CPU monitoring program in terminal for Windows.

Options:
  bar   : Display info in bar graphs.
  help  : Display this help message.
```

## Usage

Download the executable from
[Releases](https://github.com/carlbodin/cmon/releases/latest), or compile it from the
source code, and run it. Since the executable is unsigned, the popup
`Windows protected your PC` will appear at first launch.

## Development Setup

### Installation

`MSYS2`, `MinGW`, `build-essentials`, `g++`, and `gdb`. See
[guide](https://code.visualstudio.com/docs/cpp/config-mingw).

Dependency `pdh.lib` already in the Windows 11 environment.

### Build

**Optional:** If you want to add an icon, save it as .ico in the project. Since I use
MinGW, I need to compile it to `.res` using `windres` first. Then, add the `cmon.res`
input file to the build command.

```cmd
windres resources/cmon.rc -O coff -o cmon.res
```

Build binary and link `pdh.lib` library, which contains the Performance Data Helper
(PDH) API functions. Also, link `libgcc` and `libstdc++` DLL's explicitly. Link other
required DLL's using `-static`.

```cmd
x86_64-w64-mingw32-g++ -static -static-libgcc -static-libstdc++ -Ofast -o build/cmon.exe Main.cpp cmon.res -lpdh -lole32 -loleaut32 -lwbemuuid
```

If you want the program to ask for admin privileges automatically when running, you can
embed a "manifest" asking for this by using the `mt` tool in the Windows SDK. Make sure
to add its path to the user's Path environment variable:
`C:\Program Files (x86)\Windows Kits\10\bin\<version>\x64\mt.exe`. Then you can run the
following.

```cmd
mt.exe -manifest cmon.manifest -outputresource:build\cmon.exe;1
```

Check your user's PATH environment variable.

```cmd
reg query "HKCU\Environment" /v Path
```

### Run

Run the program directly from the build folder in separate cmd instance.

```cmd
start build\cmon
```

### Deploy

Copy the program to a folder that I have on my user's PATH.

```cmd
copy /Y build\cmon.exe C:\Users\carl\bin
```

### Miscellaneous

#### Clear Build Folder

To empty the build folder, run the following command. May be useful for debugging or
saving disk space.

```cmd
del build\*.exe
```

#### Check Size of Build

Check size of files in the build folder.

```cmd
dir build /o:-s
```
