#ifndef FIFTYDECODE_H
#define FIFTYDECODE_H
#include "Decode/decode.h"
#include <QWidget>

class fiftyDecode : public Decode{
    Q_OBJECT
public:
    explicit fiftyDecode(QWidget *parent = nullptr);
    virtual ~fiftyDecode();
protected:
    //LPM 下位机
//    virtual void connectToLPM();//信号和槽连接到serial变量
    virtual QString getDataFromLPM();//获取下位机发送的数据
    virtual void calculate(QString & strData);//进行计算，并得到角度值和坐标值
    virtual void updateFlag();//各类标志位的设置
private:
    int byteNumFlag;
    QByteArray byteDataone,byteDatatwo;
    QString strOne,strTwo,strResult;

    int originalByteData50[50];
    double transAngle50[12];
    int originalAngleData[38];
    int original[48];

    QList<int> crossZeroList;
    void AngleComputation(int judge_original[48],int original[38],double angle[12]);
signals:

public slots:
};

#endif // FIFTYDECODE_H
