@ECHO OFF

REM set path
PATH=C:\Qt\4.8.4\bin;C:\sonicawe\lib\sonicawe-winlib\sonicawe_snapshot_win32_base;C:\sonicawe\src\release;%PATH%

REM call FFTmoj, process output with Octave, rename output
call release\FFTmoj.exe
call C:\octave\3.2.4_gcc-4.4.0\bin\octave.exe runPrecision.m
move data\OouraPrecision.dat data\OouraPrecision1.dat
move data\OouraWallTimes.dat data\OouraWallTimes1.dat
call release\FFTmoj.exe
call C:\octave\3.2.4_gcc-4.4.0\bin\octave.exe runPrecision.m
move data\OouraPrecision.dat data\OouraPrecision2.dat
move data\OouraWallTimes.dat data\OouraWallTimes2.dat
call release\FFTmoj.exe
call C:\octave\3.2.4_gcc-4.4.0\bin\octave.exe runPrecision.m
move data\OouraPrecision.dat data\OouraPrecision3.dat
move data\OouraWallTimes.dat data\OouraWallTimes3.dat
call release\FFTmoj.exe
call C:\octave\3.2.4_gcc-4.4.0\bin\octave.exe runPrecision.m
move data\OouraPrecision.dat data\OouraPrecision4.dat
move data\OouraWallTimes.dat data\OouraWallTimes4.dat
call release\FFTmoj.exe
call C:\octave\3.2.4_gcc-4.4.0\bin\octave.exe runPrecision.m
move data\OouraPrecision.dat data\OouraPrecision5.dat
move data\OouraWallTimes.dat data\OouraWallTimes5.dat

pause