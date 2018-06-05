@ECHO OFF

REM set path
PATH=C:\Qt\4.8.4\bin;C:\sonicawe\lib\sonicawe-winlib\sonicawe_snapshot_win32_base;C:\sonicawe\src\release;%PATH%

IF NOT EXIST data\ClFftPrecision5.dat (
	IF NOT EXIST data\ClFftSizes.dat (
		ECHO No data, run FFTmoj first!
	) ELSE (
		REM process output with Octave, rename output
		call C:\octave\3.2.4_gcc-4.4.0\bin\octave.exe runPrecision.m
		IF EXIST data\ClFftPrecision1.dat (
			IF EXIST data\ClFftPrecision2.dat (
				IF EXIST data\ClFftPrecision3.dat (
					IF EXIST data\ClFftPrecision4.dat (
						move data\ClFftPrecision.dat data\ClFftPrecision5.dat
						move data\ClFftWallTimes.dat data\ClFftWallTimes5.dat
						move data\ClFftKExTimes.dat data\ClFftKExTimes5.dat
					) ELSE (
						move data\ClFftPrecision.dat data\ClFftPrecision4.dat
						move data\ClFftWallTimes.dat data\ClFftWallTimes4.dat
						move data\ClFftKExTimes.dat data\ClFftKExTimes4.dat
					)
				) ELSE (
					move data\ClFftPrecision.dat data\ClFftPrecision3.dat
					move data\ClFftWallTimes.dat data\ClFftWallTimes3.dat
					move data\ClFftKExTimes.dat data\ClFftKExTimes3.dat
				)
			) ELSE (
				move data\ClFftPrecision.dat data\ClFftPrecision2.dat
				move data\ClFftWallTimes.dat data\ClFftWallTimes2.dat
				move data\ClFftKExTimes.dat data\ClFftKExTimes2.dat
			)
		) ELSE (
			move data\ClFftPrecision.dat data\ClFftPrecision1.dat
			move data\ClFftWallTimes.dat data\ClFftWallTimes1.dat
			move data\ClFftKExTimes.dat data\ClFftKExTimes1.dat
		)
	)
	IF NOT EXIST data\ClFftPrecision5.dat (
		call runFFTmoj
	)
) ELSE (
	ECHO Nothing to do!
)

pause