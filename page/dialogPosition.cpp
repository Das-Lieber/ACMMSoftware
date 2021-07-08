#include "dialogPosition.h"
#include "ui_dialogPosition.h"

DialogPosition::DialogPosition(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPosition)
{
    ui->setupUi(this);
    mReadWriteLock = new QReadWriteLock;

    ui->lcdNumber_X->setSmallDecimalPoint(true);
    ui->lcdNumber_Y->setSmallDecimalPoint(true);
    ui->lcdNumber_Z->setSmallDecimalPoint(true);
    setPosition(0.0000,0.0000,0.0000);
    setAngle(0,0,0,0,0,0);

    mCommonData = new commonData(this);
}

DialogPosition::~DialogPosition()
{
    delete ui;
}
void DialogPosition::setPosition(double x, double y, double z)
{
    ui->lcdNumber_X->display(QString::number(x, 'f', 4));
    ui->lcdNumber_Y->display(QString::number(y, 'f', 4));
    ui->lcdNumber_Z->display(QString::number(z, 'f', 4));
}

void DialogPosition::setAngle(double a1, double a2, double a3, double a4, double a5, double a6)
{
    ui->lcdNumber_A1->display(QString::number(a1,'f',3));
    ui->lcdNumber_A2->display(QString::number(a2,'f',3));
    ui->lcdNumber_A3->display(QString::number(a3,'f',3));
    ui->lcdNumber_A4->display(QString::number(a4,'f',3));
    ui->lcdNumber_A5->display(QString::number(a5,'f',3));
    ui->lcdNumber_A6->display(QString::number(a6,'f',3));
}

void DialogPosition::getInfo()
{
    mReadWriteLock->lockForRead();
    setPosition(mCommonData->getCoordinateItem(0),
                mCommonData->getCoordinateItem(1),
                mCommonData->getCoordinateItem(2));
    setAngle(mCommonData->getAxisAngle(0),
             mCommonData->getAxisAngle(1),
             mCommonData->getAxisAngle(2),
             mCommonData->getAxisAngle(3),
             mCommonData->getAxisAngle(4),
             mCommonData->getAxisAngle(5));

    emit addMeasurePoint(gp_Pnt(mCommonData->getCoordinateItem(0),
                                mCommonData->getCoordinateItem(1),
                                mCommonData->getCoordinateItem(2)));
    mReadWriteLock->unlock();
}

void DialogPosition::pretreatment()
{    
}
