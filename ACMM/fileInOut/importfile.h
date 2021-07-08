#ifndef IMPORTFILE_H
#define IMPORTFILE_H

#include <QWidget>
#include<QDebug>
#include<QSettings>
#include<QApplication>
#include<QReadWriteLock>
#include<QFileDialog>
#include<QAxObject>
#include "commondata.h"

class importFile : public QWidget{
public:
    explicit importFile(QWidget *parent = nullptr);
    virtual ~importFile();
    virtual bool importFileMain(const bool controlFlag,const QString type);
protected:
    QReadWriteLock * mReadWriteLock;
    commonData * mCommonData;
    virtual QString getImportPath(QWidget *parent);
    virtual QString AutoImportFile(const QString type) const;
    //目前来看，偏心和结构参数此部分不一样
    virtual bool importDataPart(int rowBegin,int rowEnd,int colBegin,int colEnd,QAxObject * excel) = 0;
    virtual bool importDataFromExcel(const QString & path);
signals:

public slots:
};

#endif // IMPORTFILE_H
