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
//#define RUNTEST2
//#define RUNTEST3
//#define RUNTEST4
//#define RUNTEST5
//#define RUNTEST5A
//#define RUNTEST6
//#define RUNTEST6N
//#define RUNTEST7
//#define RUNTEST8
//#define RUNTEST9
//#define RUNTEST10
//#define RUNTEST11
//#define RUNTEST12
#define TEST4SIZE 1<<15 // 2 ^ 15
#define TEST4TIMES 1
#define PLACENESS "inplace"
#define FFTINPLACE
#define CL_PROFILING
//#define ONLYPOWERSOF2
#define repDividend 1<<24 // 2 ^ 24
#define repNumerator 0
#define PREALLOCATE
#define maxerrlim 0.00001 //1e-5
#define nrmsdlim 0.00000001 //1e-8
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
	void testCase2(); // Run fft on pre-existing random data for all those sizes
	void testCase3(); // Repeatedly run fft on random data a number of times for all those sizes
	void testCase4(); // Run fft once on each element of an array of random data chunks
	void testCase5(); // Create plans for all the sizes and benchmark baking times
	void testCase5A(); // Same, but skip batch resize and baking
	void testCase6(); // Same, but changing the size of copy of existing plan instead of creating new ones
	void testCase6n(); // Same, but benchmarks baking plan of 1920000 modified from all other sizes
	void testCase7(); // Run the fft on 128MB of random data for all sizes in size file, 
		// compare with Ooura (maxerr and nrmsd), store results and kernel baking times
	void testCase8(); // Same as 6, but repeat X times, and store kernel execution and baking times
	void testCase9(); // Create a file with 2 columns of 22 random floats each for later use. 
	void testCase10(); // Read input from file, create input vectors, run fft, store results in files.
	void testCase11(); // Generate matlab script for precision calculations.
	void testCase12(); // Generate hdf5 files from results files in testCase10.

private:
	complex<float> max(ChunkData::Ptr P);
	complex<float> max(complex<float> p, complex<float> r);
	complex<float> max(ChunkData::Ptr P, ChunkData::Ptr R);
	complex<float> min(ChunkData::Ptr P);
	complex<float> min(complex<float> p, complex<float> r);
	complex<float> min(ChunkData::Ptr P, ChunkData::Ptr R);
	complex<float> mean(ChunkData::Ptr P);
	ChunkData::Ptr abs(ChunkData::Ptr P);
	ChunkData::Ptr diff(ChunkData::Ptr P, ChunkData::Ptr R);
	ChunkData::Ptr mult(ChunkData::Ptr P, ChunkData::Ptr R);
	float maxerr(ChunkData::Ptr P, ChunkData::Ptr R);
	float nrmsd(ChunkData::Ptr P, ChunkData::Ptr R);
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

#ifdef TROLLLARS
	DoAwesomeStuff();

	int theThing = new Fixer(Category.Bug);
	theThing.Apply(3, "qobject.cpp"); // Fixing some bugs in the codez

	return 85; // DON'T CHANGE OR ELSE THINGS WILL BREAK
#endif
}

/*
#include "cudaPitchedPtrType.h"
#include "CudaException.h"

void FFTmojTestCuda( cudaPitchedPtrType<float> data );
*/

complex<float> FFTmojTest::max(ChunkData::Ptr P)
{
	complex<float> *p = P->getCpuMemory();
	complex<float> max = p[0];
	for (uint i = 1; i < P->numberOfElements(); i++)
	{
		if (std::abs(p[i]) > std::abs(max))
			max = p[i];
	}
	return max;
}

complex<float> FFTmojTest::max(complex<float> p, complex<float> r)
{
	complex<float> max = p;
	if (std::abs(r) > std::abs(p))
		max = r;
	return max;
}

complex<float> FFTmojTest::max(ChunkData::Ptr P, ChunkData::Ptr R)
{
	return max(max(P), max(R));
}

complex<float> FFTmojTest::min(ChunkData::Ptr P)
{
	complex<float> *p = P->getCpuMemory();
	complex<float> min = p[0];
	for (uint i = 1; i < P->numberOfElements(); i++)
	{
		if (std::abs(p[i]) < std::abs(min))
			min = p[i];
	}
	return min;
}

complex<float> FFTmojTest::min(complex<float> p, complex<float> r)
{
	complex<float> min = p;
	if (std::abs(r) < std::abs(p))
		min = r;
	return min;
}

complex<float> FFTmojTest::min(ChunkData::Ptr P, ChunkData::Ptr R)
{
	return min(min(P), min(R));
}

complex<float> FFTmojTest::mean(ChunkData::Ptr P)
{
	complex<float> *p = P->getCpuMemory();
	complex<float> mean = 0;
	complex<double> sum = 0;
	for (uint i = 1; i < P->numberOfElements(); i++)
	{
		sum += p[i];
	}
	mean = sum / (double)P->numberOfElements();
	return mean;
}

ChunkData::Ptr FFTmojTest::diff(ChunkData::Ptr P, ChunkData::Ptr R)
{
	complex<float> *p = P->getCpuMemory();
	complex<float> *r = R->getCpuMemory();
	ChunkData::Ptr result(new ChunkData(P->numberOfElements()));
	complex<float> *v = result->getCpuMemory();
	for (uint i = 0; i < P->numberOfElements(); i++)
	{
		v[i] = p[i] - r[i];
	}
	return result;
}

ChunkData::Ptr FFTmojTest::mult(ChunkData::Ptr P, ChunkData::Ptr R)
{
	complex<float> *p = P->getCpuMemory();
	complex<float> *r = R->getCpuMemory();
	ChunkData::Ptr result(new ChunkData(P->numberOfElements()));
	complex<float> *v = result->getCpuMemory();
	for (uint i = 0; i < P->numberOfElements(); i++)
	{
		v[i] = p[i] * r[i];
	}
	return result;
}

ChunkData::Ptr FFTmojTest::abs(ChunkData::Ptr P)
{
	complex<float> *p = P->getCpuMemory();
	ChunkData::Ptr result(new ChunkData(P->numberOfElements()));
	complex<float> *v = result->getCpuMemory();
	for (uint i = 0; i < P->numberOfElements(); i++)
	{
		v[i] = std::abs(p[i]);
	}
	return result;
}

float FFTmojTest::maxerr(ChunkData::Ptr P, ChunkData::Ptr R)
{
	complex<float> error = max(abs(diff(P, R)));
    complex<float> difference = (max(P, R)-min(P, R));
	error /= std::abs(difference);
	return error.real();
}

float FFTmojTest::nrmsd(ChunkData::Ptr P, ChunkData::Ptr R)
{
    complex<float> normRMSD = sqrt(mean(mult(abs(diff(P, R)), abs(diff(P, R))))) / (max(P, R)-min(P, R));
	return normRMSD.real();
}

class A
{
public:
	A() { cout << this << " " << __FUNCTION__ << "\n"; }
	A(int) { cout << this << " " << __FUNCTION__ << "\n"; }
	~A() { cout << this << " " << __FUNCTION__ << "\n"; }

	void operator=(A&) { cout << this << " " << __FUNCTION__ << "\n"; }
};

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
	ostringstream filename, scriptname;
	filename << "data/" << techlib << "Sizes" << ".dat";
	ofstream outputfile(filename.str().c_str());
	scriptname << "data/" << techlib << "Sizes" << ".m";
	ofstream outputscript(scriptname.str().c_str());

	int sumSize = 0;
	int numSize = 0;
	int i = startSize;
	while (i <= endSize)
	{
		sumSize += i;
		numSize++;

		outputfile << i << "\n";
		outputscript << "a = rand(" << i << ", 1);" << "\n";
		outputscript << "save rand" << i << ".dat a;" << "\n";
		outputscript << "b = rand(" << i << ", 1);" << "\n";
		outputscript << "save rand" << i << "i.dat b;" << "\n";
		outputscript << "c = a + b*i;" << "\n";
		outputscript << "d = fft(c);" << "\n";
		outputscript << "a = real(d);" << "\n";
		outputscript << "b = imag(d);" << "\n";
		outputscript << "save rand" << i << "f.dat a;" << "\n";
		outputscript << "save rand" << i << "if.dat b;" << "\n";
		i = fft.sChunkSizeG(i);
	}

	outputfile.close();
	outputscript.close();

	cout << "Number of sizes: " << numSize << "\n";
	cout << "Sum of sizes: " << sumSize << "\n";

#endif
}

void FFTmojTest::testCase2()
{
#ifdef RUNTEST2
	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream scriptname;
	scriptname << "data/" << techlib << "Comparison" << ".m";
	ofstream outputscript(scriptname.str().c_str());

	ostringstream correctnessName;
	correctnessName << "data/" << techlib << "Correctness" << ".dat";
	ofstream correctnessFile(correctnessName.str().c_str());

	srand((unsigned)time(0));

	int size = 0;
	int i = 0;
	while (i <= endSize)
	{
		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
			break;

		else
			i = size;

	#if defined(USE_OPENCL)
	#if !defined(USE_AMD)
		//if (size == 1 << 10 || size == 1 << 22)
		if (size == 1 << 22)
		{
			cout << "Size " << size << " not good for ClFft. Skipping..." << "\n" << "\n";
			continue;
		}
	#endif
	#endif

		ostringstream realfile, imagfile;
		realfile << "data/rand" << i << ".dat";
		imagfile << "data/rand" << i << "i" << ".dat";
		ifstream realdata(realfile.str().c_str());
		ifstream imagdata(imagfile.str().c_str());
		if (!realdata.good() || !imagdata.good())
		{
			cout << "Data files missing!" << "\n" << "Skipping..." << "\n";
			continue;
		}
		string tempstring;
		float tempfloat;

		ChunkData::Ptr data;
        data.reset(new ChunkData(i));
		complex<float> *p = data->getCpuMemory();

        ChunkData::Ptr result(new ChunkData(i));

		cout << "Reading " << i << " data units from file...";

		for (int j = 0; j < 5; j++)
		{
			std::getline(realdata, tempstring);
			std::getline(imagdata, tempstring);
		}

		for (int j = 0; j < i; j++)
		{
			realdata >> tempfloat;
			p[j].real(tempfloat);
			imagdata >> tempfloat;
			p[j].imag(tempfloat);
		}

		realdata.close();
		imagdata.close();

		cout << "  done!" << "\n";

		cout << "(p[0]: " << p[0].real() << ", " << p[0].imag();
		cout << ", p[" << i-1 << "]: " << p[i-1].real() << ", " << p[i-1].imag() << ")" << "\n";

		cout << "Computing FFT...";

		//try {
		for (int k = 0; k < TEST4TIMES; k++) {
			fft.compute(data, result, FftDirection_Forward);
		}
		//} catch (int e) {
		//	cout << "Error: " << e << "\n";
		//}

		cout << "                        done!" << "\n";
		cout << "Reading results back into memory...";

		complex<float> *r = result->getCpuMemory();

		cout << "     done!" << "\n";

		ostringstream realanswers, imaganswers;
		realanswers << "data/rand" << i << "f" << ".dat";
		imaganswers << "data/rand" << i << "if" << ".dat";
		ifstream realdataf(realanswers.str().c_str());
		ifstream imagdataf(imaganswers.str().c_str());

		if (!realdataf.good() || !imagdataf.good())
		{
			cout << "answers files missing!" << "\n" << "Skipping..." << "\n";
			continue;
		}

		ChunkData::Ptr answers;
        answers.reset(new ChunkData(i));
		complex<float> *f = answers->getCpuMemory();

		cout << "Reading " << i << " data units from answers...";

		for (int j = 0; j < 5; j++)
		{
			std::getline(realdataf, tempstring);
			std::getline(imagdataf, tempstring);
		}
		for (int j = 0; j < i; j++)
		{
			realdataf >> tempfloat;
			f[j].real(tempfloat);
			imagdataf >> tempfloat;
			f[j].imag(tempfloat);
		}

		realdataf.close();
		imagdataf.close();

		cout << " done!" << "\n";
		cout << "Computing MAXERR and NRMSD...";
		float maxerror = maxerr(answers, result);
		float nRSMD = nrmsd(answers, result);
		cout << " done!" << "\n";

		correctnessFile << i << " " << maxerror << " " << nRSMD << "\n";

		cout << "MAXERR is " << maxerror;
		if (maxerror > maxerrlim)
			cout << " > " << maxerrlim << ": FAIL!" << "\n";
		else
			cout << " < " << maxerrlim << ": WIN!" << "\n";

		cout << "NRMSD is " << nRSMD;
		if (nRSMD > nrmsdlim)
			cout << " > " << nrmsdlim << ": FAIL!" << "\n";
		else
			cout << " < " << nrmsdlim << ": WIN!" << "\n";

		//QVERIFY(maxerror < maxerrlim);
		//QVERIFY(nRSMD < nrmsdlim);

	#if defined(DUMP_RESULTS)

		cout << "    done!" << "\n";

		cout << "Dumping results to file...";

		ostringstream realresults;
		realresults << "data/" << techlib << "Results" << i << ".dat";
		ofstream outputreal(realresults.str().c_str());

		outputreal << "# Created by FFTmojtest" << "\n";
		outputreal << "# name: " << "data/" << techlib << "Results" << "Real" << "\n";
		outputreal << "# type: matrix" << "\n";
		outputreal << "# rows: " << i << "\n";
		outputreal << "# columns: 1" << "\n";
		
		for (int j = 0; j < i; j++)
		{
			outputreal << " " << r[j].real() << "\n";
		}

		outputreal.close();

		ostringstream imagresults;
		imagresults << "data/" << techlib << "Results" << i << "i" << ".dat";
		ofstream outputimag(imagresults.str().c_str());

		outputimag << "# Created by FFTmojtest" << "\n";
		outputimag << "# name: " << "data/" << techlib << "Results" << "Imag" << "\n";
		outputimag << "# type: matrix" << "\n";
		outputimag << "# rows: " << i << "\n";
		outputimag << "# columns: 1" << "\n";
		
		for (int j = 0; j < i; j++)
		{
			outputimag << " " << r[j].imag() << "\n";
		}

		outputimag.close();

		outputscript << "load " << "data/" << techlib << "Results" << i << ".dat;" << "\n";
		outputscript << "load " << "data/" << techlib << "Results" << i << "i" << ".dat;" << "\n";
		outputscript << "result = " << "data/" << techlib << "ResultsReal + " << "data/" << techlib << "ResultsImag * i;" << "\n";
		outputscript << "load " << "rand" << i << "f" << ".dat;" << "\n";
		outputscript << "load " << "rand" << i << "if" << ".dat;" << "\n";
		outputscript << "answers = a + b * i;" << "\n";
		outputscript << "length(result)" << "\n";
		outputscript << "comparison" << "\n";
	#endif
		cout << "\n";
	}

	sizes.close();
	outputscript.close();
	correctnessFile.close();
#endif
}


void FFTmojTest::testCase3()
{
#ifdef RUNTEST3
	int repDiv = repDividend;
	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream scriptname;
	scriptname << "data/" << techlib << "Comparison" << ".m";
	ofstream outputscript(scriptname.str().c_str());
	srand((unsigned)time(0));

	int size = 0;
	int i = 0;
	while (i <= endSize)
	{
		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
			break;

		else
			i = size;

	#if defined(USE_OPENCL)
	#if !defined(USE_AMD)
		if (size == 1 << 10 || size == 1 << 22)
		{
			cout << "Size " << size << " not good for ClFft. Skipping..." << "\n" << "\n";
			continue;
		}
	#endif
	#endif
		float tempfloatr;

		ChunkData::Ptr data;
        data.reset(new ChunkData(i));
		complex<float> *p = data->getCpuMemory();

        ChunkData::Ptr result(new ChunkData(i));
        complex<float> *r = result->getCpuMemory();

        ostringstream wallTimeFile;
        wallTimeFile << "data/" << techlib << "WallTimes" << i << ".dat";
        ofstream outputWallTimes(wallTimeFile.str().c_str());

		#if defined(USE_OPENCL)
		ostringstream kernelTimeFile;
        kernelTimeFile << "data/" << techlib << "KernelTimes" << i << ".dat";
	    ofstream outputKernelTimes(kernelTimeFile.str().c_str());
		#endif

		for (int j = 0; j < i; j++)
		{
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].real(tempfloatr);
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].imag(tempfloatr);
		}

		repDiv = repDividend;
		repDiv = repDiv/i;
		repDiv = (repDiv > 1<<8) ? 1<<8 : repDiv;
		repDiv = (repDiv < 1<<4) ? 1<<4 : repDiv;

        cout << "Doing fft of " << i << " " << 1 << " times..." << "\n";

		for (int j = 0; j < 1; j++)
		{
			fft.compute(data, result, FftDirection_Forward);
            r = result->getCpuMemory();
			outputWallTimes << fft.getWallExecTime() << "\n";
            #if defined (USE_OPENCL)
            outputKernelTimes << fft.getKernelExecTime() << "\n";
			#endif
		}

		outputWallTimes.close();
		#if defined (USE_OPENCL)
		outputKernelTimes.close();	
		#endif
        cout << "Done!" << "\n" << "\n";
    }
#endif



//    QVERIFY( cudaSuccess == cudaGLSetGLDevice( 0 ) );
//    QVERIFY( 0==glewInit() );
//    pVbo vbo( new Vbo(1024));
//    FFTmoj<float> mapping(vbo, make_cudaExtent(256,1,1));
//    FFTmojTestCuda( mapping.data->getCudaGlobal() );
//    QVERIFY2(true, "Failure");


}

void FFTmojTest::testCase4()
{
#ifdef RUNTEST4

	int size = TEST4SIZE;

#if defined(USE_OPENCL)
#if !defined(USE_AMD)
	if (size == 1 << 10 || size >= 1 << 22)
	{
		cout << "Size " << size << " not good for ClFft. Skipping..." << "\n" << "\n";
	}
#endif
#endif

	srand((unsigned)time(0));
	float tempfloatr;

	ChunkData::Ptr data;
    data.reset(new ChunkData(size));
	complex<float> *p = data->getCpuMemory();

    ChunkData::Ptr result(new ChunkData(size));
    complex<float> *r = result->getCpuMemory();

    ChunkData::Ptr dataArray[TEST4TIMES];
    ChunkData::Ptr resultArray[TEST4TIMES];
    complex<float> *pointers[TEST4TIMES];
    complex<float> *results[TEST4TIMES];
    for (int j = 0; j < TEST4TIMES; j++)
    {
        dataArray[j].reset(new ChunkData(size));
        pointers[j] = dataArray[j]->getCpuMemory();
		resultArray[j].reset(new ChunkData(size));
        results[j] = resultArray[j]->getCpuMemory();
    }

    /*
	for (int j = 0; j < size; j++)
	{
		tempfloatr = (float)rand()/(float)RAND_MAX;
		p[j].real(tempfloatr);
		tempfloatr = (float)rand()/(float)RAND_MAX;
		p[j].imag(tempfloatr);
	}
    */

    cout << "Doing fft of " << size << " " << TEST4TIMES << " times..." << "\n";

    for (int j = 0; j < TEST4TIMES; j++)
//    for (int j = 0; j < 1; j++)
    {
        fft.compute(dataArray[j], resultArray[j], FftDirection_Forward);
//        fft.compute(result, data, FftDirection_Forward);
		cout << j << "\n"; //".";
        results[j] = resultArray[j]->getCpuMemory();
        cout << j << "\n"; //".";
        pointers[j] = dataArray[j]->getCpuMemory();
        cout << j << "\n"; //".";
    }
#endif
}

void FFTmojTest::testCase5()
{
#ifdef RUNTEST5
	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream plantimesfile;
	plantimesfile << "data/" << techlib << "PlanTimes" << ".dat";
	ofstream plantimes(plantimesfile.str().c_str());

	int size = 0;
	int i = 0;
	int init = 0;
	sizes >> size;
	init = size;
	float createTime = 0;
	float bakeTime = 0;
	float reBatchBakeTime = 0;
	float reSizeBakeTime = 0;
	int batchSize = 0;

	//fft.createPlan(128);
	//fft.bake();
	//fft.setSize(size);

	//{	
	//	TIME_STFT TaskTimer tt("Baking plan for %i", size);
	//	fft.bake();
	//	reSizeBakeTime = fft.getLastBakeTime();
	//	tt.elapsedTime();
	//}

	//fft.clearPlans();
	
	while (i <= endSize)
	{
		i = size;

		{
			TIME_STFT TaskTimer tt("Creating plan for %i", size);
			fft.createPlan(size);
			createTime = tt.elapsedTime();
		}

		{
			//TIME_STFT TaskTimer tt("Baking plan for %i", size);
			cout << "Baking fresh of " << size << "\n";
			fft.bake();
			bakeTime = fft.getLastBakeTime();
		}

		batchSize = (2<<22)/size;
		fft.setBatchSize(batchSize);
		
		{
			//TIME_STFT TaskTimer tt("Baking plan for %i and batch %i", size, batchSize);
			cout << "Baking batch resize of " << batchSize << "\n";
			fft.bake();
			reBatchBakeTime = fft.getLastBakeTime();
		}

		plantimes << size << " " << createTime << " " << bakeTime;
		plantimes << " " << reBatchBakeTime << " " << reSizeBakeTime << "\n";

		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
		{
			break;
		}

		//fft.setSize(size);
		//fft.setBatchSize(1);

		//{
		//	//TIME_STFT TaskTimer tt("Baking plan for %i", size);
		//	cout << "Baking resized to " << size << "\n";
		//	fft.bake();
		//	reSizeBakeTime = fft.getLastBakeTime();
		//}

		fft.clearPlans();
	}

	sizes.close();
	plantimes.close();
#endif
}

void FFTmojTest::testCase5A()
{
#ifdef RUNTEST5A
	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream plantimesfile;
	plantimesfile << "data/" << techlib << "PlanTimes" << ".dat";
	ofstream plantimes(plantimesfile.str().c_str());

	int size = 0;
	int i = 0;
	sizes >> size;
	float createTime = 0;

	//fft.createPlan(128);
	//fft.bake();
	//fft.setSize(size);

	//{	
	//	TIME_STFT TaskTimer tt("Baking plan for %i", size);
	//	fft.bake();
	//	reSizeBakeTime = fft.getLastBakeTime();
	//	tt.elapsedTime();
	//}

	//fft.clearPlans();
	
	while (i <= endSize)
	{
		i = size;

		{
			TIME_STFT TaskTimer tt("Creating plan for %i", size);
			fft.createPlan(size);
			createTime = tt.elapsedTime();
		}

		plantimes << size << " " << createTime << "\n";

		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
		{
			break;
		}

		//fft.clearPlans();
	}

	sizes.close();
	plantimes.close();
#endif
}


void FFTmojTest::testCase6()
{
#ifdef RUNTEST6
	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream plantimesfile;
	plantimesfile << "data/" << techlib << "PlanReBakeTimes" << ".dat";
	ofstream plantimes(plantimesfile.str().c_str());

	int size = 0;
	int i = 0;
	int init = 0;
	sizes >> size;
	init = size;
	float createTime = 0;
	float bakeTime = 0;
	float reBatchBakeTime = 0;
	float reSizeBakeTime = 0;
	int batchSize = 0;

	fft.createPlan(128);
	fft.bake();

	while (i <= endSize)
	{
		i = size;

		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
		{
			break;
		}

		fft.setSize(size);
		
		{
			//TIME_STFT TaskTimer tt("Baking plan for %i", size);
			cout << "Baking resized to " << size << "\n";
			fft.bake();
			reSizeBakeTime = fft.getLastBakeTime();
		}

		plantimes << size << " " << reSizeBakeTime << "\n";

	}

	sizes.close();
	plantimes.close();
#endif
}

void FFTmojTest::testCase6n()
{
#ifdef RUNTEST6N
	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream plantimesfile;
	plantimesfile << "data/" << techlib << "PlanRe1920000BakeTimes" << ".dat";
	ofstream plantimes(plantimesfile.str().c_str());

	int size = 0;
	int i = 0;
	int init = 0;
	sizes >> size;
	init = size;
	float createTime = 0;
	float bakeTime = 0;
	float re1920000BakeTime = 0;
	float reSizeBakeTime = 0;
	int batchSize = 0;

	fft.createPlan(128);
	fft.bake();

	while (i <= endSize)
	{
		i = size;

		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
		{
			break;
		}
		if (size == 1920000)
			continue;

		fft.setSize(size);
		
		{
			//TIME_STFT TaskTimer tt("Baking plan for %i", size);
			cout << "Baking resized to " << size << "\n";
			fft.bake();
			reSizeBakeTime = fft.getLastBakeTime();
		}

		fft.setSize(1920000);
		
		{
			//TIME_STFT TaskTimer tt("Baking plan for %i", size);
			cout << "Baking resized to " << size << "\n";
			fft.bake();
			re1920000BakeTime = fft.getLastBakeTime();
		}

		plantimes << size << " " << reSizeBakeTime << " " << re1920000BakeTime << "\n";

	}

	sizes.close();
	plantimes.close();
#endif
}

void FFTmojTest::testCase7()
{
#ifdef RUNTEST7

#endif
}

void FFTmojTest::testCase8()
{
#ifdef RUNTEST8
	
#endif
}

void FFTmojTest::testCase9()
{
#ifdef RUNTEST9

	float tempfloatr;

	ostringstream randomness;
	randomness << "data/" << "randomfile" << ".dat";
	ofstream randomfile(randomness.str().c_str());

	cout << "Storing " << (1<<22) << " data units to file...";	

	for (int i = 0; i < (1<<22); i++)
	{
		tempfloatr = (float)rand()/(float)RAND_MAX;
		randomfile << tempfloatr << " ";
		tempfloatr = (float)rand()/(float)RAND_MAX;
		randomfile << tempfloatr << "\n";
	}

	randomfile.close();

	try
    {
        pBuffer data_;
        data_.reset ( new Buffer(Interval(0, 1<<22), 1, 2));
        float* re = data_->getChannel (0)->waveform_data ()->getCpuMemory ();
		float* im = data_->getChannel (1)->waveform_data ()->getCpuMemory ();

		srand(259872697194);
		//srand(698745798351);
		float tempfloatr;
		
		for (int i=0; i<data_->number_of_samples (); ++i)
		{
			tempfloatr = (float)rand()/(float)RAND_MAX;
			re[i] = tempfloatr;
			tempfloatr = (float)rand()/(float)RAND_MAX;
			im[i] = tempfloatr;
		}

        const char* filename = "data/randomfile.h5";
        Hdf5Buffer::saveBuffer ( filename, *data_, 0);

        cout << "OK" << endl;
    }
    catch (const exception& x)
    {
        cout << "Error: " << vartype(x) << endl
                  << "Details: " << x.what() << endl;
    }


#endif
}

void FFTmojTest::testCase10()
{
#ifdef RUNTEST10
	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());
		
	float tempfloatr;

	int size = 0;
	int i = 0;
	while (i <= endSize)
	{
		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
			break;

		else
			i = size;
	
		ChunkData::Ptr data;
        data.reset(new ChunkData(i));
		complex<float> *p = data->getCpuMemory();

        ChunkData::Ptr result(new ChunkData(i));

		srand(259872697194);
		//srand(698745798351);

		float tempfloatr;
		
		for (int j = 0; j < i; j++)
		{
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].real(tempfloatr);
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].imag(tempfloatr);
		}

		if (size == 1<<22)
		{
			Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
			chunk->transform_data = data;
			ostringstream randomdataname;
			randomdataname << "data/" << "RandomData" << ".h5";;
			Hdf5Chunk::saveChunk(randomdataname.str().c_str(), *chunk );
		}

		cout << "Computing FFT...";

		fft.compute(data, result, FftDirection_Forward);

		cout << "                        done!" << "\n";
		cout << "Reading results back into memory...";

		complex<float> *r = result->getCpuMemory();

		cout << "     done!" << "\n";

		cout << "dumping results... ";

		ostringstream resultsname;
		resultsname << "data/" << techlib << "Results" << i << ".h5";
		Tfr::pChunk chunk( new Tfr::StftChunk(size, Tfr::StftParams::WindowType_Rectangular, 0, true));
		chunk->transform_data = result;
		Hdf5Chunk::saveChunk( resultsname.str().c_str(), *chunk);

		//ostringstream resultsname;
		//resultsname << "data/" << techlib << "Results" << i << ".dat";
		//ofstream resultsout(resultsname.str().c_str());

		//for (int j = 0; j < i; j++)
		//{
		//	resultsout << r[j].real() << " ";
		//	resultsout << r[j].imag() << "\n";
		//}

		//resultsout.close();
		cout << " done!";
	}
#endif
}

void FFTmojTest::testCase11()
{
#ifdef RUNTEST11

	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream scriptname;
	scriptname << "data/" << techlib << "Precision.m";
	ofstream scriptfile(scriptname.str().c_str());

	int size = 0;
	int index = 0;
	int i = 0;

	while (i <= endSize)
	{
		index++;
		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
			break;

		else
			i = size;

		//scriptfile << "fid = fopen(\"" << techlib << "Results" << i << ".dat\", \"r\");" << "\n";
		//scriptfile << techlib << "Results" << i << " = fscanf(fid, '%f', [2, inf]).'; fclose(fid);" << "\n";
		scriptfile << "tic;" << "\n";
		scriptfile << "load " << techlib << "Results" << i << ".dat -ascii;" << "\n";
		scriptfile << "result = " << techlib << "Results" << i << "(:,1) + " << techlib << "Results" << i << "(:,2)*i;" << "\n";
		scriptfile << "clear " << techlib << "Results" << i << ";" << "\n";
		scriptfile << "fftnum = fft(randomfile(1:" << i << ",1) + randomfile(1:" << i << ",2)*i);" << "\n";
		scriptfile << techlib << "error(" << index << ",1) = " << i << ";" << "\n";
		scriptfile << techlib << "error(" << index << ",2) = max(abs(fftnum - result)) / (max(fftnum) - min(fftnum));" << "\n";
		scriptfile << techlib << "error(" << index << ",3) = sqrt(mean(abs(fftnum - result).^2)) / (max(fftnum) - min(fftnum));" << "\n";
		scriptfile << techlib << "error(" << index << ",:)" << "\n";
		scriptfile << "clear fftnum;" << "\n";
		scriptfile << "toc;" << "\n\n";
	}
	
	scriptfile << "\n" << "save " << techlib << "Error.dat " << techlib << "error;" << "\n";

	scriptfile.close();
	sizes.close();

#endif
}

void FFTmojTest::testCase12()
{
#ifdef RUNTEST12

	/*try
    {
        pBuffer data_;
        data_.reset ( new Buffer(Interval(0, 10), 1, 2));
        float* re = data_->getChannel (0)->waveform_data ()->getCpuMemory ();
		float* im = data_->getChannel (1)->waveform_data ()->getCpuMemory ();
        for (int i=0; i<data_->number_of_samples (); ++i)
		{
            re[i] = i*10;
			im[i] = i*100;
		}

        const char* filename = "hdf5test.h5";
        Hdf5Buffer::saveBuffer ( filename, *data_, 0);

        cout << "OK" << endl;
    }
    catch (const exception& x)
    {
        cout << "Error: " << vartype(x) << endl
                  << "Details: " << x.what() << endl;
    }*/

	ostringstream sizefile;
	sizefile << "data/" << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	int size = 0;
	int i = 0;
	while (i <= endSize)
	{
		sizes >> size;
		if (size == i) //better to check if EOF, but yeah
			break;

		else
			i = size;

		try
		{
			pBuffer data_;
			data_.reset ( new Buffer(Interval(0, i), 1, 2));
			float* re = data_->getChannel (0)->waveform_data ()->getCpuMemory ();
			float* im = data_->getChannel (1)->waveform_data ()->getCpuMemory ();

			ostringstream resultsname;
			resultsname << "data/" << techlib << "Results" << i << ".dat";
			ifstream resultsin(resultsname.str().c_str());

			for (int j=0; j<data_->number_of_samples (); ++j)
			{
				resultsin >> re[j];
				resultsin >> im[j];
			}

			resultsin.close();

			ostringstream h5name;
			h5name << "data/" << techlib << "Results" << i << ".h5";

			Hdf5Buffer::saveBuffer ( h5name.str().c_str(), *data_, 0);

			cout << "OK" << endl;
		}
		catch (const exception& x)
		{
			cout << "Error: " << vartype(x) << endl
					  << "Details: " << x.what() << endl;
		}
	}

#endif
}

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
