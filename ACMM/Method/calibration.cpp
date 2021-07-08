#include "calibration.h"
QString calibration::hHeaderItem[7];
calibration::calibration(QWidget *parent) :QDialog(parent){
    mCalibForm = new calibrationForm(this);
    mCommonData = new commonData(this);
    table_widget_menu = new QMenu(this);
    actdelete = new QAction("删除",this);
    actclear = new QAction("清空",this);
    actSave = new QAction("保存",this);
    actUnit = new QAction("单位",this);
    mReadWriteLock = new QReadWriteLock();
    mSaveFile = nullptr;
    initialForm();
}
calibration::~calibration(){
}
void calibration::getInfo(){
    //这个部分和下位机发送的位数没有任何关系 因为decode已经把下位机发送的数据都处理了，这部分只是显示
    frontTableShow();//将数据传输给前界面和绘图部分
    mReadWriteLock->lockForWrite();
    if(mCalibForm->getBoxCurrentText() == "单次模式"){
        //单次触发
//        qDebug()<<"单次模式";
        if( mCommonData->getRecordFlag() == true ){
            QApplication::beep();
            tableOperation();//前界面TableWidget
        }
    }
    else if(mCalibForm->getBoxCurrentText() == "连续模式"){
        //连续触发
//        qDebug()<<"连续模式";
        if( mCommonData->getRecordFlag() == true ){
            QApplication::beep();
            if(boolPushBottonControl == false){
                boolPushBottonControl = true;
            }
            else if(boolPushBottonControl == true){
                boolPushBottonControl = false;
            }
        }
        if(boolPushBottonControl == true){
            tableOperation();//前界面TableWidget
        }
    }
    emit SingalsToBeginDraw();
    mReadWriteLock->unlock();
}
/*************************************初始化及准备阶段********************************************/
void calibration::initialForm(){
    rowNumber = 0;
    playNumber = 0;
    unitFlag = false;
    progressIndex = nullptr;
    boolPushBottonControl = false;
    menuForTable();
    initialImageThread();
}
void calibration::initialImageThread(){
    imT = new imageThread;//分配空间
    mImageThread = new QThread(this);//指定父对象，子线程
    imT->moveToThread(mImageThread);//自定义线程加入子线程
    //启动image子线程
    mImageThread->start();//一开始就启动,执行等级更高
    connect(imT,&imageThread::SignalUpdateImage,this,&calibration::getImage);//一旦画图线程被调用，就会把画好的内容反馈到传输到getimage函数
    connect(this,&calibration::SingalsToBeginDraw,imT,&imageThread::drawImage);//发送SingalsToBeginDraw，就会启动线程中的drawimage函数，进行画图
}
void calibration::menuForTable(){
    if(table_widget_menu == nullptr){
        return;
    }
    //右键设置
    if(actdelete){
        connect(actdelete,&QAction::triggered,this,&calibration::deleteData);//删除
        table_widget_menu->addAction(actdelete);
    }
    if(actclear){
        connect(actclear,&QAction::triggered,this,&calibration::clearTable);//清空
        table_widget_menu->addAction(actclear);
    }
    if(actSave){
        connect(actSave,&QAction::triggered,this,&calibration::saveData);//save
        table_widget_menu->addAction(actSave);
    }
    if(actUnit){
        connect(actUnit,&QAction::triggered,this,&calibration::changeUnit);//单位选择
        table_widget_menu->addAction(actUnit);
    }
    //信号和槽 右键的设置
    connect(mCalibForm->getTableObject(),&QTableWidget::customContextMenuRequested,this,&calibration::showMenu);
}
void calibration::showMenu(const QPoint pt){
    //右键的设置
    QPoint point = pt; //得到窗口坐标
    QTableWidgetItem *item = mCalibForm->getItemAt(point);
    if(item != nullptr){
        table_widget_menu->clear(); //清除原有菜单
        table_widget_menu->addAction(actclear);
        table_widget_menu->addAction(actdelete);
        table_widget_menu->addAction(actSave);
        table_widget_menu->addAction(actUnit);
        //菜单出现的位置为当前鼠标的位置
        table_widget_menu->exec(QCursor::pos());
    }
}
void calibration::changeUnit(){
    QApplication::beep();
    if(unitFlag){
        unitFlag = false;
    }
    else{
        unitFlag = true;
    }
}
//预处理部分 UI的显示 数据库的创建
/*************************************calibration预处理及主函数********************************************/
void calibration::pretreatment(){
    if( mCalibForm == nullptr) {
        return;
    }
    mCalibForm->SetCalibrationForm(this);
    imageUiSet();
    emit SingalsToBeginDraw();//绘图
    this->show();
}
void calibration::imageUiSet(){
    //UIPArt
    m_factorX = mCalibForm->getXFactor();
    m_factorY = mCalibForm->getYFactor();
    imT->setPaintFactor(mCalibForm->getXFactor(),mCalibForm->getYFactor());
}
/*************************************tableWidget部分********************************************/
void calibration::tableOperation(){
    //前界面表格的具体内容
    double justforangle[6];
    QString stringData[6];//显示数据转换
    //画图用这个数据
    for(int numj = 0;numj<6;numj++){
        //直接输出角度值，没有经过计算angle 0,1
        //做个标志位
        if(unitFlag == false){
            justforangle[numj] = mCommonData->getAxisAngleRadian(numj);
        }
        else if(unitFlag == true){
            justforangle[numj] = mCommonData->getAxisAngle(numj);
        }
        //显示精度问题
        stringData[numj] = QString::number(justforangle[numj],10,6);
    }
    //主窗口包含子窗口的头文件，所以,直接把子窗口的数据赋值给主窗口的数组中
    mCalibForm->setTableRowCount(rowNumber + 1);
    QString num_str=QString("%1").arg(rowNumber+1);
    mCalibForm->setTableItem(playNumber,0,new QTableWidgetItem(num_str));
    for(int col = 1;col<7;col++){
        mCalibForm->setTableItem(playNumber,col,new QTableWidgetItem(stringData[col-1]));
    }
    rowNumber++;
    playNumber++;
    //显示最后一行
    mCalibForm->setScrollToBottom();
    mCalibForm->setNumTextItem( QString("%1").arg(playNumber) );
    /*************数据库调用*************/
    //摁下之后会进行操作 这些数据都是要被保存下来的
    for(int i = 0,pos = 1;i<6;++i,++pos){
        mCommonData->setAxisSampleList(pos,mCommonData->getAxisAngle(i));
    }
}
void calibration::deleteData(){
    //删除某行
    int rowIndex = mCalibForm->getTableCurrentRow();
    if(rowIndex == -1){
        return;
    }
    mCalibForm->removeTableRow(rowIndex);
    playNumber--;
    rowNumber--;
    mReadWriteLock->lockForWrite();
    if( mCommonData->isAxisSampleListEmpty() == false && rowIndex >= 0 ){
        for(int i = 1;i<6;++i){
            mCommonData->ereaseAxisSampleList(i,rowIndex);
        }
    }
    mReadWriteLock->unlock();
}
void calibration::clearTable(){
    mReadWriteLock->lockForWrite();
    //表格全部清空
    rowNumber = 0;
    playNumber = 0;
    mCalibForm->setNumTextItem("0");
    mCalibForm->clearTableItem();
    mCalibForm->setTableRowCount(0);
    mCommonData->clearAllAxisSampleList();
    mReadWriteLock->unlock();
}
void calibration::frontTableShow(){
    //动态绘图部分
    for(int i = 0;i<6;++i){
        mCommonData->setImageAxisAngle(i,mCommonData->getAxisAngle(i));
    }
//    实时显示坐标值部分
    mCalibForm->setXTextItem(mCommonData->getCoordinateItemStr(0,3));
    mCalibForm->setYTextItem(mCommonData->getCoordinateItemStr(1,3));
    mCalibForm->setZTextItem(mCommonData->getCoordinateItemStr(2,3));
}
void calibration::getImage(QImage temp){
    image = temp;
    //更新paintEvent事件
    update();
}
/*************************************事件部分********************************************/
void calibration::closeEvent(QCloseEvent *event){
    int ret = QMessageBox::question(this,"退出","是否确定退出？",QMessageBox::Yes|QMessageBox::No);
    if(ret == QMessageBox::Yes){
        Decode::removeAll();//从通知队列中移除全部内容（只针对一个观察者）
        clearTable();
        emit readyClose();
        event->accept();
    }
    else if(ret == QMessageBox::No){
        event->ignore();
    }
}
//画图事件
void calibration::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);
    QPainter p(this);//指定对象为窗口
    p.drawPixmap(0,0,width(),height(),QPixmap("://image/uk.png"));
    int truedrawf = (int)(1015*m_factorX);
    int truedraws = (int)(120*m_factorY);
    p.drawImage(truedrawf,truedraws,image);//起点坐标，之后是图片
}
/*************************************进度条部分********************************************/
void calibration::progressMain(const int uoLimit){
    //进度条
    pgNumb=0;//进度条初始化
    if(progressIndex != nullptr){
        delete progressIndex;
        progressIndex = nullptr;
    }
    progressIndex = new QProgressDialog(this);
    progressIndex->setMinimum(0);//最小值
    progressIndex->setMaximum(uoLimit);//最大值
    progressIndex->reset();//让进度条重新回到开始
    progressIndex->setWindowModality(Qt::WindowModal);
    progressIndex->setMinimumDuration(5);
    progressIndex->setWindowTitle(tr("plese waiting"));
    progressIndex->setLabelText("Copying...");
    progressIndex->setCancelButtonText(tr("Cancel"));
    progressIndex->setRange(0,uoLimit);//new0428
    progressIndex->autoReset();//自动关闭

//    QRect rect = this->geometry();
//    //计算显示原点
//    int x = rect.x() + rect.width()/2 - this->width() /2;
//    int y = rect.y() + rect.height()/2 - this->height()/2;
//    progressIndex->move(x, y);
}
void calibration::progressValue(){
    //进度条赋值
    pgNumb++;
    progressIndex->setValue(pgNumb);
}
/*************************************数据保存部分********************************************/
void calibration::saveData(){
    if(mSaveFile == nullptr){
        mSaveFile = new calibration(this);
    }
    int rowCount = mCalibForm->getTableRowCount();
    int colCount = mCalibForm->getTableColumnCount();
    bool result = mSaveFile->saveFileMain(rowCount,colCount,-1,1);
    if(result == true){
        QMessageBox::about(nullptr,tr("save"),tr("导出成功"));
    }
    else{
        QMessageBox::about(nullptr,tr("save"),tr("导出失败"));
    }
}
QString * calibration::getSaveExcelHeader(){
    hHeaderItem[0] = "序号";
    QString headerName = "编码器";
    for(int i=1;i<7;i++){
        //从CommonData中直接读取
//        hHeaderItem[i] = QString("%1%2").arg(headerName).arg(i); //读取表头
        //直接从前界面直接读取
        hHeaderItem[i] = mCalibForm->getHorizontalHeaderItem(i);
    }
    return hHeaderItem;
}
QString calibration::getTableItemStr(const int row,const int col){
//    return mCalibForm->getTableItem(i,j);
    //方案二，直接从list里面读取数据 getAxisSampleList(轴号（列号就是col,固定的col+1），位置(行号))
    double result = mCommonData->getAxisSampleListRadian(col,row);
    return QString::number(result,10,6);
}
void calibration::assigAbstractObject(calibration *arg){
    if(mSaveFile == nullptr){
        mSaveFile = arg;
    }
}
bool calibration::saveDataPart(const int rowCount,const int colCount,QAxObject * worksheet){
    //数据区
    if(rowCount == 0||colCount == 0){
        return false;
    }
    progressMain(rowCount);
    QString data;
//    mReadWriteLock->lockForRead();
    for(int row = 0;row<rowCount;row++){
        for(int col = 0;col<colCount;col++){
            //从CommonData中直接读取  （注意死锁问题）
//            qDebug()<<row<<col;
            //换个形式 直接从前界面读取
//            if(col == 0){
//                data = QString::number(row+1,10);
//            }
//            else{
//                data = getTableItemStr(row,col);
//            }
            //直接从前界面直接读取
            data = mCalibForm->getTableItem(row,col);
            worksheet->querySubObject("Cells(int,int)", row+2, col+1)->dynamicCall("SetValue(const QString&)",data);
        }
        //进度条
        progressValue();
    }
//    mReadWriteLock->unlock();
    return true;
}
/*************************************进程调度部分********************************************/
void calibration::dropImageThread(){
    if(mImageThread->isRunning() == true){
        mImageThread->quit();
        mImageThread->wait();
    }
}
