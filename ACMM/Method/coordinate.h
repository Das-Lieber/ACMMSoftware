#ifndef COORDINATE_H
#define COORDINATE_H

#include <QDialog>
#include <QEvent>
#include <QPaintEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QAxObject>
#include <QApplication>
#include <QFileDialog>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QReadWriteLock>
#include "UI/coordinateform.h"
#include "Method/method.h"
#include "commondata.h"
#include "Decode/decode.h"
#include "fileInOut/savefile.h"
class coordinate : public QDialog,public Method,public saveFile{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *event);
public:
    explicit coordinate(QWidget *parent = nullptr);
    ~coordinate();
    virtual void pretreatment();
    virtual void assigAbstractObject(coordinate *arg);
signals:
    void readyClose();

public slots:
private slots:
    void clearData();
    void saveData();
private:
    coordinateForm * mcoordinateForm;
    QTcpServer * tcpserver;
    QTcpSocket * tcpsocker;
    QReadWriteLock * mReadWriteLock;
    commonData * mCommonData;
    saveFile * mSaveFile;

    void initialForm();
    void tcpMain();
    void stringToHtml(QString str,QColor crl);
    void ASME();
    void runASME(QAxObject * worksheet);
    virtual void getInfo();
    void TCPConnect();
    void updataCoordinate();
    void updataCoordinateList();
    void updateSamplePart();
    void updateFromText();
    void getSampleListItem(QAxObject * worksheet,int arg);

    QString * getSaveExcelHeader();
    bool saveDataPart(const int rowCount,const int colCount,QAxObject * worksheet);

    bool boolPushBottonControl;
    QList<double> sampleDataList;
    int numForSample;
    bool boolQMess;
    double deviation[10];//偏差
    double Sspat;//二倍标准差
    double maxDevia;
    double excelSTDEV[3];
    double xAver,yAver,zAver;
    static QString hHeaderItem[3];
};

#endif // COORDINATE_H
