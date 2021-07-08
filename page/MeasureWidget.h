#ifndef MEASUREWIDGET_H
#define MEASUREWIDGET_H

#include <QWidget>

#include <gp_Pnt.hxx>

namespace Ui {
class MeasureWidget;
}

class MeasureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MeasureWidget(QWidget *parent = nullptr);
    ~MeasureWidget();

    void AppendPnt(const gp_Pnt &pnt);
    void AppendVirtualPnt(const gp_Pnt &pnt);
    void SetTitle(const QString &str);
    void SetFeatureSize(int val);

private slots:
    void on_pushButton_sure_clicked();
    void on_pushButton_cancle_clicked();
    void on_pushButton_nextFeature_clicked();

private:
    Ui::MeasureWidget *ui;

    QList<QList<gp_Pnt>> measurePnts;
    QList<QList<gp_Pnt>> virtualPnts;

    int featureSize;

signals:
    void measureEnd(const QList<QList<gp_Pnt>> &result, const QList<QList<gp_Pnt>> &virtDtat);
    void readyClose();
};

#endif // MEASUREWIDGET_H
