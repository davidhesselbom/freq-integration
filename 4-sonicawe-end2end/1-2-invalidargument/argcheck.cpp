#include "sawetest.h"
#include "compareimages.h"
#include "callslotevent.h"

#include "sawe/application.h"

#include <QtTest/QtTest>
#include <QGLWidget>

class ArgCheck : public SaweTestClass
{
    Q_OBJECT
public:
    ArgCheck();

   // virtual void projectOpened();
    //virtual void finishedWorkSection(int workSectionCounter);

private slots:
    void saweTestClassTest();
//    void compareImagesTestResult();
//    void compareImagesTest();
//    void calledSlotTest();

//protected slots:
//    void hasCalledSlotTestSlot();

private:
    CompareImages compare;

    bool project_was_opened_;
    bool has_done_work_;
    bool has_called_slot_;
};


ArgCheck::
        ArgCheck()
            :
    project_was_opened_(false),
    has_done_work_(false),
    has_called_slot_(false)
{}


void ArgCheck::
        saweTestClassTest()
{
    TaskTimer ti("%s::%s", vartype(*this).c_str(), __FUNCTION__, NULL);

    project( Sawe::Application::global_ptr()->slotNew_recording() );

    exec();

/*
    QVERIFY( project_was_opened_ );
    QVERIFY( has_called_slot_ );
#ifdef __APPLE__
    QVERIFY( !has_done_work_ );
#else
    QVERIFY( has_done_work_ );
#endif
*/
}

/*
void ArgCheck::
        projectOpened()
{
    TaskTimer ti("%s::%s", vartype(*this).c_str(), __FUNCTION__, NULL);

    SaweTestClass::projectOpened();

    project_was_opened_ = true;

    QWidget* glwidget = project()->tools().render_view()->glwidget;
    QTestEventList tel;
    tel.push_back( new CallSlotEvent(this, SLOT(hasCalledSlotTestSlot())) );
    tel.simulate(glwidget);
}

*/

//void ArgCheck::
//        finishedWorkSection(int /*workSectionCounter*/)
//{
//    has_done_work_ = true;
//}


//void ArgCheck::
//        compareImagesTestResult()
//{
//    TaskTimer ti("%s::%s", vartype(*this).c_str(), __FUNCTION__, NULL);

//    compare.verifyResult();
//}


//void ArgCheck::
//        compareImagesTest()
//{
//    CompareImages ci("predefined", CompareImages::PlatformIndependent, CompareImages::DeviceIndependent);
//    QFile::copy(ci.goldFileName, ci.resultFileName);
//    ci.verifyResult();
//}


//void ArgCheck::
//        calledSlotTest()
//{
//    QVERIFY( has_called_slot_ );
//}


//void ArgCheck::
//        hasCalledSlotTestSlot()
//{
//    TaskTimer ti("%s::%s", vartype(*this).c_str(), __FUNCTION__, NULL);

//    has_called_slot_ = true;

//    compare.saveImage( project() );

//    Sawe::Application::global_ptr()->slotClosed_window( project()->mainWindowWidget() );
//}

SAWETEST_MAIN(ArgCheck)

#include "argcheck.moc"
