#include "commondata.h"
const QString commonData::structureType = "Structure";
const QString commonData::coeffType = "Coeff";
const int commonData::SizeOfAngle;
const int commonData::SizeOfCoordinate;
const int commonData::SizeOFStruct;
double commonData::PI = 3.14159265359;
//三个坐标值
double commonData::coordinate[SizeOfCoordinate] = {0};
//偏心参数
QList<double> commonData::coeffParameter;
//结构参数
double commonData::structParameter[SizeOFStruct] = {0};
//圆光栅过零标志 int
bool commonData::flagForCrossZero[SizeOfAngle] = {false};
//角度值 oriangle 角度值
double commonData::Angle[SizeOfAngle] = {0};//保存的内容需要送到实时显示的部分
//摁键标志位 true表示需要记录 false表示不需要记录
bool commonData::dataRecordFlag = false;
//上偏差
double commonData::Upper = 0.010 ;
//下偏差
double commonData::Lower = -0.010 ;
//50位通讯
bool commonData::flagFor50Bytes = false;
//26位通讯
bool commonData::flagFor26Bytes = false;
//前界面Label功能摁键触发
bool commonData::flagForEnableForLabel = false;
//唯一标识符储存
QList<QString> commonData::uniqueCode;
//false就是减去90 true就是不减去90

//无数据库版本 六个list保存数据结果
QList<double> commonData::firstAxisList;
QList<double> commonData::secondAxisList;
QList<double> commonData::thirdAxisList;
QList<double> commonData::fourthAxisList;
QList<double> commonData::fifthAxisList;
QList<double> commonData::sixthAxisList;

//动态绘图部分记录
int commonData::imageAxisAngle[SizeOfAngle] = {0};
//
bool commonData::clearListFlag = false;

commonData::commonData(QWidget *parent) : QWidget(parent){

}
/*************************************各类常量********************************************/
double commonData::getPI(){
    return PI;
}
//上偏差
double commonData::getUpper(){
    return Upper;
}
//下偏差
double commonData::getLower(){
    return Lower;
}
void commonData::setLabelEnable(const bool value){
    flagForEnableForLabel = value;
}
bool commonData::getLabelEnable() const{
    return flagForEnableForLabel;
}
void commonData::setRecordFlag(const bool value){
    dataRecordFlag = value;
}
bool commonData::getRecordFlag() const{
    return dataRecordFlag;
}
bool commonData::getFlagFor50Bytes() const{
    return flagFor50Bytes;
}
void commonData::setFlagFor50Bytes(const bool value){
    flagFor50Bytes = value;
}
bool commonData::getFlagFor26Bytes() const{
    return flagFor26Bytes;
}
void commonData::setFlagFor26Bytes(const bool value){
    flagFor26Bytes = value;
}
/*************************************X,Y,Z的设置和获取函数********************************************/
double commonData::getCoordinateItem(const int position) const{
    if( position > (SizeOfCoordinate - 1) ){
        qDebug()<<"数组越界操作";
        return 0;
    }
    return coordinate[position];
}
void commonData::setCoordinate(const int position,const double value){
    if( position > (SizeOfCoordinate - 1) ){
        qDebug()<<"数组越界操作";
        return;
    }
    coordinate[position] = value;
}
/*************************************X,Y,Z的设置和获取函数（QString）********************************************/
QString commonData::getCoordinateItemStr(const int position,const int significantNum){
    if( position > (SizeOfCoordinate - 1) ){
        qDebug()<<"数组越界操作";
        return "";
    }
    return QString::number(coordinate[position],10,significantNum);
}
/*************************************偏心误差********************************************/
int commonData::getCoeffParameterSize() const{
    return coeffParameter.size();
}
double commonData::getCoeffParameterItem(const int position) const{
    if( position >= getCoeffParameterSize() ){
        return 0;
    }
    return coeffParameter.at(position);
}
void commonData::clearCoeffParameter(){
    coeffParameter.clear();
}
void commonData::setCoeffParameterItem(const double value){
    coeffParameter.append(value);
    return;
}
void commonData::initialCoeffParameter(const int size){
    for(int i = 0;i<size;++i){
        coeffParameter.append(0);
    }
}
/*************************************结构参数********************************************/
double commonData::getStructParameterItem(const int position) const{
    if( position > (SizeOFStruct - 1) ){
        qDebug()<<"数组越界操作";
        return 0;
    }
    return structParameter[position];
}
void commonData::setStructParameterItem(const int position,const double value){
    if( position > (SizeOFStruct - 1) ){
        qDebug()<<"数组越界操作";
        return;
    }
    structParameter[position] = value;
}
/*************************************圆光栅传感器数值********************************************/
double commonData::getAxisAngle(const int position) const{
    if( position > (SizeOfAngle - 1) ){
        qDebug()<<"数组越界操作";
        return 0;
    }
    return Angle[position];
}
double commonData::getAxisAngleRadian(const int position) const{
    if( position > (SizeOfAngle - 1) ){
        qDebug()<<"数组越界操作";
        return 0;
    }
    double AngleRadian = (Angle[position]/180)*this->PI;
    return AngleRadian;
}
void commonData::setAxisAngle(const int position,const double value){
    if( position > (SizeOfAngle - 1) ){
        qDebug()<<"数组越界操作";
        return;
    }
    Angle[position] = value;
}
/*************************************圆光栅过零标志********************************************/
void commonData::setAxisCrossZero(const int axisNum,const bool value){
    if( axisNum >= SizeOfAngle ){
        return;
    }
    flagForCrossZero[axisNum] = value;
}
bool commonData::getAxisCrossZero(const int axisNum) const{
    if( axisNum >= SizeOfAngle ){
        return false;
    }
    return flagForCrossZero[axisNum];
}
void commonData::initialAxisCrossZero(const bool value){
    for(int i = 0;i<SizeOfAngle;++i){
        flagForCrossZero[i] = value;
    }
}
/*************************************标定实验采样数据保存********************************************/
//无数据库版本 六个list保存数据结果
void commonData::setAxisSampleList(const int axisNum,const double value){
    switch (axisNum) {
    case 1:
        firstAxisList.push_back(value);
        break;
    case 2:
        secondAxisList.push_back(value);
        break;
    case 3:
        thirdAxisList.push_back(value);
        break;
    case 4:
        fourthAxisList.push_back(value);
        break;
    case 5:
        fifthAxisList.push_back(value);
        break;
    case 6:
        sixthAxisList.push_back(value);
        break;
    default:
        break;
    }
    return;
}
double commonData::getAxisSampleList(const int axisNum,const int position) const{
    double result = 0;
    if( isAxisSampleListEmpty() || position >=  firstAxisList.size()){
        return result;
    }
    switch (axisNum) {
    case 1:
        result = firstAxisList.at(position);
        break;
    case 2:
        result = secondAxisList.at(position);
        break;
    case 3:
        result = thirdAxisList.at(position);
        break;
    case 4:
        result = fourthAxisList.at(position);
        break;
    case 5:
        result = fifthAxisList.at(position);
        break;
    case 6:
        result = sixthAxisList.at(position);
        break;
    default:
        result = 0;
        break;
    }
    return result;
}
bool commonData::isAxisSampleListEmpty() const{
    //is Empty return true
    //else return false
    bool isListEmpty = firstAxisList.size()&&secondAxisList.size()&&thirdAxisList.size()
            &&fourthAxisList.size()&&fifthAxisList.size()&&sixthAxisList.size();
    //如果有元素 那么 isListEmpty == true 返回false
    //如果没有元素 那么 isListEmpty == false 返回true
    return isListEmpty == true ? false : true;
}
void commonData::clearAllAxisSampleList(){
    if( isAxisSampleListEmpty() ){
        return;
    }
    firstAxisList.clear();
    secondAxisList.clear();
    thirdAxisList.clear();
    fourthAxisList.clear();
    fifthAxisList.clear();
    sixthAxisList.clear();
    return;
}
void commonData::ereaseAxisSampleList(const int axisNum,const int position){
    switch (axisNum) {
    case 1:
        firstAxisList.erase(firstAxisList.begin() + position);
        break;
    case 2:
        secondAxisList.erase(secondAxisList.begin() + position);
        break;
    case 3:
        thirdAxisList.erase(thirdAxisList.begin() + position);
        break;
    case 4:
        fourthAxisList.erase(fourthAxisList.begin() + position);
        break;
    case 5:
        fifthAxisList.erase(fifthAxisList.begin() + position);
        break;
    case 6:
        sixthAxisList.erase(sixthAxisList.begin() + position);
        break;
    default:
        break;
    }
    return;
}
double commonData::getAxisSampleListRadian(const int axisNum,const int position) const{
    double result = 0;
    if( isAxisSampleListEmpty() || position >=  firstAxisList.size()){
        return result;
    }
    switch (axisNum) {
    case 1:
        result = (firstAxisList.at(position)/180)*this->PI;
        break;
    case 2:
        result = (secondAxisList.at(position)/180)*this->PI;
        break;
    case 3:
        result = (thirdAxisList.at(position)/180)*this->PI;
        break;
    case 4:
        result = (fourthAxisList.at(position)/180)*this->PI;
        break;
    case 5:
        result = (fifthAxisList.at(position)/180)*this->PI;
        break;
    case 6:
        result = (sixthAxisList.at(position)/180)*this->PI;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}
int commonData::getAxisSampleListSize() const{
    return firstAxisList.size();
}
/*************************************动态绘图部分记录********************************************/
double commonData::getImageAxisAngle(const int position) const{
    if( position > (SizeOfAngle - 1) ){
        qDebug()<<"数组越界操作";
        return 0;
    }
    return imageAxisAngle[position];
}
double commonData::getImageAxisAngleRadian(const int position) const{
    if( position > (SizeOfAngle - 1) ){
        qDebug()<<"数组越界操作";
        return 0;
    }
    double imageAngleRadian = (imageAxisAngle[position]/180)*this->PI;
    return imageAngleRadian;
}
void commonData::setImageAxisAngle(const int position,const int value){
    if( position > (SizeOfAngle - 1) ){
        qDebug()<<"数组越界操作";
        return;
    }
    imageAxisAngle[position] = value;
}
/*************************************唯一标识符储存********************************************/
int commonData::getUniqueCodeSize() const{
    return uniqueCode.size();
}
QString commonData::getUniqueCodeItem(const int position) const{
    if( position >= getUniqueCodeSize() ){
        return "";
    }
    return uniqueCode.at(position);
}
void commonData::clearUniqueCode(){
    uniqueCode.clear();
    return;
}
void commonData::setUniqueCodeItem(const QString value){
    uniqueCode.append(value);
    return;
}
