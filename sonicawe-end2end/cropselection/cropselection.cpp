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

#include "tools/selections/peakcontroller.h"

using namespace std;
using namespace Tfr;
using namespace Signal;

class CropSelection : public SaweTestClass
{
    Q_OBJECT

public:
    CropSelection();

private slots:
    void initOpenAudio();
    void openAudio();

    void verifyResult();
    void cleanupVerifyResult();

protected slots:
    void saveImage();

private:
    virtual void projectOpened();
    virtual void finishedWorkSection(int workSectionCounter);

    QString sourceAudio;

    CompareImages compareImages;
};


CropSelection::
        CropSelection()
{
    sourceAudio = "music-1.ogg";
    compareImages.limit = 100;
    compareImages.limit2 = 4;
    compareImages.limitinf = 0.1;
}


void CropSelection::
        initOpenAudio()
{
    project( Sawe::Application::global_ptr()->slotOpen_file( sourceAudio.toStdString() ) );
}


void CropSelection::
        openAudio()
{
    exec();
}


void CropSelection::
        projectOpened()
{
    TaskTimer tt("CropSelection::projectOpened");

    Tools::RenderController* rc = project()->tools().getObject<Tools::RenderController>();
    QVERIFY( 0!=rc );

    for (int i=0; i<40; ++i)
        rc->yscaleDecrease();

    SaweTestClass::projectOpened();
}


void CropSelection::
        finishedWorkSection(int finishedWorkSections)
{
    switch (finishedWorkSections)
    {
    case 0:
        {
            Ui::SaweMainWindow* main = project()->mainWindow();
            main->activateWindow();
            main->raise();

            Ui::MainWindow* ui = main->getItems();
            QWidget* glwidget = project()->tools().render_view()->glwidget;

            ui->actionPeakSelection->trigger();

            QPoint p(658, 241);

            QTestEventList tel;
            tel.addMouseMove(p, 100);
            tel.addMouseClick(Qt::LeftButton, 0, p, 100);
            tel.simulate(glwidget);

            ui->actionCropSelection->trigger();

            break;
        }

    case 1:
        QTimer::singleShot(1, this, SLOT(saveImage()));
        break;
    }
}


void CropSelection::
        saveImage()
{
    compareImages.saveImage( project() );

    Sawe::Application::global_ptr()->slotClosed_window( project()->mainWindowWidget() );
}


void CropSelection::
        verifyResult()
{
    compareImages.verifyResult();
}


void CropSelection::
        cleanupVerifyResult()
{
}


SAWETEST_MAIN(CropSelection)

#include "cropselection.moc"
