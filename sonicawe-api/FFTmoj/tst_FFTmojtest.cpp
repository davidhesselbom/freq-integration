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
	complex<float> min(ChunkData::Ptr P);
	complex<float> mean(ChunkData::Ptr P);
	ChunkData::Ptr abs(ChunkData::Ptr P);
	ChunkData::Ptr diff(ChunkData::Ptr P, ChunkData::Ptr R);
	ChunkData::Ptr mult(ChunkData::Ptr P, ChunkData::Ptr R);
	float maxerr(ChunkData::Ptr P, ChunkData::Ptr R);
	float nrmsd(ChunkData::Ptr P, ChunkData::Ptr R);
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
	complex<float> difference = max(P)-min(P);
	error /= std::abs(difference);
	return error.real();
}

float FFTmojTest::nrmsd(ChunkData::Ptr P, ChunkData::Ptr R)
{
    complex<float> normRMSD = sqrt(mean(mult(abs(diff(P, R)), abs(diff(P, R))))) / (max(P)-min(P));
	return normRMSD.real();
}

void FFTmojTest::initTestCase()
{
    a.show(); // glew needs an OpenGL context
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
			static FftClFft fft;
		    string techlib = "ClFft";
		#endif
	#elif USE_CUDA
		static FftCufft;
	    string techlib = "CuFft";
	#else 
		static FftOoura fft;
	    string techlib = "Ooura";
	#endif

	ostringstream filename, scriptname;
	filename << techlib << "Sizes" << ".dat";
	ofstream outputfile(filename.str().c_str());
	scriptname << techlib << "Sizes" << ".m";
	ofstream outputscript(scriptname.str().c_str());

	const int startSize = 1 << 8;
	const int endSize = 1 << 22;

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
			static FftClFft fft;
		    string techlib = "ClFft";
		#endif
	#elif USE_CUDA
		static FftCufft;
	    string techlib = "CuFft";
	#else 
		static FftOoura fft;
	    string techlib = "Ooura";
	#endif

	ostringstream sizefile;
	sizefile << techlib << "Sizes" << ".dat";
	ifstream sizes(sizefile.str().c_str());

	ostringstream scriptname;
	scriptname << techlib << "Comparison" << ".m";
	ofstream outputscript(scriptname.str().c_str());

	const int startSize = 1 << 8;
	const int endSize = 1 << 22;

	const float maxerrlim = 0.00001; //1e-5
	const float nrmsdlim = 0.00000001; //1e-8

	int size = 0;
	int i = 0;
	while (i <= endSize)
	{
		sizes >> size;
		if (i == size) //better to check if EOF, but yeah
			break;
		else
			i = size;
		ostringstream realfile, imagfile;
		realfile << "rand" << i << ".dat";
		imagfile << "rand" << i << "i" << ".dat";
		ifstream realdata(realfile.str().c_str());
		ifstream imagdata(imagfile.str().c_str());
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

		cout << "MAXERR is " << maxerror;
		if (maxerror > maxerrlim)
		{
			cout << " > " << maxerrlim << ": FAIL!" << endl;
		}
		else
		{
			cout << " < " << maxerrlim << ": WIN!" << endl;
		}
        QVERIFY(maxerror < maxerrlim);

		cout << "NRMSD is " << nRSMD;
		if (nRSMD > nrmsdlim)
		{
			cout << " > " << nrmsdlim << ": FAIL!" << endl;
		}
		else
		{
			cout << " < " << nrmsdlim << ": WIN!" << endl;
		}
		QVERIFY(nRSMD < nrmsdlim);

		/*			
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
		*/
		cout << endl;
	}

	sizes.close();
	outputscript.close();

}

void FFTmojTest::testCase3()
{
    using namespace std;
    using namespace Tfr;
    string techlib = "";
	return;
#ifdef USE_OPENCL
    #ifdef USE_AMD
        FftClAmdFft fft = FftClAmdFft();
        ofstream outputfile("ClAmdFft");
    #else
        static FftClFft fft;
        ofstream outputfile("ClFft");
    #endif
#elif USE_CUDA
    static FftCufft;
    ofstream outputfile("CuFft");
#else 
	static FftOoura fft;
    ofstream outputfile("Ooura");
#endif
    ChunkData::Ptr data;
//    ifstream inputfile("rand12.dat");
    const int size1 = 1 << 18;
	const int size2 = 1 << 18;

    //float input[size1];
//    for (int i = 0; i < size1; i++)
//	{
        //inputfile >> input[i];
//        input[i] = ((float)rand()/(float)RAND_MAX);
//    }
//    inputfile.close();

//    srand ( time(NULL) );

    // Chunk size:
    // fails on fusion cpu at 200000000 (134217728)
    // largest ok on fusion cpu at 100000000 (67108864), 30 sec
    // largest ok on rampage cpu at 100000000 (67108864), 12.7 sec
    // fails on fusion gpu at 40000000
    // largest ok on fusion gpu at 20000000, 15 sec

    // We want to test for (2^10, 2^20)

        // Or maybe 2^8 to 2^22?

    int lastsize = 0;
    int uniques = 0;




    for (int N = size1; N <= size2; N++)
    {
		const int currentsize = fft.lChunkSizeS(N);
		
		if (lastsize == currentsize) { continue; }
        else { lastsize = currentsize; }
        uniques++;
		cout << "size: " << currentsize << "\n";

        data.reset(new ChunkData(currentsize));
		complex<float> *p = data->getCpuMemory();


        {
//            for (int i = 0; i < N; i++)
//            {
//                p[i].real(0);
//                p[i].imag(1);
//            }

            ChunkData::Ptr result(new ChunkData(N));
//            TaskTimer timer("Running FFT, run #1");
              fft.compute(data, result, FftDirection_Forward);
//            fft.compute(data, data, DataStorageSize(1, 4), FftDirection_Forward);
//            timeElapsed = timer.elapsedTime();
        }


		// Compute on new data with the same size, now that we have a plan
//        data.reset(new ChunkData(currentsize));
//        p = data->getCpuMemory();

        {
/*
            for (int i = 0; i < N; i++)
            {
                //p[i].real((float)rand()/(float)RAND_MAX);
                //p[i].imag((float)rand()/(float)RAND_MAX);
                //p[i].real(input[i]);
                //p[i].imag(0);
            }
*/
//			cout << p[0].real() << ", " << p[0].imag() << endl;

//            ChunkData::Ptr result(new ChunkData(N));
//			TaskTimer timer("Running ClFft, run #2");
//            fft.compute(data, data, FftDirection_Forward);
//			complex<float> *r = result->getCpuMemory();
//			ofstream outputfile("rand12clfft.dat");
            //p = data->getCpuMemory();
/*
            for(int j = 0; j < N; j++)
			{
				outputfile << p[j] << endl;
			}
			outputfile.close();
*/
        }
    }

    cout << "uniques: " << uniques << endl;
	lastsize = 0;
	uniques = 0;


    for (int N = size1; N <= size2; N++)
    {
		float timeElapsed = 0;
		const int currentsize = fft.lChunkSizeS(N);

        if (lastsize == currentsize) { continue; }
        else { lastsize = currentsize; }
        uniques++;
        cout << "size: " << currentsize << endl;

        data.reset(new ChunkData(currentsize));
        complex<float> *p = data->getCpuMemory();

        /*
        {
            for (int i = 0; i < N; i++)
            {
                p[i].real(0);
                p[i].imag(1);
            }

            //ChunkData::Ptr result(new ChunkData(N));
            TaskTimer timer("Running ClFft, run #1");
            fft.compute(data, data, DataStorageSize(1, 4), FftDirection_Forward);
        }
        */

        // Compute on new data with the same size, now that we have a plan
//        data.reset(new ChunkData(currentsize));
//        p = data->getCpuMemory();

        {
/*
            for (int i = 0; i < N; i++)
            {
                //p[i].real((float)rand()/(float)RAND_MAX);
                //p[i].imag((float)rand()/(float)RAND_MAX);
                //p[i].real(input[i]);
                //p[i].imag(0);
            }
*/
//			cout << p[0].real() << ", " << p[0].imag() << endl;

            ChunkData::Ptr result(new ChunkData(currentsize));
            TaskTimer timer("Running Fft, run #3");
			int reps = (1 << 24) / N;
            for (int i = 0; i < reps; i++)
            {
                fft.compute(data, data, FftDirection_Forward);
                complex<float> *r = data->getCpuMemory();
            }
            complex<float> *r = data->getCpuMemory();
            timeElapsed = timer.elapsedTime();
            outputfile << currentsize << " " << reps << " " << timeElapsed << endl;
//			ofstream outputfile("rand12clfft.dat");
            //p = data->getCpuMemory();
/*
            for(int j = 0; j < N; j++)
            {
                outputfile << p[j] << endl;
            }
            outputfile.close();
*/
        }
    }

    outputfile.close();

//    QVERIFY( cudaSuccess == cudaGLSetGLDevice( 0 ) );
//    QVERIFY( 0==glewInit() );
//    pVbo vbo( new Vbo(1024));
//    FFTmoj<float> mapping(vbo, make_cudaExtent(256,1,1));
//    FFTmojTestCuda( mapping.data->getCudaGlobal() );
//    QVERIFY2(true, "Failure");


}

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
