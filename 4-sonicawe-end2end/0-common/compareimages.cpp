#include "compareimages.h"

#include "sawe/configuration.h"

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
    saveImage(p->tools().render_view()->glwidget);
}


void CompareImages::
        saveWindowImage(Sawe::pProject p)
{
    saveWindowImage(p->mainWindowWidget(), p->tools().render_view()->glwidget);
}


void CompareImages::
        saveImage(QGLWidget *glwidget)
{
    TaskTimer ti("CompareImages::saveImage");

    glwidget->swapBuffers();
    QImage glimage = glwidget->grabFrameBuffer();
    glimage.save(resultFileName);
}


void CompareImages::
        saveWindowImage(QWidget* mainwindow, QGLWidget *glwidget)
{
    TaskTimer ti("CompareImages::saveWindowImage");

    QPixmap pixmap(mainwindow->size());
    QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);
    QPainter painter(&pixmap);

    // update layout by calling render
    mainwindow->activateWindow();
    mainwindow->raise();
    mainwindow->render(&painter);

    // draw OpenGL window
    QPoint p2 = glwidget->mapTo( mainwindow, QPoint() );
    glwidget->swapBuffers();
    QImage glimage = glwidget->grabFrameBuffer();
    painter.drawImage(p2, glimage);

#ifdef Q_OS_LINUX
    // draw Qt widgets that are on top of the opengl window
    mainwindow->render(&painter);
#endif

    pixmap.save(resultFileName);
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
