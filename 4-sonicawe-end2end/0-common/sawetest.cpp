#include "sawetest.h"

#include "sawe/application.h"
#include "tools/dropnotifyform.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "TaskTimer.h"

#include <QtTest/QtTest>
#include <QGLWidget>

SaweTestClass::
        SaweTestClass()
            : work_sections_( 0 ),
              project_is_opened_( false )
{}


Sawe::pProject SaweTestClass::
        project()
{
    return Sawe::pProject(project_);
}


void SaweTestClass::
        project(Sawe::pProject p)
{
    if (!project_.expired())
    {
        Sawe::pProject oldp = project();
        if (oldp)
        {
            disconnect( oldp->tools().render_view(), SIGNAL(postPaint()), this, SLOT(postPaint()));
            disconnect( oldp->tools().render_view(), SIGNAL(finishedWorkSection()), this, SLOT(renderViewFinishedWorkSection()));
        }
    }

    project_ = p;
    project_is_opened_ = false;

    QVERIFY( p.get() );

    connect( p->tools().render_view(), SIGNAL(postPaint()), this, SLOT(postPaint()), Qt::QueuedConnection);
}


void SaweTestClass::
        exec()
{
    TaskTimer ti("%s::%s", vartype(*this).c_str(), __FUNCTION__, NULL);

    Sawe::Application::global_ptr()->exec();
}


void SaweTestClass::
        closeDropNotifications()
{
    foreach (QObject* o, project()->mainWindow()->centralWidget()->children())
    {
        if (Tools::DropNotifyForm* dnf = dynamic_cast<Tools::DropNotifyForm*>(o))
        {
            dnf->close();
        }
    }
}


void SaweTestClass::
        timeLineVisibility(bool visible)
{
    QAction* timelineAction = project()->mainWindow()->getItems()->actionToggleTimelineWindow;
    if (visible != timelineAction->isChecked())
        timelineAction->trigger();
}


void SaweTestClass::
        postPaint()
{
    if (project_is_opened_)
        return;

    project_is_opened_ = true;

    TaskTimer tt("%s::%s", vartype(*this).c_str(), __FUNCTION__, NULL);

    if (project_.expired())
    {
        TaskInfo("%s::%s: project_ has expired", vartype(*this).c_str(), __FUNCTION__);
        return;
    }
    Sawe::pProject p = project();
    if (!p)
    {
        TaskInfo("%s::%s: project() was null", vartype(*this).c_str(), __FUNCTION__);
        return;
    }

    disconnect( p->tools().render_view(), SIGNAL(postPaint()), this, SLOT(postPaint()));

    projectOpened();

    connect( p->tools().render_view(), SIGNAL(finishedWorkSection()), this, SLOT(renderViewFinishedWorkSection()), Qt::QueuedConnection);
}


void SaweTestClass::
        projectOpened()
{
    closeDropNotifications();
    timeLineVisibility(true);

    Sawe::pProject p = project();
    QGLWidget* qgl = p->tools().render_view()->glwidget;
    QSize wantedSize(840, 436);
    QSize actualSize = qgl->size();
    QSize windowSize = p->mainWindow()->size();

    p->mainWindow()->resize( windowSize + wantedSize - actualSize );
}


void SaweTestClass::
        renderViewFinishedWorkSection()
{
    TaskTimer tt("%s::%s", vartype(*this).c_str(), __FUNCTION__, NULL);

    if (project_.expired())
    {
        TaskInfo("%s::%s: project_ has expired", vartype(*this).c_str(), __FUNCTION__);
        return;
    }
    Sawe::pProject p = project();
    if (!p)
    {
        TaskInfo("%s::%s: project() was null", vartype(*this).c_str(), __FUNCTION__);
        return;
    }

    finishedWorkSection(work_sections_++);
}
