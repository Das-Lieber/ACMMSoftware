#ifndef EXPLANATION_H
#define EXPLANATION_H

#include <QDialog>
#include<QDebug>
#include<QTextEdit>
#include<QGridLayout>
#include<QLabel>
#include<QPaintEvent>
#include<QPainter>
#include<QPushButton>
#include<QMessageBox>
#include<QReadWriteLock>

#include "commondata.h"

namespace Ui {
class explanation;
}

class explanation : public QDialog
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *);
public:
    explicit explanation(QWidget *parent = 0);
    ~explanation();
    //Function
    void showUi();
    QPushButton * getOKButtonPointer() const;
    void closeForm();
private:
    commonData * mCommonData;//公共数据部分
    QReadWriteLock * mReadWriteLock;
    //Variable
    QPushButton * OKButton;//确定摁键
    QPushButton * cannelButton;//取消摁键
    QGridLayout * gridLayoutEX;//界面布局

    QTextEdit * printUniqueCode;//完整的识别符
    QTextEdit * communicationBits;//通讯位数
    QTextEdit * dataProcessMethod;//数据的处理方式
    QTextEdit * errorAngleNum;//测角误差编号
    QTextEdit * assemblyDate;//装配日期
    QTextEdit * sensorSituation;//传感器的情况
    QTextEdit * acmmModel;//型号
    QTextEdit * parameterName;//偏心和结构参数的名称


    QLabel * labelUniqueCode;//完整的识别符
    QLabel * labelcommunicationBits;//通讯位数
    QLabel * labeldataProcessMethod;//数据的处理方式
    QLabel * labelerrorAngleNum;//测角误差编号
    QLabel * labelassemblyDate;//装配日期
    QLabel * labelsensorSituation;//传感器的情况
    QLabel * labelacmmModel;//型号
    QLabel * labelparameterName;//偏心和结构参数的名称

    void exUisetMain();
    void ExplanPretreatment();

signals:
    void connectDone();
};

#endif // EXPLANATION_H
