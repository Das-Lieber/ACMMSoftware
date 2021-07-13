#include "Robot.h"

#include <QDebug>

#include "occ/RobotLink.h"
#include "occ/modelmaker.h"

Robot::Robot()
{
}

void Robot::PerformFK(const QList<double> &angs)
{
    TCP = gp_Pnt(0,0,0);
    gp_Trsf CumulativeTrsf;//set an Cumulative Trsf to change the axis after the current one
    for(int i=0;i<MotionAxis.size();++i)
    {
        gp_Trsf aTrsf;
        double angle = angs[i]+correctAngs[i];

        if(angle < -360) angle += 360;
        if(angle > 0) angle -= 360;
        angle += 360;

        aTrsf.SetRotation(MotionAxis.at(i),angle*M_PI/180);

        CumulativeTrsf.Multiply(aTrsf);

        gp_Trsf workTrsf;//the real work Trsf ,E * Cumulative * Assemble
        workTrsf.Multiply(CumulativeTrsf);
        workTrsf.Multiply(AssembelTrsfs[i]);

        //when u change here ,u needn't to display them again in the main function, update the occWidget is enough
        RLinks[i+1]->ApplyTrsf(workTrsf);
    }

    gp_Trsf aTrsf;
    aTrsf.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)),0);

    CumulativeTrsf.Multiply(aTrsf);

    gp_Trsf workTrsf;
    workTrsf.Multiply(CumulativeTrsf);
    workTrsf.Multiply(TCPTrsf);

    TCP.Transform(workTrsf);
}
