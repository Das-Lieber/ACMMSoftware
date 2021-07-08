#ifndef SERIAL_H
#define SERIAL_H
// 单纯的串口连接和断开操作
#include <QObject>
#include<QtSerialPort/QSerialPort>
#include<QtSerialPort/QSerialPortInfo>
#include<QList>
#include<QMessageBox>
#include<QDebug>
#include<QReadWriteLock>

class Serial : public QObject
{
    Q_OBJECT
public:
    explicit Serial(QObject *parent = nullptr);
    //串口自动连接接口
    bool autoConnect();
    //串口断开程序
    void serialClose();
    void serialOpen();
    //向下位机发送数据
    void sendDataToSlave();
    //断开全部连接
    void disConnect();
    QString comPort;//端口号
    QSerialPort * getSerialPointer() const;
    QByteArray readAllData() const;
signals:

public slots:
private:
    QReadWriteLock * mReadWriteLock;
    static QSerialPort * serial;
};

#endif // SERIAL_H
