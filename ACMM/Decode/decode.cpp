#include "decode.h"
using namespace std;
QList<Method *> Decode::mMethodList;
Decode::Decode(QWidget *parent) : QWidget(parent){
    mSerial = new Serial(this);
    mReadWriteLock = new QReadWriteLock();
    mCommonData = new commonData(this);
}
Decode::~Decode(){

}
/*************************************观察者模式部分设置函数********************************************/
void Decode::sendInfoToMethod(){
    for(auto item:mMethodList){
        item->getInfo();
    }
}
void Decode::addMethodObject(Method * object){
    mMethodList.push_back(object);
}
void Decode::removeMethodObject(Method * object){
    mMethodList.removeOne(object);
}
bool Decode::findMethodObject(Method * object){
    return std::find( mMethodList.begin(),mMethodList.end(),object ) == mMethodList.end() ? false:true;
}
void Decode::removeAll(){
    mMethodList.clear();
}
/*************************************解码的主函数********************************************/
//根据参数进行多态的连接
void Decode::connectToLPM(Decode * parent){
    mReadWriteLock->lockForWrite();
    if(mSerial == nullptr){
        mReadWriteLock->unlock();
        return;
    }
    mSerial->disConnect();//先断开再连接
    connect(mSerial->getSerialPointer(),&QSerialPort::readyRead,parent,&Decode::decodeMain);
    mReadWriteLock->unlock();
}
//解码主函数
void Decode::decodeMain(){
    //照顾到 26位时：strData只执行一次 50位时;strData执行两次
    QString strData = getDataFromLPM();//获取下位机发送的数据
    if(strData == ""){
        return;
    }
    calculate(strData);//针对串口数据进行解包
    updateFlag();//更新各个位置的标志位 过零或摁键
    sendInfoToMethod();//选择将这些数据展示在哪儿些Form上
    return;
}
//公共函数
double Decode::AngleAxis(double Angledeg,int Begin_axis,const int Fourier_Size) const{
    double AfterAngle;
    double AfA;

    if(Angledeg>0){
        Angledeg = Angledeg -360;
    }
    AfA = (Angledeg/180)*commonData::getPI(); //将角度转换为弧度制

//    ERR = Coeffcor[0] + Coeffcor[1]*sin(AfA) + Coeffcor[2]*cos(AfA) + Coeffcor[3]*sin(2*AfA) + Coeffcor[4]*cos(2*AfA) + Coeffcor[5]*sin(3*AfA) + Coeffcor[6]*cos(3*AfA)+ Coeffcor[7]*sin(4*AfA) + Coeffcor[8]*cos(4*AfA)+
//        Coeffcor[9]*sin(5*AfA) + Coeffcor[10]*cos(5*AfA)+Coeffcor[11]*sin(6*AfA) + Coeffcor[12]*cos(6*AfA)+Coeffcor[13]*sin(7*AfA) + Coeffcor[14]*cos(7*AfA);
//    AfterAngle = AfA - ERR;
    if(mCommonData->getCoeffParameterSize() == 0){
        return (AfA/commonData::getPI())*180;//返回角度制
    }
    //循环模式
    int modulusFortheta = 1;//阶数

    double new_ERR = mCommonData->getCoeffParameterItem(Begin_axis);
    ++Begin_axis;
    for(int j = 0;j<(Fourier_Size-1)/2;++j){
        new_ERR = new_ERR + mCommonData->getCoeffParameterItem(Begin_axis)*sin(modulusFortheta*AfA);
        ++Begin_axis;
        new_ERR = new_ERR + mCommonData->getCoeffParameterItem(Begin_axis)*cos(modulusFortheta*AfA);
        ++modulusFortheta;
        ++Begin_axis;
    }
    AfterAngle = AfA - new_ERR;//0415new
//    qDebug()<<"single偏心量"<<(new_ERR/mCommonData->getPI())*180*3600;
    if (AfterAngle >= 0){
        AfterAngle = AfterAngle - 2*commonData::getPI();
    }
    return (AfterAngle/commonData::getPI())*180;//返回角度制
}
//双读数头处理方式 输入参数为角度制,返回角度值
double Decode::DualAngle(double Angledeg1,double Angledeg2,int Begin_axis,const int Fourier_Size) const{
    double AngleAver;
    double AfterAngle;

    AngleAver = ( Angledeg1 + Angledeg2 )/2;

    //角度制变成弧度制
    AngleAver = (AngleAver/180)*commonData::getPI();

    if(mCommonData->getCoeffParameterSize() == 0){
        return (AngleAver/commonData::getPI())*180;//返回角度制
    }

    int modulusFortheta = 1;//阶数
    double new_ERR = mCommonData->getCoeffParameterItem(Begin_axis);
    ++Begin_axis;

    //    ERR = Coeffcor[0] + Coeffcor[1]*sin(AfA) + Coeffcor[2]*cos(AfA) + Coeffcor[3]*sin(2*AfA) + Coeffcor[4]*cos(2*AfA) + Coeffcor[5]*sin(3*AfA) + Coeffcor[6]*cos(3*AfA)+ Coeffcor[7]*sin(4*AfA) + Coeffcor[8]*cos(4*AfA)+
    //        Coeffcor[9]*sin(5*AfA) + Coeffcor[10]*cos(5*AfA)+Coeffcor[11]*sin(6*AfA) + Coeffcor[12]*cos(6*AfA)+Coeffcor[13]*sin(7*AfA) + Coeffcor[14]*cos(7*AfA);
    //    AfterAngle = AfA - ERR;

    for(int j = 0;j<(Fourier_Size-1)/2;++j){
        new_ERR += mCommonData->getCoeffParameterItem(Begin_axis)*sin(modulusFortheta*AngleAver);
        ++Begin_axis;
        new_ERR += mCommonData->getCoeffParameterItem(Begin_axis)*cos(modulusFortheta*AngleAver);
        ++modulusFortheta;
        ++Begin_axis;
    }
    AfterAngle = AngleAver - new_ERR;
//    qDebug()<<AngleAver<<"double偏心量"<<(new_ERR/commonData::getPI())*180*3600;

    if (AfterAngle >= 0){
        AfterAngle = AfterAngle - 2*commonData::getPI();
    }
    return (AfterAngle/commonData::getPI())*180;//返回角度
}
//公共部分（计算空间坐标）
struct Matrix Decode::CoordTransf(){
        Matrix T1,T2,T3,T4,T5,T6,T7,T;
        double StructParaTemp[6][4];
        double ProLen;
        Matrix Temp;
        //结构参数分组处理
        StructParaTemp[0][0] = 0;
        StructParaTemp[0][1] = mCommonData->getStructParameterItem(5);
        StructParaTemp[0][2] = mCommonData->getStructParameterItem(10);
        StructParaTemp[0][3] = 0;


        StructParaTemp[1][0] = mCommonData->getStructParameterItem(0);
        StructParaTemp[1][1] = mCommonData->getStructParameterItem(6);
        StructParaTemp[1][2] = mCommonData->getStructParameterItem(11);
        StructParaTemp[1][3] = mCommonData->getStructParameterItem(16);

        StructParaTemp[2][0] = mCommonData->getStructParameterItem(1);
        StructParaTemp[2][1] = mCommonData->getStructParameterItem(7);
        StructParaTemp[2][2] = mCommonData->getStructParameterItem(12);
        StructParaTemp[2][3] = mCommonData->getStructParameterItem(17);

        StructParaTemp[3][0] = mCommonData->getStructParameterItem(2);
        StructParaTemp[3][1] = mCommonData->getStructParameterItem(8);
        StructParaTemp[3][2] = mCommonData->getStructParameterItem(13);
        StructParaTemp[3][3] = mCommonData->getStructParameterItem(18);

        StructParaTemp[4][0] = mCommonData->getStructParameterItem(3);
        StructParaTemp[4][1] = mCommonData->getStructParameterItem(9);
        StructParaTemp[4][2] = mCommonData->getStructParameterItem(14);
        StructParaTemp[4][3] = mCommonData->getStructParameterItem(19);

        StructParaTemp[5][0] = mCommonData->getStructParameterItem(4);
        StructParaTemp[5][1] = 1.5707963267949;
        StructParaTemp[5][2] = 1*-mCommonData->getStructParameterItem(14);
        StructParaTemp[5][3] = mCommonData->getStructParameterItem(20);

        ProLen = mCommonData->getStructParameterItem(15);
        //计算齐次转换矩阵
        T1 = TransfMatrix(StructParaTemp[0],mCommonData->getAxisAngleRadian(0));//调用TransfMatrix函数
        T2 = TransfMatrix(StructParaTemp[1],mCommonData->getAxisAngleRadian(1));
        T3 = TransfMatrix(StructParaTemp[2],mCommonData->getAxisAngleRadian(2));
        T4 = TransfMatrix(StructParaTemp[3],mCommonData->getAxisAngleRadian(3));
        T5 = TransfMatrix(StructParaTemp[4],mCommonData->getAxisAngleRadian(4));
        T6 = TransfMatrix(StructParaTemp[5],mCommonData->getAxisAngleRadian(5));
        T7 = TranslMatrix(ProLen);

        Temp = MultiMatrix(T1,T2);//调用MultiMatrix函数
        Temp = MultiMatrix(Temp,T3);
        Temp = MultiMatrix(Temp,T4);
        Temp = MultiMatrix(Temp,T5);
        Temp = MultiMatrix(Temp,T6);
        Temp = MultiMatrix(Temp,T7);

        T = Temp;
        return T;
}
struct Matrix Decode::TransfMatrix(double StructPara[],double Angle){
    Matrix T;
    double Theta,Alfa,l,d;
    Theta = StructPara[0] + Angle;
    Alfa = StructPara[1];
    l = StructPara[2];
    d = StructPara[3];

    T.data[0][0] = cos(Theta);
    T.data[0][1] = -sin(Theta)*cos(Alfa);
    T.data[0][2] = sin(Theta)*sin(Alfa);
    T.data[0][3] = l*cos(Theta);
    T.data[1][0] = sin(Theta);
    T.data[1][1] = cos(Theta)*cos(Alfa);
    T.data[1][2] = -cos(Theta)*sin(Alfa);
    T.data[1][3] = l*sin(Theta);
    T.data[2][0] = 0;
    T.data[2][1] = sin(Alfa);
    T.data[2][2] = cos(Alfa);
    T.data[2][3] = d;
    T.data[3][0] = 0;
    T.data[3][1] = 0;
    T.data[3][2] = 0;
    T.data[3][3] = 1;

    return T;
}
struct Matrix Decode::TranslMatrix(double ProLen){
    Matrix T;
    T.data[0][0] = 1;
    T.data[0][1] = 0;
    T.data[0][2] = 0;
    T.data[0][3] = 0;

    T.data[1][0] = 0;
    T.data[1][1] = 1;
    T.data[1][2] = 0;
    T.data[1][3] = 0;

    T.data[2][0] = 0;
    T.data[2][1] = 0;
    T.data[2][2] = 1;
    T.data[2][3] = ProLen;

    T.data[3][0] = 0;
    T.data[3][1] = 0;
    T.data[3][2] = 0;
    T.data[3][3] = 1;
    return T;
}
struct Matrix Decode::MultiMatrix(Matrix A,Matrix B){
    int i,j,k;
    Matrix C;
    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            C.data[i][j] = 0; //初始化
            for(k = 0; k < 4; k++){
                C.data[i][j] += A.data[i][k]*B.data[k][j];
            }
        }
    }
    return C;
}
