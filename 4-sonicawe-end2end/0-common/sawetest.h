#ifndef SAWETEST
#define SAWETEST

#include "sawe/project.h"

#include <QObject>
#include <boost/weak_ptr.hpp>


/**
  @see SaweTestClass::project
  @see SaweTestClass::projectOpened
  @see SaweTestClass::finishedWorkSection
  */
class SaweTestClass: public QObject
{
    Q_OBJECT
protected:
    SaweTestClass();


    /**
      Makes SaweTestClass follow a specific project and call
      projectOpened and finishedWorkSection for this project.
     */
    void project(Sawe::pProject p);


    /**
      Returns the current project. Will contain a null pointer
      if no project has been set yet, or if the application has
      closed the project.
      */
    Sawe::pProject project();


    /**
      If a project has been added to the application this method
      starts the application main event loop and doesn't return
      until it's finished.

      If a project is opened and made visible the virtual method
      projectOpened is called. projectOpened() is a good place
      to initialize gui settings since all instances have been
      created with their default values. Calling the default
      implementation SaweTestClass::projectOpened will hide
      dropdown notifications and show the timeline.
      */
    void exec();


    /**
      Hides all active drop down notifications (instances of
      Tools::DropNotifyForm that are children of the mainwindow
      centralWidget).
      */
    void closeDropNotifications();


    /**
      Toggles the timeline visibility action unless the timline
      visibility already equals param 'visible'.

      Call after projectOpened has been called.
      */
    void timeLineVisibility(bool visible);


    /**
      Virtual method called when a project has been initialized
      and displayed. The default implementation
      SaweTestClass::projectOpened will hide dropdown notifications
      and show the timeline.
      */
    virtual void projectOpened();


    /**
      Virtual method called each time worker has worked on something
      and completed everything currently needed.

      Param 'int workSectionCounter' is increased by one each time
      this method is called from SaweTestClass. The first value is 0.
      */
    virtual void finishedWorkSection(int /*workSectionCounter*/) {}

private:
    /**
      The project is owned by the application. If the application
      closes the project. SaweTestClass also loses it.
      */
    boost::weak_ptr<Sawe::Project> project_;

    /// @see finishedWorkSection
    int work_sections_;

    /// @see projectOpened
    bool project_is_opened_;

protected slots: // not private because that would make them execute as test cases by QTest::qExec
    /// @see projectOpened
    void postPaint();

    /// @see finishedWorkSection
    void renderViewFinishedWorkSection();
};


typedef std::vector<const char*> ArgvectorT;
template<typename T> void TestClassArguments(ArgvectorT&) {}

// expanded QTEST_MAIN but for Sawe::Application
#define SAWETEST_MAIN(TestClass)                               \
    int main(int argc, char *argv[])                           \
    {                                                          \
        ArgvectorT argvector(argc);                            \
        for (int i=0; i<argc; ++i)                             \
            argvector[i] = argv[i];                            \
                                                               \
        argvector.push_back("--use_saved_state=0");            \
        argvector.push_back("--skip_update_check=1");          \
        argvector.push_back("--skipfeature=overlay_navigation");\
        argvector.push_back("--skipfeature=transform_info");   \
                                                               \
        TestClassArguments<TestClass>( argvector );            \
        argc = argvector.size();                               \
                                                               \
        Sawe::Application application(argc, (char**)&argvector[0]); \
        QTEST_DISABLE_KEYPAD_NAVIGATION                        \
        try {                                                  \
            TestClass tc;                                      \
            return QTest::qExec(&tc, argc, (char**)&argvector[0]); \
        }                                                      \
        catch (const std::exception& x)                        \
        {                                                      \
            std::cout << "Error: " << vartype(x) << std::endl  \
                      << "Details: " << x.what() << std::endl; \
            return -1;                                         \
        }                                                      \
    }

// SAWETEST_MAIN with navigation overlay
#define SAWETEST_MAIN_NORMAL(TestClass)                        \
    int main(int argc, char *argv[])                           \
    {                                                          \
        ArgvectorT argvector(argc);                            \
        for (int i=0; i<argc; ++i)                             \
            argvector[i] = argv[i];                            \
                                                               \
        argvector.push_back("--use_saved_state=0");            \
        argvector.push_back("--skip_update_check=1");          \
                                                               \
        TestClassArguments<TestClass>( argvector );            \
        argc = argvector.size();                               \
                                                               \
        Sawe::Application application(argc, (char**)&argvector[0]); \
        QTEST_DISABLE_KEYPAD_NAVIGATION                        \
        try {                                                  \
            TestClass tc;                                      \
            return QTest::qExec(&tc, argc, (char**)&argvector[0]); \
        }                                                      \
        catch (const std::exception& x)                        \
        {                                                      \
            std::cout << "Error: " << vartype(x) << std::endl  \
                      << "Details: " << x.what() << std::endl; \
            return -1;                                         \
        }                                                      \
    }
#endif // SAWETEST
