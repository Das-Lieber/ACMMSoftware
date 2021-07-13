#ifndef ROBOT_H
#define ROBOT_H

#include <QList>

#include <XCAFPrs_AISObject.hxx>

class RobotLink;

class Robot
{
public:
    Robot();

    void SetDof(int dof) {
        DOF = dof;
    }

    QList<RobotLink*> Links() const {
        return RLinks;
    }

    gp_Pnt GetTCP() const {
        return TCP;
    }

    void SetLinkShapes(const QList<RobotLink*> &shapes) {
        RLinks = shapes;
    }

    void SetMotionAxis(const QList<gp_Ax1> &axis) {
        MotionAxis = axis;
    }

    void Assembel(const QList<gp_Trsf> &trsfs) {
        AssembelTrsfs = trsfs;
    }

    void SetCorrectAng(const QList<double> &angs) {
        correctAngs = angs;
    }

    void SetTCPTrsf(const gp_Trsf &trsf) {
        TCPTrsf = trsf;
    }

    void PerformFK(const QList<double> &angs);

private:
    int DOF;
    QList<RobotLink*> RLinks;
    QList<gp_Ax1> MotionAxis;
    QList<gp_Trsf> AssembelTrsfs;
    QList<double> correctAngs;
    gp_Trsf TCPTrsf;

    gp_Pnt TCP;
};

#endif // ROBOT_H
