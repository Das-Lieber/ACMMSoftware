#include "MeasureWidget.h"
#include "ui_MeasureWidget.h"

MeasureWidget::MeasureWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeasureWidget)
{
    ui->setupUi(this);
    ui->tableWidget_data->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    
}

MeasureWidget::~MeasureWidget()
{
    delete ui;
}

void MeasureWidget::AppendPnt(const gp_Pnt &pnt)
{
    if(measurePnts.isEmpty()) {
        QList<gp_Pnt> tmp;
        QList<gp_Pnt> tmp2;
        measurePnts.append(tmp);
        virtualPnts.append(tmp2);
    }

    int index = measurePnts.last().size();
    ui->tableWidget_data->setRowCount(index+1);
    ui->tableWidget_data->setItem(index,0,new QTableWidgetItem(QString::number(pnt.X())));
    ui->tableWidget_data->setItem(index,1,new QTableWidgetItem(QString::number(pnt.Y())));
    ui->tableWidget_data->setItem(index,2,new QTableWidgetItem(QString::number(pnt.Z())));
    ui->tableWidget_data->scrollToBottom();

    measurePnts.last().append(pnt);
}

void MeasureWidget::AppendVirtualPnt(const gp_Pnt &pnt)
{
    virtualPnts.last().append(pnt);
}

void MeasureWidget::SetTitle(const QString &str)
{
    ui->label_title->clear();
    ui->label_title->setText("测量"+str);
}

void MeasureWidget::SetFeatureSize(int val)
{
    featureSize = val;
    if(val>1) {
        ui->stackedWidget->setCurrentIndex(1);
    }
    else
        ui->stackedWidget->setCurrentIndex(0);
}

void MeasureWidget::on_pushButton_sure_clicked()
{
    emit measureEnd(measurePnts,virtualPnts);
    measurePnts.clear();
    virtualPnts.clear();
    ui->tableWidget_data->clear();
    ui->tableWidget_data->setRowCount(0);
    emit readyClose();
}

void MeasureWidget::on_pushButton_cancle_clicked()
{
    measurePnts.clear();
    virtualPnts.clear();
    ui->tableWidget_data->clear();
    ui->tableWidget_data->setRowCount(0);
    emit readyClose();
}

void MeasureWidget::on_pushButton_nextFeature_clicked()
{
    if(measurePnts.isEmpty())
        return;

    if(measurePnts.last().isEmpty())
        return;

    QList<gp_Pnt> tmp;
    QList<gp_Pnt> tmp2;
    measurePnts.append(tmp);
    virtualPnts.append(tmp2);
}
