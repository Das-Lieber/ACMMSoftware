#ifndef DECODE_H
#define DECODE_H

#include <QWidget>
#include<QString>
#include<QList>
#include<iostream>
#include<algorithm>
#include<math.h>
#include "serial.h"
#include"commondata.h"
#include"Method/method.h"
#define sizeOfRow 4
#define sizeOfCol 4

typedef struct Matrix{
    double data[sizeOfRow][sizeOfCol];
    int row;
    int col;
}Matrix;

class Decode : public QWidget
{
    Q_OBJECT
public:
    explicit Decode(QWidget *parent = nullptr);
    virtual ~Decode();
    //主函数-主骨架
    void decodeMain();
    virtual void connectToLPM(Decode * parent);//信号和槽连接到serial变量
    //观察者的各类设置函数
    static void addMethodObject(Method * object);
    static void removeMethodObject(Method * object);
    static bool findMethodObject(Method * object);
    static void removeAll();
protected:
    Serial * mSerial;
    commonData * mCommonData;
    QReadWriteLock * mReadWriteLock;
    //LPM 下位机
    virtual QString getDataFromLPM() = 0;//获取下位机发送的数据
    virtual void calculate(QString & strData) = 0;//进行计算，并得到角度值和坐标值
    virtual void updateFlag() = 0;//各类标志位的设置
    //说明 update之后，还要有个模式选择 数据要发给三个甚至更多的观察者 就是calibration coordinate还有未来的扩展

    //不管什么情况都会用到的函数
    virtual double AngleAxis(double Angledeg,int Begin_axis,const int Fourier_Size) const;
    //双读数头计算，返回角度值
    virtual double DualAngle(double Angledeg1,double Angledeg2,int Begin_axis,const int Fourier_Size) const;
    //公共部分（计算空间坐标）
    struct Matrix CoordTransf();
    struct Matrix TransfMatrix(double StructPara[],double Angle);
    struct Matrix TranslMatrix(double ProLen);
    struct Matrix MultiMatrix(Matrix A,Matrix B);
    //因为完成解包后，需要将Common中的各类数据传送给method中的各类方法，此处加一个观察者模式，如果method里面还要增加功能，直接继承method重写关键函数就行
private:
    static QList<Method *> mMethodList;//目前是一对一，未来可能一对多
    void sendInfoToMethod();

signals:

public slots:
};

#endif // DECODE_H
