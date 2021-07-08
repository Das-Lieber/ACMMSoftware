#include "coordinateform.h"
coordinateForm::coordinateForm(QWidget *parent) : QDialog(parent){
    saveButton = new QPushButton("保存",parent);//保存
    clearButton = new QPushButton("清空",parent);//清空

    xLabel = new QLabel("X",parent);//X
    yLabel = new QLabel("Y",parent);//Y
    zLabel = new QLabel("Z",parent);//Z

    xTestEdit = new QTextEdit(parent);//显示x
    yTestEdit = new QTextEdit(parent);//显示y
    zTestEdit = new QTextEdit(parent);//显示z

    gridLayoutTcp = new QGridLayout(parent);//TCP

    sampleNumberLabel = new QLabel("采样数据",parent);//采样数据
    numTextEdit = new QTextEdit(parent);//采样数据的显示
    modelComboBox = new QComboBox(parent);//Combo选框
    modelComboBox->addItem(tr("A S M E"));
    modelComboBox->addItem(tr("连续模式"));
}
coordinateForm::~coordinateForm(){

}
void coordinateForm::SetCoordinateForm(QWidget *parent){
    if(gridLayoutTcp->count()!=0){
        QLayoutItem *child;
        while ((child = gridLayoutTcp->takeAt(0)) != 0){
            gridLayoutTcp->removeWidget(child->widget());
            child->widget()->setParent(0);
            delete child;
        }
        delete gridLayoutTcp;
        gridLayoutTcp = new QGridLayout(this);
    }
    parent->setWindowTitle("ACMM TCP&SinglePoint Test");
    parent->setMaximumSize(540,400);
    parent->setMinimumSize(530,360);

    QFont mFont;
    mFont.setFamily("Microsoft YaHei");
    mFont.setBold(false);
    mFont.setPixelSize(18);

    modelComboBox->setFont(mFont);
    sampleNumberLabel->setFont(mFont);
    saveButton->setFont(mFont);
    clearButton->setFont(mFont);

    mFont.setBold(true);
    mFont.setPixelSize(30);

    numTextEdit->setFont(mFont);

    mFont.setBold(true);
    mFont.setPixelSize(42);

    xLabel->setFont(mFont);
    yLabel->setFont(mFont);
    zLabel->setFont(mFont);

    modelComboBox->setMinimumSize(110,40);

    saveButton->setMinimumSize(60,30);
    clearButton->setMinimumSize(60,30);

    xTestEdit->setFont(mFont);//显示x
    yTestEdit->setFont(mFont);//显示y
    zTestEdit->setFont(mFont);//显示

    xTestEdit->setMaximumSize(300,80);//显示x
    yTestEdit->setMaximumSize(300,80);//显示y
    zTestEdit->setMaximumSize(300,80);//显示z
    numTextEdit->setMinimumSize(120,40);
    numTextEdit->setMaximumSize(150,50);


    saveButton->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    clearButton->setStyleSheet("background:transparent;border-width:0;border-style:outset");

    xTestEdit->setStyleSheet("border-width:0;border-style:outset");//无边框且 透明
    yTestEdit->setStyleSheet("border-width:0;border-style:outset");//无边框且 透明
    zTestEdit->setStyleSheet("border-width:0;border-style:outset");//无边框且 透明

    xTestEdit->setAlignment(Qt::AlignRight);
    yTestEdit->setAlignment(Qt::AlignRight);
    zTestEdit->setAlignment(Qt::AlignRight);
    numTextEdit->setAlignment(Qt::AlignRight);

    QColor xcol;
    xcol.setRgb(0,163,255);
    QPalette xcolor,ycolor,zcolor;
    //字号设置
    xcolor.setColor(QPalette::WindowText,Qt::red);
    xLabel->setPalette(xcolor);
    //字号设置
    ycolor.setColor(QPalette::WindowText,Qt::blue);
    yLabel->setPalette(ycolor);
    //颜色设置
    //字号设置
    zcolor.setColor(QPalette::WindowText,xcol);
    zLabel->setPalette(zcolor);

    gridLayoutTcp->addWidget(modelComboBox,1,0,1,1,Qt::AlignRight);//Combo
    gridLayoutTcp->addWidget(sampleNumberLabel,1,1,1,1,Qt::AlignLeft);//Label
    gridLayoutTcp->addWidget(numTextEdit,1,2,1,1,Qt::AlignCenter);//TextEdit

    gridLayoutTcp->addWidget(saveButton,1,3,1,1,Qt::AlignCenter);//save
    gridLayoutTcp->addWidget(clearButton,1,4,1,1,Qt::AlignCenter);//clear

    gridLayoutTcp->addWidget(xLabel,2,0,1,2,Qt::AlignRight);
    gridLayoutTcp->addWidget(xTestEdit,2,2,1,3,Qt::AlignLeft);

    gridLayoutTcp->addWidget(yLabel,3,0,1,2,Qt::AlignRight);
    gridLayoutTcp->addWidget(yTestEdit,3,2,1,3,Qt::AlignLeft);

    gridLayoutTcp->addWidget(zLabel,4,0,1,2,Qt::AlignRight);
    gridLayoutTcp->addWidget(zTestEdit,4,2,1,3,Qt::AlignLeft);

    // 设置水平间距
    gridLayoutTcp->setHorizontalSpacing(40);
    // 设置垂直间距
    gridLayoutTcp->setVerticalSpacing(15);
    gridLayoutTcp->setContentsMargins(10, 10, 10, 10);
    parent->setLayout(gridLayoutTcp);
}
/*************************************TextEdit的各类设置函数********************************************/
void coordinateForm::setNumTextItem(const QString &text) const {
    if(numTextEdit == nullptr){
        return;
    }
    numTextEdit->setText(nullptr);
    numTextEdit->setText(text);
}
void coordinateForm::setXTextItem(const QString &text) const{
    if(xTestEdit == nullptr){
        return;
    }
    xTestEdit->setText(nullptr);
    xTestEdit->setText(text);
}
void coordinateForm::setYTextItem(const QString &text) const{
    if(yTestEdit == nullptr){
        return;
    }
    yTestEdit->setText(nullptr);
    yTestEdit->setText(text);
}
void coordinateForm::setZTextItem(const QString &text) const{
    if(zTestEdit == nullptr){
        return;
    }
    zTestEdit->setText(nullptr);
    zTestEdit->setText(text);
}
QString coordinateForm::getBoxCurrentText() const{
    if(modelComboBox == nullptr){
        return "";
    }
    return modelComboBox->currentText();
}
/*************************************为对应应用部分的信号和槽服务(封装性和耦合性不好)********************************************/
QPushButton * coordinateForm::getSaveButtonPointer() const{
    if(saveButton == nullptr){
        return nullptr;
    }
    return saveButton;
}
QPushButton * coordinateForm::getClearButtonPointer() const{
    if(clearButton == nullptr){
        return nullptr;
    }
    return clearButton;
}
