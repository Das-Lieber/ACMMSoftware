#ifndef CONSTRUCTIONWIDGET_H
#define CONSTRUCTIONWIDGET_H

#include <QWidget>
#include <QFile>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

namespace Ui {
class constructionWidget;
}

struct Line
{
    gp_Pnt startPnt;
    gp_Pnt endPnt;
};
struct Cylinder
{
    int cylinderRadius;
    int height;
    gp_Pnt position;
    gp_Dir direction;
};
struct Ball
{
    int radius;
    gp_Pnt position;
};

class constructionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit constructionWidget(QWidget *parent = nullptr);
    ~constructionWidget();

    gp_Pnt Point;
    Line line;
    Cylinder cylinder;
    Ball ball;

    void setConstrutObject(int objectType);

private slots:
    void on_pushButton_construct_clicked();
    void on_pushButton_cancelConstruct_clicked();

private:
    Ui::constructionWidget *ui;

    void getArguments();

signals:
    void doConstruct(int constructType);
};

#endif // CONSTRUCTIONWIDGET_H
