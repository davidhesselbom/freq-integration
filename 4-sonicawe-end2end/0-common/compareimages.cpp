#include "compareimages.h"

#include "sawe/configuration.h"
#include "tools/support/printscreen.h"

#include <QtTest/QtTest>
#include <QGLWidget>
#include <QImage>
#include <QColor>
#include <QFile>

#ifdef min
#undef min
#undef max
#endif


CompareImages::
        CompareImages( QString testName, PlatformDependency platformspecific, DeviceDependency computationdevicespecific )
:    limit(40),
     limit2(2),
     limitinf(64./256)
{
    QString target;

    if (DeviceSpecific==computationdevicespecific) switch (Sawe::Configuration::computationDeviceType())
    {
    case Sawe::Configuration::DeviceType_Cuda: target += "-cuda"; break;
    case Sawe::Configuration::DeviceType_OpenCL: target += "-opencl"; break;
    case Sawe::Configuration::DeviceType_CPU: target += "-cpu"; break;
    default: target = "unknown_target"; break;
    }

    if (PlatformSpecific==platformspecific)
    {
        target += "-";
#ifndef Q_WS_WIN
        target += Sawe::Configuration::operatingSystemFamilyName().c_str();
#else
        switch(QSysInfo::WindowsVersion)
        {
        case QSysInfo::WV_NT_based:
            target += "win8";
            break;
        default:
            target += "win";
            break;
        }
#endif
    }

    resultFileName = QString("%1%2-result.png").arg(testName).arg(target);
    goldFileName = QString("%1%2-gold.png").arg(testName).arg(target);
    diffFileName = QString("%1%2-diff.png").arg(testName).arg(target);

    QFile::remove(resultFileName);
}


void CompareImages::
        saveImage(Sawe::pProject p)
{
    Tools::Support::PrintScreen(p.get ()).saveImage ().save(resultFileName);
}


void CompareImages::
        saveWindowImage(Sawe::pProject p)
{
    Tools::Support::PrintScreen(p.get ()).saveWindowImage ().save(resultFileName);
}


void CompareImages::
        verifyResult()
{
    if (!QFile::exists(goldFileName))
    {
        QFAIL( QString("Couldn't find expected image '%1' to compare against the "
                       "result image '%2'. If this is the first time you're running "
                       "this test you could create the expected image by renaming "
                       "'%2' to '%1'.")
               .arg(goldFileName)
               .arg(resultFileName)
               .toLocal8Bit().data());
    }

    QImage goldimage(goldFileName);
    QImage resultimage(resultFileName);

    QCOMPARE( goldimage.size(), resultimage.size() );

    QImage diffImage( goldimage.size(), goldimage.format() );

    double norm1 = 0.f, norm2 = 0.f;
    float norminf = 0.f;
    for (int y=0; y<goldimage.height(); ++y)
    {
        for (int x=0; x<goldimage.width(); ++x)
        {
            float gold = QColor(goldimage.pixel(x,y)).lightnessF();
            float result = QColor(resultimage.pixel(x,y)).lightnessF();
            float diff = std::fabs(gold - result);
            norm1 += diff;
            norm2 += diff*diff;
            norminf = std::max(norminf, diff);
            float greenoffset = 1./3;
            float hue = fmod(10 + greenoffset + (gold - result)*0.5f, 1.f);
            diffImage.setPixel( x, y,
                                QColor::fromHsvF(
                                        hue,
                                        std::min(1.f, gold - result == 0
                                                      ? 0
                                                      : 0.5f+0.5f*std::fabs( gold - result )),
                                        0.5f+0.5f*gold
                                ).rgba() );
        }
    }

    diffImage.save( diffFileName );

    TaskInfo tt("compareImages, ligtness difference between '%s' and '%s' saved diff image in '%s'",
             goldFileName.toStdString().c_str(),
             resultFileName.toStdString().c_str(),
             diffFileName.toStdString().c_str() );

    TaskInfo("1-norm %g (<= %g)", norm1, limit);
    TaskInfo("2-norm %g (<= %g)", norm2, limit2);
    TaskInfo("inf-norm %g (<= %g)", norminf, limitinf);

    QVERIFY(norm1 <= limit);
    QVERIFY(norm2 <= limit2);
    QVERIFY(norminf <= limitinf);
}
