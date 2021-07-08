#include "twentysixdecode.h"

twentysixDecode::twentysixDecode(QWidget *parent) : Decode(parent){

}
twentysixDecode::~twentysixDecode(){

}
//获取下位机发送的数据
QString twentysixDecode::getDataFromLPM(){
    QString strResult;
    //接受数据的时候,readserial会一直执行,这段程序会一直执行，从输出标志位能够看出来（qDebug）
    /*******进行采集数据的强制转换******/
    QByteArray byte_data = mSerial->readAllData();//字节数组
    strResult = byte_data.toHex().data();//变成十六进制
    if(strResult.size() != 26*2){
        return "";
    }
//    qDebug()<<strResult<<strResult.size();
    return strResult;
}
//进行计算，并得到角度值和坐标值
void twentysixDecode::calculate(QString & strData){
    Matrix T;
    QString strDisplay;
    for(int i = 0,j=0;i<52;i+=2,++j){
        //输出形式为每个字符之间有空格
        bool ok;
        QString st = strData.mid(i,2);
        strDisplay += st;
        originalByteData26[j] = st.toInt(&ok,16);//字母也成数字,
    }
   /*******完成******/
    AngleComputation(originalByteData26,transAngle26);//计算出八个角度值****************
    //经过上一个函数的调用，angle已经不是空数组了，angle中已有八个角度值

    int length = 15;//15个 7阶
    int BeginPoint = 0;
    //A1 0~14 blink 15~29 30~44 A2 45~59 A3 60~74 A4 75~89 A5 90~104 A6 105~119

    double value = DualAngle(transAngle26[0],transAngle26[1],BeginPoint,length);//返回角度值
    mCommonData->setAxisAngle(0,value);

    //旧版本的excel中有三行空白，需要人为的剔除，此部分已经在coeff中进行了，链表的起点直接接着累加就行
    //下面的角度值是从3开始的，2是空白，第二个关节是假双读数头
    BeginPoint = BeginPoint + 45;
    for(int i = 1;i < 6;i++){
        value = AngleAxis(transAngle26[i+2],BeginPoint,length);//Coeff是偏心误差
        mCommonData->setAxisAngle(i,value);
        BeginPoint += length;
    }
    //计算三维坐标T是结构体
    T.col = CoordTransf().col;//structpara是结构误差，也是要导入的
    T.row = CoordTransf().row;
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < sizeOfCol; ++j){
            T.data[i][j] = CoordTransf().data[i][j];//CoordTransf，返回结构体类型的函数
        }
    }
    for(int i = 0;i < 3;++i){
          mCommonData->setCoordinate(i,T.data[i][3]);
    }
}
//各类标志位的设置
void twentysixDecode::updateFlag(){
    if(originalByteData26[0] == 80){
        mCommonData->setRecordFlag(true);
    }
    else {
        mCommonData->setRecordFlag(false);
    }
    //过零标志位
    //因为26位模式没有过零，所以全部显示已经过零
    mCommonData->initialAxisCrossZero(true);
//    chooseRunningPart();
}
void twentysixDecode::AngleComputation(int original[26],double angle[8]){
    int input = 0;
    for (int i = 1; i < 25; i++){
        input ^= original[i];//input = input ^ original[i];按位异或
    }
    if (input == original[25]){
        double Da = 0.3955078125, Db = 0.791015625;
        for (int j = 0; j < 24; j++){
            int number = j / 3;//序号
            if (j % 3 == 0 && j <= 11)  {
                //1____________4
                int R = original[j + 1] * 65536 + original[j + 2] * 256 + original[j + 3];
                if (R>5000000){
                    R = 10000000 - R;
                    angle[number] = -R*Da / 3600.0;
                }
                else{
                    angle[number] = R*Da / 3600.0;
                }
        }
        else if (j % 3 == 0 && j >= 12){
                //5__________8
                int R = original[j + 1] * 65536 + original[j + 2] * 256 + original[j + 3];
                if (R>5000000){
                    R = 10000000 - R;
                    angle[number] = -R*Db / 3600.0;
                }
                else{
                    angle[number] = R*Db / 3600.0;
                }
            }
        }
        for (int k = 0; k < 8; k++){
            if (k == 4 || k == 5){
                int l = k + 2;
                double tempt = angle[k];
                angle[k]=angle[l];
                angle[l] = tempt;
            }
        }
    }
}
