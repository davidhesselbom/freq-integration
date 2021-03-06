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
#include "tfr/stftdesc.h"
#include "filters/normalizespectra.h"
#include "sawe/configuration.h"

#include "sawe/application.h"
#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "neat_math.h"

using namespace std;
using namespace Tfr;
using namespace Signal;

int minhz=0, maxhz=1500;

class Lofargram : public SaweTestClass
{
    Q_OBJECT

public:
    Lofargram();

private slots:
    void initOpenAudio();
    void openAudio();

    void verifyResult();

protected slots:
    void saveImage();

private:
    virtual void projectOpened();
    virtual void finishedWorkSection(int workSectionCounter);

    string sourceAudio;

    CompareImages compareImages;
};


Lofargram::
        Lofargram()
{
    sourceAudio = Sawe::Configuration::input_file ();
    if (sourceAudio.empty ())
        sourceAudio = "music-1.ogg";

    compareImages.limit = 50.;
    compareImages.limit2 = 13.f;
    compareImages.limitinf = 0.8f;
}


void Lofargram::
        initOpenAudio()
{
    project( Sawe::Application::global_ptr()->slotOpen_file( sourceAudio ) );
}


void Lofargram::
        openAudio()
{
    exec();
}


void Lofargram::
        projectOpened()
{
    Tools::RenderView* view = project()->toolRepo().render_view();
    ::Ui::MainWindow* actions = project()->mainWindow()->getItems();

    /**
     * Setup the transform settings
     * (look in opencwt for an example)
     */
    // linear frequency scale (see Tools::RenderController::receiveLinearScale)
    {
        float fs = view->model->project()->extent ().sample_rate.get ();

        Tfr::FreqAxis fa;
        fa.setLinear( fs );

        view->model->display_scale( fa );
    }

    // green (see Tools::RenderController::receiveSetGreenColors)
    {
        actions->actionSet_green_colors->trigger();
    }

    // amplitude (see Tools::RenderController::receiveSetYScale)
    {
        view->model->renderer->render_settings.y_scale = 2.0f;
    }

    // logarithmic amplitude (see Tools::RenderController::receiveSetGreenColors)
    {
        view->model->amplitude_axis ( Heightmap::AmplitudeAxis_Logarithmic );
    }

    // window size (see Tools::RenderController::receiveSetTimeFrequencyResolution)
    {
        int chunk_size = 65536;

        view->model->transform_descs ()->getParam<Tfr::StftDesc>()
                .set_approximate_chunk_size( chunk_size );
    }

    // averaging (see TransformInfoForm)
    {
        view->model->transform_descs ()->getParam<Tfr::StftDesc>()
                .averaging( 1 );
    }

    // normalization (see TransformInfoForm)
    {
        Heightmap::TfrMappings::StftBlockFilterParams::ptr stft_params =
                view->model->project ()->tools ().render_model.get_stft_block_filter_params ();
        EXCEPTION_ASSERT( stft_params );

        stft_params->freq_normalization = Tfr::pChunkFilter(
                    new Filters::NormalizeSpectra(96));
    }


    /**
     * Setup the camera settings
     */

    // Flip to left-handed coordinate system
    {
        actions->actionToggleOrientation->trigger();
    }

    // 1. Run a test without closing the application automatically
    //    (no Sawe::Application::global_ptr()->slotClosed_window and
    //     use SAWETEST_MAIN_NORMAL instead of SAWETEST_MAIN)
    // 2. Move the camera to a desired position
    // 3. Put a breakpoint in Tools::Command::NavigationCommand
    // 4. Inspect model->_q/p/r and store the values here. Or use
    //    as an inspiration for analytical values.
    {
        float length = view->model->project()->length ();
        float fs = view->model->project()->extent ().sample_rate.get ();
        // Approximately show the entire signal up to 1500 Hz
        view->model->_qx = length/2;
        view->model->_qz = (maxhz+minhz)/fs;
        view->model->xscale = 4.f/view->model->_qx;
        view->model->zscale = 4.f/(maxhz-minhz)*fs*1.95;
    }


    // Notify Sonic AWE that we've fiddled with a lot of transform settings.
    {
        view->emitTransformChanged ();
    }


    /**
     * When rendering is complete finishedWorkSection will be called.
     */
    SaweTestClass::projectOpened();


    // Remove the timeline
    {
        timeLineVisibility(false);
    }
}


void Lofargram::
        finishedWorkSection(int workSectionCounter)
{
    if (0!=workSectionCounter)
        return;

    QTimer::singleShot(1, this, SLOT(saveImage()));
}


void Lofargram::
        saveImage()
{
    compareImages.saveImage( project() );

    Sawe::Application::global_ptr()->slotClosed_window( project()->mainWindowWidget() );
}


void Lofargram::
        verifyResult()
{
    compareImages.verifyResult();
}


int main(int argc, char *argv[])
{
    ArgvectorT argvector;
    for (int i=0; i<argc; ++i) {
        if (0 == strncmp("--minhz=", argv[i], 8) && strlen(argv[i])>8)
            minhz = atoi(argv[i]+8);
        else if (0 == strncmp("--maxhz=", argv[i], 8) && strlen(argv[i])>8)
            maxhz = atoi(argv[i]+8);
        else
            argvector.push_back ( argv[i] );
    }

    argvector.push_back("--use_saved_state=0");
    argvector.push_back("--skip_update_check=1");
    argvector.push_back("--skipfeature=timeline_dock");
    argvector.push_back("--skipfeature=overlay_navigation");
    argvector.push_back("--skipfeature=transform_info");

    TestClassArguments<Lofargram>( argvector );
    argc = argvector.size();

    Sawe::Application application(argc, (char**)&argvector[0]);
    QTEST_DISABLE_KEYPAD_NAVIGATION
    try {
        Lofargram tc;
        return QTest::qExec(&tc, argc, (char**)&argvector[0]);
    }
    catch (const std::exception& x)
    {
        std::cout << "Error: " << vartype(x) << std::endl
                  << "Details: " << x.what() << std::endl;
        return -1;
    }
}

#include "lofargram.moc"
