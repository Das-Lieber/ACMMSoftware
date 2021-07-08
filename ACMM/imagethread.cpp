#include "imagethread.h"

imageThread::imageThread(QObject *parent) : QObject(parent){
    x = 0,y = 0;
    paintfactorx = 0;
    paintfactory = 0;
    exAngle[commonData::SizeOfAngle] = {0};
    updateAngle[commonData::SizeOfAngle] = {0};
    mCommonData = new commonData();
    mReadWriteLock = new QReadWriteLock();
    myMutex = new QMutex();
}
void imageThread::setPaintFactor(const double x,const double y){
    paintfactorx = x;
    paintfactory = y;
}
void imageThread::drawImage(){
    myMutex->lock();
//    定义QImage绘图设备
    int truesize = (int)(320*10);
    int truesizeh = (int)(560*10);
    QImage image(truesize,truesizeh,QImage::Format_ARGB32);//画幅大小
    //定义画家，指定绘图设备
    QPainter p(&image);

    //定义画笔对象
    QPen pen;
    pen.setWidth(5);
    //把画笔交给画家
    p.setPen(pen);

    int fe = (int)(58*paintfactorx);
    int osf = (int)(165*paintfactory);//

    int pne = (int)(208*paintfactorx);
    int ttz = (int)(320*paintfactory);//

    int fsf = (int)(475*paintfactory);//
    int of = (int)(15*paintfactory);//

    p.drawText(fe,osf,"编码器1");
    p.drawText(pne,osf,"编码器2");
    p.drawText(fe,ttz,"编码器3");
    p.drawText(pne,ttz,"编码器4");
    p.drawText(fe,fsf,"编码器5");
    p.drawText(pne,fsf,"编码器6");
    p.drawText(0,of,"编码器显示区域");


    //定义画刷
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::white);
    //把画刷交给画家
    p.setBrush(brush);

    // 设置画笔颜色、宽度,RGB,画笔的宽度
    // 绘制椭圆
    int trueR,trueRdyn;
    if(paintfactorx<1){
        trueR = 60;
        trueRdyn = 52;
    }
    else{
        trueR = (int)(60*paintfactorx);
        trueRdyn = (int)(52*paintfactorx);
    }
    int sf = (int)(75*paintfactorx);
    int tof = (int)(225*paintfactorx);

    int ef = (int)(85*paintfactory);
    int tfz = (int)(240*paintfactory);

    int tnf = (int)(395*paintfactory);

    setDrawEllipse(p,sf,ef,trueR,0);
    setDrawEllipse(p,tof,ef,trueR,1);
    setDrawEllipse(p,sf,tfz,trueR,2);
    setDrawEllipse(p,tof,tfz,trueR,3);
    setDrawEllipse(p,sf,tnf,trueR,4);
    setDrawEllipse(p,tof,tnf,trueR,5);

    /****************************画实线********************************/
    QPen constpen;
    constpen.setWidth(1.5);
    constpen.setColor(QColor(52,96,239));//画实线
    p.setPen(constpen);

    for(int i = 0;i<mCommonData->getAxisSampleListSize();++i){
        //第一个圆
        x=trueRdyn*sin( mCommonData->getAxisSampleListRadian(1,i) )+sf;//sin（角度值）弧度制
        y=trueRdyn*cos( mCommonData->getAxisSampleListRadian(1,i) )+ef;
        p.drawLine(sf,ef,x,y);
        //第二个圆
        x=trueRdyn*sin( mCommonData->getAxisSampleListRadian(2,i) )+tof;
        y=trueRdyn*cos( mCommonData->getAxisSampleListRadian(2,i) )+ef;
        p.drawLine(tof,ef,x,y);
        //第三个圆
        x=trueRdyn*sin( mCommonData->getAxisSampleListRadian(3,i) )+sf;
        y=trueRdyn*cos( mCommonData->getAxisSampleListRadian(3,i) )+tfz;
        p.drawLine(sf,tfz,x,y);
        //第四个圆
        x=trueRdyn*sin( mCommonData->getAxisSampleListRadian(4,i) )+tof;
        y=trueRdyn*cos( mCommonData->getAxisSampleListRadian(4,i) )+tfz;
        p.drawLine(tof,tfz,x,y);
        //第五个
        x=trueRdyn*sin( mCommonData->getAxisSampleListRadian(5,i) )+sf;
        y=trueRdyn*cos( mCommonData->getAxisSampleListRadian(5,i) )+tnf;
        p.drawLine(sf,tnf,x,y);
        //第六个
        x=trueRdyn*sin( mCommonData->getAxisSampleListRadian(6,i) )+tof;
        y=trueRdyn*cos( mCommonData->getAxisSampleListRadian(6,i) )+tnf;
        p.drawLine(tof,tnf,x,y);
    }
    /***************************动态绘图*********************************/
    //画第一个圆
    drawDynamicPart(p,sf,ef,trueRdyn,0);
    //画第二个圆
    drawDynamicPart(p,tof,ef,trueRdyn,1);
    //画第3个圆
    drawDynamicPart(p,sf,tfz,trueRdyn,2);
    //画第4个圆
    drawDynamicPart(p,tof,tfz,trueRdyn,3);
    //画第5个圆
    drawDynamicPart(p,sf,tnf,trueRdyn,4);
    //画第6个圆
    drawDynamicPart(p,tof,tnf,trueRdyn,5);
    emit SignalUpdateImage(image);
    myMutex->unlock();
}
void imageThread::setDrawEllipse(QPainter & p,int xpos,int ypos,int R,int axisNum){
    //编码器1~6
    QPen ypen;
    ypen.setWidth(6);
    if(mCommonData->getAxisCrossZero(axisNum) == false){
        ypen.setColor(QColor(236, 89, 141));
    }
    else {
        ypen.setColor(QColor(0, 160, 230));
    }
    p.setPen(ypen);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawEllipse(QPointF(xpos, ypos),R,R);
}
void imageThread::drawDynamicPart(QPainter & p,int xpos,int ypos,int R,int axisNum){
    updateAngle[axisNum] = mCommonData->getImageAxisAngle(axisNum);//每次都接受最新的数据
    double exXAngle = 0,updateYAngel = 0;
    exXAngle = exAngle[axisNum]/180*mCommonData->getPI();
    updateYAngel = updateAngle[axisNum]/180*mCommonData->getPI();
    if(int( exAngle[axisNum] ) == int( updateAngle[axisNum] )){
        QPen datapen;
        datapen.setWidth(3);
        datapen.setColor(QColor(228,107,45));
        p.setPen(datapen);
        x=R*sin( exXAngle )+xpos;
        y=R*cos( exXAngle )+ypos;
    }
    else if( int( exAngle[axisNum] ) != int( updateAngle[axisNum] )){
        /****遮盖****/
        QPen whitepen;
        whitepen.setWidth(3);
        whitepen.setColor(QColor(255,255,255));//变白
        p.setPen(whitepen);
        x=R*sin( exXAngle )+xpos;
        y=R*cos( exXAngle )+ypos;
        p.drawLine(xpos,ypos,x,y);
        /****遮盖****/
        /*****更新***/
        QPen freshpen;
        freshpen.setWidth(3);
        freshpen.setColor(QColor(228,107,45));
        p.setPen(freshpen);
        x=R*sin( updateYAngel )+xpos;
        y=R*cos( updateYAngel )+ypos;
        /*****更新***/
        exAngle[axisNum] = updateAngle[axisNum];
    }
    p.drawLine(xpos,ypos,x,y);
}
