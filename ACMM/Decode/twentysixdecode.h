#ifndef TWENTYSIXDECODE_H
#define TWENTYSIXDECODE_H
#include "decode.h"
#include <QWidget>

class twentysixDecode : public Decode{
    Q_OBJECT
public:
    explicit twentysixDecode(QWidget *parent = nullptr);
    virtual ~twentysixDecode();
protected:
    //LPM 下位机
//    virtual void connectToLPM();//信号和槽连接到serial变量
    virtual QString getDataFromLPM();//获取下位机发送的数据
    virtual void calculate(QString & strData);//进行计算，并得到角度值和坐标值
    virtual void updateFlag();//各类标志位的设置
private:
    int originalByteData26[26];
    double transAngle26[8];
    int originalAngleData[38];
    int original[48];
    void AngleComputation(int original[26],double angle[8]);
signals:

public slots:
};

#endif // TWENTYSIXDECODE_H
