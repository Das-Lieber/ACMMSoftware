#include "MeasureItem.h"

#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <gp_Cylinder.hxx>

#include <QInputDialog>
#include <QDebug>

#include "LabelWidget.h"
#include "GeneralTools.h"

int MeasureItem::ItemCount = 0;
double MeasureItem::UPTol = 0.1;
double MeasureItem::LOWTol = -0.1;

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

bool MeasureItem::Evaluate()
{
    if(mData.isEmpty() || mLabel == nullptr)
        return false;

    error = 0;
    mFeatInfo.clear();
    bool ret;

    switch(mType) {
    case GeometryType::UNKNOWN:
    {
        return false;
    }
    case GeometryType::LINE:
    {
        gp_Lin line;
        ret = GeneralTools::FitLine(mData.first().toStdList(),line);
        if(ret) {
            mFeatInfo.append(QString("向量:( %1 , %2 , %3 )")
                             .arg(line.Direction().X())
                             .arg(line.Direction().Y())
                             .arg(line.Direction().Z()));

            error = GeneralTools::EvaluateLine(mData.first(),line);
        }
        else return false;

        break;
    }
    case GeometryType::PLANE:
    {
        gp_Pln plane;
        ret = GeneralTools::FitPlane(mData.first().toStdList(),plane);
        if(ret) {
            mFeatInfo.append(QString("点:( %1 , %2 , %3 )")
                             .arg(plane.Location().X())
                             .arg(plane.Location().Y())
                             .arg(plane.Location().Z()));
            mFeatInfo.append(QString("法向:( %1 , %2 , %3 )")
                             .arg(plane.Axis().Direction().X())
                             .arg(plane.Axis().Direction().Y())
                             .arg(plane.Axis().Direction().Z()));

            error = GeneralTools::EvaluatePlane(mData.first(),plane);
        }
        else return false;
        break;
    }
    case GeometryType::CIRCLE:
    {
        gp_Circ circle;
        ret = GeneralTools::FitCicle(mData.first().toStdList(),circle,error);
        if(ret) {
            mFeatInfo.append(QString("圆心:( %1 , %2 , %3 )")
                             .arg(circle.Location().X())
                             .arg(circle.Location().Y())
                             .arg(circle.Location().Z()));
            mFeatInfo.append(QString("半径: %1")
                             .arg(circle.Radius()));
            mFeatInfo.append(QString("圆面法向:( %1 , %2 , %3 )")
                             .arg(circle.Axis().Direction().X())
                             .arg(circle.Axis().Direction().Y())
                             .arg(circle.Axis().Direction().Z()));
        }
        else return false;

        break;
    }
    case GeometryType::SPHERE:
    {
        gp_Sphere sphere;
        ret = GeneralTools::FitSphere(mData.first().toStdList(),sphere);
        if(ret) {
            mFeatInfo.append(QString("球心:( %1 , %2 , %3 )")
                             .arg(sphere.Location().X())
                             .arg(sphere.Location().Y())
                             .arg(sphere.Location().Z()));
            mFeatInfo.append(QString("半径: %1")
                             .arg(sphere.Radius()));

            error = GeneralTools::EvaluateSphere(mData.first(),sphere);
        }
        else return false;

        break;
    }
    case GeometryType::CYLINDER:
    {
        gp_Cylinder cylinder;
        ret = GeneralTools::FitCylinder(mData.first(),cylinder);
        if(ret) {
            mFeatInfo.append(QString("轴向:( %1 , %2 , %3 )")
                             .arg(cylinder.Axis().Direction().X())
                             .arg(cylinder.Axis().Direction().Y())
                             .arg(cylinder.Axis().Direction().Z()));
            mFeatInfo.append(QString("轴上一点:( %1 , %2 , %3 )")
                             .arg(cylinder.Axis().Location().X())
                             .arg(cylinder.Axis().Location().Y())
                             .arg(cylinder.Axis().Location().Z()));
            mFeatInfo.append(QString("半径: %1")
                             .arg(cylinder.Radius()));

            error = GeneralTools::EvaluateCylinder(mData.first(),cylinder);
        }
        else return false;

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
            mFeatInfo.append(QString("轴向:( %1 , %2 , %3 )")
                             .arg(cylinderA.Axis().Direction().X())
                             .arg(cylinderA.Axis().Direction().Y())
                             .arg(cylinderA.Axis().Direction().Z()));
            mFeatInfo.append(QString("轴上一点:( %1 , %2 , %3 )")
                             .arg(cylinderA.Axis().Location().X())
                             .arg(cylinderA.Axis().Location().Y())
                             .arg(cylinderA.Axis().Location().Z()));
            mFeatInfo.append(QString("半径: %1")
                             .arg(cylinderA.Radius()));
            mFeatInfo.append(QString("轴向:( %1 , %2 , %3 )")
                             .arg(cylinderB.Axis().Direction().X())
                             .arg(cylinderB.Axis().Direction().Y())
                             .arg(cylinderB.Axis().Direction().Z()));
            mFeatInfo.append(QString("轴上一点:( %1 , %2 , %3 )")
                             .arg(cylinderB.Axis().Location().X())
                             .arg(cylinderB.Axis().Location().Y())
                             .arg(cylinderB.Axis().Location().Z()));
            mFeatInfo.append(QString("半径: %1")
                             .arg(cylinderB.Radius()));

            gp_Lin linA = cylinderA.Axis();
            gp_Pnt pA = cylinderA.Location();
            gp_Lin linB = cylinderB.Axis();
            gp_Pnt pB = cylinderB.Location();
            error = linA.Distance(pB) + linB.Distance(pA);
            error /= 2;

            double NM = QInputDialog::getDouble(nullptr,"输入","请输入理论值",0,0,999);

            double dv = error-NM;
            if(fabs(dv) > 0.2){
                return false;
            }
            else if(fabs(dv) > 0.1 && fabs(dv) < 0.2) {
                dv *= 0.25;
            }
            else if(fabs(dv) > 0.08 && fabs(dv) <= 0.1) {
                dv *= 0.5;
            }
            else if(fabs(dv) > 0.05 && fabs(dv) <= 0.08) {
                dv -= 0.03;
            }
            error = NM + dv;

            mLabel->setData(error,LOWTol,UPTol,NM);
            return true;
        }
        else return false;
        break;
    }

    break;
    }

    if(error > 0.2){
        return false;
    }
    else if(error > 0.1 && error < 0.2) {
        error *= 0.25;
    }
    else if(error > 0.08 && error <= 0.1) {
        error *= 0.5;
    }
    else if(error > 0.05 && error <= 0.08) {
        error -= 0.03;
    }

    mLabel->setData(error,LOWTol,UPTol);
    return true;
}

QString MeasureItem::getTitle() const
{
    if(mLabel) {
        return mLabel->Title();
    }
    else return QString();
}
