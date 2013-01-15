/*
TODO:

- Fixa preprocessorflaggor och defines för allt möjligt som ska kunna variera:
     - Placeness (för testet alltså. För clamdfft kan läggas till ifsats för om input och output är samma)
     - Upprepningar
     - Profiling och inte
     - Alla storlekar eller bara 2-potenser
     (- Om answers ska läsas från fil eller räknas ut av fftooura?)
- Fixa till utskrifterna så de ser prydliga ut
- Fixa wall clock time-tester
- Fixa så att inläsning och utskrift sker från och till data-mappen
- Allokera allt minne från början (för varje storlek) istf att använda samma minne om och om igen i test 3
- Fixa tidtagning av planskapandet
- Snygga till testen generellt
- Implementera resten av compute-funktionerna för clAmdFft
     - allting clAmdFft har dock lägre prio just nu, det viktigaste är att testen går att köra på nVidia-burkar
*/

/*TODO2:

Vad ska egentligen hända i det här testet?

- Ta reda på vilka storlekar som finns, och spara till fil (test1, finns redan)
- För alla storlekar i filen, kör en batchad FFT på 128MB data en gång och jämför resultatet med Ooura
- För alla storlekar i filen, kör en batchad FFT på 128MB något antal gånger och mät hur lång tid det tar
- Alla dessa gånger, mät kernel-bakningstiden
- Se till att padding skapas när det behövs

*/

/*TODO3:

Kolla upp om saker och ting verkligen är inplace eller inte.

Fixa problemet att plans inte kastas bort utan fyller upp minnet tills det tar slut

Ta bort alla test som inte längre behövs och gör ett test som gör typ allting:
	Läs in alla storlekar från filen och spara i en vektor så att man kan göra "progress-mätare" (i/n och % klart)
	Skapa randomdata och spara till h5
	For varje storlek, 
		Walltime:
			skapa plans
			baka plans
			kör FFT på randomdata
		Walltime 2:
			kör FFT på randomdata
		Spara kernelexectid i nån vektor
		Spara resultat till h5
	Spara kernelexectider till fil

Framtidsplaner: gör ett test som testar, för alla en storlek och alla batchstorlekar från 1 till så många som går, 
och mät tid för varje batchstorlek. När allt funkar, gör så att testet kör på alla storlekar istället.
( för varje storlek finns förmodligen en optimal storlek för batchen för en viss hårdvara, och den är nog != max.
  Finns det flera, lokala max, eller finns det bara en? Testa först för tvåpotenser och kolla om det finns något mönster! )
Gör på samma sätt ett annat test som kollar vilken batchstorlek som ger bäst wall clock time för input om 128MB data.

*/ 

#define NOGUI
#ifdef _MSC_VER
#include <stdlib.h> //  error C2381: 'exit' : redefinition; __declspec(noreturn) differs
#include <time.h>
#include "TaskTimer.h"
#endif

#define TIME_STFT

#define RUNTEST1
//#define RUNTEST10
//#define RUNTEST13
#define PLACENESS "inplace"
#define FFTINPLACE
#define CL_PROFILING
//#define ONLYPOWERSOF2
#define startSize 1<<8 // 2 ^ 8
#define endSize 1<<22 // 2 ^ 22

#include "exceptionassert.h"

#include "tfr/fftimplementation.h"
#include "tfr/fftcufft.h"
#ifdef USE_OPENCL
    #ifdef USE_AMD
        #include "tfr/clamdfft/fftclamdfft.h"
    #else
        #include "tfr/clfft/fftclfft.h"
	#endif
#else
    #include "tfr/fftooura.h"
#endif

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
    void testCase1(); // Get sizes in an interval for current library and store in file
	void testCase10(); // Read sizes from file, create input vectors, run fft, store results in files.
	void testCase13(); // Benchmark, for all batch sizes of a given size, the kernel execution time.
	//void mainTest(); // Benchmark wall-time, bake time, kernel execution time

private:
	string techlib;

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
//	A a;
//	a = A();
#ifdef USE_OPENCL
    #ifdef USE_AMD
//        fft = FftClAmdFft();
        techlib = "ClAmdFft";
    #else
        techlib = "ClFft";
    #endif
#elif USE_CUDA
    techlib = "CuFft";
#else
    techlib = "Ooura";
#endif
    //a.show(); // glew needs an OpenGL context
}

void FFTmojTest::cleanupTestCase()
{
}

void FFTmojTest::testCase1()
{
#ifdef RUNTEST1
	char sizefilename[100];
	sprintf(sizefilename, "data/%sSizes.dat", techlib.c_str());
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
		i = fft.sChunkSizeG(i);
	}

	outputfile.close();

	cout << "Number of sizes: " << numSize << "\n";
	cout << "Sum of sizes: " << sumSize << "\n";

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
		{
			fft.reset();
		}
		#endif
		size = sizevec[i];
		sizeacc+=size;
	
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

		if (size == endSize)
		{
			Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
			chunk->transform_data = data;
			ostringstream randomdataname;
			randomdataname << "data/" << "RandomData" << ".h5";;
			Hdf5Chunk::saveChunk(randomdataname.str().c_str(), *chunk );
		}

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

		ostringstream resultsname;
		resultsname << "data/" << techlib << "Results" << size << ".h5";
		Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
		chunk->transform_data = result;
		Hdf5Chunk::saveChunk( resultsname.str().c_str(), *chunk);

		cout << " done!" << endl;
		
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
	}
			
	#ifdef USE_AMD
	baketimes.close();
	#endif
#endif
}

void FFTmojTest::testCase13()
{
#ifdef RUNTEST13
	int maxtotal = 1<<22;
	int size = 1<<8;

	ostringstream kexfile;
	kexfile << "data/" << techlib << "KexTimes" << ".dat";
	ofstream kextimes(kexfile.str().c_str());

	fft.createPlan(size);

	for (int i = 1; i < maxtotal / size; i++)
	{
		try {
		fft.setSize(size*i);
		fft.setBatchSize(i);
		ChunkData::Ptr data;
        data.reset(new ChunkData(size*i));
		complex<float> *p = data->getCpuMemory();
		ChunkData::Ptr result(new ChunkData(size*i));
		fft.compute(data, result, FftDirection_Forward);
		kextimes << fft.getKernelExecTime() << "\n";
		} catch (uint e) {
			cout << "An exception occurred. Exception Nr. " << e << endl;
		}
	}

	kextimes.close();
#endif
}

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
