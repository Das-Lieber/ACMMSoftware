#ifndef DMISREPORT_H
#define DMISREPORT_H

#include <QPixmap>

class MeasureItem;

class DMISReport
{
public:
    DMISReport();

    QPixmap GetReport();

    void AddItems(QList<MeasureItem*> items);
    void AddPixmap(const QPixmap &map);

private:
    QPixmap mPixmap;
    QList<MeasureItem*> mItems;
};

#endif // DMISREPORT_H
