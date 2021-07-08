#include "savefile.h"
//saveFile * saveFile::mSaveFile = nullptr;
saveFile::saveFile(){
}
saveFile::~saveFile(){

}
/*************************************导出部分********************************************/
bool saveFile::saveFileMain(const int rowCount,const int colCount,int rangeLine,int headerBegin,QWidget * parent){
    QString path = getSavePath(parent);
    if(path.isEmpty() == false){
        return saveDataToExcel(path,rowCount,colCount,rangeLine,headerBegin,parent);
    }
    else {
        return false;
    }
}
QString saveFile::getSavePath(QWidget * parent){
    //手动导入时，需要记忆路径
    QSettings savesetting("./Setting.ini", QSettings::IniFormat);
    QString savepath = savesetting.value("LastFilePath").toString();
    QString path = QFileDialog::getSaveFileName(parent, "保存",savepath,"Excel 文件(*.xls *.xlsx)");
    return path;
}
bool saveFile::saveDataToExcel(const QString & path,const int rowCount,const int colCount,int rangeLine,int headerBegin,QWidget * parent){
    //返回应写入文件类型的目录,返回包含用户文档文件的目录
    if (path.isEmpty()==false){
        QAxObject *excel = new QAxObject();
        //建立excel操作对象
        if (excel->setControl("Excel.Application")) {
            //连接Excel控件
            excel->dynamicCall("SetVisible (bool Visible)","false");//不显示窗体
            excel->setProperty("DisplayAlerts", false);
            //不显示任何警告信息。如果为true那么在关闭是会出现类似“文件已修改，是否保存”的提示
            QAxObject *workbooks = excel->querySubObject("WorkBooks");//获取工作簿集合
            workbooks->dynamicCall("Add");//新建一个工作簿
            QAxObject *workbook = excel->querySubObject("ActiveWorkBook");//获取当前工作簿
            QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);
            //访问excel中的工作表中第一个单元格
            //传输表头文件
            QString * hHeaderItem = getSaveExcelHeader();//bug place
            QAxObject *cell;
            QAxObject *range;//= worksheet->querySubObject("Range(const QString&)", cellTitle);
            for(int i = 0;i<colCount;i++){
                //读取
                QString columnName;
                //修改内容
                cell = worksheet->querySubObject("Cells(int,int)", 1, i+headerBegin);//确定操作单元格，将数据保存到哪儿，行的表头，所以要从第二列开始
                columnName = hHeaderItem[i];//获取此处的文本内容，i是列号，就是第几列中的文本内容
                cell->dynamicCall("SetValue(const QString&)", columnName);//打印到excel
                //前半句是获取单元格字体,后半句是字体加粗
                cell->querySubObject("Font")->setProperty("Bold", true);
                cell->querySubObject("Interior")->setProperty("Color",QColor(191, 191, 191));
                cell->setProperty("HorizontalAlignment", -4108);//xlCenter
                cell->setProperty("VerticalAlignment", -4108);//xlCenter
            }
            //谁保存谁负责具体写这个部分
            bool saveRes = saveDataPart(rowCount,colCount,worksheet);
            if(saveRes == false){
                delete excel;
                excel = nullptr;
                return false;
            }
            //
            //画框线
            QString lrange;
            lrange.append("A2:");//起始行列位置（原点）,修改后，开始的位置会变化，A2的从A2那一行开始，A3就是第三行
            //..........有差异
            lrange.append(colCount + rangeLine + 'A');//终止列，-1，就是正常状态，改变colcount后面的
            lrange.append(QString::number(rowCount +1));//终止行
            //这个起始的位置，相当于原点，最终给一个行列的终止位置就行
            range = worksheet->querySubObject("Range(const QString&)", lrange);
            //querySubObject("Borders")是对边框的设置，必须有
            range->querySubObject("Borders")->setProperty("LineStyle", QString::number(1));
            range->querySubObject("Borders")->setProperty("Color", QColor(0, 0, 0));//颜色的设置

            //调整数据区行高
            QString rowsName;
            rowsName.append("A2:");//起始行列位置
            rowsName.append(colCount + rangeLine + 'A');//终止列
            //上面两句也可以变成
            //rowsName.append("2:");//起始列
            rowsName.append(QString::number(rowCount + 1));//终止行
            range = worksheet->querySubObject("Range(const QString&)", rowsName);
            range->setProperty("RowHeight", 20);//设置行高
            range->setProperty("ColumnWidth", 20);  //设置单元格列宽

            workbook->dynamicCall("SaveAs(const QString&)",QDir::toNativeSeparators(path));
            workbook->dynamicCall("Close()");//关闭工作簿
            excel->dynamicCall("Quit()");//关闭excel
            delete excel;
            excel = nullptr;
        }
        return true;
    }
    return false;
}
