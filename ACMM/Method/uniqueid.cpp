#include "uniqueid.h"

uniqueID::uniqueID(QWidget *parent) : QWidget(parent){
    mSerial = new Serial(this);
    mReadWriteLock = new QReadWriteLock();
    mCommonData = new commonData(this);
    mExplanation = new explanation(this);
    mImportFile = nullptr;
    mDecode = nullptr;
}
uniqueID::~uniqueID(){

}
//主界面点击连接后，执行main函数
void uniqueID::uniqueIDMain(){
//    qDebug()<<"uniqueID2：uniqueIDMain 信号和槽连接";
    if(mSerial == nullptr){
        QMessageBox::warning(this,"警告","请检查decede部分是否使用野指针",QMessageBox::Yes,QMessageBox::No);
        return;
    }
    //1:自动连接
    bool flag = mSerial->autoConnect();
    //判断，true表示连接成功，false表示连接失败
    if(flag == true){
        connect(mSerial->getSerialPointer(),&QSerialPort::readyRead,this,&uniqueID::decodeUniqueCode);
    }
    else if(flag == false){
        QMessageBox::warning(this,"串口情况","打开串口失败，请重试！",QMessageBox::Yes,QMessageBox::No);
    }
}
//解包接口
void uniqueID::decodeUniqueCode(){
//    qDebug()<<"uniqueID2：decodeUniqueCode 唯一标识符解码";
    mReadWriteLock->lockForWrite();
    //重要标志位全部置零 不管是第一次启动软件 还是中途更换功能 只要是断点续传 都需要经过此部分 在这个部分把该置零的都置零
    mCommonData->setFlagFor50Bytes(false);
    mCommonData->setFlagFor26Bytes(false);
    QByteArray byte_data = mSerial->readAllData();//字节数组
    if(  (  byte_data.size() == 32||byte_data.size() == 18 ) && mCommonData->getFlagFor50Bytes() ){
        //50位 进入这个判断只有一种可能，就是重新插拔
        mSerial->serialOpen();//再次连接
        mSerial->disConnect();//此部分断开连接，下位机一直在发送识别符，接受一次即可
        emit formButtonEnable();
        mCommonData->setLabelEnable(true);//恢复label的摁键功能
        QMessageBox::about(this,"提示","50位通讯模式重新连接成功");
    }
    else if(  ( byte_data.size() == 26 ) && mCommonData->getFlagFor26Bytes() ){
        //26位 进入这个判断只有一种可能性，就是重新插拔
        //说明已经解包了，已经完成了，现在需要的就是继续连接到50位即可
        mSerial->serialOpen();//再次连接
        mSerial->disConnect();//此部分断开连接，下位机一直在发送识别符，接受一次即可
        emit formButtonEnable();
        mCommonData->setLabelEnable(true);//恢复label的摁键功能
        QMessageBox::about(this,"提示","26位通讯模式重新连接成功");
    }
    else if( byte_data.size() == 27 ){

        //解包前，首先将相关的bool控制变量全部初始化或者清零（因为你无法判断是第一次连接还是中途断电重新上电）
        mCommonData->setFlagFor50Bytes(false);
        mCommonData->setFlagFor26Bytes(false);
//        mCommonData->Bool_FlagForAverage = false;
//        mCommonData->Bool_FlagForAverage_90 = false;
        //过零 标志位回复初值
        mCommonData->initialAxisCrossZero(false);
        //解包前，首先将相关的bool控制变量全部初始化或者清零（因为你无法判断是第一次连接还是中途断电重新上电）
        QString strDisplay = byte_data.toHex().data();//变成十六进制;
        //26 ;02 02 02 02   02   02;   0A    01;    02    00    01    09    01   02   00   09   0A;   02   00   01   09   01   02   00   09; 36
        //01  23 45 67 89 1011  1213  1415  1617   1819  2021  2223  2425  2627 2829 3031 3233 3435  3637 3839 4041 4243 4445 4647 4849 5051 52 53
        /**********************************************解包过程**********************************************/
        if(mCommonData->getUniqueCodeSize() != 0){
            //每次解包都需要清除旧的部分
            mCommonData->clearUniqueCode();
        }
        //1完整的识别符
        //2通讯位数
        QString String_C = strDisplay.mid(0,2);
        if( String_C.toInt(nullptr,10) == 26 ){
            //第一位 26/50
            mCommonData->setFlagFor26Bytes(true);
            mCommonData->setFlagFor50Bytes(false);
            qDebug()<<"26位通讯";
        }
        else if( String_C.toInt(nullptr,10) == 50 ){
            mCommonData->setFlagFor26Bytes(false);
            mCommonData->setFlagFor50Bytes(true);
            qDebug()<<"50位通讯";
        }
        else if( String_C.toInt(nullptr,10) != 26&&String_C.toInt(nullptr,10) != 50 ){
            qDebug()<<"通讯位数解包失败";
            return;
        }
        //3传感器情况
        QString String_SituationOfSensor;//传感器情况
        for(int i = 2;i<14;i+=2){
            QString Temp = strDisplay.mid(i,2);
            String_SituationOfSensor += Temp.mid(1);
        }
        //4数据的处理方式
        QString String_WayToDealData = strDisplay.mid(14,4).toUpper();//数据的处理方式
        //5转角误差
        QString String_ErrorForAngle;//转角误差
        for(int i = 18;i<36;i+=2){
            QString Temp = strDisplay.mid(i,2);
            String_ErrorForAngle += Temp.mid(1);
        }
        //6装配日期
        QString String_Date;//装配日期
        for(int i = 36;i<52;i+=2){
            QString Temp = strDisplay.mid(i,2);
            String_Date += Temp.mid(1);
        }
        //7ACMM型号
    //    _211111(传感器编码情况)_A/AS(传感器的数据处理情况)_(36/30/24/18/12)型号_(01/02/03)如果前面都相同，就靠后面的编号识别了(主要由装配时间来判断)
        //同类型关节臂编号确定
        QString ID_EndNumber;
        if(String_C.toInt(nullptr,10) == 26){
            //26位旧版本的关节臂都是均值减去90°的 新安装的都是直接均值的结果
            if(String_Date.mid(0,4).toInt(nullptr,10) == 2016){
                ID_EndNumber = "01";
            }
            else if(String_Date.mid(0,4).toInt(nullptr,10) == 2017){
                ID_EndNumber = "02";
            }
            else if(String_Date.mid(0,4).toInt(nullptr,10) == 2018){
                ID_EndNumber = "03";
            }
        }
        else if(String_C.toInt(nullptr,10) == 50){
            if(String_Date.mid(0,4).toInt(nullptr,10) == 2019||String_Date.mid(0,4).toInt(nullptr,10) == 2020){
                ID_EndNumber = "01";
            }
            else if(String_Date.mid(0,4).toInt(nullptr,10) == 2021){
                ID_EndNumber = "02";
            }
        }
        QString IdForACMM = QString("%1%2%3%4%5%6%7%8")
                .arg("_").arg(String_SituationOfSensor)
                .arg("_").arg(String_WayToDealData)
                .arg("_").arg(strDisplay.mid(52,2))
                .arg("_").arg(ID_EndNumber);

        //将解包内容压缩到mCommonData->uniqueCode中
        mCommonData->setUniqueCodeItem(strDisplay);//1完整的识别符
        mCommonData->setUniqueCodeItem(String_C);//2通讯位数
        mCommonData->setUniqueCodeItem(String_SituationOfSensor);//3传感器情况
        mCommonData->setUniqueCodeItem(String_WayToDealData);//4数据的处理方式
        mCommonData->setUniqueCodeItem(String_ErrorForAngle);//5转角误差
        mCommonData->setUniqueCodeItem(String_Date);//6装配日期
        mCommonData->setUniqueCodeItem(strDisplay.mid(52,2));//7ACMM型号
        mCommonData->setUniqueCodeItem(IdForACMM);//8完整命名方式
        /**********************************************解包过程**********************************************/
        //解包过程 能走到这一步都是解包成功的
        mSerial->disConnect();//此部分断开连接，下位机一直在发送识别符，接受一次即可
        //当explanation_main界面上的确定键摁下时，自动进行结构参数和偏心误差的导入
        connect(mExplanation->getOKButtonPointer(),&QPushButton::clicked,this,&uniqueID::autoInsertParameter);
        mExplanation->showUi();//展示界面给用户
    }
    else {
        mSerial->disConnect();
        QMessageBox::warning(this,"警告","请重启硬件",QMessageBox::Yes,QMessageBox::No);
    }
    mReadWriteLock->unlock();
}
//自动带入结构参数和偏心误差的函数
void uniqueID::autoInsertParameter(){
//    qDebug()<<"uniqueID2：autoInsertParameter 解码成功 进行预处理操作";
    autoInsertStructPara();
    autoInsertCoeffPara();
    //发送信号给Widget，可以使得【结构参数】【偏心误差】【说明】三个摁键回复功能
    emit formButtonEnable();
    //发送信号给下位机，可以正常发送数据了（发送函数还是写在communication中，此处只是调用）
    mSerial->sendDataToSlave();
    //改变Label的标志位
    mCommonData->setLabelEnable(true);//恢复label的摁键功能
    connectPart();
}
void uniqueID::autoInsertStructPara(){
    //结构参数
    if(mImportFile == nullptr){
        mImportFile = new importStructFile(this);
    }
    else {
        delete mImportFile;
        mImportFile = new importStructFile(this);
    }
    bool flagForStr = mImportFile->importFileMain(true,commonData::structureType);//表示自动读取结构参数
    if( flagForStr == false) {
        QMessageBox::warning(this,"警告","结构参数导入失败",QMessageBox::Yes,QMessageBox::No);
        return;
    }
    else{
        QString coffInfo = QString("%1%2%3").arg("结构参数导入成功(").arg(commonData::SizeOFStruct).arg("项）");
        QMessageBox::about(this,tr("参数导入提醒"),coffInfo);
    }
}
void uniqueID::autoInsertCoeffPara(){
    //结构参数
    if(mImportFile == nullptr){
        mImportFile = new importCoeffFile(this);
    }
    else {
        delete mImportFile;
        mImportFile = new importCoeffFile(this);
    }

    bool flagForStr = mImportFile->importFileMain(true,commonData::coeffType);//表示自动读取结构参数
    if( flagForStr == false) {
        mCommonData->clearCoeffParameter();
        QMessageBox::warning(this,"警告","偏心误差导入失败",QMessageBox::Yes,QMessageBox::No);
        return;
    }
    else{
        QString coffInfo = QString("%1%2%3").arg("偏心误差导入成功(").arg(mCommonData->getCoeffParameterSize()).arg("项）");
        QMessageBox::about(this,tr("参数导入提醒"),coffInfo);
    }
}
void uniqueID::connectPart(){
    //需要判断是26位还是50位 根据不同的情况进行解包
    if( mCommonData->getFlagFor50Bytes() == true ){
        qDebug()<<"50位 calibration";
        fiftyDecode * mFiftyDecode = new fiftyDecode(this);
        if(mDecode != nullptr){
            delete mDecode;
        }
        mDecode = mFiftyDecode;
        mDecode->connectToLPM(mFiftyDecode);//进行连接
    }
    else if( mCommonData->getFlagFor26Bytes() == true ){
        qDebug()<<"26位 calibration";
        twentysixDecode * mtwentysixDecode = new twentysixDecode(this);
        if(mDecode != nullptr){
            delete mDecode;
        }
        mDecode = mtwentysixDecode;
        mDecode->connectToLPM(mtwentysixDecode);//进行连接
    }
}
void uniqueID::manulConnect(){
    bool flag = mSerial->autoConnect();
    if(flag == true){
        connectPart();
        mCommonData->setLabelEnable(true);//恢复label的摁键功能
        emit formButtonEnable();
    }
    else if(flag == false){
        QMessageBox::warning(this,"串口情况","打开串口失败，请重试！",QMessageBox::Yes,QMessageBox::No);
    }
}
