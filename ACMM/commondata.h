#ifndef COMMONDATA_H
#define COMMONDATA_H

#include<QDebug>
#include <QWidget>
#include<QList>
#include<QSettings>
class commonData : public QWidget
{
    Q_OBJECT
public:
    explicit commonData(QWidget *parent = nullptr);
    const static int SizeOfAngle = 6;
    const static int SizeOfCoordinate = 3;
    const static int SizeOFStruct = 21;

    const static QString coeffType;
    const static QString structureType;
    //获取PI值（做成静态static）
    static double getPI();
    //上偏差
    static double getUpper();
    //下偏差
    static double getLower();
    //三个坐标值
    double getCoordinateItem(const int position) const;
    void setCoordinate(const int position,const double value);
    QString getCoordinateItemStr(const int position,const int significantNum);
    //偏心误差
    int getCoeffParameterSize() const;
    double getCoeffParameterItem(const int position) const;
    void clearCoeffParameter();
    void setCoeffParameterItem(const double value);
    void initialCoeffParameter(const int size);
    //结构参数
    double getStructParameterItem(const int position) const;
    void setStructParameterItem(const int position,const double value);
    //圆光栅传感器
    double getAxisAngle(const int position) const;
    void setAxisAngle(const int position,const double value);
    double getAxisAngleRadian(const int position) const;
    //动态绘图部分记录
    void setImageAxisAngle(const int position,const int value);
    double getImageAxisAngle(const int position) const;
    double getImageAxisAngleRadian(const int position) const;
    //采样数据记录部分
    void setAxisSampleList(const int axisNum,const double value);
    double getAxisSampleList(const int axisNum,const int position) const;
    double getAxisSampleListRadian(const int axisNum,const int position) const;
    bool isAxisSampleListEmpty() const;
    void clearAllAxisSampleList();
    void ereaseAxisSampleList(const int axisNum,const int position);
    int getAxisSampleListSize() const;
    //圆光栅过零标志位
    void setAxisCrossZero(const int axisNum,const bool value);
    bool getAxisCrossZero(const int axisNum) const;
    void initialAxisCrossZero(const bool value);
    //唯一标识符储存
    int getUniqueCodeSize() const;
    QString getUniqueCodeItem(const int position) const;
    void clearUniqueCode();
    void setUniqueCodeItem(const QString value);
    //摁键触发标志位
    void setRecordFlag(const bool value);
    bool getRecordFlag() const;
    //摁键赋能
    void setLabelEnable(const bool value);
    bool getLabelEnable() const;
    //标志位
    bool getFlagFor50Bytes() const;
    void setFlagFor50Bytes(const bool value);
    bool getFlagFor26Bytes() const;
    void setFlagFor26Bytes(const bool value);
private:
    //上下位机连接成功
    static double PI;
    //三个坐标值
    static double coordinate[SizeOfCoordinate];//三个坐标值
    //偏心参数 内容存储机制 6个轴系 6组参数 7阶为例 一组参数15个double类型的元素组成
    static QList<double> coeffParameter;
    //结构参数
    static double structParameter[SizeOFStruct];
    //圆光栅过零标志
    static bool flagForCrossZero[SizeOfAngle];
    //角度值
    static double Angle[6];
    //摁键标志位
    static bool dataRecordFlag;
    //unique code 完整版
    static QList<QString> uniqueCode;
    //50位通讯标志位，如果是true表示目前是50位通讯模式
    static bool flagFor50Bytes;
    //26位通讯标志位，如果是true表示目前是26位通讯模式
    static bool flagFor26Bytes;
    //前界面三个摁键的功能赋予控制
    static bool flagForEnableForLabel;
    //公差
    //上偏差
    static double Upper;
    //下偏差
    static double Lower;
    //标定程序 启动标志位
    static bool runCalibration;
    //Tcp 启动标志位
    static bool runTcp;

    //保存六个读数头的数据
    static QList<double> firstAxisList;
    static QList<double> secondAxisList;
    static QList<double> thirdAxisList;
    static QList<double> fourthAxisList;
    static QList<double> fifthAxisList;
    static QList<double> sixthAxisList;

    static int imageAxisAngle[SizeOfAngle];

    static bool clearListFlag;
signals:

public slots:
};

#endif // COMMONDATA_H
