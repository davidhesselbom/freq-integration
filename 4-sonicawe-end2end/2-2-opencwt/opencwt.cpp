#include "sawe/project_header.h"

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <iostream>
#include <QGLWidget> // libsonicawe uses gl, so we need to include a gl header in this project as well
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QRgb>

#include "sawetest.h"
#include "compareimages.h"

#include "sawe/application.h"
#include "ui/mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
using namespace Tfr;
using namespace Signal;

class OpenCwt : public SaweTestClass
{
    Q_OBJECT

public:
    OpenCwt();

private slots:
    void initOpenAudio();
    void openAudio();

    void verifyResult();

protected slots:
    void saveImage();

private:
    virtual void projectOpened();
    virtual void finishedWorkSection(int workSectionCounter);

    QString sourceAudio;

    CompareImages compareImages;
};


OpenCwt::
        OpenCwt()
{
    sourceAudio = "music-1.ogg";

    compareImages.limit = 200.;
    compareImages.limit2 = 1.6;
    compareImages.limitinf = 0.1;
}


void OpenCwt::
        initOpenAudio()
{
    project( Sawe::Application::global_ptr()->slotOpen_file( sourceAudio.toStdString() ) );
}


void OpenCwt::
        openAudio()
{
    exec();
}


void OpenCwt::
        projectOpened()
{
    TaskTimer tt("%s", __FUNCTION__);

    project()->mainWindow()->getItems()->actionTransform_Cwt->trigger();

    SaweTestClass::projectOpened();
}


void OpenCwt::
        finishedWorkSection(int workSectionCounter)
{
    if (0!=workSectionCounter)
        return;

    QTimer::singleShot(1, this, SLOT(saveImage()));
}


void OpenCwt::
        saveImage()
{
    compareImages.saveImage( project() );

    Sawe::Application::global_ptr()->slotClosed_window( project()->mainWindowWidget() );
}


void OpenCwt::
        verifyResult()
{
    compareImages.verifyResult();
}


SAWETEST_MAIN(OpenCwt)

#include "opencwt.moc"
