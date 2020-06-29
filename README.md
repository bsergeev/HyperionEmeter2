# HyperionEmeter2
Software for working with Hyperion Emeter 2 sensors: RCU, MCU, and LCU.  

Requires Qt. 
QTDIR environment variable sould point at the base of the Qt installation, the folder containing _bin_, _lib_, etc., and Qt5_DIR should point at the location of Qt CMake files, e.g.:
```
Qt5_DIR=c:/Frameworks/Qt/5.14.1/msvc2017_64/lib/cmake/Qt5
QTDIR=c:/Frameworks/Qt/5.14.1/msvc2017_64  
```

To build the application, run _build.sh_ (in Windows, do this from the Git Bash shell). 

*Pre-built Windows 64-bit binaries can be downloaded from _build/Win64_ folder. If you encounter errors about missing C/C++ runtime DLLs, download them from Microsoft.*