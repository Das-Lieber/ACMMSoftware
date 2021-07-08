#ifndef SAVEFILE_H
#define SAVEFILE_H
#include<QString>
#include<QAxObject>
#include<QFileDialog>
#include<QSettings>
#include<QDebug>

class saveFile {
public:
    saveFile();
    virtual ~saveFile();
    bool saveFileMain(const int rowCount,const int colCount,int rangeLine,int headerBegin,QWidget * parent = nullptr);
protected:
//    static saveFile * mSaveFile;
    virtual bool saveDataPart(const int rowCount,const int colCount,QAxObject * worksheet) = 0;
    virtual QString * getSaveExcelHeader() = 0;
private:
    //保存数据部分
    QString getSavePath(QWidget * parent = nullptr);
    bool saveDataToExcel(const QString & path,const int rowCount,const int colCount,int rangeLine,int headerBegin,QWidget * parent = nullptr);

};

#endif // SAVEFILE_H
