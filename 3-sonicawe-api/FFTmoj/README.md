# FFTmoj

This test benchmarks FFT library performance and accuracy.

## Building

To build and run, you will need

- Windows 8
- Git BASH
- Qt 4.8.4
- Visual C++ 2008 Express

For OpenCL libraries, you will also need

- OpenCL (1.2) compatible hardware
- AMD APP SDK 2.8 for x64
- clAmdFft 1.8.239

Other versions may also work, but have not been tested.

After installing the above, `Qt\bin` needs to be in `PATH`.
For OpenCL, the environment variable `AMDAPPSDKROOT` needs to be defined (typically, `C:\Program Files (x86)\AMD APP\`)

First, run one of the following to create (and overwrite) the appropriate `vcproj` files:

- `makeamd.sh` for ClAmdFft
- `makeapple.sh` for ClFft
- `makecpu.sh` for Ooura

Then, open the solution file in Visual C++ 2008 and build. Copy the file `sonicawe.dll` from `..\..\..\..\src\release` into

- `release\opencl\amdamd`
- `release\opencl\amdapple`, or
- `release\cpu`,

respectively.

TODO: Put as much of this as possible into makefiles or scripts
TODO: Why the naming inconsistency?

Finally, the environment variable `MACHINE` needs to be set, in order to help identify the origin of the benchmark results.

## Running

Launch the program with one of

- `runclamdfft.bat`
- `runclfft.bat`
- `runfft.bat`

TODO: Again with the naming inconsistency
