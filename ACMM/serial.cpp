#include "serial.h"
QSerialPort * Serial::serial = nullptr;
Serial::Serial(QObject *parent) : QObject(parent){
    mReadWriteLock = new QReadWriteLock();

}
//串口自动连接接口
bool Serial::autoConnect(){
    mReadWriteLock->lockForWrite();
    QList<QSerialPortInfo> serial_list = QSerialPortInfo::availablePorts();
    //这个函数将返回QList<QSerialPortInfo>类型的内容，返回内容是可用的串口信息
    int serialsize = serial_list.size();//返回内容的长度
    for(int i = 0;i<serialsize;++i){
         QString flag = serial_list.at(i).description();
         //description()的功能：类型转换，将QSerialPortInfo类型转换成QString类型，description()的功能
         if(flag == "USB-SERIAL CH340"){
             comPort = serial_list.at(i).portName();
             qDebug()<<comPort;//输出端口号
         }
    }
    //具体的设置部分
    if(serial != nullptr){
        delete serial;
        serial = nullptr;
    }
    serial = new QSerialPort(this);
    serial->setPortName(comPort);//new0429
    //打开串口
    bool flag = serial->open(QIODevice::ReadWrite);
    if(flag == false){
        mReadWriteLock->unlock();
        return false;
    }
    //设置波特率
    serial->setBaudRate(QSerialPort::Baud115200);
    //设置数据位
    serial->setDataBits(QSerialPort::Data8);
    //设置校验位
    serial->setParity(QSerialPort::NoParity);
    //设置停止位
    serial->setStopBits(QSerialPort::OneStop);
    //设置流量控制
//    serial->setFlowControl(QSerialPort::NoFlowControl);
    mReadWriteLock->unlock();
    return true;
}
//串口断开程序
void Serial::serialClose(){
    mReadWriteLock->lockForWrite();
    if(serial == nullptr){
        mReadWriteLock->unlock();
        return;//如果目前串口已经关闭，不再重复关闭
    }
    else{
        serial->close();//关闭串口
    }
    mReadWriteLock->unlock();
}
void Serial::serialOpen(){
    mReadWriteLock->lockForWrite();
    if(serial != nullptr){
        serial->open(QIODevice::ReadWrite);
    }
    else{
        bool flag = autoConnect();
        if(flag == false){
        }
    }
    mReadWriteLock->unlock();
}
//向下位机发送数据
void Serial::sendDataToSlave(){
    mReadWriteLock->lockForWrite();
    QString data = "B";
    QByteArray sendBuf;
    sendBuf = data.toLatin1();//类型转换，转换为QByteArray类型
    serial->write(sendBuf);
    mReadWriteLock->unlock();
}
//返回QSerialPort
QSerialPort * Serial::getSerialPointer() const{
    return serial;
}
//断开全部的信号和槽
void Serial::disConnect(){
    serial->disconnect();//先断开再连接
}
//读取串口数据
QByteArray Serial::readAllData() const{
    QByteArray result;
    result = serial->readAll();
    return result;
}
