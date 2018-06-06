/*
TODO:

- Fixa preprocessorflaggor och defines f�r allt m�jligt som ska kunna variera:
     - Placeness (f�r testet allts�. F�r clamdfft kan l�ggas till ifsats f�r om input och output �r samma)
     - Upprepningar
     - Profiling och inte
     - Alla storlekar eller bara 2-potenser
     (- Om answers ska l�sas fr�n fil eller r�knas ut av fftooura?)
- Fixa till utskrifterna s� de ser prydliga ut
- Fixa wall clock time-tester
- Fixa s� att inl�sning och utskrift sker fr�n och till data-mappen
- Allokera allt minne fr�n b�rjan (f�r varje storlek) istf att anv�nda samma minne om och om igen i test 3
- Fixa tidtagning av planskapandet
- Snygga till testen generellt
- Implementera resten av compute-funktionerna f�r clAmdFft
     - allting clAmdFft har dock l�gre prio just nu, det viktigaste �r att testen g�r att k�ra p� nVidia-burkar
*/

/*TODO2:

Vad ska egentligen h�nda i det h�r testet?

- Ta reda p� vilka storlekar som finns, och spara till fil (test1, finns redan)
- F�r alla storlekar i filen, k�r en batchad FFT p� 128MB data en g�ng och j�mf�r resultatet med Ooura
- F�r alla storlekar i filen, k�r en batchad FFT p� 128MB n�got antal g�nger och m�t hur l�ng tid det tar
- Alla dessa g�nger, m�t kernel-bakningstiden
- Se till att padding skapas n�r det beh�vs

*/

/*TODO3:

Kolla upp om saker och ting verkligen �r inplace eller inte.

Fixa problemet att plans inte kastas bort utan fyller upp minnet tills det tar slut

Ta bort alla test som inte l�ngre beh�vs och g�r ett test som g�r typ allting:
	L�s in alla storlekar fr�n filen och spara i en vektor s� att man kan g�ra "progress-m�tare" (i/n och % klart)
	Skapa randomdata och spara till h5
	For varje storlek, 
		Walltime:
			skapa plans
			baka plans
			k�r FFT p� randomdata
		Walltime 2:
			k�r FFT p� randomdata
		Spara kernelexectid i n�n vektor
		Spara resultat till h5
	Spara kernelexectider till fil

Framtidsplaner: g�r ett test som testar, f�r alla en storlek och alla batchstorlekar fr�n 1 till s� m�nga som g�r, 
och m�t tid f�r varje batchstorlek. N�r allt funkar, g�r s� att testet k�r p� alla storlekar ist�llet.
( f�r varje storlek finns f�rmodligen en optimal storlek f�r batchen f�r en viss h�rdvara, och den �r nog != max.
  Finns det flera, lokala max, eller finns det bara en? Testa f�rst f�r tv�potenser och kolla om det finns n�got m�nster! )
G�r p� samma s�tt ett annat test som kollar vilken batchstorlek som ger b�st wall clock time f�r input om 128MB data.

*/ 

#define NOGUI
#ifdef _MSC_VER
#include <stdlib.h> //  error C2381: 'exit' : redefinition; __declspec(noreturn) differs
#include <time.h>
#include "TaskTimer.h"
#endif

#define TIME_STFT

#define GENERATESIZEVECTOR
#define READSIZEVECTOR
//#define RUNTEST10
//#define RUNTEST13
//#define RUNTEST14
#define PLACENESS "inplace"
#define FFTINPLACE
#define CL_PROFILING
#define ONLYPOWERSOF2

#include "tfr/fftimplementation.h"
#include "tfr/fftcufft.h"
#ifdef USE_OPENCL
    #ifdef USE_AMD
        #include "tfr/clamdfft/fftclamdfft.h"
    #else
	#define USE_APPLE
        #include "tfr/clfft/fftclfft.h"
	#endif
#else
    #include "tfr/fftooura.h"
#endif

#define startSize 1<<8 // 2 ^ 8, 750000
#ifdef USE_APPLE
#define endSize 1<<21
#else
#define endSize 1<<22 // 2 ^ 22
#endif

#include "exceptionassert.h"


//#include "sawe/project_header.h"
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <fstream>
#include <iostream>
#include "adapters/hdf5.h"
#include "signal/buffer.h"
#include "tfr/stft.h"

using namespace std;
using namespace Signal;
using namespace Adapters;
using namespace Tfr;

class FFTmojTest : public QObject
{
    Q_OBJECT

public:
    FFTmojTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
	void generateRandomData();
	void generateSizeVector();
	void readSizeVector();
	void testCase10(); // , create input vectors, run fft, store results in files.
	void testCase13(); // Benchmark, for all batch sizes of a given size, the kernel execution time.
	void testCase14(); // Benchmark wall-time, bake time, kernel execution time, store first FFT result.

private:
	void getSizesFromFile(std::vector<int> *sizes, int *sumsizes);
	string techlib;
	int maxsize, sizesum;
	std::vector<int> sizes;
	int seedVal;
	int run;

#ifdef USE_OPENCL
    #ifdef USE_AMD
        FftClAmdFft fft;
    #else
        FftClFft fft;
    #endif
#elif USE_CUDA
    FftCufft fft;
#else
    FftOoura fft;
#endif

};

FFTmojTest::FFTmojTest()
{
    ExceptionAssert::installEventHandler();
}

/*
#include "cudaPitchedPtrType.h"
#include "CudaException.h"

void FFTmojTestCuda( cudaPitchedPtrType<float> data );
*/

void FFTmojTest::initTestCase()
{
#ifdef USE_OPENCL
    #ifdef USE_AMD
        techlib = "ClAmdFft";
    #else
        techlib = "ClFft";
    #endif
#elif USE_CUDA
    techlib = "CuFft";
#else
    techlib = "Ooura";
#endif
	sizesum = maxsize = seedVal = run = 0;
	seedVal = time(0);
}

void FFTmojTest::cleanupTestCase()
{
}

void FFTmojTest::generateRandomData()
{
	// Make 5 random vectors with the seeds 1..5,
	// store them in data/RandomFile#.h5,
	// but don't overwrite if exist
	
	cout << "About to generate random data..." << endl;
	
	for (int i = 1; i <= 5; i++)
	{
		char randomfilename[100];
		sprintf(randomfilename, "data/RandomData%d.h5", i);

		cout << "Generating " << randomfilename << "... " << flush;
		
		ifstream infile(randomfilename);
		if (infile)
		{
			cout << "already exists, skipping" << endl;
			continue;
		}

		srand(i);

		int maxSize = 1 << 22;
		
		float tempfloatr;
			
		ChunkData::Ptr data;
		data.reset(new ChunkData(maxSize));
		complex<float> *p = data->getCpuMemory();

		for (int j = 0; j < maxSize; j++)
		{
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].real(tempfloatr);
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].imag(tempfloatr);
		}
		
		Tfr::pChunk chunk( new Tfr::StftChunk(maxSize, Tfr::StftParams::WindowType_Rectangular, 0, true));

		chunk->transform_data = data;

		Hdf5Chunk::saveChunk( randomfilename, *chunk);
		
		cout << "done." << endl;
	}
}

void FFTmojTest::generateSizeVector()
{
	// Get sizes in an interval for current library and store in file
#ifdef GENERATESIZEVECTOR
	char sizefilename[100];
	sprintf(sizefilename, "data/%s/Sizes.dat", techlib.c_str());
	ofstream outputfile(sizefilename);

	int sumSize = 0;
	int numSize = 0;
	int i = startSize;
	while (i <= endSize)
	{
		sumSize += i;
		numSize++;

		outputfile << i;
		if (i != endSize)
			outputfile << "\n";
#ifdef ONLYPOWERSOF2
			i = i * 2;
#else
			i = fft.sChunkSizeG(i);
#endif
	}

	outputfile.close();

	cout << "Number of sizes: " << numSize << "\n";
	cout << "Sum of sizes: " << sumSize << "\n";

#endif
}

void FFTmojTest::readSizeVector()
{
	// Read sizes from file
#ifdef READSIZEVECTOR
	char sizefilename[100];
	sprintf(sizefilename, "data/%s/Sizes.dat", techlib.c_str());
	ifstream sizefile(sizefilename);

	int size = 0, prevsize = 0;
	
	while (sizefile.good())
	{
		sizefile >> size;
		if (prevsize == size) 
		{
			break;
		}
		else 
		{
			prevsize = size;
			sizes.push_back(size);
			sizesum += size;
		}
	}

	sizefile.close();
	maxsize = sizes.back();
#ifdef ONLYPOWERSOF2
	int i = i / 2;
#else
	int i = fft.lChunkSizeS(startSize);
#endif
	sizes.push_back(i);
	reverse(sizes.begin(), sizes.end());

	//sizes._Reverse(sizes.begin(), sizes.end());

	
	printf("FYI first size: %d, last size: %d, # of sizes: %d\n", sizes.front(), sizes.back(), sizes.size());

#endif
}

void FFTmojTest::testCase10()
{
#ifdef RUNTEST10
	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	std::vector<int> sizevec;
	int size = 0;
	int i = 0;

	int sumsize = 0;
	while (sizes.good())
	{
		sizes >> size;
		if (i == size)
			break;
		i = size;
		sizevec.push_back(size);
		sumsize+=size;
	}
	cout << "FYI sizevec[0] = " << sizevec[0] << ", size = " << sizevec.size() << endl;
	sizes.close();
	
	int numsize = sizevec.size();

	#ifdef USE_AMD
	ostringstream bakefile;
	bakefile << "data/" << techlib << "BakeTimes" << ".dat";
	ofstream baketimes(bakefile.str().c_str());
	#endif

	size = 0;

	int sizeacc = 0;
	float bakeacc = 0;
	
	
	TIME_STFT TaskTimer ttx("Running FFT!");
	for (i = 0; i < sizevec.size(); i++)
	{
		#ifdef USE_AMD
//		if (i% == 400 && i != 0)
//		{
//			fft.reset();
//		}
		#endif
		size = sizevec[i];
		sizeacc+=size;
		
		cout << "Size: " << i+1 << "/" << sizevec.size() << "\n";
	
		ChunkData::Ptr data;
        data.reset(new ChunkData(size));
		complex<float> *p = data->getCpuMemory();

        ChunkData::Ptr result(new ChunkData(size));

		srand(2);
		//srand(698745798351);

		float tempfloatr;
		
		for (int j = 0; j < size; j++)
		{
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].real(tempfloatr);
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].imag(tempfloatr);
		}

		/*
		if (size == endSize)
		{
			Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
			chunk->transform_data = data;
			ostringstream randomdataname;
			randomdataname << "data/" << "RandomData" << ".h5";;
			Hdf5Chunk::saveChunk(randomdataname.str().c_str(), *chunk );
		}
		*/

		cout << "Computing FFT...";

		fft.compute(data, result, FftDirection_Forward);

		#ifdef USE_AMD
		float bake = fft.getLastBakeTime();
		baketimes << size << " " << bake << endl;
		bakeacc += bake;
		#endif

		cout << "                        done!" << "\n";
		cout << "Reading results back into memory...";

		complex<float> *r = result->getCpuMemory();

		cout << "     done!" << "\n";

		cout << "dumping results... ";

		/*
		ostringstream resultsname;
		resultsname << "data/" << techlib << "Results" << size << ".h5";
		Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
		chunk->transform_data = result;
		Hdf5Chunk::saveChunk( resultsname.str().c_str(), *chunk);
		*/

		cout << " done!" << endl;
		/*
		float progress = (float)sizeacc / (float)sumsize;
		#ifdef USE_AMD
		float bakeavg = bakeacc / (float)(i+1);
		#endif
		float elapsed = ttx.elapsedTime();
		int toc = (int)elapsed;
		#ifdef USE_AMD
		cout << "Bakeavg: " << bakeavg << " Done: " << ((float)(i+1)/(float)numsize) << endl;
		int timeleft = (bakeavg/((float)(i+1)/(float)numsize))-elapsed;
		#else
		int timeleft = (elapsed/progress)-elapsed;
		#endif
		printf("\n\nDone: %4i/%i, %i/%i (%3.1f%%, %.2i:%.2i:%.2i elapsed, %.2i:%.2i:%.2is remaining)\n\n", i+1, numsize, sizeacc, sumsize, progress*100, toc/3600, toc/60, toc%60, timeleft/3600, timeleft/60, timeleft%60);
		*/
		}
			
	#ifdef USE_AMD
	baketimes.close();
	#endif
#endif
}

void FFTmojTest::testCase13()
{
#ifdef RUNTEST13
// Create random data
	srand(seedVal);
	
	float tempfloat;

	for (int size = 1<<10; size <= 1<<10; size = size*2)
	{
		char wallTimeFileName[100];
		sprintf(wallTimeFileName, "data/%sWallTimes%d.dat", techlib.c_str(), size);
		ofstream wallTimes(wallTimeFileName);

	#ifdef USE_OPENCL
		char kExTimeFileName[100];
		sprintf(kExTimeFileName, "data/%sKExTimes%d.dat", techlib.c_str(), size);
		ofstream kExTimes(kExTimeFileName);
	#endif
		
		for (int i = 1; i <= (1<<25)/size; i = i*2)
		{
			cout << "Batchsize: " << i << "/" << (1<<25)/size << "\n";

			/*if (size == 800000)
			{
				fft.reset();
			}*/
		
			fft.setBatchSize(i);

			ChunkData::Ptr data;
			data.reset(new ChunkData(size*i));
			complex<float> *input = data->getCpuMemory();

			ChunkData::Ptr result(new ChunkData(size*i));
			
			for (int j = 0; j < size*i; j++)
			{
				tempfloat = (float)rand()/(float)RAND_MAX;
				input[j].real(tempfloat);
				tempfloat = (float)rand()/(float)RAND_MAX;
				input[j].imag(tempfloat);
			}
			/*
			if (size == maxsize)
			{
				Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
				chunk->transform_data = data;
				char randomfilename[100];
				sprintf(randomfilename, "data/RandomData.h5");
				Hdf5Chunk::saveChunk(randomfilename, *chunk );
			}
			*/
							
	// CLFFT {
		 // walltimewithbake
			  // fft.compute
		 // endwalltimewithbake
		 // getexectime
	// }

	// CLAMDFFT { 
		 // bake
		 // baketime = getbaketime
	// }
			wallTimes << i;
	#ifdef USE_OPENCL
			kExTimes << i;
	#endif
			
			for (int j = 0; j < 100; j++)
			{
				TIME_STFT TaskTimer wallTimer("Wall-clock timer started");
				fft.compute(data, result, FftDirection_Forward);
				complex<float> *r = data->getCpuMemory();
				float wallTime = wallTimer.elapsedTime();
				/*
				if (j == 0)
				{
					char resultsFileName[100];
					sprintf(resultsFileName, "data/%sResults%d.h5", techlib.c_str(), size);
					Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
					chunk->transform_data = result;
					Hdf5Chunk::saveChunk( resultsFileName, *chunk);
				}
				*/
				wallTimes << " " << wallTime;
	#ifdef USE_OPENCL
				kExTimes << " " << fft.getKernelExecTime();
	#endif
			}
			
			if (i < (1<<25)/size)
			{
				wallTimes << endl;
	#ifdef USE_OPENCL
				kExTimes << endl;
	#endif
			}
		}
	}
#endif
}

void FFTmojTest::testCase14()
{
#ifdef RUNTEST14
// Create random data	
	float tempfloat;
	
	int size = 0, sizeacc = 0;
	
	char wallTimeFileName[100];
	sprintf(wallTimeFileName, "data/%sWallTimes.dat", techlib.c_str());
	ofstream wallTimes(wallTimeFileName);

#ifdef USE_OPENCL
	char kExTimeFileName[100];
	sprintf(kExTimeFileName, "data/%sKExTimes.dat", techlib.c_str());
	ofstream kExTimes(kExTimeFileName);
#endif
	
	TIME_STFT TaskTimer test14timer("testCase14 timer started\n");
	int toc = 0;
	for (int i = 0; i < sizes.size(); i++)
	{
		size = sizes[i];

		float progress = (float)sizeacc / (float)sizesum;
		toc = (int)test14timer.elapsedTime();
		printf("Size: %i, Done: %4i/%i, %i/%i (%3.1f%%), %.2i:%.2i:%.2i elapsed\n", 
			size, i, sizes.size(), sizeacc, sizesum, progress*100, toc/3600, (toc/60)%60, toc%60);
		
		sizeacc += size;
	
		ChunkData::Ptr data;
        data.reset(new ChunkData(size));
		complex<float> *input = data->getCpuMemory();

#ifndef USE_OPENCL
        ChunkData::Ptr result(new ChunkData(size));
#endif
		srand(seedVal);
		
		for (int j = 0; j < size; j++)
		{
			tempfloat = (float)rand()/(float)RAND_MAX;
			input[j].real(tempfloat);
			tempfloat = (float)rand()/(float)RAND_MAX;
			input[j].imag(tempfloat);
		}

		if (size == maxsize)
		{
			Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
			chunk->transform_data = data;
			char randomfilename[100];
			sprintf(randomfilename, "data/RandomData.h5");
			Hdf5Chunk::saveChunk(randomfilename, *chunk );
		}
		
		//cout << "\nData[10]: " << input[10] << endl;

						
// CLFFT {
     // walltimewithbake
          // fft.compute
     // endwalltimewithbake
     // getexectime
// }

// CLAMDFFT { 
     // bake
     // baketime = getbaketime
// }
		try 
		{
			//fft.reset();
			for (int j = 0; j < 25; j++)
			{
				TIME_STFT TaskTimer wallTimer("Wall-clock timer started");
#ifdef USE_OPENCL
				fft.compute(data, data, FftDirection_Forward);
				complex<float> *r = data->getCpuMemory();
#else
				fft.compute(data, result, FftDirection_Forward);
				complex<float> *r = result->getCpuMemory();
#endif
				float wallTime = wallTimer.elapsedTime();
				
				if (size >= startSize)
				{
					if (j == 0)
					{
						wallTimes << size;
					#ifdef USE_OPENCL
						kExTimes << size;
					#endif

						//cout << "\nData[10]: " << input[10] << " " << r[10] << endl;
						
						char resultsFileName[100];
						sprintf(resultsFileName, "data/%sResults%d.h5", techlib.c_str(), size);
						Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
#ifdef USE_OPENCL
						chunk->transform_data = data;
#else
						chunk->transform_data = result;
#endif
						Hdf5Chunk::saveChunk( resultsFileName, *chunk);
						
					}
					
					wallTimes << " " << wallTime;
					#ifdef USE_OPENCL
					kExTimes << " " << fft.getKernelExecTime();
					#endif
				}
			}
			
			if (i < sizes.size() && size > startSize)
			{
				wallTimes << "\n";
				#ifdef USE_OPENCL
				kExTimes << "\n";
				#endif
			}
		}
catch( std::exception& e )
		{
			cout << e.what() << endl;
			#ifdef USE_AMD
			fft.reset();
			#endif
			i--;
		}
	}

	wallTimes.close();
#ifdef USE_OPENCL
	kExTimes.close();
#endif

// CLFFT baketime = walltimewithbake-walltime

// OPENCL {
     // exectime = getexectime
     // spara batchtider
     // spara exectider
// }

#endif
}

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
