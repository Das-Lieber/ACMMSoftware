#include "coordinate.h"
QString coordinate::hHeaderItem[3] = {"x","y","z"};
coordinate::coordinate(QWidget *parent) :
    QDialog(parent){
    mReadWriteLock = new QReadWriteLock();
    mcoordinateForm = new coordinateForm(this);
    mCommonData = new commonData(this);
    //监听套接字,tcp的内容不要放在loop里面，直接放在主函数里面就OK了
    tcpserver = new QTcpServer(this);//自动回收空间
    tcpsocker = new QTcpSocket(this);
    mSaveFile = nullptr;
    initialForm();
}
coordinate::~coordinate(){

}
/*************************************初始化及准备阶段********************************************/
void coordinate::initialForm(){
    tcpserver->listen(QHostAddress::Any,2200);//端口2200
    boolPushBottonControl = false;
    sampleDataList.clear();
    numForSample = 0;
    boolQMess = true;
    //保存
    connect(mcoordinateForm->getSaveButtonPointer(),&QPushButton::clicked,this,&coordinate::saveData);
    //清空
    connect(mcoordinateForm->getClearButtonPointer(),&QPushButton::clicked,this,&coordinate::clearData);
}
//预处理部分 UI的显示
void coordinate::pretreatment(){
    if( mcoordinateForm == nullptr ){
        return;
    }
    mcoordinateForm->SetCoordinateForm(this);//启动UI
    tcpserver->disconnect();//先断开全部的tcp连接，再进行连接
    connect(tcpserver,&QTcpServer::newConnection,[=](){
        tcpsocker = tcpserver->nextPendingConnection();
    });
    this->show();
}
/*************************************主函数********************************************/
void coordinate::getInfo(){
    mReadWriteLock->lockForRead();
    updateSamplePart();
    QString numstr = QString::number(numForSample,10);//显示十个点记录了几个
    mcoordinateForm->setNumTextItem(numstr);
    updateFromText();
    TCPConnect();
    mReadWriteLock->unlock();
}
void coordinate::updateSamplePart(){
    //采样情况
    if( mcoordinateForm->getBoxCurrentText()=="连续模式" ){
       if( mCommonData->getRecordFlag() == true ){
           //有摁键摁下
            if(boolPushBottonControl == false){
                boolPushBottonControl = true;
            }
            else if(boolPushBottonControl == true){
               boolPushBottonControl = false;
            }
       }
       if(boolPushBottonControl == true&&boolQMess == true){
//           else if(boolPushBottonControl == true){
//              boolPushBottonControl = false;
//           }
           //有摁键摁下
           updataCoordinateList();
           ++numForSample;//个数加1
       }
       if(numForSample == 600 && boolQMess==true){
           QMessageBox::warning(this,"采样点数提醒","采样点数已经达到上限（600个）",QMessageBox::Yes);
           boolPushBottonControl = false;
           boolQMess = false;
        }
    }
    else  if( mcoordinateForm->getBoxCurrentText()=="A S M E" ){
        if( mCommonData->getRecordFlag() == true ){
            //有摁键摁下
            updataCoordinateList();
            ++numForSample;//个数加1
       }
    }
}
void coordinate::updateFromText(){
    if( mCommonData->getFlagFor50Bytes() == true){
        //进入50位模式
        bool BoolFlag_Color =
                mCommonData->getAxisCrossZero(0)&&
                mCommonData->getAxisCrossZero(1)&&
                mCommonData->getAxisCrossZero(2)&&
                mCommonData->getAxisCrossZero(3)&&
                mCommonData->getAxisCrossZero(4)&&
                mCommonData->getAxisCrossZero(5);

        if(BoolFlag_Color == false){
            QColor clrR(255,0,0);
            stringToHtml( mCommonData->getCoordinateItemStr(0,3) ,clrR);
            stringToHtml( mCommonData->getCoordinateItemStr(1,3) ,clrR);
            stringToHtml( mCommonData->getCoordinateItemStr(2,3) ,clrR);
            /*****更新坐标数值*******/
            updataCoordinate();
        }
        else{
            /*****更新坐标数值*******/
            updataCoordinate();
        }
    }
    else if(mCommonData->getFlagFor26Bytes() == true){
        //进入26位模式
        updataCoordinate();
    }
}
void coordinate::TCPConnect(){
    /******TCP******/
    QString str_Send;//str_Show;
    str_Send =',' + mCommonData->getCoordinateItemStr(0,3) +
            ',' + mCommonData->getCoordinateItemStr(1,3) +
            ',' + mCommonData->getCoordinateItemStr(2,3);

    if(mCommonData->getRecordFlag() == true){
        //80（d） 50（hex）表示摁键摁下
        str_Send = 'P'+str_Send;
    }
    else  if(mCommonData->getRecordFlag() == false){
        //84（d） 54（hex）表示摁键摁下
        str_Send = 'T'+str_Send;
    }
    else{
        str_Send = "ERROR!";
    }
    str_Send = str_Send + "\r\n";//这个是要TCP通讯发出去的
    tcpsocker->write(str_Send.toLatin1().data());
}
void coordinate::updataCoordinate(){
    mcoordinateForm->setXTextItem( mCommonData->getCoordinateItemStr(0,3) );
    mcoordinateForm->setYTextItem( mCommonData->getCoordinateItemStr(1,3) );
    mcoordinateForm->setZTextItem( mCommonData->getCoordinateItemStr(2,3) );
}
void coordinate::updataCoordinateList(){
    sampleDataList.append(mCommonData->getCoordinateItem(0));
    sampleDataList.append(mCommonData->getCoordinateItem(1));
    sampleDataList.append(mCommonData->getCoordinateItem(2));
}
/*************************************特殊功能区域********************************************/
void coordinate::stringToHtml(QString str,QColor crl){
    //不需要返回值，取的str的地址，直接修改原值
     QByteArray array;
     array.append(crl.red());
     array.append(crl.green());
     array.append(crl.blue());//拼接一下颜色（RGB）
     QString strC(array.toHex());
     str = QString("<span style=\" color:#%1;\">%2</span>").arg(strC).arg(str);
}
void coordinate::ASME(){
    double size = (double)numForSample;
    if(sampleDataList.size()/3 != numForSample){
        QMessageBox::warning(this,"警告","数据发生错误，检查程序",QMessageBox::Yes,QMessageBox::No);
        return;//改成bool
    }
    //平均值
    double sumx=0.0,sumy=0.0,sumz=0.0;
    int iF0rdata = 0;
    for(int i = 0;i<size;i++){
        sumx += sampleDataList.at(iF0rdata);//x坐标求和
        sumy += sampleDataList.at(iF0rdata+1);//y坐标求和
        sumz += sampleDataList.at(iF0rdata+2);//z坐标求和
        iF0rdata += 3;
     }
    xAver = sumx/size;
    yAver = sumy/size;
    zAver = sumz/size;
    //偏差
    double partone[10],parttwo[10],partthree[10];
    iF0rdata = 0;
    for(int j = 0;j<size;j++){
        partone[j] = (sampleDataList.at(iF0rdata) - xAver)*(sampleDataList.at(iF0rdata) - xAver);//X-Xi的平方
        parttwo[j] = (sampleDataList.at(iF0rdata+1) - yAver)*(sampleDataList.at(iF0rdata+1) - yAver);
        partthree[j] = (sampleDataList.at(iF0rdata+2) - zAver)*(sampleDataList.at(iF0rdata+2) - zAver);

        deviation[j] = sqrt(partone[j]+parttwo[j]+partthree[j]);
        iF0rdata += 3;
    }

    //标准差
    double sumSPAT = 0.0;
    for(int p = 0;p <size;p++){
        sumSPAT = sumSPAT+( deviation[p] )*( deviation[p] );
    }
    Sspat = sumSPAT/(size - 1);
    Sspat = sqrt(Sspat);
    Sspat  = 2*Sspat;
    //最大偏差
    maxDevia = deviation[0];
    for(int k = 1;k<size;k++){
        if(deviation[k]>maxDevia){
            maxDevia = deviation[k];
        }
    }
    //普通的三个标准差excelSTDEV
    double exex=0.0,exey=0.0,exez=0.0;
    for(int e = 0;e<size;e++){
        exex =exex + partone[e];
        exey =exey + parttwo[e];
        exez =exez + partthree[e];
    }
    excelSTDEV[0]= sqrt( exex/(size-1) );
    excelSTDEV[1]= sqrt( exey/(size-1) );
    excelSTDEV[2]= sqrt( exez/(size-1) );
}
/*************************************绘图及关闭事件********************************************/
void coordinate::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);
    QPainter p(this);//指定对象为窗口
    p.drawPixmap(0,0,width(),height(),QPixmap(":/image/uk.png"));
}
void coordinate::closeEvent(QCloseEvent *event){
    int ret = QMessageBox::question(this,"关闭","是否关闭窗口：",QMessageBox::Yes,QMessageBox::No);
    if(ret == QMessageBox::Yes){
        clearData();
        tcpserver->disconnect();
        Decode::removeAll();//从通知队列中移除全部内容（只针对一个观察者）
        emit readyClose();
        event->accept();
    }
    else if(ret == QMessageBox::No){
        event->ignore();
    }
}
/*************************************保存及清空功能区********************************************/
void coordinate::clearData(){
    sampleDataList.clear();
    numForSample = 0;
}
void coordinate::assigAbstractObject(coordinate * arg){
    if(mSaveFile == nullptr){
        mSaveFile = arg;
    }
}
//保存数据的功能,保存成excel吧
void coordinate::saveData(){
    if(mSaveFile == nullptr){
        mSaveFile = new coordinate(this);
    }
    bool result = mSaveFile->saveFileMain(numForSample,3,1,2);
    if(result == true){
        QMessageBox::about(nullptr,tr("save"),tr("导出成功"));
    }
    else{
        QMessageBox::about(nullptr,tr("save"),tr("导出失败"));
    }
}
QString * coordinate::getSaveExcelHeader(){
    return hHeaderItem;
}
void coordinate::getSampleListItem(QAxObject * worksheet,int arg){
    int iF0rdata = 0;
    //0 1 2 3 4 5
    for(int ci = 0;ci<numForSample;ci++){
        QString data;
        for(int cj=0;cj<3;cj++){
            int index = iF0rdata+cj;
            if( index >= sampleDataList.size() ){
                break;
            }
            data = QString::number(sampleDataList.at(index),10,6);
            worksheet->querySubObject("Cells(int,int)", ci+2, cj+arg)->dynamicCall("SetValue(const QString&)",
                                                            data);
        }
        iF0rdata += 3;
    }
}
void coordinate::runASME(QAxObject * worksheet){
    ASME();
    //数值插入
    for(int di = 0;di<numForSample;di++){
        QString rowdata;
        rowdata = QString::number(deviation[di],10,8);
        worksheet->querySubObject("Cells(int,int)", di+2,5)->dynamicCall("SetValue(const QString&)",rowdata);
    }
    //标准差
    QString e_x =QString::number(excelSTDEV[0],10,10);
    QString e_y =QString::number(excelSTDEV[1],10,10);
    QString e_z =QString::number(excelSTDEV[2],10,10);
    worksheet->querySubObject("Cells(int,int)", 12, 2)->dynamicCall("SetValue(const QString&)",e_x);
    worksheet->querySubObject("Cells(int,int)", 12, 3)->dynamicCall("SetValue(const QString&)",e_y);
    worksheet->querySubObject("Cells(int,int)", 12, 4)->dynamicCall("SetValue(const QString&)",e_z);
    worksheet->querySubObject("Cells(int,int)", 12, 1)->dynamicCall("SetValue(const QString&)","standard deviation");
    //三个平均值
    worksheet->querySubObject("Cells(int,int)", 13, 1)->dynamicCall("SetValue(const QString&)","Average");
    QString e_xAver =QString::number(xAver,10,8);
    QString e_yAver =QString::number(yAver,10,8);
    QString e_zAver =QString::number(zAver,10,8);
    worksheet->querySubObject("Cells(int,int)", 13, 2)->dynamicCall("SetValue(const QString&)",e_xAver);
    worksheet->querySubObject("Cells(int,int)", 13, 3)->dynamicCall("SetValue(const QString&)",e_yAver);
    worksheet->querySubObject("Cells(int,int)", 13, 4)->dynamicCall("SetValue(const QString&)",e_zAver);
    //deviation
    worksheet->querySubObject("Cells(int,int)", 14, 1)->dynamicCall("SetValue(const QString&)","maxDeviation");
    //SPAT
    worksheet->querySubObject("Cells(int,int)", 15, 1)->dynamicCall("SetValue(const QString&)","2Sspat");
    QString e_dev =QString::number(maxDevia,10,8);
    QString e_spat =QString::number(Sspat,10,8);
    //deviation
    worksheet->querySubObject("Cells(int,int)", 14, 5)->dynamicCall("SetValue(const QString&)", e_dev);
    //SPAT
    worksheet->querySubObject("Cells(int,int)", 15, 2)->dynamicCall("SetValue(const QString&)", e_spat);
}
bool coordinate::saveDataPart(const int rowCount,const int colCount,QAxObject * worksheet){
    getSampleListItem(worksheet,2);
    if( mcoordinateForm->getBoxCurrentText() == "A S M E" ){
        runASME(worksheet);
    }
    return true;
}
