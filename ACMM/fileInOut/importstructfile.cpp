#include "importstructfile.h"

importStructFile::importStructFile(QWidget *parent) : importFile(parent){

}
importStructFile::~importStructFile(){

}
//偏心误差部分
bool importStructFile::importDataPart(int rowBegin,int rowEnd,int colBegin,int colEnd,QAxObject * excel){
    if( (colEnd - colBegin) != 21 ){
        return false;
    }
    int position = 0;
    for (int i = rowBegin; i < rowEnd; i++){
        for (int j = colBegin; j < colEnd; j++,position++){
            QAxObject *cellStructPara = new  QAxObject(this);
            cellStructPara = excel->querySubObject("Cells(Int, Int)", i, j );
            QVariant cellValue = cellStructPara->dynamicCall("value");
            mCommonData->setStructParameterItem(position,cellValue.toDouble());
            delete cellStructPara;
        }
    }
    return true;
}
