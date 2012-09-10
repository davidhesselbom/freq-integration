#pragma once

#ifdef QT_GUI_LIB
#define Application QApplication
#else
#define Application QCoreApplication
#endif

// expanded QTEST_MAIN but for an exception catching application
#define TEST_MAIN_TRYCATCH(TestObject)                        \
    int main(int argc, char *argv[])                           \
    {                                                          \
        try                                                    \
        {                                                      \
            Application app(argc, argv);                       \
            TestObject tc;                                     \
            return QTest::qExec(&tc, argc, argv);              \
        }                                                      \
        catch (const std::exception& x)                        \
        {                                                      \
            std::cout << "Error: " << vartype(x) << std::endl  \
                      << "Details: " << x.what() << std::endl; \
            return -1;                                         \
        }                                                      \
    }
