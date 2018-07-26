#define NOGUI
#ifdef _MSC_VER
#include <stdlib.h> //  error C2381: 'exit' : redefinition; __declspec(noreturn) differs
#include <time.h>
#include "TaskTimer.h"
#endif

#define TIME_STFT

#define GENERATESIZEVECTOR
#define READSIZEVECTOR
#define RUNBATCHTEST
#define RUNBENCHMARK
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
	void generateBatchRandomData();
	void runBatchTest();
	void runBenchmark();

private:
	void getSizesFromFile(std::vector<int> *sizes, int *sumsizes);
	string techlib;
	int maxsize, sizesum;
	std::vector<int> sizes;
	int seedVal;
	int run;
	string machine;
	int set;
	string mode;

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
	sizesum = maxsize = seedVal = 0;
	seedVal = time(0);
	run = 1;
	set = 1;
	machine = "Rampage";
	mode = "bench";
	
	// So, this is dumb, but without argv, what do I do instead?
	char argvfilename[100];
	sprintf(argvfilename, "argv.txt");
	ifstream argvfile(argvfilename);
	// TODO: Error handling...
	argvfile >> machine; // which machine we're on
	argvfile >> set; // which of the 3 sets to run
	argvfile >> run; // which of the 5 runs to run
	argvfile >> mode; // whether to run benchmarks or batches
	cout << machine << " " << set << " " << run << " " << mode << endl;
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
		sprintf(randomfilename, "data/%s/set%d/RandomData%d.h5", machine.c_str(), set, i);

		cout << "Generating " << randomfilename << "... " << flush;
		
		ifstream infile(randomfilename);
		if (infile)
		{
			cout << "already exists, verifying" << endl;
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

		if (infile)
		{
			pChunk randomchunk = Hdf5Chunk::loadChunk ( randomfilename );
			complex<float> *random = randomchunk->transform_data->getCpuMemory();

			/*
			TODO: This should be faster, but I can't get it to build.
			TODO: Is there no way to terminate the whole test if QVERIFY2 fails?
			Is there another QTest function or macro that does this?
			QVERIFY2(*randomchunk->transform_data == &data, "Input random data differs from generated random data!");
			*/
			for (int j = 0; j < maxSize; j++)
			{
				if (p[j] != random[j])
				{
					cout << "\nFAIL: Input random data differs from generated random data!\n" << endl;
					abort();
				}
			}
		}
		else
		{
			Hdf5Chunk::saveChunk( randomfilename, *chunk);
		}
		cout << "done." << endl;
	}
}

void FFTmojTest::generateSizeVector()
{
	// Get sizes in an interval for current library and store in file
#ifdef GENERATESIZEVECTOR
	char sizefilename[100];
	sprintf(sizefilename, "data/%s/set%d/%s/Sizes.dat", machine.c_str(), set, techlib.c_str());
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
	sprintf(sizefilename, "data/%s/set%d/%s/Sizes.dat", machine.c_str(), set, techlib.c_str());
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
	int i = startSize / 2;
#else
	int i = fft.lChunkSizeS(startSize);
#endif
	sizes.push_back(i);
	reverse(sizes.begin(), sizes.end());

	//sizes._Reverse(sizes.begin(), sizes.end());

	
	printf("FYI first size: %d, last size: %d, # of sizes: %d\n", sizes.front(), sizes.back(), sizes.size());

#endif
}

void FFTmojTest::generateBatchRandomData()
{
	// Make 5 random vectors with the seeds 1..5,
	// store them in data/ClAmdFft/RandomFile#.h5,
	// but don't overwrite if exist
#ifdef RUNBATCHTEST
	if (mode != "batch")
		return;
	cout << "About to generate random data..." << endl;

	for (int i = 1; i <= 5; i++)
	{
		char randomfilename[100];
		sprintf(randomfilename, "data/%s/set%d/BatchRandomData%d.h5", machine.c_str(), set, i);

		cout << "Generating " << randomfilename << "... " << flush;

		ifstream infile(randomfilename);
		if (infile)
		{
			cout << "already exists, skipping" << endl;
			continue;
		}

		srand(i);

		int maxSize = 1 << (10+14);

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

		if (infile)
		{
			pChunk randomchunk = Hdf5Chunk::loadChunk ( randomfilename );
			complex<float> *random = randomchunk->transform_data->getCpuMemory();

			/*
			TODO: This should be faster, but I can't get it to build.
			TODO: Is there no way to terminate the whole test if QVERIFY2 fails?
			Is there another QTest function or macro that does this?
			QVERIFY2(*randomchunk->transform_data == &data, "Input random data differs from generated random data!");
			*/
			for (int j = 0; j < maxSize; j++)
			{
				if (p[j] != random[j])
				{
					cout << "\nFAIL: Input batch random data differs from generated batch random data!\n" << endl;
					abort();
				}
			}
		}
		else
		{
			Hdf5Chunk::saveChunk( randomfilename, *chunk);
		}
		cout << "done." << endl;
	}
#endif
}

void FFTmojTest::runBatchTest() 
{
	// Benchmark, for all batch sizes of a given size, the kernel execution time.
#ifdef RUNBATCHTEST
	if (mode != "batch")
		return;
// Load random data
	char randomfilename[100];
	sprintf(randomfilename, "data/%s/set%d/BatchRandomData%d.h5", machine.c_str(), set, run);

	cout << "Loading random data from " << randomfilename << "... " << flush;
	pChunk randomchunk = Hdf5Chunk::loadChunk ( randomfilename );
	complex<float> *random = randomchunk->transform_data->getCpuMemory();
	cout << "done." << endl;

	for (int size = 1<<10; size <= 1<<10; size = size*2)
	{
		char wallTimeFileName[100];
		sprintf(wallTimeFileName, "data/%s/set%d/%s/batch%d/WallTimes%d.dat", machine.c_str(), set, techlib.c_str(), run, size);
		ofstream wallTimes(wallTimeFileName);

	#ifdef USE_OPENCL
		char kExTimeFileName[100];
		sprintf(kExTimeFileName, "data/%s/set%d/%s/batch%d/KernelExecutionTimes%d.dat", machine.c_str(), set, techlib.c_str(), run, size);
		ofstream kExTimes(kExTimeFileName);
	#endif

		for (int i = 1; i <= (1<<24)/size; i = i*2)
		{
			cout << "Batchsize: " << i << "/" << (1<<24)/size << "\n";

			/*if (size == 800000)
			{
				fft.reset();
			}*/
		
#ifdef USE_AMD
			fft.setBatchSize(i);
#endif
			ChunkData::Ptr data;
			data.reset(new ChunkData(size*i));
			complex<float> *input = data->getCpuMemory();
							
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
			
			for (int j = 0; j < 25; j++)
			{
				//TODO: Can't seem to get this right, so using a loop instead...
				//memcpy(&input, &random, size);
				for (int k = 0; k < size*i; k++)
				{
					input[k] = random[k];
				}

				TIME_STFT TaskTimer wallTimer("Wall-clock timer started");
				fft.compute(data, data, FftDirection_Forward);
				complex<float> *r = data->getCpuMemory();
				float wallTime = wallTimer.elapsedTime();

				if (j == 0)
				{
					char resultsFileName[100];
					sprintf(resultsFileName, "data/%s/set%d/%s/batch%d/%dResults%d.h5", machine.c_str(), set, techlib.c_str(), run, i, size);
					Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
					chunk->transform_data = data;
					Hdf5Chunk::saveChunk( resultsFileName, *chunk);
				}

				wallTimes << " " << wallTime;
	#ifdef USE_OPENCL
				kExTimes << " " << fft.getKernelExecTime();
	#endif
			}
			
			if (i < (1<<24)/size)
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

void FFTmojTest::runBenchmark()
{
	// Benchmark wall-time, bake time, kernel execution time, store first FFT result.
#ifdef RUNBENCHMARK
// Create random data	
	if (mode != "bench")
		return;

	int size = 0, sizeacc = 0;
	
	char wallTimeFileName[100];
	sprintf(wallTimeFileName, "data/%s/set%d/%s/run%d/WallTimes.dat", machine.c_str(), set, techlib.c_str(), run);
	ofstream wallTimes(wallTimeFileName);

#ifdef USE_OPENCL
	char kExTimeFileName[100];
	sprintf(kExTimeFileName, "data/%s/set%d/%s/run%d/KernelExecutionTimes.dat", machine.c_str(), set, techlib.c_str(), run);
	ofstream kExTimes(kExTimeFileName);
#endif
	
	TIME_STFT TaskTimer runBenchmarkTimer("runBenchmark timer started\n");
	int toc = 0;

	char randomfilename[100];
	sprintf(randomfilename, "data/%s/set%d/RandomData%d.h5", machine.c_str(), set, run);

	cout << "Loading random data from " << randomfilename << "... " << flush;
	pChunk randomchunk = Hdf5Chunk::loadChunk ( randomfilename );
	complex<float> *random = randomchunk->transform_data->getCpuMemory();
	cout << "done." << endl;

	for (int i = 0; i < sizes.size(); i++)
	{
		size = sizes[i];

		float progress = (float)sizeacc / (float)sizesum;
		toc = (int)runBenchmarkTimer.elapsedTime();
		printf("Size: %i, Done: %4i/%i, %i/%i (%3.1f%%), %.2i:%.2i:%.2i elapsed\n", 
			size, i, sizes.size(), sizeacc, sizesum, progress*100, toc/3600, (toc/60)%60, toc%60);
		
		sizeacc += size;
	
		ChunkData::Ptr data;
        data.reset(new ChunkData(size));
		complex<float> *input = data->getCpuMemory();

#ifndef USE_OPENCL
        ChunkData::Ptr result(new ChunkData(size));
#endif
		
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
#ifndef USE_OPENCL
				// Unless inplace, this only needs doing the first iteration.
				if (j == 0)
				{
#endif
				//TODO: Can't seem to get this right, so using a loop instead...
				//memcpy(&input, &random, size);
				for (int k = 0; k < size; k++)
				{
					input[k] = random[k];
				}
#ifndef USE_OPENCL
				}
#endif
				TIME_STFT TaskTimer wallTimer("Wall-clock timer started");
#ifdef USE_OPENCL
				fft.compute(data, data, FftDirection_Forward);
				complex<float> *r = data->getCpuMemory();
#else
				fft.compute(data, result, FftDirection_Forward);
				complex<float> *r = result->getCpuMemory();
#endif
				float wallTime = wallTimer.elapsedTime();

				// Verify output != input
				for (int k = 0; k < size; k++)
				{
					// Good enough for single batch, but for multi-batch, this needs to be done for each batch...
					if (r[k] != random[k])
					{
						break;
					}
					if (k == size - 1)
					{
						cout << "\nFAIL: FFT results are identical to input!\n" << endl;
						abort();
					}
				}
				
				if (size >= startSize)
				{
					if (j == 0)
					{
						wallTimes << size;
					#ifdef USE_OPENCL
						kExTimes << size;
					#endif
						char resultsFileName[100];
						sprintf(resultsFileName, "data/%s/set%d/%s/run%d/Results%d.h5", machine.c_str(), set, techlib.c_str(), run, size);
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
