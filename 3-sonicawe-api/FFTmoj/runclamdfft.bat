@ECHO OFF

mkdir data
mkdir data\Rampage\rerun1\Ooura
mkdir data\Rampage\rerun1\Ooura\run1
mkdir data\Rampage\rerun1\Ooura\run2
mkdir data\Rampage\rerun1\Ooura\run3
mkdir data\Rampage\rerun1\Ooura\run4
mkdir data\Rampage\rerun1\Ooura\run5
mkdir data\Rampage\rerun2\Ooura
mkdir data\Rampage\rerun2\Ooura\run1
mkdir data\Rampage\rerun2\Ooura\run2
mkdir data\Rampage\rerun2\Ooura\run3
mkdir data\Rampage\rerun2\Ooura\run4
mkdir data\Rampage\rerun2\Ooura\run5
mkdir data\Rampage\rerun1\ClFft
mkdir data\Rampage\rerun1\ClFft\run1
mkdir data\Rampage\rerun1\ClFft\run2
mkdir data\Rampage\rerun1\ClFft\run3
mkdir data\Rampage\rerun1\ClFft\run4
mkdir data\Rampage\rerun1\ClFft\run5
mkdir data\Rampage\rerun2\ClFft
mkdir data\Rampage\rerun2\ClFft\run1
mkdir data\Rampage\rerun2\ClFft\run2
mkdir data\Rampage\rerun2\ClFft\run3
mkdir data\Rampage\rerun2\ClFft\run4
mkdir data\Rampage\rerun2\ClFft\run5
mkdir data\Rampage\rerun1\ClAmdFft
mkdir data\Rampage\rerun1\ClAmdFft\run1
mkdir data\Rampage\rerun1\ClAmdFft\run2
mkdir data\Rampage\rerun1\ClAmdFft\run3
mkdir data\Rampage\rerun1\ClAmdFft\run4
mkdir data\Rampage\rerun1\ClAmdFft\run5
mkdir data\Rampage\rerun2\ClAmdFft
mkdir data\Rampage\rerun2\ClAmdFft\run1
mkdir data\Rampage\rerun2\ClAmdFft\run2
mkdir data\Rampage\rerun2\ClAmdFft\run3
mkdir data\Rampage\rerun2\ClAmdFft\run4
mkdir data\Rampage\rerun2\ClAmdFft\run5

REM set path
PATH=C:\Qt\4.8.4\bin;C:\sonicawe\lib\sonicawe-winlib\sonicawe_snapshot_win32_base;C:\sonicawe\lib\sonicawe-winlib\clamdfft;C:\sonicawe\src\release;%PATH%

REM call FFTmoj 5 times
echo Rampage > argv.txt
echo 2 >> argv.txt
echo 3 >> argv.txt
echo bench >> argv.txt
call release\opencl\amdamd\FFTmoj.exe
pause