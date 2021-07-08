#ifndef IMPORTCOEFFFILE_H
#define IMPORTCOEFFFILE_H

#include <QWidget>
#include<QDebug>
#include"fileInOut/importfile.h"

class importCoeffFile : public importFile{
    Q_OBJECT
public:
    explicit importCoeffFile(QWidget *parent = nullptr);
    virtual ~importCoeffFile();
private:
    virtual bool importDataPart(int rowBegin,int rowEnd,int colBegin,int colEnd,QAxObject * excel);
signals:

public slots:
};

#endif // IMPORTCOEFFFILE_H
