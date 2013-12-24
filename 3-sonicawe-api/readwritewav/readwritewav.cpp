#include "sawe/project_header.h"
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <iostream>
#include <QGLWidget> // libsonicawe uses gl, so we need to include a gl header in this project as well

#include "adapters/audiofile.h"
#include "adapters/writewav.h"

using namespace std;

class ReadWriteWav : public QObject
{
    Q_OBJECT

public:
    ReadWriteWav();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void readWriteWav();
    void writeNormalized();

private:
    std::string source;
    std::string output;
    std::string normalizedOutput;
    std::string normalizedGold;
};


ReadWriteWav::
        ReadWriteWav()
{
    source = "source.wav";
    output = "output.wav";
    normalizedOutput = "normalized-result.wav";
    normalizedGold = "normalized-gold.wav";
}


void ReadWriteWav::
        initTestCase()
{
}


void ReadWriteWav::
        cleanupTestCase()
{
}


void ReadWriteWav::
        readWriteWav()
{
    try
    {
    TaskTimer tt("%s", __FUNCTION__);
    if (!QFile::exists(source.c_str()))
    {
        QFAIL("You need to run the script source.m to create some source data first");
    }

    Signal::pOperation audiofile(new Adapters::Audiofile(source));
    {
        TaskTimer t2("Writing audiofile '%s' while copying source '%s'",
                 output.c_str(), source.c_str());

        Adapters::WriteWav* w = 0;
        Signal::Operation::Ptr wavwrite(w = new Adapters::WriteWav(output));

        Signal::pBuffer b = audiofile->readFixedLength(audiofile->getInterval());
        w->invalidate_samples(b->getInterval());
        w->put(b);
    }

    TaskTimer t2("Reading output '%s' and verifying against original input '%s'",
             output.c_str(), source.c_str());

    Signal::pOperation audiofile2(new Adapters::Audiofile(output));

    QCOMPARE( audiofile2->getInterval(), audiofile->getInterval() );

    Signal::pBuffer b = audiofile->readFixedLength(audiofile->getInterval());
    Signal::pBuffer b2 = audiofile->readFixedLength(audiofile->getInterval());

    QCOMPARE( b->mergeChannelData ()->numberOfBytes(), b2->mergeChannelData ()->numberOfBytes() );
    int bufferdiff = memcmp(b->mergeChannelData ()->getCpuMemory(), b2->mergeChannelData ()->getCpuMemory(), b2->mergeChannelData ()->numberOfBytes() );
    QVERIFY( 0 == bufferdiff );
    }
    catch (std::exception &x)
    {
        QFAIL(QString("caught %1: %2")
                 .arg(vartype(x).c_str()).arg(x.what()).toStdString().c_str());
    }
}


void ReadWriteWav::
        writeNormalized()
{
    try
    {
    TaskTimer ti("ReadWriteWav::writeNormalized");
    Signal::pOperation audiofile(new Adapters::Audiofile(source));
    if (!QFile::exists(source.c_str()))
    {
        QFAIL("You need to run the script source.m to create some source data first");
    }

    Adapters::WriteWav* w = 0;
    Signal::Operation::Ptr wavwrite(w = new Adapters::WriteWav(normalizedOutput));

    Signal::pBuffer b = audiofile->readFixedLength(audiofile->getInterval());
    w->invalidate_samples(audiofile->getInterval());
    w->put(b);

    for (int i=0; i<4; ++i)
    {
        TaskTimer ti("ReadWriteWav::writeNormalized i=%d", i);

        w->normalize(0 == i%2);

        Signal::pOperation normalizedAudiofile(new Adapters::Audiofile(normalizedOutput));
        std::string goldname = w->normalize() ? normalizedGold : source;
        Signal::pOperation normalizedAudiofileGold(new Adapters::Audiofile(goldname));
        if (!QFile::exists(normalizedOutput.c_str()))
        {
            QFAIL(QString("Couldn't write '%1'")
                  .arg(normalizedOutput.c_str()).toStdString().c_str());
        }
        if (!QFile::exists(goldname.c_str()))
        {
            QFAIL(QString("You need to validate the output of the previous test manually and rename '%1' to '%2'")
                  .arg(normalizedOutput.c_str())
                  .arg(goldname.c_str()).toStdString().c_str());
        }

        QCOMPARE( normalizedAudiofile->getInterval(), audiofile->getInterval() );
        QCOMPARE( normalizedAudiofile->getInterval(), normalizedAudiofileGold->getInterval() );

        Signal::pBuffer b = normalizedAudiofile->readFixedLength(audiofile->getInterval());
        Signal::pBuffer b2 = normalizedAudiofileGold->readFixedLength(audiofile->getInterval());

        TaskTimer t2("ReadWriteWav::writeNormalized i=%d", i);

        Signal::pTimeSeriesData data = b->mergeChannelData ();
        Signal::pTimeSeriesData data2 = b2->mergeChannelData ();
        QCOMPARE( data->numberOfBytes(), data2->numberOfBytes() );
        float maxdiff = 0;

        float *p = data->getCpuMemory();
        float *p2 = data2->getCpuMemory();
        for (unsigned x=0; x<data->numberOfElements(); ++x )
        {
            float& v = p[x];
            float& v2 = p2[x];
            maxdiff = std::max( maxdiff, std::fabs(v - v2) );
        }

        if (maxdiff > 1e-4)
            QCOMPARE( maxdiff, 0.f );
    }
    }
    catch (std::exception &x)
    {
        QFAIL(QString("caught %1: %2")
                 .arg(vartype(x).c_str()).arg(x.what()).toStdString().c_str());
    }
}


QTEST_MAIN(ReadWriteWav);
#include "readwritewav.moc"
