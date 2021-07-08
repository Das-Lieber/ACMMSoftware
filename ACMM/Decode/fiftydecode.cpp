#include "fiftydecode.h"

fiftyDecode::fiftyDecode(QWidget *parent) : Decode(parent){
    byteNumFlag = 0;
    strOne = "";
    strTwo = "";
    strResult = "";
    //圆光栅过零更新
    for(int i = 0;i<6;i++){
        crossZeroList.append(0);
    }
}
fiftyDecode::~fiftyDecode(){

}
//获取下位机发送的数据
QString fiftyDecode::getDataFromLPM(){
    //因为要分成两次接收，接收标志位，最后要清零
    byteNumFlag++;
    if(byteNumFlag == 1){
        byteDataone = mSerial->readAllData();//字节数组
        if(byteDataone.size() != 32){
            byteNumFlag = 0;
            return "";
        }
        strOne = byteDataone.toHex().data();//变成十六进制
        return "";
    }
    else if(byteNumFlag == 2){
        byteDatatwo = mSerial->readAllData();//字节数组,限定个数
        strTwo = byteDatatwo.toHex().data();//变成十六进制
        if(byteDatatwo.size()!=18){
            byteNumFlag = 1;
            return "";
        }
        //拼接
        strResult = QString("%1%2").arg(strOne).arg(strTwo);
        //标志位
        byteNumFlag = 0;//000：因为要分成两次接收，接收标志位，最后要清零，此处为清零
        return strResult;
    }
    if(strResult.size() != 50){
        return "";
    }
    return "";
}
//进行计算，并得到角度值和坐标值
void fiftyDecode::calculate(QString & strData){
    qDebug()<<strData;
    QString strDisplay;
    Matrix T;
    /*******进行采集数据的强制转换******/
    for(int i = 0,j = 0;i < 100;i+=2,++j){
        //循环了（50x2）100次,因为串口发送数据时候，有可能直接发平常两倍的数据，直接给52，发再多我也不读。
        //输出形式为每个字符之间有空格
        bool ok;
        QString st = strData.mid(i,2);
        strDisplay+=st;
        originalByteData50[j]=st.toInt(&ok,16);//字母也成数字,
    }
    //硬件会在50之后，上传一组除了标志物是54，之后全是0的一组数据，因为硬件全部清空了一次
    //应对措施就是，跳过这组数据，但是绘图的内容和表格中的数据部分还是会继续采集，因为54全是0那组被return了，但是X数组里面保留的还是50那一组的数据
    //所以，要把pushflag变成54，否则第一次读到50，之后因为54全是0跳过了，所以成员变量相关的参数保留的内容还是50那组的，会再读一次50的数据
    //这样摁键在什么情况下都会变成单点模式，因为接受第一个50后，又紧接着来了另外一个50 ，所以此时要变成54（16HEx=84 ）
    //放在此处，是为了其他程序用DH模型的时候，都会解决这个问题，而不是放在dhforcalibration或者dhforcorrection

    int zeroIndex = 0,oriIndex = 0,adIndex = 0;
    //把目前不需要的温度参数剔除 7和8分别是温度和过零
    for(int i = 0;i<50;i++){
        if(i == 0){
            originalAngleData[adIndex++] = originalByteData50[i];//除去温度和过零标志位，其他保存在数组中（包含标志位和校验位）
        }
        else if(i == 49){
            originalAngleData[adIndex++] = originalByteData50[i];//除去温度和过零标志位，其他保存在数组中（包含标志位和校验位）
        }
        else{
            original[oriIndex++] = originalByteData50[i];//除去过零标志位和校验位,其它都要
            //i会去除多余的，j是原本累加
            if(i%8 == 0){
                //7,8;15,16;23,24;31,32;39,40;47,48; 7是过零 8是温度
                continue;
            }
            else if((i+1)%8 == 0){
                //7,8;15,16;23,24;31,32;39,40;47,48; 7是过零 8是温度
                crossZeroList.replace(zeroIndex++,originalByteData50[i]);//保存过零标志位
                continue;//过滤掉温度内容
            }
            originalAngleData[adIndex++] = originalByteData50[i];//除去温度和过零标志位，其他保存在数组中（包含标志位和校验位）
        }
    }
    //第一个是标志物54，最后是校验位ae，3个字节对应一个读数头96 d3 2a，第一个关节92 32 ec
    //第二个大轴因为没有办法转动360度，没办法安装两个读数头，所以程序上虽然表现为双读数头，但是其实是作废的，因为传输的内容是00 00 00
    //旧式：第一位是标志物，最后一位是校验位，一共六个关节，一共七个个读数头，2+1+1+1+1+1=7，但是程序在第二个关节处加入了一个废弃的读数头，表现为8个读数头，24个字节，总共26个
    //目前要全部改成双读数头的，第一位是标志物，最后一位是校验位，一共六个关节，一共十二个读数头，每个读数头三个字节，总共36个字节，加上标志位和校验位，总共38
    /*******完成******/
    AngleComputation(original,originalAngleData,transAngle50);//计算出十二个角度值
    //经过上一个函数的调用，angle已经不是空数组了，angle中已有十二个角度值
    //根据前面板的选择，执行程序
    /**********************************************************圆光栅处理部分*******************************************************/
    int BeginPoint = 0;
    int length = 15;
    for(int i=0,j=0;i<6;++i){
        //0 1;2 3；4 5；6 7；8 9；10 11；
        //循环六次，最终得到六个角度值,i是角度值，j是读数头的个数
        double value = DualAngle(transAngle50[j],transAngle50[j+1],BeginPoint,length);//返回值为角度值
        mCommonData->setAxisAngle(i,value);
        j+=2;
        BeginPoint+=length;
    }
    //能不能把参数去掉，下面两个函数，直接从commonData里面走
    T.col = CoordTransf().col;//structpara是结构误差，也是要导入的
    T.row = CoordTransf().row;
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
            T.data[i][j] = CoordTransf().data[i][j];//CoordTransf，返回结构体类型的函数
        }
    }
    for(int i=0;i<3;++i){
        mCommonData->setCoordinate(i,T.data[i][3]);
    }
}
//各类标志位的设置
void fiftyDecode::updateFlag(){
    //摁键触发
    if(originalByteData50[0] == 80){
        mCommonData->setRecordFlag(true);
    }
    else {
        mCommonData->setRecordFlag(false);
    }
    //过零标志位
    for(int i = 0;i<6;++i){
        mCommonData->setAxisCrossZero(i,crossZeroList[i] == 1?true:false);
    }
}
void fiftyDecode::AngleComputation(int judge_original[48],int original[38],double angle[12]){
    int input = 0;
    for (int i = 0; i < 48; i++){
        input ^= judge_original[i];
    }
    if (input == original[37]){
        double Da = 0.3955078125, Db = 0.648;//Da = Db=0.648
        for (int j = 0; j < 36; j++){
            int number = j / 3;//序号
            if (j % 3 == 0 && j <= 11){
                int R = original[j + 1] * 65536 + original[j + 2] * 256 + original[j + 3];
                if (R>5000000){
                    R = 10000000 - R;
                    angle[number] = -R*Da / 3600.0;
                }
                else{
                    angle[number] = R*Da / 3600.0;
                }
                //2021 0413 针对ARM芯片，新读数头（体积较小），当前为50通讯模式下，全部的角度值，经过计算后，要乘上-1（取反）
                angle[number] = -1*angle[number];//20210413
            }
            else if (j % 3 == 0 && j >= 12){
                //5__________12
                int R = original[j + 1] * 65536 + original[j + 2] * 256 + original[j + 3];
                if (R>5000000){
                    R = 10000000 - R;
                    angle[number] = -R*Db / 3600.0;
                }
                else{
                    angle[number] = R*Db / 3600.0;
                }
                //2021 0413 针对ARM芯片，新读数头（体积较小），当前为50通讯模式下，全部的角度值，经过计算后，要乘上-1（取反）
                angle[number] = -1*angle[number];//20210413
            }
        }
    }
}
