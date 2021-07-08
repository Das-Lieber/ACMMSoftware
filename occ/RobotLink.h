#ifndef ROBOTLINK_H
#define ROBOTLINK_H

#include <QList>

#include <XCAFPrs_AISObject.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>

class RLAPI_Reader;

class RobotLink
{
    friend class RLAPI_Reader;

public:
    RobotLink();

    QList<Handle(XCAFPrs_AISObject)> Shapes() const {
        return RShapes;
    }

    void SetColors(const QList<Quantity_Color> &colors) {
        RColors = colors;
    }

    void ApplyTrsf(const gp_Trsf &trsf);

private:
    QList<Handle(XCAFPrs_AISObject)> RShapes;
    QList<Quantity_Color> RColors;
};

#endif // ROBOTLINK_H
