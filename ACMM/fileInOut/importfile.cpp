#include "importfile.h"

importFile::importFile(QWidget *parent) : QWidget(parent){
    mReadWriteLock = new QReadWriteLock();
    mCommonData = new commonData(this);
}
importFile::~importFile(){

}
/*************************************导入部分********************************************/
bool importFile::importFileMain(const bool controlFlag,const QString type){
    QString path;
    if(controlFlag == true){
        path = AutoImportFile(type);
    }
    else if(controlFlag == false){
        path = getImportPath(this);
    }
    qDebug()<<path;
    if(path.isEmpty() == false){
        return importDataFromExcel(path);
    }
    else{
        return false;
    }
}
QString importFile::getImportPath(QWidget *parent){
    //手动导入时，需要记忆路径
    QSettings opensetting("./Setting.ini", QSettings::IniFormat);
    QString openpath = opensetting.value("LastFilePath").toString();
    QString path = QFileDialog::getOpenFileName(parent,"open",openpath,"execl(*.xlsx *.xls)");
    return path;
}
QString importFile::AutoImportFile(const QString type) const{
    //自动导入数据，路径从txt文件中读取,如果是自动读取，一定不用断开串口，因为还没有连接
    //打开txt文件，txt文件中有结构参数的路径
//    QString path;
//    QString Path_P1 = qApp->applicationDirPath()+"/data";
//    mReadWriteLock->lockForRead();
//    QString NameForFile = QString("%1%2%3%4").arg("/").arg(type).arg( mCommonData->getUniqueCodeItem(7) ).arg(".txt");
//    mReadWriteLock->unlock();
//    QString pathForTxt = QString("%1%2").arg(Path_P1).arg(NameForFile);
//    if (pathForTxt.isEmpty()){
//        return "";
//    }
//    QFile file(pathForTxt);
//    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
//        while (!file.atEnd()){
//            QByteArray line = file.readLine();
//            path = line.data();//QByteArray和QString（char *）的转换
//        }
//        file.close();
//    }
//    else{
//        return "";
//    }
//    return path;

    QString Path_P1 = qApp->applicationDirPath()+"/data";
    mReadWriteLock->lockForRead();
    QString NameForFile = QString("%1%2%3%4%5").arg("/").arg( mCommonData->getUniqueCodeItem(7) ).arg("/").arg(type).arg(".xlsx");
    mReadWriteLock->unlock();
    QString pathForExcel = QString("%1%2").arg(Path_P1).arg(NameForFile);
    if (pathForExcel.isEmpty()){
        return "";
    }
    else
        return pathForExcel;
}
bool importFile::importDataFromExcel(const QString & path){
    //文件对象
    QFile file(path);
    //打开文件,默认为utf8变量，
    bool flag = file.open(QIODevice::ReadOnly);
    if(flag == true){
        QAxObject *excel = new QAxObject(this);//建立excel操作对象
        excel->setControl("Excel.Application");//连接Excel控件
        excel->setProperty("Visible", false);//不显示窗体看效果
        excel->setProperty("DisplayAlerts", false);//不显示警告看效果
        /*********获取COM文件的一种方式************/
        QAxObject *workbooks = excel->querySubObject("WorkBooks");
        //获取工作簿(excel文件)集合
        workbooks->dynamicCall("Open(const QString&)", path);//path至关重要，获取excel文件的路径
        //打开一个excel文件
        QAxObject *workbook = excel->querySubObject("ActiveWorkBook");
        QAxObject *worksheet = workbook->querySubObject("WorkSheets(int)",1);//访问excel中的工作表中第一个单元格
        QAxObject *usedRange = worksheet->querySubObject("UsedRange");//sheet的范围
        /*********获取COM文件的一种方式************/
        //获取打开excel的起始行数和列数和总共的行数和列数
        int intRowStart = usedRange->property("Row").toInt();//起始行数
        int intColStart = usedRange->property("Column").toInt(); //起始列数
        QAxObject *rows, *columns;
        rows = usedRange->querySubObject("Rows");//
        columns = usedRange->querySubObject("Columns");//
        int intRow = rows->property("Count").toInt();//
        int intCol = columns->property("Count").toInt();//
//        qDebug()<<"intRowStart"<<intRowStart
//               <<"intRowStart + intRow"<<intRowStart + intRow
//              <<"intColStart"<<intColStart
//             <<"intColStart + intCol"<<intColStart + intCol;
        // 变化的部分
        bool importResult = importDataPart(intRowStart,intRowStart + intRow,intColStart,intColStart + intCol,excel);
        if(importResult == false){
            return false;
        }
        // 变化的部分
        workbook->dynamicCall( "Close(Boolean)", false );
        excel->dynamicCall( "Quit(void)" );
        delete excel;
        file.close();
        return true;
    }
    else{
        file.close();
        return false;
    }
}
