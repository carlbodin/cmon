# cmon

A lightweight CPU monitoring program in terminal for Windows. Uses 4.2 MB of RAM, in
addition to the RAM of your terminal window, and has a negligable CPU footprint.

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

Run with argument `bar` to see this instead.

```plaintext
 CPU [██                                                              3%]
 Mem [████████████████████████████                                   43%]
Swap [███████████████████████████████████████████████████            77%]
```

Run `help` to see program instructions.

```plaintext
Usage: cmon [bar|help]

A lightweight CPU monitoring program for Windows 11 running in terminal.

Options:\carl\git\cmon>
  bar   : Display info in bar graphs.
  help  : Display this help message.
```

## Usage

Download the executable from
[Releases](https://github.com/carlbodin/cmon/releases/latest), or compile it from the
source code, and run it. Since the executable is unsigned, the popup
`Windows protected your PC` will appear at first launch. Exit with `Ctrl + C`.

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
x86_64-w64-mingw32-g++ -static -static-libgcc -static-libstdc++ -Ofast -o build/cmon.exe cmon.cpp cmon.res -lpdh
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

Run the program by using this command while the `cmon.exe` is on the path.

```cmd
cmon
```

Append command line argument `bar` to see a bar representation instead.

```cmd
cmon bar
```

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
