#ifndef UNIQUEID_H
#define UNIQUEID_H

#include <QWidget>
#include<QMessageBox>
#include<QDebug>
#include<QReadWriteLock>
#include"serial.h"
#include"commondata.h"
#include"Method/explanation.h"
#include"fileInOut/importfile.h"
#include"fileInOut/importcoefile.h"
#include"fileInOut/importstructfile.h"
#include"Decode/decode.h"
#include"Decode/fiftydecode.h"
#include"Decode/twentysixdecode.h"
class uniqueID : public QWidget
{
    Q_OBJECT
public:
    explicit uniqueID(QWidget *parent = nullptr);
    ~uniqueID();
    void uniqueIDMain();
    void manulConnect();
private:
    Serial * mSerial;
    QReadWriteLock * mReadWriteLock;
    commonData * mCommonData;
    explanation * mExplanation;
    importFile * mImportFile;

    Decode * mDecode;

    void autoInsertParameter();
    void decodeUniqueCode();
    void autoInsertStructPara();
    void autoInsertCoeffPara();
    void connectPart();
signals:
    void formButtonEnable();
    void connectDone();
public slots:
};

#endif // UNIQUEID_H
