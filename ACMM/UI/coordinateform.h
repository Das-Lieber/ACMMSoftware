#ifndef COORDINATEFORM_H
#define COORDINATEFORM_H

#include <QDialog>
#include<QPainter>
#include<QDebug>
#include<QColor>
#include<QLabel>
#include<QTextEdit>
#include<QGridLayout>
#include<QPushButton>
#include<QComboBox>
#include<QFileDialog>
#include<QPushButton>

class coordinateForm : public QDialog{
    Q_OBJECT
public:
    explicit coordinateForm(QWidget *parent = nullptr);
    ~coordinateForm();
    void SetCoordinateForm(QWidget *parent);
    //TextEdit
    void setNumTextItem(const QString & text) const;
    void setXTextItem(const QString & text) const;
    void setYTextItem(const QString & text) const;
    void setZTextItem(const QString & text) const;
    QString getBoxCurrentText() const;
    QPushButton * getSaveButtonPointer() const;
    QPushButton * getClearButtonPointer() const;
signals:

public slots:
private:
    QLabel * xLabel;//X
    QLabel * yLabel;//Y
    QLabel * zLabel;//Z

    QTextEdit *numTextEdit;//显示误差
    QTextEdit *xTestEdit;//显示x
    QTextEdit *yTestEdit;//显示y
    QTextEdit *zTestEdit;//显示z
    QComboBox *modelComboBox;//读数头

    QGridLayout * gridLayoutTcp;//TCP
    QLabel * sampleNumberLabel;//采样数据
    QPushButton *saveButton;//保存
    QPushButton *clearButton;//清空

};

#endif // COORDINATEFORM_H
