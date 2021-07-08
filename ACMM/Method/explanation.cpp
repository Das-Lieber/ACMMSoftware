#include "explanation.h"

explanation::explanation(QWidget *parent) :
    QDialog(parent)   {
    mCommonData = new commonData(this);//公共数据部分
    mReadWriteLock = new QReadWriteLock();
    printUniqueCode = new QTextEdit(this);//完整的识别符
    communicationBits = new QTextEdit(this);//通讯位数
    dataProcessMethod = new QTextEdit(this);//数据的处理方式
    errorAngleNum = new QTextEdit(this);//转角误差
    assemblyDate = new QTextEdit(this);//装配日期
    sensorSituation = new QTextEdit(this);//传感器的情况
    acmmModel = new QTextEdit(this);//型号
    parameterName = new QTextEdit(this);//偏心和结构参数的名称
    labelUniqueCode = new QLabel("唯一识别符",this);//完整的识别符
    labelcommunicationBits = new QLabel("通讯位数：",this);//通讯位数
    labeldataProcessMethod = new QLabel("数据处理方式：",this);//数据的处理方式
    labelerrorAngleNum = new QLabel("转角误差编号：",this);//转角误差
    labelassemblyDate = new QLabel("装配日期:",this);//装配日期
    labelsensorSituation = new QLabel("传感器情况:",this);//传感器的情况
    labelacmmModel = new QLabel("ACMM型号:",this);;//型号
    labelparameterName = new QLabel("参数名称:",this);;//偏心和结构参数的名称
    OKButton = new QPushButton("确定",this);
    cannelButton = new QPushButton("取消",this);
    gridLayoutEX = new QGridLayout(this);
    connect(cannelButton,&QPushButton::clicked,[=](){
        this->close();
    }
    );
}
explanation::~explanation(){
}
QPushButton * explanation::getOKButtonPointer() const{
    return OKButton;
}
void explanation::closeForm(){
    OKButton->disconnect();
    this->close();
}
void explanation::ExplanPretreatment(){
    mReadWriteLock->lockForRead();
    printUniqueCode->setText( mCommonData->getUniqueCodeItem(0) );
    communicationBits->setText( mCommonData->getUniqueCodeItem(1) );//通讯位数
    sensorSituation->setText( mCommonData->getUniqueCodeItem(2) );//传感器情况
    dataProcessMethod->setText( mCommonData->getUniqueCodeItem(3) );//数据的处理方式
    errorAngleNum->setText( mCommonData->getUniqueCodeItem(4) );//转角误差
    assemblyDate->setText( mCommonData->getUniqueCodeItem(5) );//装配日期
    acmmModel->setText( mCommonData->getUniqueCodeItem(6) );//ACMM型号
    parameterName->setText( mCommonData->getUniqueCodeItem(7) );
    mReadWriteLock->unlock();
}
void explanation::showUi(){
    ExplanPretreatment();
    exUisetMain();
    connect(OKButton,&QPushButton::clicked,[=](){
        this->close();
        emit connectDone();
    });
}
void explanation::exUisetMain(){
    //清空当前页面布局
    qDebug()<<gridLayoutEX->count();
    if(gridLayoutEX->count()!=0){
        QLayoutItem *child;
        while ((child = gridLayoutEX->takeAt(0)) != 0){
            gridLayoutEX->removeWidget(child->widget());
            child->widget()->setParent(0);
            delete child;
        }
        delete gridLayoutEX;
        gridLayoutEX = new QGridLayout(this);
    }
    setWindowTitle("ACMM Explanation");
    setMinimumSize(460,246);
    setMaximumSize(520,300);

    QFont fontExplan;
    fontExplan.setFamily("Microsoft YaHei");
    fontExplan.setBold(false);
    fontExplan.setPixelSize(12);

    printUniqueCode->setFont(fontExplan);
    fontExplan.setPixelSize(16);

    communicationBits->setFont(fontExplan);
    dataProcessMethod->setFont(fontExplan);
    errorAngleNum->setFont(fontExplan);
    assemblyDate->setFont(fontExplan);
    sensorSituation->setFont(fontExplan);
    acmmModel->setFont(fontExplan);
    parameterName->setFont(fontExplan);

    labelUniqueCode->setFont(fontExplan);
    labelcommunicationBits->setFont(fontExplan);
    labeldataProcessMethod->setFont(fontExplan);
    labelerrorAngleNum->setFont(fontExplan);
    labelassemblyDate->setFont(fontExplan);
    labelsensorSituation->setFont(fontExplan);
    labelacmmModel->setFont(fontExplan);
    labelparameterName->setFont(fontExplan);

    printUniqueCode->setFixedSize(400,30);
    communicationBits->setFixedSize(400,30);
    dataProcessMethod->setFixedSize(400,30);
    errorAngleNum->setFixedSize(400,30);
    assemblyDate->setFixedSize(400,30);
    sensorSituation->setFixedSize(400,30);
    acmmModel->setFixedSize(400,30);
    parameterName->setFixedSize(400,30);

    printUniqueCode->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    communicationBits->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    dataProcessMethod->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    errorAngleNum->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    assemblyDate->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    sensorSituation->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    acmmModel->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    parameterName->setStyleSheet("background:transparent;border-width:0;border-style:outset");

    gridLayoutEX->addWidget(labelUniqueCode,0,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(printUniqueCode,0,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);

    gridLayoutEX->addWidget(labelsensorSituation,1,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(sensorSituation,1,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);

    gridLayoutEX->addWidget(labelcommunicationBits,2,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(communicationBits,2,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);


    gridLayoutEX->addWidget(labeldataProcessMethod,3,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(dataProcessMethod,3,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);

    gridLayoutEX->addWidget(labelerrorAngleNum,4,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(errorAngleNum,4,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);

    gridLayoutEX->addWidget(labelassemblyDate,5,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(assemblyDate,5,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);

    gridLayoutEX->addWidget(labelacmmModel,6,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(acmmModel,6,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);

    gridLayoutEX->addWidget(labelparameterName,7,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(parameterName,7,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);

    gridLayoutEX->addWidget(OKButton,8,0,1,1,Qt::AlignHCenter|Qt::AlignCenter);
    gridLayoutEX->addWidget(cannelButton,8,1,1,1,Qt::AlignHCenter|Qt::AlignCenter);

    setLayout(gridLayoutEX);
    show();
}
void explanation::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);
    QPainter p(this);//指定对象为窗口
    p.drawPixmap(0,0,width(),height(),QPixmap(":/image/uk.png"));
}
void explanation::closeEvent(QCloseEvent * event){
//    int ret = QMessageBox::question(this,"关闭","是否关闭窗口？",QMessageBox::Yes,QMessageBox::No);
//    if(ret == QMessageBox::Yes){
//        event->accept();
//        OKButton->disconnect();
//    }
//    else if(ret == QMessageBox::No){
//        event->ignore();
//    }
    event->accept();
    OKButton->disconnect();
}
