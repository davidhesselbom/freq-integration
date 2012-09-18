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

#include "sawe/project_header.h"
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QGLWidget>
#include <fstream>
#include <iostream>

using namespace std;
using namespace Tfr;

class FFTmojTest : public QObject
{
    Q_OBJECT

public:
    FFTmojTest();

    QGLWidget a;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1();
	void testCase2();
	void testCase3();

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

#define maxerrlim 0.00001 //1e-5
#define nrmsdlim 0.00000001 //1e-8
#define startSize 1 << 8
#define endSize 1 << 22
};

FFTmojTest::FFTmojTest()
{
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

void FFTmojTest::initTestCase()
{
    //a.show(); // glew needs an OpenGL context
}

void FFTmojTest::cleanupTestCase()
{
}

void FFTmojTest::testCase1()
{
	#ifdef USE_OPENCL
		#ifdef USE_AMD
			FftClAmdFft fft = FftClAmdFft();
		    string techlib = "ClAmdFft";
		#else
            FftClFft fft;
		    string techlib = "ClFft";
		#endif
	#elif USE_CUDA
        FftCufft fft;
	    string techlib = "CuFft";
	#else 
        FftOoura fft;
	    string techlib = "Ooura";
	#endif

	ostringstream filename, scriptname;
	filename << techlib << "Sizes" << ".dat";
	ofstream outputfile(filename.str().c_str());
	scriptname << techlib << "Sizes" << ".m";
	ofstream outputscript(scriptname.str().c_str());

	int i = startSize;
	while (i <= endSize)
	{
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
}

void FFTmojTest::testCase2()
{
	#ifdef USE_OPENCL
		#ifdef USE_AMD
			FftClAmdFft fft = FftClAmdFft();
		    string techlib = "ClAmdFft";
		#else
            FftClFft fft;
		    string techlib = "ClFft";
		#endif
	#elif USE_CUDA
        FftCufft fft;
	    string techlib = "CuFft";
	#else 
        FftOoura fft;
	    string techlib = "Ooura";
	#endif

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
		realfile << "rand" << i << ".dat";
		imagfile << "rand" << i << "i" << ".dat";
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
		fft.compute(data, result, FftDirection_Forward);
		//} catch (int e) {
		//	cout << "Error: " << e << endl;
		//}

		cout << "                        done!" << endl;
		cout << "Reading results back into memory...";

		complex<float> *r = result->getCpuMemory();

		cout << "     done!" << endl;

		ostringstream realfacit, imagfacit;
		realfacit << "rand" << i << "f" << ".dat";
		imagfacit << "rand" << i << "if" << ".dat";
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
}

void FFTmojTest::testCase3()
{
	#ifdef USE_OPENCL
		#ifdef USE_AMD
			FftClAmdFft fft = FftClAmdFft();
		    string techlib = "ClAmdFft";
		#else
            FftClFft fft;
		    string techlib = "ClFft";
		#endif
	#elif USE_CUDA
        FftCufft fft;
	    string techlib = "CuFft";
	#else 
        FftOoura fft;
	    string techlib = "Ooura";
	#endif

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

		for (int j = 0; j < i; j++)
		{
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].real(tempfloatr);
			tempfloatr = (float)rand()/(float)RAND_MAX;
			p[j].imag(tempfloatr);
		}

        cout << "Doing fft of " << i << " " << (1 << 24)/i << " times..." << endl;

		for (int j = 0; j <= (1 << 24)/i; j++)
		{
			fft.compute(data, result, FftDirection_Forward);
		}

        cout << "Done!" << endl << endl;
    }

//    QVERIFY( cudaSuccess == cudaGLSetGLDevice( 0 ) );
//    QVERIFY( 0==glewInit() );
//    pVbo vbo( new Vbo(1024));
//    FFTmoj<float> mapping(vbo, make_cudaExtent(256,1,1));
//    FFTmojTestCuda( mapping.data->getCudaGlobal() );
//    QVERIFY2(true, "Failure");


}

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
