#include "sawe/project_header.h"
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <iostream>
#include <QGLWidget> // libsonicawe uses gl, so we need to include a gl header in this project as well

#include "signal/intervals.h"

Q_DECLARE_METATYPE(Signal::Intervals)

using namespace std;
using namespace Tfr;
using namespace Signal;

namespace QTest {
    template<>
    char * toString ( const Interval & value )
    {
        return qstrdup( value.toString().c_str() );
    }

    template<>
    char * toString ( const Intervals & value )
    {
        return qstrdup( value.toString().c_str() );
    }

    template<>
    bool qCompare( const Interval &t1, const Intervals &t2, const char *actual, const char *expected, const char *file, int line )
    {
        return compare_helper(t1 == t2, "Compared values are not the same",
                             toString(t1), toString(t2), actual, expected, file, line);
    }

    template<>
    bool qCompare( const Intervals &t1, const Interval &t2, const char *actual, const char *expected, const char *file, int line )
    {
        return compare_helper(t1 == t2, "Compared values are not the same",
                             toString(t1), toString(t2), actual, expected, file, line);
    }

    bool qCompare( const string &t1, const char* t2, const char *actual, const char *expected, const char *file, int line )
    {
        return qCompare(t1.c_str(), t2, actual, expected, file, line );
    }

    bool qCompare( const char* t1, const string &t2, const char *actual, const char *expected, const char *file, int line )
    {
        return qCompare(t1, t2.c_str(), actual, expected, file, line );
    }
}

class IntervalsTest : public QObject
{
    Q_OBJECT

public:
    IntervalsTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void oroperator_data();
    void andoperator_data();
    void minusoperator_data();
    void oroperator();
    void andoperator();
    void minusoperator();

    void aux1();
    void aux2();
    void aux3();

private:
    vector<Intervals> T;
    Intervals I;
};


IntervalsTest::IntervalsTest()
{
}


void IntervalsTest::initTestCase()
{
    I = Intervals(100, 300);
    T.clear();
    T.push_back( Intervals(50,80) );
    T.push_back( Intervals(50,100));
    T.push_back( Intervals(50,200));
    T.push_back( Intervals(50,400));
    T.push_back( Intervals(100,300));
    T.push_back( Intervals(200,250));
    T.push_back( Intervals(200,400));
    T.push_back( Intervals(300,400));
    T.push_back( Intervals(350,400));
    T.push_back( Intervals(50,99));
    T.push_back( Intervals(50,301));
    T.push_back( Intervals(99,400));
    T.push_back( Intervals(301,400));
    T.push_back( Intervals(99,299));
    T.push_back( Intervals(101,301));
}


void IntervalsTest::cleanupTestCase()
{
}


void IntervalsTest::
        oroperator_data()
{
    QTest::addColumn<Intervals>("A");
    QTest::addColumn<Intervals>("B");
    QTest::addColumn<Intervals>("R");

    QTest::newRow("") << I << T[0] << (Intervals(100,300) | Intervals(50,80));
    QTest::newRow("") << I << T[1] << Intervals(50,300);
    QTest::newRow("") << I << T[2] << Intervals(50,300);
    QTest::newRow("") << I << T[3] << Intervals(50,400);
    QTest::newRow("") << I << T[4] << Intervals(100,300);
    QTest::newRow("") << I << T[5] << Intervals(100,300);
    QTest::newRow("") << I << T[6] << Intervals(100,400);
    QTest::newRow("") << I << T[7] << Intervals(100,400);
    QTest::newRow("") << I << T[8] << (Intervals(350,400) | Intervals(100,300));
    QTest::newRow("") << I << T[9] << (Intervals(50,99) | Intervals(100,300));
    QTest::newRow("") << I << T[10] << Intervals(50,301);
    QTest::newRow("") << I << T[11] << Intervals(99,400);
    QTest::newRow("") << I << T[12] << (Intervals(301,400) | Intervals(100,300));
    QTest::newRow("") << I << T[13] << Intervals(99,300);
    QTest::newRow("") << I << T[14] << Intervals(100,301);
}


void IntervalsTest::
        minusoperator_data()
{
    QTest::addColumn<Intervals>("A");
    QTest::addColumn<Intervals>("B");
    QTest::addColumn<Intervals>("R");

    QTest::newRow("") << I << T[0] << Intervals(100,300);
    QTest::newRow("") << I << T[1] << Intervals(100,300);
    QTest::newRow("") << I << T[2] << Intervals(200,300);
    QTest::newRow("") << I << T[3] << Intervals();
    QTest::newRow("") << I << T[4] << Intervals();
    QTest::newRow("") << I << T[5] << (Intervals(100,200) | Intervals(250,300));
    QTest::newRow("") << I << T[6] << Intervals(100,200);
    QTest::newRow("") << I << T[7] << Intervals(100,300);
    QTest::newRow("") << I << T[8] << Intervals(100,300);
    QTest::newRow("") << I << T[9] << Intervals(100,300);
    QTest::newRow("") << I << T[10] << Intervals();
    QTest::newRow("") << I << T[11] << Intervals();
    QTest::newRow("") << I << T[12] << Intervals(100,300);
    QTest::newRow("") << I << T[13] << Intervals(299,300);
    QTest::newRow("") << I << T[14] << Intervals(100,101);
}


void IntervalsTest::
        andoperator_data()
{
    QTest::addColumn<Intervals>("A");
    QTest::addColumn<Intervals>("B");
    QTest::addColumn<Intervals>("R");

    QTest::newRow("") << I << T[0] << Intervals();
    QTest::newRow("") << I << T[1] << Intervals();
    QTest::newRow("") << I << T[2] << Intervals(100,200);
    QTest::newRow("") << I << T[3] << Intervals(100,300);
    QTest::newRow("") << I << T[4] << Intervals(100,300);
    QTest::newRow("") << I << T[5] << Intervals(200,250);
    QTest::newRow("") << I << T[6] << Intervals(200,300);
    QTest::newRow("") << I << T[7] << Intervals();
    QTest::newRow("") << I << T[8] << Intervals();
    QTest::newRow("") << I << T[9] << Intervals();
    QTest::newRow("") << I << T[10] << Intervals(100,300);
    QTest::newRow("") << I << T[11] << Intervals(100,300);
    QTest::newRow("") << I << T[12] << Intervals();
    QTest::newRow("") << I << T[13] << Intervals(100,299);
    QTest::newRow("") << I << T[14] << Intervals(101,300);
}


void IntervalsTest::
    oroperator()
{
    // note how low operator precendence is for bit operators:
    // 'a | b == c' is equivalent to 'a | (b == c)'.
    // Thas is: not '(a | b) == c' which was probably intended.

    QFETCH(Intervals, A);
    QFETCH(Intervals, B);
    QFETCH(Intervals, R);

    QCOMPARE(A | B, R);
    QCOMPARE(B | A, R);

    Interval a = A.fetchFirstInterval();
    Interval b = B.fetchFirstInterval();
    Interval r = R.fetchFirstInterval();

    if (A.numSubIntervals() <= 1)
        QCOMPARE(A, a);
    if (B.numSubIntervals() <= 1)
        QCOMPARE(B, b);
    if (R.numSubIntervals() <= 1)
        QCOMPARE(R, r);

    if (A.numSubIntervals() <= 1)
    {
        QCOMPARE(a | B, R);
        QCOMPARE(B | a, R);

        if (B.numSubIntervals() <= 1)
        {
            QCOMPARE(a | b, R);
            QCOMPARE(b | a, R);

            if (R.numSubIntervals() <= 1)
            {
                QCOMPARE(a | b, r);
                QCOMPARE(b | a, r);
            }
        }

        if (R.numSubIntervals() <= 1)
        {
            QCOMPARE(a | B, r);
            QCOMPARE(B | a, r);
        }
    }

    if (B.numSubIntervals() <= 1)
    {
        QCOMPARE(A | b, R);
        QCOMPARE(b | A, R);

        if (R.numSubIntervals() <= 1)
        {
            QCOMPARE(A | b, r);
            QCOMPARE(b | A, r);
        }
    }

    if (R.numSubIntervals() <= 1)
    {
        QCOMPARE(A | B, r);
        QCOMPARE(B | A, r);
    }
}


void IntervalsTest::
    minusoperator()
{
    QFETCH(Intervals, A);
    QFETCH(Intervals, B);
    QFETCH(Intervals, R);

    QCOMPARE(A - B, R);

    Interval a = A.fetchFirstInterval();
    Interval b = B.fetchFirstInterval();
    Interval r = R.fetchFirstInterval();

    if (A.numSubIntervals() <= 1)
    {
        QCOMPARE(A, a);
        QCOMPARE(a - B, R);
    }

    if (B.numSubIntervals() <= 1)
    {
        QCOMPARE(B, b);
        QCOMPARE(A - b, R);
    }

    if (R.numSubIntervals() <= 1)
    {
        QCOMPARE(R, r);
        QCOMPARE(A - B, r);
    }
}


void IntervalsTest::
    andoperator()
{
    // note how low operator precendence is for bit operators:
    // 'a & b == c' is equivalent to 'a & (b == c)'.
    // Thas is: not '(a & b) == c' which was probably intended.

    QFETCH(Intervals, A);
    QFETCH(Intervals, B);
    QFETCH(Intervals, R);

    QCOMPARE(A & B, R);
    QCOMPARE(B & A, R);

    Interval a = A.fetchFirstInterval();
    Interval b = B.fetchFirstInterval();
    Interval r = R.fetchFirstInterval();

    if (A.numSubIntervals() <= 1)
        QCOMPARE(A, a);
    if (B.numSubIntervals() <= 1)
        QCOMPARE(B, b);
    if (R.numSubIntervals() <= 1)
        QCOMPARE(R, r);

    if (A.numSubIntervals() <= 1)
    {
        QCOMPARE(a & B, R);
        QCOMPARE(B & a, R);

        if (B.numSubIntervals() <= 1)
        {
            QCOMPARE(a & b, R);
            QCOMPARE(b & a, R);

            if (R.numSubIntervals() <= 1)
            {
                QCOMPARE(a & b, r);
                QCOMPARE(b & a, r);
            }
        }

        if (R.numSubIntervals() <= 1)
        {
            QCOMPARE(a & B, r);
            QCOMPARE(B & a, r);
        }
    }

    if (B.numSubIntervals() <= 1)
    {
        QCOMPARE(A & b, R);
        QCOMPARE(b & A, R);

        if (R.numSubIntervals() <= 1)
        {
            QCOMPARE(A & b, r);
            QCOMPARE(b & A, r);
        }
    }

    if (R.numSubIntervals() <= 1)
    {
        QCOMPARE(A & B, r);
        QCOMPARE(B & A, r);
    }
}


void IntervalsTest::
        aux1()
{
    Intervals I(403456,403457);
    Intervals J(0,403456);
    QCOMPARE( (I-J) & J, Interval());
    QCOMPARE( (I-J) & J, Intervals());
}


void IntervalsTest::
        aux2()
{
    Intervals I(100, 300);
    const Intervals J = I;
    QCOMPARE( I, J );
    QCOMPARE( I.toString(), "[100, 300)200#" );
    QCOMPARE( I.spannedInterval().toString(), "[100, 300)200#" );
    I ^= Interval(150,150);
    QCOMPARE( I, J );
    QCOMPARE( I.toString(), "[100, 300)200#" );
    QCOMPARE( I.spannedInterval().toString(), "[100, 300)200#" );
    I ^= Interval(50,50);
    QCOMPARE( I, J );
    QCOMPARE( I.toString(), "[100, 300)200#" );
    QCOMPARE( I.spannedInterval().toString(), "[100, 300)200#" );
    I ^= Intervals(50,150);
    QCOMPARE( I, Interval(50,100) | Interval(150,300) );
    QCOMPARE( I.toString(), "{[50, 100)50#, [150, 300)150#}" );
}


void IntervalsTest::
        aux3()
{
    Intervals I(100, 300);
    const Intervals J = I;
    QCOMPARE( I, J );
    QCOMPARE( I.toString(), "[100, 300)200#" );
    QCOMPARE( I.spannedInterval().toString(), "[100, 300)200#" );
    I -= Interval(150,150);
    QCOMPARE( I, J );
    QCOMPARE( I.toString(), "[100, 300)200#" );
    QCOMPARE( I.spannedInterval().toString(), "[100, 300)200#" );
    I |= Interval(50,50);
    QCOMPARE( I, J );
    QCOMPARE( I.toString(), "[100, 300)200#" );
    QCOMPARE( I.spannedInterval().toString(), "[100, 300)200#" );
    I &= Intervals(150,150);
    QCOMPARE( I, Intervals() );
    QCOMPARE( I.toString(), "{}" );
    QCOMPARE( I.spannedInterval().toString(), "[0, 0)" );
}


#include "../common.h"

TEST_MAIN_TRYCATCH(IntervalsTest)

#include "intervals.moc"
