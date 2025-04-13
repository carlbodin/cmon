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

`MSYS2`, `MinGW`, `build-essentials`, `g++`, and `gdb`.
[Here](https://code.visualstudio.com/docs/cpp/config-mingw) is a guide on getting this
setup.

Dependencies `pdh.lib`, `wbemuuid.lib`, `ole32.lib`, and `oleaut32.lib` are already in
the Windows 11 environment. No [vcpkg](https://github.com/microsoft/vcpkg) environment
is needed.

### Build

**Optional:** If you want to add an icon, save it as .ico in the project. Since I use
MinGW, I need to compile it to `.res` using `windres` first. Then, add the `cmon.res`
input file to the build command.

```cmd
windres res/cmon.rc -O coff -o res/cmon.res
```

Build using the following command. Omit the `res/cmon.res` if you skip icon.

```cmd
x86_64-w64-mingw32-g++ -static -static-libgcc -static-libstdc++ -Ofast -o build/cmon.exe src/Main.cpp res/cmon.res -lpdh -lole32 -loleaut32 -lwbemuuid
```

**Links**

- The library `pdh.lib` is linked, which contains the Performance Data Helper (PDH) API
  functions.
- `wbemuuid.lib`, `ole32.lib`, `oleaut32.lib` are linked to include the WMI library, the
  COM library, and the OLE Automation library, which are used for querying the system
  for the processor's model name.
- The DLL's `libgcc` and `libstdc++` are needed, they are linked statically.
- Also, link other required static DLL's using `-static`.

If you want the program to ask for admin privileges automatically when running, which is
useful for automatic terminal window resizing, you can embed a "manifest" asking for
this by using the `mt` tool in the Windows SDK. Make sure to add its path to the user's
Path environment variable:
`C:\Program Files (x86)\Windows Kits\10\bin\<version>\x64\mt.exe`. Then you can run the
following.

```cmd
mt.exe -manifest res/cmon.manifest -outputresource:build\cmon.exe;1
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

### Clear Build Folder

To empty the build folder, run the following command. May be useful for debugging or
saving disk space.

```cmd
del build\*.exe
```

### Size of Binary

Check size of files in the build folder.

```cmd
dir build /o:-s
```
