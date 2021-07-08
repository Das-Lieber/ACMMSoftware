#include "calibrationform.h"
calibrationForm::calibrationForm(QWidget *parent) : QDialog(parent){
    //Qlabel设置
    numLabel = new QLabel("采样点数量:",parent);
    //QTextEdit
    numTextEdit = new QTextEdit(parent);
    xTestEdit = new QTextEdit(parent);
    yTestEdit = new QTextEdit(parent);
    zTestEdit = new QTextEdit(parent);
    modelComboBox = new QComboBox(parent);
    modelComboBox->addItem(tr("连续模式"));
    modelComboBox->addItem(tr("单次模式"));
    tableWidget = new QTableWidget(parent);//指定父对象
}
calibrationForm::~calibrationForm(){
}
/*************************************UI的各类设置********************************************/
void calibrationForm::uiResolution(){
    currentScreenWid = QApplication::desktop()->width();
    currentScreenHei  = QApplication::desktop()->height();
    //correction
    factor_textedit = 1;
    factor_combox = 1;
    factor_label = 1;
    factor_pushbotton = 1;

    factor_painter_x = 1;
    factor_painter_y = 1;
    factor_painter_font = 1;
    int control_x = (int)currentScreenWid;
    int control_y = (int)currentScreenHei;
    switch (control_x) {
    case 1920:
        qDebug()<<"分辨率:1920x1080";
        factor_textedit = 1.2;
        factor_combox = 1.2;
        factor_label = 1.2;
        factor_pushbotton = 1.2;
        factor_painter_x = 1.4;
        factor_painter_y = 1.4;
        factor_painter_font = 1.3 ;
        break;
    case 1768:
        qDebug()<<"分辨率:1920x992";
        factor_textedit = 1.2;
        factor_combox = 1.2;
        factor_label = 1.2;
        factor_pushbotton = 1.2;
        factor_painter_x = 1.2;
        factor_painter_y = 1.2;
        factor_painter_font = 1.1 ;
        break;
    case 1680:
        qDebug()<<"分辨率:1680x1050";
        factor_textedit = 1.1;
        factor_combox = 1.2;
        factor_label = 1.2;
        factor_pushbotton = 1.2;
        factor_painter_x = 1.2;
        factor_painter_y = 1.2;
        factor_painter_font = 1.1 ;
        break;
    case 1600:
        if(control_y == 1024)
        {
            qDebug()<<"分辨率:1600x1024";
            factor_textedit = 1.1;
            factor_combox = 1.1;
            factor_label = 1.1;
            factor_pushbotton = 1.1;
            factor_painter_x = 1.2;
            factor_painter_y = 1.2;
            factor_painter_font = 1.2 ;
        }
        else if(control_y == 900)
        {
            qDebug()<<"分辨率:1600x900";
            factor_textedit = 1.1;
            factor_combox = 1.1;
            factor_label = 1.1;
            factor_pushbotton = 1.1;
            factor_painter_x = 1.1;
            factor_painter_y = 1.1;
            factor_painter_font = 1.1 ;
        }
        break;
    case 1440:
        qDebug()<<"分辨率:1440x900";
        qDebug()<<"开发分辨率，不做任何修改";
        factor_textedit = 1;
        factor_combox = 1;
        factor_label = 1;
        factor_pushbotton = 1;

        factor_painter_x = 1;
        factor_painter_y = 1;
        factor_painter_font = 1;
        break;
    case 1366:
        qDebug()<<"分辨率:1366x768";
        factor_textedit = 0.9;
        factor_combox = 0.9;
        factor_label = 0.9;
        factor_pushbotton = 0.9;
        factor_painter_x = 1;
        factor_painter_y = 1;
        factor_painter_font = 1;
        break;
    case 1360:
        qDebug()<<"分辨率:1360x768";
        factor_textedit = 0.9;
        factor_combox = 0.9;
        factor_label = 0.9;
        factor_pushbotton = 0.9;
        factor_painter_x = 1;
        factor_painter_y = 1;
        factor_painter_font = 1;
        break;
    case 1280:
        switch (control_y) {
        case 1024:
            qDebug()<<"分辨率:1280x1024";
            factor_textedit = 0.9;
            factor_combox = 0.8;
            factor_label = 0.8;
            factor_pushbotton = 0.8;
            factor_painter_x = 0.9;
            factor_painter_y = 0.9;
            factor_painter_font = 0.9;
            break;
        case 960:
            qDebug()<<"分辨率:1280x960";
            factor_textedit = 0.9;
            factor_combox = 0.8;
            factor_label = 0.8;
            factor_pushbotton = 0.8;
            factor_painter_x = 0.9;
            factor_painter_y = 0.9;
            factor_painter_font = 0.9;
            break;
        case 800:
            qDebug()<<"分辨率:1280x800";
            factor_textedit = 0.9;
            factor_combox = 0.8;
            factor_label = 0.8;
            factor_pushbotton = 0.8;
            factor_painter_x = 0.9;
            factor_painter_y = 0.9;
            factor_painter_font = 0.9;
            break;
        case 768:
            qDebug()<<"分辨率:1280x768";
            factor_textedit = 0.9;
            factor_combox = 0.8;
            factor_label = 0.8;
            factor_pushbotton = 0.8;
            factor_painter_x = 0.9;
            factor_painter_y = 0.9;
            factor_painter_font = 0.9;
            break;
        case 720:
            qDebug()<<"分辨率:1280x720";
            factor_textedit = 0.9;
            factor_combox = 0.8;
            factor_label = 0.8;
            factor_pushbotton = 0.8;
            factor_painter_x = 0.9;
            factor_painter_y = 0.9;
            factor_painter_font = 0.9;
            break;
        default:
            break;
        }
        break;
    case 1176:
        qDebug()<<"分辨率:1176x664";
        factor_textedit = 0.71;
        factor_combox = 0.8;
        factor_label = 0.8;
        factor_pushbotton = 0.8;
        factor_painter_x = 0.83;
        factor_painter_y = 0.83;
        factor_painter_font = 0.83;
        break;
    case 1152:
        qDebug()<<"分辨率:1152x864";
        factor_textedit = 0.7;
        factor_combox = 0.75;
        factor_label = 0.75;
        factor_pushbotton = 0.75;
        factor_painter_x = 0.83;
        factor_painter_y = 0.83;
        factor_painter_font = 0.83;
        break;
    case 1024:
        qDebug()<<"分辨率:1024x768";
        factor_textedit = 0.55;
        factor_combox = 0.75;
        factor_label = 0.75;
        factor_pushbotton = 0.75;
        factor_painter_x = 0.73;
        factor_painter_y = 0.73;
        factor_painter_font = 0.73;
        break;
    case 800:
        qDebug()<<"分辨率:800x600";
        factor_textedit = 0.4;
        factor_combox = 0.5;
        factor_label = 0.5;
        factor_pushbotton = 0.5;
        factor_painter_x = 0.6;
        factor_painter_y = 0.6;
        factor_painter_font = 0.6;
        break;
    default:
        break;
    }
}
void calibrationForm::resetGrid(QWidget *widget,double Double_factorx,double Double_factory){
    int widgetX = widget->x();
    int widgetY = widget->y();
    int widgetWid = widget->width();
    int widgetHei = widget->height();

    int nWidgetX = (int)(widgetX*Double_factorx);
    int nWidgetY = (int)(widgetY*Double_factory);
    int nWidgetWid = (int)(widgetWid*Double_factorx);
    int nWidgetHei = (int)(widgetHei*Double_factory);

    widget->setGeometry(nWidgetX,nWidgetY,nWidgetWid,nWidgetHei);
}
void calibrationForm::SetCalibrationForm(QWidget *parent){
    uiResolution();//每次打开UI界面之前的看一下分辨率
    double hzb = 1440;
    double zzb = 900;
//    double hzb = 2880;
//    double zzb = 1800;

    double goldenage = hzb/zzb;
    qDebug()<<"goldenage"<<goldenage;
    factorX= 0;
    factorY= 0;
    //目前的缩放，以长度指标作为主要依据
//    factorX = currentScreenWid/hzb;
    factorX = 1;
    factorY = (zzb*factorX)/zzb;
    qDebug()<<"Double_factorx"<<factorX;
    qDebug()<<"Double_factory"<<factorY;

    //ui开发设备上，分辨率为1400,900
    int truesizeo = (int)(1340*factorX);
    int truesizeh = (int)(640*factorY);

    parent->setFixedSize(truesizeo,truesizeh);//只能在此处设置界面大小 其他位置失效
    parent->setWindowTitle("ACMM Calibration");

    //字体设置
    numLabel->setGeometry(1125,25,70,30);
    //20190828UI更改
    //字体设置
    QFont font_label;
    font_label.setFamily("Microsoft YaHei");
    font_label.setPointSize((int)(10*factor_label));
    font_label.setBold(false);
    //字体设置
    numLabel->setFont(font_label);
    //20190828UI更改

    //做成全局变量
    //QTextEdit定位
    numTextEdit->setGeometry(1220,22,100,35);
    xTestEdit->setGeometry(1020,64,100,35);
    yTestEdit->setGeometry(1120,64,100,35);
    zTestEdit->setGeometry(1220,64,100,35);

    //字体设置
    Font_textedit.setFamily("Microsoft YaHei");
    Font_textedit.setPointSize((int)(14*factor_textedit));
    Font_textedit.setBold(false);
    numTextEdit->setFont(Font_textedit);
    xTestEdit->setFont(Font_textedit);
    yTestEdit->setFont(Font_textedit);
    zTestEdit->setFont(Font_textedit);
    //20190828UI更改
    //字体设置
    numTextEdit->setAlignment(Qt::AlignRight);
    xTestEdit->setAlignment(Qt::AlignRight);
    yTestEdit->setAlignment(Qt::AlignRight);
    zTestEdit->setAlignment(Qt::AlignRight);
    modelComboBox->setGeometry(1020,22,100,35);
    //20190828UI更改

    //字体设置
    QFont font_combobox;
    font_combobox.setFamily("Microsoft YaHei");
    font_combobox.setPointSize((int)(10*factor_combox));
    font_combobox.setBold(false);
    modelComboBox->setFont(font_combobox);
    //20190828UI更改
    /****************统一分辨率***********************/
    //Qlabel统一化
    resetGrid(numLabel,factorX,factorY);
    //QTextEdit统一化
    resetGrid(numTextEdit,factorX,factorY);
    resetGrid(xTestEdit,factorX,factorY);
    resetGrid(yTestEdit,factorX,factorY);
    resetGrid(zTestEdit,factorX,factorY);
    //QComboBox
    resetGrid(modelComboBox,factorX,factorY);
    SetTablePart();
}
//绘制前界面的表格
void calibrationForm::SetTablePart(){
    if(tableWidget == nullptr){
        qDebug()<<"删除旧的table";
        return;
    }
    int trueph = (int)(40*factorX);
    int truepw = (int)(25*factorY);

    tableWidget->move(trueph,truepw);
    tableWidget->setColumnCount(7);

    int trueh = (int)(940*factorX);
    int truew = (int)(580*factorY);

    tableWidget->resize(trueh,truew);

    QStringList header;
    header<<"序号"<<"编码器1"<<"编码器2"<<"编码器3"<<"编码器4"<<"编码器5"<<"编码器6";
    tableWidget->setHorizontalHeaderLabels(header);//设置表头（横）
    tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows);//一行为选中模式

    int truehead = (int)(30*factorX);
    tableWidget->horizontalHeader()->setFixedHeight(truehead); //设置表头的高度为25
    tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);//20190903
    int trueotherh = (int)(135*factorX);
    for(int i = 1;i<7;i++){
       tableWidget->horizontalHeader()->resizeSection(i,trueotherh); //设置表头第1列的宽度为180
    }
    //数据来了一行一行增加吧insertRow(RowCont)
    tableWidget->verticalHeader()->hide();
    tableWidget->setAutoScroll(true);//自动滚动
    tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    tableWidget->show();
}
QTableWidget * calibrationForm::getTableObject(){
    return tableWidget == nullptr ? nullptr:tableWidget;
}
/*************************************TableWidget的各类设置函数********************************************/
QTableWidgetItem * calibrationForm::getItemAt(const QPoint point)  const{
    if(tableWidget == nullptr) {
        return nullptr;
    }
   return tableWidget->itemAt(point);
}
void calibrationForm::setTableItem(const int row,const int column,QTableWidgetItem *item){
    if(tableWidget == nullptr) {
        return;
    }
    tableWidget->setItem(row,column,item);
}
void calibrationForm::setTableRowCount(const int arg){
    if(tableWidget == nullptr) {
        return;
    }
    tableWidget->setRowCount(arg);//自动增加
}
void calibrationForm::setScrollToBottom(){
    if(tableWidget == nullptr) {
        return;
    }
    //显示最后一行
    tableWidget->scrollToBottom();
}
int calibrationForm::getTableCurrentRow() const{
    if(tableWidget == nullptr) {
        return -1;
    }
    return tableWidget->currentRow();
}
void calibrationForm::removeTableRow(const int position){
    if(tableWidget == nullptr) {
        return;
    }
    tableWidget->removeRow(position);//删除remove这一行
}
void calibrationForm::clearTableItem(){
    if(tableWidget == nullptr){
        return;
    }
    tableWidget->clearContents();
}
int calibrationForm::getTableRowCount() const{
    if(tableWidget == nullptr){
        return 0;
    }
    return tableWidget->rowCount();
}
int calibrationForm::getTableColumnCount() const{
    if(tableWidget == nullptr){
        return 0;
    }
    return tableWidget->columnCount();
}
QString calibrationForm::getHorizontalHeaderItem(const int position) const{
    if(tableWidget == nullptr){
        return "";
    }
    //做一段吧List转化为Qstring
    QTableWidgetItem * item = tableWidget->horizontalHeaderItem(position);
    if(item == nullptr) {
        return "";
    }
    return item->text();
}
QString calibrationForm::getTableItem(const int row,const int column) const{
    if(tableWidget == nullptr){
        return "";
    }
    QTableWidgetItem * item  = tableWidget->item(row,column);
    if(item == nullptr){
        return "";
    }
    return item->text();
}
/*************************************TextEdit的各类设置函数********************************************/
void calibrationForm::setNumTextItem(const QString text){
    if(numTextEdit == nullptr){
        return;
    }
    numTextEdit->setText(nullptr);
    numTextEdit->setText(text);
}
void calibrationForm::setXTextItem(const QString text){
    if(xTestEdit == nullptr){
        return;
    }
    xTestEdit->setText(nullptr);
    xTestEdit->setText(text);
}
void calibrationForm::setYTextItem(const QString text){
    if(yTestEdit == nullptr){
        return;
    }
    yTestEdit->setText(nullptr);
    yTestEdit->setText(text);
}
void calibrationForm::setZTextItem(const QString text){
    if(zTestEdit == nullptr){
        return;
    }
    zTestEdit->setText(nullptr);
    zTestEdit->setText(text);
}
QString calibrationForm::getBoxCurrentText() const{
    if(modelComboBox == nullptr){
        return "";
    }
    return modelComboBox->currentText();
}
//UI缩放因子
double calibrationForm::getXFactor(){
    return factorX;
}
double calibrationForm::getYFactor(){
    return factorY;
}
