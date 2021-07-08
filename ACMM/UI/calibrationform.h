#ifndef CALIBRATIONFORM_H
#define CALIBRATIONFORM_H

#include <QDialog>
#include<QTableWidget>
#include<QFileDialog>
#include<QImage>
#include<QPainter>
#include<QDebug>
#include<math.h>//数学
#include<QProgressDialog>
#include<QList>
#include<QHeaderView>
#include<QComboBox>
#include<QTextEdit>
#include<QFont>
#include<QLabel>
#include<QDesktopWidget>
#include<QApplication>
#include<QTableWidgetItem>

class calibrationForm : public QDialog{
    Q_OBJECT
public:
    explicit calibrationForm(QWidget *parent = nullptr);
    ~calibrationForm();
    void SetCalibrationForm(QWidget *parent);

    //TableWidget Part(数据的再次封装)
    QTableWidgetItem * getItemAt(const QPoint point)  const;
    QTableWidget * getTableObject();
    void setTableItem(const int row,const int column,QTableWidgetItem *item);
    void setTableRowCount(const int arg);
    void setScrollToBottom();
    int getTableCurrentRow() const;
    void removeTableRow(const int position);
    void clearTableItem();
    int getTableRowCount() const;
    int getTableColumnCount() const;
    QString getTableItem(const int row,const int column) const;
    QString getHorizontalHeaderItem(const int position) const;

    //TextEdit
    void setNumTextItem(const QString text);
    void setXTextItem(const QString text);
    void setYTextItem(const QString text);
    void setZTextItem(const QString text);

    QString getBoxCurrentText() const;

    double getXFactor();
    double getYFactor();

signals:

public slots:
private:
    //Function
    void resetGrid(QWidget *widget,double factorX,double factorY);
    //Variable
    QLabel * numLabel;
    QFont Font_textedit;
    //Variable
    QTextEdit *numTextEdit;//显示误差
    QTextEdit *xTestEdit;//显示x
    QTextEdit *yTestEdit;//显示y
    QTextEdit *zTestEdit;//显示z
    QComboBox *modelComboBox;//读数头

    //前界面绘图
    QTableWidget *tableWidget;//前界面表格


    //缩放因子
    double factorX;
    double factorY;

    double Double_Factorx_Cali;//ui因子
    double Double_Factory_Cali;//ui因子

    //correction
    double currentScreenWid;
    double currentScreenHei;

    double factor_textedit;
    double factor_combox;
    double factor_label;
    double factor_pushbotton;

    double factor_painter_x;
    double factor_painter_y;
    double factor_painter_font;

    void uiSetCali();
    void uiResolution();
    void SetTablePart();
};

#endif // CALIBRATIONFORM_H
