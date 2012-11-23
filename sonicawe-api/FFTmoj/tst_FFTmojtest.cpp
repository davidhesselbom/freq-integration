/*
TODO:

- Fixa preprocessorflaggor och defines f�r allt m�jligt som ska kunna variera:
     - Placeness (f�r testet allts�. F�r clamdfft kan l�ggas till ifsats f�r om input och output �r samma)
     - Upprepningar
     - Profiling och inte
     - Alla storlekar eller bara 2-potenser
     (- Om facit ska l�sas fr�n fil eller r�knas ut av fftooura?)
- Fixa till utskrifterna s� de ser prydliga ut
- Fixa wall clock time-tester
- Fixa s� att inl�sning och utskrift sker fr�n och till data-mappen
- Allokera allt minne fr�n b�rjan (f�r varje storlek) istf att anv�nda samma minne om och om igen i test 3
- Fixa tidtagning av planskapandet
- Snygga till testen generellt
- Implementera resten av compute-funktionerna f�r clAmdFft
     - allting clAmdFft har dock l�gre prio just nu, det viktigaste �r att testen g�r att k�ra p� nVidia-burkar
- Kolla upp varf�r ClAmdFft kraschar vid upprepade anrop p� mer �n 2^13...
*/

#define NOGUI
#ifdef _MSC_VER
#include <stdlib.h> //  error C2381: 'exit' : redefinition; __declspec(noreturn) differs
#include <time.h>
#endif

#define RUNTEST1
//#define RUNTEST2
//#define RUNTEST3
#define RUNTEST4
#define TEST4SIZE 1<<15
#define TEST4TIMES 10
#define PLACENESS "inplace"
#define CL_PROFILING
#define ALLSIZES
#define REPDIVIDEND 1 << 24
#define REPNUMERATOR 0
#define PREALLOCATE
#define maxerrlim 0.00001 //1e-5
#define nrmsdlim 0.00000001 //1e-8
#define startSize 1 << 8
#define endSize 1 << 22

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

using namespace std;
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
	A() { cout << this << " " << __FUNCTION__ << endl; }
	A(int) { cout << this << " " << __FUNCTION__ << endl; }
	~A() { cout << this << " " << __FUNCTION__ << endl; }

	void operator=(A&) { cout << this << " " << __FUNCTION__ << endl; }
};

void FFTmojTest::initTestCase()
{
//	A a;
//	a = A();
#ifdef USE_OPENCL
    #ifdef USE_AMD
//        fft = FftClAmdFft();
        techlib = "data/ClAmdFft";
    #else
        techlib = "data/ClFft";
    #endif
#elif USE_CUDA
    techlib = "data/CuFft";
#else
    techlib = "data/Ooura";
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
	filename << techlib << "Sizes" << ".dat";
	ofstream outputfile(filename.str().c_str());
	scriptname << techlib << "Sizes" << ".m";
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

	cout << "Number of sizes: " << numSize << endl;
	cout << "Sum of sizes: " << sumSize << endl;

#endif
}

void FFTmojTest::testCase2()
{
#ifdef RUNTEST2
	ostringstream sizefile;
	sizefile << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream scriptname;
	scriptname << techlib << "Comparison" << ".m";
	ofstream outputscript(scriptname.str().c_str());

	ostringstream correctnessName;
	correctnessName << techlib << "Correctness" << ".dat";
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
		if (size == 1 << 10 || size == 1 << 22)
		{
			cout << "Size " << size << " not good for ClFft. Skipping..." << endl << endl;
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
			cout << "Data files missing!" << endl << "Skipping..." << endl;
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

		cout << "  done!" << endl;

		cout << "(p[0]: " << p[0].real() << ", " << p[0].imag();
		cout << ", p[" << i-1 << "]: " << p[i-1].real() << ", " << p[i-1].imag() << ")" << endl;

		cout << "Computing FFT...";

		//try {
		for (int k = 0; k < 10; k++) {
			fft.compute(data, result, FftDirection_Forward);
		}
		//} catch (int e) {
		//	cout << "Error: " << e << endl;
		//}

		cout << "                        done!" << endl;
		cout << "Reading results back into memory...";

		complex<float> *r = result->getCpuMemory();

		cout << "     done!" << endl;

		ostringstream realfacit, imagfacit;
		realfacit << "data/rand" << i << "f" << ".dat";
		imagfacit << "data/rand" << i << "if" << ".dat";
		ifstream realdataf(realfacit.str().c_str());
		ifstream imagdataf(imagfacit.str().c_str());

		if (!realdataf.good() || !imagdataf.good())
		{
			cout << "Facit files missing!" << endl << "Skipping..." << endl;
			continue;
		}

		ChunkData::Ptr facit;
        facit.reset(new ChunkData(i));
		complex<float> *f = facit->getCpuMemory();

		cout << "Reading " << i << " data units from facit...";

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

		cout << " done!" << endl;
		cout << "Computing MAXERR and NRMSD...";
		float maxerror = maxerr(facit, result);
		float nRSMD = nrmsd(facit, result);
		cout << " done!" << endl;

		correctnessFile << i << " " << maxerror << " " << nRSMD << endl;

		cout << "MAXERR is " << maxerror;
		if (maxerror > maxerrlim)
			cout << " > " << maxerrlim << ": FAIL!" << endl;
		else
			cout << " < " << maxerrlim << ": WIN!" << endl;

		cout << "NRMSD is " << nRSMD;
		if (nRSMD > nrmsdlim)
			cout << " > " << nrmsdlim << ": FAIL!" << endl;
		else
			cout << " < " << nrmsdlim << ": WIN!" << endl;

		//QVERIFY(maxerror < maxerrlim);
		//QVERIFY(nRSMD < nrmsdlim);

	#if defined(DUMP_RESULTS)

		cout << "    done!" << endl;

		cout << "Dumping results to file...";

		ostringstream realresults;
		realresults << techlib << "Results" << i << ".dat";
		ofstream outputreal(realresults.str().c_str());

		outputreal << "# Created by FFTmojtest" << "\n";
		outputreal << "# name: " << techlib << "Results" << "Real" << "\n";
		outputreal << "# type: matrix" << "\n";
		outputreal << "# rows: " << i << "\n";
		outputreal << "# columns: 1" << "\n";
		
		for (int j = 0; j < i; j++)
		{
			outputreal << " " << r[j].real() << "\n";
		}

		outputreal.close();

		ostringstream imagresults;
		imagresults << techlib << "Results" << i << "i" << ".dat";
		ofstream outputimag(imagresults.str().c_str());

		outputimag << "# Created by FFTmojtest" << "\n";
		outputimag << "# name: " << techlib << "Results" << "Imag" << "\n";
		outputimag << "# type: matrix" << "\n";
		outputimag << "# rows: " << i << "\n";
		outputimag << "# columns: 1" << "\n";
		
		for (int j = 0; j < i; j++)
		{
			outputimag << " " << r[j].imag() << "\n";
		}

		outputimag.close();

		outputscript << "load " << techlib << "Results" << i << ".dat;" << "\n";
		outputscript << "load " << techlib << "Results" << i << "i" << ".dat;" << "\n";
		outputscript << "result = " << techlib << "ResultsReal + " << techlib << "ResultsImag * i;" << "\n";
		outputscript << "load " << "rand" << i << "f" << ".dat;" << "\n";
		outputscript << "load " << "rand" << i << "if" << ".dat;" << "\n";
		outputscript << "facit = a + b * i;" << "\n";
		outputscript << "length(result)" << "\n";
		outputscript << "comparison" << "\n";
	#endif
		cout << endl;
	}

	sizes.close();
	outputscript.close();
	correctnessFile.close();
#endif
}


void FFTmojTest::testCase3()
{
#ifdef RUNTEST3
	ostringstream sizefile;
	sizefile << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream scriptname;
	scriptname << techlib << "Comparison" << ".m";
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
			cout << "Size " << size << " not good for ClFft. Skipping..." << endl << endl;
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

        ostringstream timefile;
        timefile << techlib << "Times" << i << ".dat";
        ofstream outputtimes(timefile.str().c_str());

		for (int j = 0; j < i; j++)
		{
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].real(tempfloatr);
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].imag(tempfloatr);
		}

        cout << "Doing fft of " << i << " " << "1" << " times..." << endl;

		for (int j = 0; j <= 1; j++)
		{
			fft.compute(data, result, FftDirection_Forward);
            //r = result->getCpuMemory();
            #if defined (USE_OPENCL)
            outputtimes << fft.getKernelExecTime() << "\n";
			#endif
		}
		outputtimes.close();
        cout << "Done!" << endl << endl;
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
		cout << "Size " << size << " not good for ClFft. Skipping..." << endl << endl;
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

    cout << "Doing fft of " << size << " " << TEST4TIMES << " times..." << endl;

    for (int j = 0; j < TEST4TIMES; j++)
//    for (int j = 0; j < 1; j++)
    {
        fft.compute(dataArray[j], resultArray[j], FftDirection_Forward);
//        fft.compute(result, data, FftDirection_Forward);
		cout << j << endl; //".";
        results[j] = resultArray[j]->getCpuMemory();
        cout << j << endl; //".";
        pointers[j] = dataArray[j]->getCpuMemory();
        cout << j << endl; //".";
    }
#endif
}

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
