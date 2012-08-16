#include "tfr/fftimplementation.h"
#include "tfr/clamdfft/fftclamdfft.h"
//#include "tfr/clfft/fftclfft.h"
//#include "tfr/fftooura.h"
#include "sawe/project_header.h"
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QGLWidget>
#include <fstream>;
#include <iostream>;

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
};

FFTmojTest::FFTmojTest()
{
}

/*
#include "cudaPitchedPtrType.h"
#include "CudaException.h"

void FFTmojTestCuda( cudaPitchedPtrType<float> data );
*/

void FFTmojTest::initTestCase()
{
    a.show(); // glew needs an OpenGL context
}

void FFTmojTest::cleanupTestCase()
{
}

void FFTmojTest::testCase1()
{
    using namespace std;
    using namespace Tfr;
    //static FftOoura fft;
    //static FftClFft fft;
    FftClAmdFft fft = FftClAmdFft();

    ChunkData::Ptr data;
    ifstream inputfile("rand12.dat");
    const int size1 = 1024;
    int lastsize = 0;
    int currentsize = size1;
    int uniques = 0;


    float input[size1];
    for (int i = 0; i < size1; i++)
	{
        //inputfile >> input[i];
        input[i] = ((float)rand()/(float)RAND_MAX);
    }
    inputfile.close();

    srand ( time(NULL) );

    // Chunk size:
    // fails on fusion cpu at 200000000 (134217728)
    // largest ok on fusion cpu at 100000000 (67108864), 30 sec
    // largest ok on rampage cpu at 100000000 (67108864), 12.7 sec
    // fails on fusion gpu at 40000000
    // largest ok on fusion gpu at 20000000, 15 sec

    // We want to test for (2^10, 2^20)

    for (int N = size1; N <= size1+200; N++)
    {
        currentsize = fft.lChunkSizeS(N+1);
		
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

            //ChunkData::Ptr result(new ChunkData(N));
//			TaskTimer timer("Running ClFft, run #2");
            fft.compute(data, data, FftDirection_Forward);
			//complex<float> *r = result->getCpuMemory();
//			ofstream outputfile("rand12clfft.dat");
            p = data->getCpuMemory();
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

    for (int N = size1; N <= size1+200; N++)
    {
        currentsize = fft.lChunkSizeS(N+1);

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

            //ChunkData::Ptr result(new ChunkData(N));
//			TaskTimer timer("Running ClFft, run #2");
            for (int i = 0; i < 100; i++)
            {
                fft.compute(data, data, FftDirection_Forward);
            }
            //complex<float> *r = result->getCpuMemory();
//			ofstream outputfile("rand12clfft.dat");
            p = data->getCpuMemory();
/*
            for(int j = 0; j < N; j++)
            {
                outputfile << p[j] << endl;
            }
            outputfile.close();
*/
        }
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
