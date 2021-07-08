#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QDialog>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QTableWidget>
#include <QFileDialog>

#include <QImage>
#include <QPainter>
#include <math.h>
#include <QProgressDialog>
#include <QThread>
#include <QAxObject>
#include <QList>
#include <QHeaderView>

#include<QMenu>
#include<QReadWriteLock>
#include<QMutex>
#include<QMessageBox>
#include<QSettings>
//UI
#include <QTableWidget>
#include <QTextEdit>
#include "UI/calibrationform.h"
#include "Decode/decode.h"
#include "commondata.h"
#include "Method/method.h"
#include "fileInOut/savefile.h"
#include "imagethread.h"

namespace Ui {
class calibration;
}
//继承一个基类QDialog，多个接口Method & saveFile
class calibration : public QDialog,public Method,public saveFile{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
public:
    explicit calibration(QWidget *parent = 0);
    ~calibration();
    //Variable
    void dropImageThread();
    void calibrationMain();
    virtual void pretreatment();
    virtual void assigAbstractObject(calibration *arg);
private:
    QReadWriteLock * mReadWriteLock;
    calibrationForm * mCalibForm;
    commonData * mCommonData;
    saveFile * mSaveFile;

    void menuForTable();
    //保存
    QString saveFilePath;
    QString saveUsedPath;
    //进度条
    int pgNumb;//进度条读书
    QProgressDialog *progressIndex;
    void progressMain(const int uoLimit);
    void progressValue();//进度条赋值程序
    int rowNumber;
    int playNumber;
    QMenu *table_widget_menu;
    QAction *actdelete;
    QAction *actclear;
    QAction *actSave;
    QAction *actUnit;

    //缩放因子
    double m_factorX;
    double m_factorY;

    void initialForm();

    //绘图
    imageThread * imT;
    QImage image;
    QThread *mImageThread;
    void getImage(QImage);
    void frontTableShow();//从con读取到ima
    void tableOperation();

    bool boolPushBottonControl;
    virtual void getInfo();

    bool saveDataPart(const int rowCount,const int colCount,QAxObject * worksheet);

    static QString hHeaderItem[7];

    bool unitFlag;//true为角度值 false为弧度制
    QString * getSaveExcelHeader();
    QString getTableItemStr(const int i,const int j);

    void initialImageThread();
    void imageUiSet();
private slots:
    void showMenu(const QPoint pt);
    void deleteData();
    void clearTable();
    void saveData();
    void changeUnit();
signals:
    void closeSerialConectFC();
    void SingalsToBeginDraw();
    void readyClose();
};

#endif // CALIBRATION_H
