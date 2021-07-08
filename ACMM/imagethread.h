#ifndef IMAGETHREAD_H
#define IMAGETHREAD_H

#include <QObject>
#include<QPainter>
#include<QPen>
#include<QBrush>
#include<QImage>
#include<math.h>
#include<QList>
#include<QDebug>
#include<QReadWriteLock>
#include<QMutex>
#include "commondata.h"


class imageThread : public QObject
{
    Q_OBJECT
public:
    explicit imageThread(QObject *parent = nullptr);
    void drawImage();
    void setPaintFactor(const double x,const double y);
private:
    QReadWriteLock * mReadWriteLock;
    commonData * mCommonData;
    int x,y;
    double exAngle[commonData::SizeOfAngle];
    double updateAngle[commonData::SizeOfAngle];

    QMutex * myMutex;
    void setDrawEllipse(QPainter & p,int xpos,int ypos,int R,int axisNum);
    void drawDynamicPart(QPainter & p,int xpos,int ypos,int R,int axisNum);

    double paintfactorx;//缩放因子
    double paintfactory;//缩放因子
signals:
    //发送信号,把画好的图片进行传送
    void SignalUpdateImage(QImage temp);
public slots:
};

#endif // IMAGETHREAD_H
