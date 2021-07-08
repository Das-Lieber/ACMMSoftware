#ifndef PCA_H
#define PCA_H

#include <list>
#include <vector>
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"
class PCA
{
public:
    //PCA();
    PCA(const std::list<gp_Pnt>& aPts);
    bool Run();
    //获取特征值
    std::vector<float> GetEigenvalue();
    //获取特征向量
    std::vector<gp_Vec> GetEigenVector();
    //获取中心点
    gp_Pnt GetCenterPoint();
private:
    const std::list<gp_Pnt>* _pPtsList;
    std::vector<float> _eigenValues;
    std::vector<gp_Vec> _eigenVectors;//特征向量
    gp_Pnt _ptCenter;
};

#endif // PCA_H
