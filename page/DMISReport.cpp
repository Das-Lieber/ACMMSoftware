#include "DMISReport.h"

#include <QPainter>

#include "occ/MeasureItem.h"

DMISReport::DMISReport()
{
}

QPixmap DMISReport::GetReport()
{
    const int width = mPixmap.width();
    int height = mPixmap.height();
    int totalHei = height;
    for(int i=0;i<mItems.size();++i) {
        totalHei += 30;
        QList<QList<gp_Pnt>> list = mItems[i]->getData();
        for(int j=0;j<list.size();++j) {
            for(int k=0;k<list[j].size();++k) {
                totalHei += 30;
            }

            totalHei += 10;
        }
    }

    QPixmap result(width,totalHei+10);
    result.fill();
    QPainter aPainter(&result);

    aPainter.drawPixmap(0,0,width,height,mPixmap);

    for(int i=0;i<mItems.size();++i) {
        height += 30;
        QString content = mItems[i]->getTitle()+" : "+QString::number(mItems[i]->getEvaluate(),'f');
        QPen apen = aPainter.pen();
        apen.setColor(Qt::blue);
        aPainter.setPen(apen);
        aPainter.drawText(0,height,content);
        apen.setColor(Qt::black);
        aPainter.setPen(apen);

        QList<QList<gp_Pnt>> list = mItems[i]->getData();
        for(int j=0;j<list.size();++j) {
            height += 10;
            for( int k=0;k<list[j].size();++k) {
                height += 30;
                QString content = QString("(%1 , %2 , %3)").arg(list[j][k].X()).arg(list[j][k].Y()).arg(list[j][k].Z());
                aPainter.drawText(0,height,content);
            }
        }
    }
    return result;
}

void DMISReport::AddItems(QList<MeasureItem *> items)
{
    mItems = items;
}

void DMISReport::AddPixmap(const QPixmap &map)
{
    mPixmap = map;
}
