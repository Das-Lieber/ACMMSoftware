#include "importcoefile.h"

importCoeffFile::importCoeffFile(QWidget *parent) : importFile(parent){

}
importCoeffFile::~importCoeffFile(){

}
//偏心误差部分
bool importCoeffFile::importDataPart(int rowBegin,int rowEnd,int colBegin,int colEnd,QAxObject * excel){
    if( (rowEnd - rowBegin) > 8 ){
        return false;
    }
    mReadWriteLock->lockForWrite();
    if( mCommonData->getCoeffParameterSize() != 0 ){
       mCommonData->clearCoeffParameter();
    }
    for (int i = rowBegin; i < rowEnd; i++){
        for (int j = colBegin; j < colEnd; j++){
            QAxObject *cellCoeff = new  QAxObject(this);
            cellCoeff = excel->querySubObject("Cells(Int, Int)", i, j );
            QVariant cellValue = cellCoeff->dynamicCall("value");
            mCommonData->setCoeffParameterItem(cellValue.toDouble());
            delete cellCoeff;
        }
    }
    mReadWriteLock->unlock();
    return true;
}
