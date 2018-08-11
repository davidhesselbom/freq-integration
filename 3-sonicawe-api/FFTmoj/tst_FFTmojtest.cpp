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
	ifstream argvfile("argv.txt");
	// TODO: Error handling...
	argvfile >> machine; // which machine we're on
	argvfile >> set; // which of the sets to run
	argvfile >> run; // which of the runs to run
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
	
	std::string randomfilename = (boost::format("data/%s/RandomData%d.h5") % machine % set).str();

	cout << "Generating " << randomfilename << "... " << flush;
		
	ifstream infile(randomfilename.c_str());
	if (infile)
	{
		cout << "already exists, verifying... " << flush;
	}

	srand(set);

	int maxSize = 1 << (10+14);

	float tempfloatr;
		
	ChunkData::Ptr data;
	data.reset(new ChunkData(maxSize));
	complex<float> *p = data->getCpuMemory();

	for (int i = 0; i < maxSize; i++)
	{
		tempfloatr = (float)rand()/(float)RAND_MAX;
		p[i].real(tempfloatr);
		tempfloatr = (float)rand()/(float)RAND_MAX;
		p[i].imag(tempfloatr);
	}

	Tfr::pChunk chunk( new Tfr::StftChunk(maxSize, Tfr::StftParams::WindowType_Rectangular, 0, true));

	chunk->transform_data = data;

	if (infile)
	{
		pChunk randomchunk = Hdf5Chunk::loadChunk ( randomfilename );
		complex<float> *random = randomchunk->transform_data->getCpuMemory();

		/*
		TODO: Is there no way to terminate the whole test if QVERIFY2 fails?
		Is there another QTest function or macro that does this?
		QVERIFY2(*randomchunk->transform_data == &data, "Input random data differs from generated random data!");
		*/

		if (0 != memcmp(p, random, maxSize*sizeof(complex<float>)))
		{
			cout << "\nFAIL: Input random data differs from generated random data!\n" << endl;
			abort();
		}
	}
	else
	{
		Hdf5Chunk::saveChunk( randomfilename, *chunk);
	}
	cout << "done." << endl;
}

void FFTmojTest::generateSizeVector()
{
	// Get sizes in an interval for current library and store in file
#ifdef GENERATESIZEVECTOR
	// TODO: Don't overwrite existing files!
	std::string sizefilename = (boost::format("data/%s/%s/Sizes.dat") % machine % techlib).str();
	ofstream outputfile(sizefilename.c_str());

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
	sprintf(sizefilename, "data/%s/%s/Sizes.dat", machine.c_str(), techlib.c_str());
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

	printf("FYI first size: %d, last size: %d, # of sizes: %d\n", sizes.front(), sizes.back(), sizes.size());

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
	sprintf(wallTimeFileName, "data/%s/%s/set%d/run%d/WallTimes.dat", machine.c_str(), techlib.c_str(), set, run);
	ofstream wallTimes(wallTimeFileName);

#ifdef USE_OPENCL
	char kExTimeFileName[100];
	sprintf(kExTimeFileName, "data/%s/%s/set%d/run%d/KernelExecutionTimes.dat", machine.c_str(), techlib.c_str(), set, run);
	ofstream kExTimes(kExTimeFileName);
#endif
	
	TIME_STFT TaskTimer runBenchmarkTimer("runBenchmark timer started\n");
	int toc = 0;

	char randomfilename[100];
	sprintf(randomfilename, "data/%s/RandomData%d.h5", machine.c_str(), set);

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

		char resultsFileName[100];
		sprintf(resultsFileName, "data/%s/%s/set%d/Results%d.h5", machine.c_str(), techlib.c_str(), set, size);

		complex<float> *results = 0;
		pChunk resultchunk;

		try 
		{
			//fft.reset();
			// TODO: 25 is a magic number that I should specify elsewhere.
			// BTW, does it have to be the same for all window sizes?
			// Wouldn't it make sense to do it fewer times for large sizes?
			for (int j = 0; j < 25; j++)
			{
#ifndef USE_OPENCL
				// Unless inplace, this only needs doing the first iteration.
				if (j == 0)
				{
#endif
				memcpy(input, random, size*sizeof(complex<float>));
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
				// Good enough for single batch, but for multi-batch, this needs to be done for each batch...
				if (0 == memcmp(r, random, size*sizeof(complex<float>)))
				{
					cout << "\nFAIL: FFT results are identical to input!\n" << endl;
					abort();
				}
				
				if (size >= startSize)
				{
					if (j == 0)
					{
						wallTimes << size;
#ifdef USE_OPENCL
						kExTimes << size;
#endif
					}

					if (!results)
					{
						ifstream infile(resultsFileName);
						if (!infile)
						{
							Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
#ifdef USE_OPENCL
							chunk->transform_data = data;
#else
							chunk->transform_data = result;
#endif
							Hdf5Chunk::saveChunk( resultsFileName, *chunk);
						}
						resultchunk = Hdf5Chunk::loadChunk ( resultsFileName );
						results = resultchunk->transform_data->getCpuMemory();
					}
					if (0 != memcmp(results, r, size*sizeof(complex<float>)))
					{
						cout << "\nFAIL: FFT results differ from previous results with the same library!\n" << endl;
						abort();
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

#endif
}

void FFTmojTest::runBatchTest()
{
	// Benchmark, for all batch sizes of a given size, the kernel execution time.
#ifdef RUNBATCHTEST
	if (mode != "batch")
		return;

	int size = 0, sizeacc = 0;

	TIME_STFT TaskTimer runBenchmarkTimer("runBenchmark timer started\n");
	int toc = 0;

	char randomfilename[100];
	sprintf(randomfilename, "data/%s/RandomData%d.h5", machine.c_str(), set, run);

	cout << "Loading random data from " << randomfilename << "... " << flush;
	pChunk randomchunk = Hdf5Chunk::loadChunk ( randomfilename );
	complex<float> *random = randomchunk->transform_data->getCpuMemory();
	cout << "done." << endl;

	// TODO: This shouldn't be this hardcoded...
	for (int i = 0; i < sizes.size(); i++)
	{
		size = sizes[i];

		float progress = (float)sizeacc / (float)sizesum;
		toc = (int)runBenchmarkTimer.elapsedTime();
		printf("Size: %i, Done: %4i/%i, %i/%i (%3.1f%%), %.2i:%.2i:%.2i elapsed\n",
			size, i, sizes.size(), sizeacc, sizesum, progress*100, toc/3600, (toc/60)%60, toc%60);

		sizeacc += size;

		char wallTimeFileName[100];
		sprintf(wallTimeFileName, "data/%s/%s/set%d/batch%d/WallTimes%d.dat", machine.c_str(), techlib.c_str(), set, run, size);
		ofstream wallTimes(wallTimeFileName);

#ifdef USE_OPENCL
		char kExTimeFileName[100];
		sprintf(kExTimeFileName, "data/%s/%s/set%d/batch%d/KernelExecutionTimes%d.dat", machine.c_str(), techlib.c_str(), set, run, size);
		ofstream kExTimes(kExTimeFileName);
#endif

		complex<float> *results = 0;
		pChunk resultchunk;

		char resultsFileName[100];
		sprintf(resultsFileName, "data/%s/%s/set%d/%dResults%d.h5", machine.c_str(), techlib.c_str(), set, (1<<24)/size, size);

		for (int batchSize = (1<<24)/size; batchSize > 0; batchSize = batchSize/2)
		{
			cout << "Batchsize: " << batchSize << "/" << (1<<24)/size << "\n";

#ifdef USE_AMD
			fft.setBatchSize(batchSize);
#endif
			ChunkData::Ptr data;
			data.reset(new ChunkData(size*batchSize));
			complex<float> *input = data->getCpuMemory();

			wallTimes << batchSize;
#ifdef USE_OPENCL
			kExTimes << batchSize;
#endif
			// TODO: 25 is a magic number that I should specify elsewhere.
			// BTW, does it have to be the same for all window sizes?
			// Wouldn't it make sense to do it fewer times for large sizes?
			for (int j = 0; j < 25; j++)
			{
#ifndef USE_OPENCL
				// Unless inplace, this only needs doing the first iteration.
				if (j == 0)
				{
#endif
				memcpy(input, random, size*batchSize*sizeof(complex<float>));
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
				// It's a little more complicated for multi-batch...
				for (int k = 0; k < batchSize; k++)
				{
					int offset = k*size;
					if (0 == memcmp(r+offset, random+offset, size*sizeof(complex<float>)))
					{
						cout << "\nFAIL: FFT results at batch " << k << " are identical to input!\n" << endl;
						abort();
					}
				}
				if (!results)
				{
					ifstream infile(resultsFileName);
					if (!infile)
					{
						Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
						chunk->transform_data = data;
						Hdf5Chunk::saveChunk( resultsFileName, *chunk);
					}
					resultchunk = Hdf5Chunk::loadChunk ( resultsFileName );
					results = resultchunk->transform_data->getCpuMemory();
				}
				if (0 != memcmp(results, r, size*batchSize*sizeof(complex<float>)))
				{
					cout << "\nFAIL: Batch FFT results differ from previous results with the same library!\n" << endl;
					abort();
				}

				wallTimes << " " << wallTime;
#ifdef USE_OPENCL
				kExTimes << " " << fft.getKernelExecTime();
#endif
			}

			if (batchSize > 1)
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

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
