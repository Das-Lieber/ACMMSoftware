#include "RobotLink.h"

RobotLink::RobotLink()
{
}

void RobotLink::ApplyTrsf(const gp_Trsf &trsf)
{
    for(int i=0;i<RShapes.size();++i) {
        RShapes[i]->SetLocalTransformation(trsf);
    }
}
