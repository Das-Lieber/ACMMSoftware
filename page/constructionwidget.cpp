#include "constructionwidget.h"
#include "ui_constructionwidget.h"

constructionWidget::constructionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::constructionWidget)
{
    ui->setupUi(this);
}

constructionWidget::~constructionWidget()
{
    delete ui;
}

void constructionWidget::setConstrutObject(int objectType)
{
    switch (objectType) {
    case 0:ui->stackedWidget->setCurrentIndex(0);
    break; case 1:ui->stackedWidget->setCurrentIndex(1);
    break; case 2:ui->stackedWidget->setCurrentIndex(2);
    break; case 3:ui->stackedWidget->setCurrentIndex(3);
    }
}

void constructionWidget::getArguments()
{
    switch(ui->stackedWidget->currentIndex())
    {
    case 0:Point = gp_Pnt(ui->doubleSpinBox_pointX->value(),
                             ui->doubleSpinBox_pointY->value(),
                             ui->doubleSpinBox_pointZ->value());
        break;
    case 1:line.startPnt = gp_Pnt(ui->doubleSpinBox_lineStartPntX->value(),
                                     ui->doubleSpinBox_lineStartPntY->value(),
                                     ui->doubleSpinBox_lineStartPntZ->value());
        line.endPnt = gp_Pnt(ui->doubleSpinBox_lineEndPntX->value(),
                                ui->doubleSpinBox_lineEndPntY->value(),
                                ui->doubleSpinBox_lineEndPntZ->value());
        break;
    case 2:cylinder.cylinderRadius = ui->doubleSpinBox_cylinderRadius->value();
        cylinder.height = ui->doubleSpinBox_cylinderHeight->value();
        cylinder.position = gp_Pnt(ui->doubleSpinBox_cylinderPosX->value(),
                                      ui->doubleSpinBox_cylinderPosY->value(),
                                      ui->doubleSpinBox_cylinderPosZ->value());
        cylinder.direction = gp_Dir(ui->doubleSpinBox_cylinderDirectionX->value(),
                                       ui->doubleSpinBox_cylinderDirectionY->value(),
                                       ui->doubleSpinBox_cylinderDirectionZ->value());
        break;
    case 3:ball.radius = ui->doubleSpinBox_ballRadius->value();
        ball.position = gp_Pnt(ui->doubleSpinBox_ballPosX->value(),
                                  ui->doubleSpinBox_ballPosY->value(),
                                  ui->doubleSpinBox_ballPosZ->value());
    }
}

void constructionWidget::on_pushButton_construct_clicked()
{
    getArguments();
    emit doConstruct(ui->stackedWidget->currentIndex());
    this->close();
}

void constructionWidget::on_pushButton_cancelConstruct_clicked()
{
    this->close();
}
