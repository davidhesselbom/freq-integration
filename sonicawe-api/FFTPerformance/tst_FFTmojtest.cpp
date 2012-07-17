#include "tfr/fftimplementation.h"
#include "tfr/fftclfft.h"
#include "sawe/project_header.h"
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QGLWidget>

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

#include "cudaPitchedPtrType.h"
#include "CudaException.h"

void FFTmojTestCuda( cudaPitchedPtrType<float> data );

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
    static FftClFft fft;

    ChunkData::Ptr data;
    //int N = 4096;
    int N = fft.lChunkSizeS(20000000);
	//fails on fusion cpu at 200000000 (134217728)
	//largest ok on fusion cpu at 100000000 (67108864), 30 sec
	//largest ok on rampage cpu at 100000000 (67108864), 12.7 sec
	//fails on fusion gpu at 40000000
	//largest ok on fusion gpu at 20000000, 15 sec
	cout << "size: " << N << endl;
    data.reset(new ChunkData(N));

    complex<float> *p = data->getCpuMemory();
    for (int i = 0; i < N; i++)
    {
        p[i].real(0);
        p[i].imag(1);
    }

    ChunkData::Ptr result(new ChunkData(N));

    TaskTimer timer("Running OOURA");
    fft.compute(data, result, FftDirection_Forward);


//    QVERIFY( cudaSuccess == cudaGLSetGLDevice( 0 ) );
//    QVERIFY( 0==glewInit() );
//    pVbo vbo( new Vbo(1024));
//    FFTmoj<float> mapping(vbo, make_cudaExtent(256,1,1));
//    FFTmojTestCuda( mapping.data->getCudaGlobal() );
//    QVERIFY2(true, "Failure");
}

QTEST_MAIN(FFTmojTest)

#include "tst_FFTmojtest.moc"
