@ECHO OFF

REM set path
PATH=C:\Qt\4.8.4\bin;C:\sonicawe\lib\sonicawe-winlib\sonicawe_snapshot_win32_base;C:\sonicawe\src\release;%PATH%

IF NOT EXIST data\ClAmdFftPrecision5.dat (
	IF NOT EXIST data\ClAmdFftSizes.dat (
		ECHO No data, run FFTmoj first!
	) ELSE (
		REM process output with Octave, rename output
		call C:\octave\3.2.4_gcc-4.4.0\bin\octave.exe runPrecision.m
		IF EXIST data\ClAmdFftPrecision1.dat (
			IF EXIST data\ClAmdFftPrecision2.dat (
				IF EXIST data\ClAmdFftPrecision3.dat (
					IF EXIST data\ClAmdFftPrecision4.dat (
						move data\ClAmdFftPrecision.dat data\ClAmdFftPrecision5.dat
						move data\ClAmdFftWallTimes.dat data\ClAmdFftWallTimes5.dat
						move data\ClAmdFftKExTimes.dat data\ClAmdFftKExTimes5.dat
					) ELSE (
						move data\ClAmdFftPrecision.dat data\ClAmdFftPrecision4.dat
						move data\ClAmdFftWallTimes.dat data\ClAmdFftWallTimes4.dat
						move data\ClAmdFftKExTimes.dat data\ClAmdFftKExTimes4.dat
					)
				) ELSE (
					move data\ClAmdFftPrecision.dat data\ClAmdFftPrecision3.dat
					move data\ClAmdFftWallTimes.dat data\ClAmdFftWallTimes3.dat
					move data\ClAmdFftKExTimes.dat data\ClAmdFftKExTimes3.dat
				)
			) ELSE (
				move data\ClAmdFftPrecision.dat data\ClAmdFftPrecision2.dat
				move data\ClAmdFftWallTimes.dat data\ClAmdFftWallTimes2.dat
				move data\ClAmdFftKExTimes.dat data\ClAmdFftKExTimes2.dat
			)
		) ELSE (
			move data\ClAmdFftPrecision.dat data\ClAmdFftPrecision1.dat
			move data\ClAmdFftWallTimes.dat data\ClAmdFftWallTimes1.dat
			move data\ClAmdFftKExTimes.dat data\ClAmdFftKExTimes1.dat
		)
	)
	IF NOT EXIST data\ClAmdFftPrecision5.dat (
		call runFFTmoj
	)
) ELSE (
	ECHO Nothing to do!
)

pause