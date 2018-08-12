#define NOGUI
#ifdef _MSC_VER
#include <stdlib.h> //  error C2381: 'exit' : redefinition; __declspec(noreturn) differs
#include <time.h>
#include "TaskTimer.h"
#endif

#define TIME_STFT

#define GENERATESIZEVECTOR
#define READSIZEVECTOR
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
	// Make a random vector using the "set" variable as the seed,
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
		p[i].real((float)rand()/(float)RAND_MAX);
		p[i].imag((float)rand()/(float)RAND_MAX);
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
	std::string sizefilename = (boost::format("data/%s/%s/Sizes.dat") % machine % techlib).str();
	ifstream infile(sizefilename.c_str());
	if (infile)
	{
		cout << "Size file " << sizefilename << " already exists." << endl;
	}
	else
	{
		cout << "Generating " << sizefilename << endl;
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
		cout << "Sum of sizes: " << sumSize << endl;
	}


#endif
}

void FFTmojTest::readSizeVector()
{
	// Read sizes from file
#ifdef READSIZEVECTOR
	std::string sizefilename = (boost::format("data/%s/%s/Sizes.dat") % machine % techlib).str();
	ifstream sizefile(sizefilename.c_str());

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
	if (mode != "bench")
		return;

	int size = 0, sizeacc = 0;

	TIME_STFT TaskTimer runBenchmarkTimer("runBenchmark timer started\n");
	int toc = 0;

	std::string randomfilename = (boost::format("data/%s/RandomData%d.h5") % machine % set).str();

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

		std::string wallTimeFileName = (boost::format("data/%s/%s/set%d/run%d/WallTimes%d.dat") % machine % techlib % set % run % size).str();
		ofstream wallTimes(wallTimeFileName.c_str());

#ifdef USE_OPENCL
		std::string kExTimeFileName = (boost::format("data/%s/%s/set%d/run%d/KernelExecutionTimes%d.dat") % machine % techlib % set % run % size).str();
		ofstream kExTimes(kExTimeFileName.c_str());
#endif

		std::string resultsFileName = (boost::format("data/%s/%s/set%d/Results%d.h5") % machine % techlib % set % size).str();

		complex<float> *results = 0;
		pChunk resultchunk;

#ifdef USE_AMD
		int maxBatchSize = (1 << 24) / size;
#else
		int maxBatchSize = 1;
#endif

		for (int batchSize = maxBatchSize; batchSize > 0; batchSize = batchSize/2)
		{
			// TODO: This is still wrong...
			cout << "Batchsize: " << batchSize << "/" << maxBatchSize << endl;

#ifdef USE_AMD
			fft.setBatchSize(batchSize);
#endif
			ChunkData::Ptr data;
			data.reset(new ChunkData(size*batchSize));
			complex<float> *input = data->getCpuMemory();

#ifndef USE_OPENCL
			ChunkData::Ptr result(new ChunkData(size*batchSize));
#endif
			wallTimes << batchSize;
#ifdef USE_OPENCL
			kExTimes << batchSize;
#endif
			try
			{
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
					for (int k = 0; k < batchSize; k++)
					{
						int offset = k*size;
						if (0 == memcmp(r+offset, random+offset, size*sizeof(complex<float>)))
						{
							cout << "\nFAIL: FFT results at batch " << k << " are identical to input!\n" << endl;
							abort();
						}
					}

					if (size >= startSize)
					{
						if (!results)
						{
							ifstream infile(resultsFileName.c_str());
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
						if (0 != memcmp(results, r, size*batchSize*sizeof(complex<float>)))
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
	}
#endif
}

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
