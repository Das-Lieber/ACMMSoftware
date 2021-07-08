#ifndef IMPORTSTRUCTFILE_H
#define IMPORTSTRUCTFILE_H

#include <QWidget>
#include<QDebug>
#include"fileInOut/importfile.h"

class importStructFile : public importFile{
    Q_OBJECT
public:
    explicit importStructFile(QWidget *parent = nullptr);
    virtual ~importStructFile();
private:
    virtual bool importDataPart(int rowBegin,int rowEnd,int colBegin,int colEnd,QAxObject * excel);
signals:

public slots:
};
#endif // IMPORTSTRUCTFILE_H
