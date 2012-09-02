#ifndef COMPAREIMAGES_H
#define COMPAREIMAGES_H

#include <QString>

#include "sawe/project.h"

class QWidget;
class QGLWidget;

class CompareImages
{
public:
    enum PlatformDependency {
        PlatformIndependent,
        PlatformSpecific
    };
    enum DeviceDependency {
        DeviceIndependent,
        DeviceSpecific
    };

    CompareImages( QString testName = "test", PlatformDependency platformspecific=PlatformIndependent, DeviceDependency computationdevicespecific=DeviceIndependent );

    QString resultFileName, goldFileName, diffFileName;
    double limit;
    double limit2;
    double limitinf; // max

    void saveImage(Sawe::pProject p);
    void saveWindowImage(Sawe::pProject p);
    void verifyResult();

private:
    void saveImage(QGLWidget *glwidget);
    void saveWindowImage(QWidget* mainwindow, QGLWidget *glwidget);
};


#endif // COMPAREIMAGES_H
