#include "MeasureItem.h"

#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <gp_Cylinder.hxx>

#include <QInputDialog>
#include <QDebug>

#include "LabelWidget.h"
#include "GeneralTools.h"

int MeasureItem::ItemCount = 0;
double MeasureItem::UPTol = 0.5;
double MeasureItem::LOWTol = -0.5;

MeasureItem::MeasureItem()
{
    mLabel = nullptr;
    ItemCount++;
}

void MeasureItem::setLabel(LabelWidget *lab)
{
    mLabel = lab;
    if(mLabel->m_id == -1) {
        mLabel->setID(ItemCount);
        mLabel->setInfo(mType);
    }
}

void MeasureItem::setData(const QList<QList<gp_Pnt>> &data)
{
    mData = data;
}

void MeasureItem::Evaluate()
{
    if(mData.isEmpty() || mLabel == nullptr)
        return;

    error = 0;
    bool ret;

    switch(mType) {
    case GeometryType::UNKNOWN:
    {
        break;
    }
    case GeometryType::LINE:
    {
        gp_Lin line;
        ret = GeneralTools::FitLine(mData.first().toStdList(),line);
        if(ret) {
            qDebug()<<"直线向量"<<line.Direction().X()<<line.Direction().Y()<<line.Direction().Z();

            error = GeneralTools::EvaluateLine(mData.first(),line);
        }

        break;
    }
    case GeometryType::PLANE:
    {
        gp_Pln plane;
        ret = GeneralTools::FitPlane(mData.first().toStdList(),plane);
        if(ret) {
            qDebug()<<"平面一点"<<plane.Location().X()<<plane.Location().Y()<<plane.Location().Z();
            qDebug()<<"平面法向"<<plane.Axis().Direction().X()<<plane.Axis().Direction().Y()<<plane.Axis().Direction().Z();

            error = GeneralTools::EvaluatePlane(mData.first(),plane);
        }
        break;
    }
    case GeometryType::CIRCLE:
    {
        gp_Circ circle;
        ret = GeneralTools::FitCicle(mData.first().toStdList(),circle,error);
        if(ret) {
            qDebug()<<"圆心"<<circle.Location().X()<<circle.Location().Y()<<circle.Location().Z();
            qDebug()<<"半径"<<circle.Radius();
            qDebug()<<"圆面法向"<<circle.Axis().Direction().X()<<circle.Axis().Direction().Y()<<circle.Axis().Direction().Z();
        }

        break;
    }
    case GeometryType::SPHERE:
    {
        gp_Sphere sphere;
        ret = GeneralTools::FitSphere(mData.first().toStdList(),sphere);
        if(ret) {
            qDebug()<<"球心"<<sphere.Location().X()<<sphere.Location().Y()<<sphere.Location().Z();
            qDebug()<<"半径"<<sphere.Radius();

            error = GeneralTools::EvaluateSphere(mData.first(),sphere);
        }

        break;
    }
    case GeometryType::CYLINDER:
    {
        gp_Cylinder cylinder;
        ret = GeneralTools::FitCylinder(mData.first(),cylinder);
        if(ret) {
            qDebug()<<"轴向"<<cylinder.Axis().Direction().X()<<cylinder.Axis().Direction().Y()<<cylinder.Axis().Direction().Z();
            qDebug()<<"半径"<<cylinder.Radius();

            error = GeneralTools::EvaluateCylinder(mData.first(),cylinder);
        }

        break;
    }
    case GeometryType::CONE:
    {
        break;
    }
    case GeometryType::DISTANCE:
    {
        Q_ASSERT(mData.size() == 2);
        QList<gp_Pnt> featA = mData.first();
        QList<gp_Pnt> featB = mData.last();

        gp_Cylinder cylinderA;
        gp_Cylinder cylinderB;
        ret = GeneralTools::FitCylinder(featA,cylinderA);
        ret &= GeneralTools::FitCylinder(featB,cylinderB);
        if(ret) {
            qDebug()<<"轴向"<<cylinderA.Axis().Direction().X()<<cylinderA.Axis().Direction().Y()<<cylinderA.Axis().Direction().Z();
            qDebug()<<"轴向"<<cylinderB.Axis().Direction().X()<<cylinderB.Axis().Direction().Y()<<cylinderB.Axis().Direction().Z();
            qDebug()<<"位置"<<cylinderA.Axis().Location().X()<<cylinderA.Axis().Location().Y()<<cylinderA.Axis().Location().Z();
            qDebug()<<"位置"<<cylinderB.Axis().Location().X()<<cylinderB.Axis().Location().Y()<<cylinderB.Axis().Location().Z();

            gp_Lin linA = cylinderA.Axis();
            gp_Pnt pA = cylinderA.Location();
            gp_Lin linB = cylinderB.Axis();
            gp_Pnt pB = cylinderB.Location();
            error = linA.Distance(pB) + linB.Distance(pA);
            error /= 2;

            double NM = QInputDialog::getDouble(nullptr,"输入","请输入理论值",0,0,999);

            mLabel->setData(error,LOWTol,UPTol,NM);
            return;
        }
        break;
    }

    break;
    }

    mLabel->setData(error,LOWTol,UPTol);
}

QString MeasureItem::getID() const
{
    if(mLabel) {
        return mLabel->Title();
    }
    else return QString();
}
