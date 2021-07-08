#ifndef DIALOGPOSITION_H
#define DIALOGPOSITION_H

#include <QDialog>
#include <QReadWriteLock>

#include <gp_Pnt.hxx>

#include "Method/method.h"
#include "commondata.h"

namespace Ui {
class DialogPosition;
}

class DialogPosition : public QDialog, public Method
{
    Q_OBJECT

public:
    explicit DialogPosition(QWidget *parent = 0);
    ~DialogPosition();
    void setPosition(double x, double y, double z);
    void setAngle(double a1, double a2, double a3, double a4, double a5, double a6);

    void getInfo();
    void pretreatment();

private:
    Ui::DialogPosition *ui;
    commonData * mCommonData;
    QReadWriteLock * mReadWriteLock;

signals:
    void addMeasurePoint(const gp_Pnt &pnt);
};

#endif // DIALOGPOSITION_H
