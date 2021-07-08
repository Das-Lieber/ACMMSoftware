#ifndef MEASUREITEM_H
#define MEASUREITEM_H

#include <gp_Pnt.hxx>
#include <QList>

class LabelWidget;
enum class GeometryType;

class MeasureItem
{
public:
    MeasureItem();

    void setLabel(LabelWidget *lab);
    void setData(const QList<QList<gp_Pnt> > &data);
    void Evaluate();

    QString getID() const;

    LabelWidget* getLabel() const {
        return mLabel;
    }

    QList<QList<gp_Pnt>> getData() const {
        return mData;
    }

    double getEvaluate() const {
        return error;
    }

    GeometryType Type() const {
        return mType;
    }
    void setType(const GeometryType &tp) {
        mType = tp;
    }

private:
    LabelWidget *mLabel;
    QList<QList<gp_Pnt>> mData;
    GeometryType mType;
    double error;

    static int ItemCount;
    static double UPTol;
    static double LOWTol;
};

#endif // MEASUREITEM_H
