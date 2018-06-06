@ECHO OFF

mkdir data
mkdir data\Ooura
mkdir data\Ooura\run1
mkdir data\Ooura\run2
mkdir data\Ooura\run3
mkdir data\Ooura\run4
mkdir data\Ooura\run5
mkdir data\ClFft
mkdir data\ClFft\run1
mkdir data\ClFft\run2
mkdir data\ClFft\run3
mkdir data\ClFft\run4
mkdir data\ClFft\run5
mkdir data\ClAmdFft
mkdir data\ClAmdFft\run1
mkdir data\ClAmdFft\run2
mkdir data\ClAmdFft\run3
mkdir data\ClAmdFft\run4
mkdir data\ClAmdFft\run5

REM set path
PATH=C:\Qt\4.8.4\bin;C:\sonicawe\lib\sonicawe-winlib\sonicawe_snapshot_win32_base;C:\sonicawe\src\release;%PATH%

REM call FFTmoj 5 times
call release\cpu\FFTmoj.exe
call release\cpu\FFTmoj.exe
call release\cpu\FFTmoj.exe
call release\cpu\FFTmoj.exe
call release\cpu\FFTmoj.exe
pause