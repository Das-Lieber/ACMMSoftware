#include "pca.h"
#include <math_Matrix.hxx>
#include "Standard_Stream.hxx"
#include <math_Vector.hxx>
#include <math_Jacobi.hxx>
#include "math_Jacobi.hxx"
PCA::PCA(const std::list<gp_Pnt>& aPts)
{
    _pPtsList=&aPts;
    _eigenValues.reserve(3);
    _eigenVectors.reserve(3);
}
bool PCA::Run()
{//进行pca分析
    if(_pPtsList==NULL)return false;
    int nPts=_pPtsList->size();
    std::list<gp_Pnt> newdata;
    {//算出xyz的平均值,生成新的数据
        float sumX,sumY,sumZ;
        sumX=sumY=sumZ=0;
        for (std::list<gp_Pnt>::const_iterator it=_pPtsList->begin();it!=_pPtsList->end();it++)
        {
            sumX+=(*it).X();
            sumY+=(*it).Y();
            sumZ+=(*it).Z();
        }
        //求平均值
        sumX/=nPts;
        sumY/=nPts;
        sumZ/=nPts;
        _ptCenter.SetCoord(sumX,sumY,sumZ);//中心点

        gp_Pnt pttmp;
        for(std::list<gp_Pnt>::const_iterator it=_pPtsList->begin();it!=_pPtsList->end();it++)
        {//每个点减去平均值
            pttmp.SetCoord((*it).X()-sumX,(*it).Y()-sumY,(*it).Z()-sumZ);
            newdata.push_back(pttmp);
        }
    }
    math_Matrix covMatrix(1, 3, 1,3);
    {//生成协方差矩阵并计算特征值
        float covxx,covxy,covxz,covyy,covyz,covzz;
        covxx=covxy=covxz=covyy=covyz=covzz=0;
        for (std::list<gp_Pnt>::iterator it=newdata.begin();it!=newdata.end();it++)
        {
            covxx+=it->X()*it->X();
            covxy+=it->X()*it->Y();
            covxz+=it->X()*it->Z();
            covyy+=it->Y()*it->Y();
            covyz+=it->Y()*it->Z();
            covzz+=it->Z()*it->Z();
        }
        covxx/=(nPts-1);
        covxy/=(nPts-1);
        covxz/=(nPts-1);
        covyy/=(nPts-1);
        covyz/=(nPts-1);
        covzz/=(nPts-1);
        covMatrix(1, 1) = covxx;  covMatrix(1, 2) = covxy;covMatrix(1,3)=covxz;
        covMatrix(2, 1) = covxy;  covMatrix(2, 2) = covyy;covMatrix(2,3)=covyz;
        covMatrix(3, 1) = covxz;  covMatrix(3, 2) = covyz;covMatrix(3,3)=covzz;
    }
    {//根据协方差矩阵求特征值和特征向量
        _eigenValues.clear();
        _eigenVectors.clear();
        math_Jacobi J(covMatrix);
        if (J.IsDone())
        {
            for (Standard_Integer i = covMatrix.LowerRow(); i <= covMatrix.UpperRow(); ++i)
            {
                math_Vector V(1, covMatrix.RowNumber());
                J.Vector(i, V);
                gp_Vec tmpVec(V.Value(1),V.Value(2),V.Value(3));
                _eigenVectors.push_back(tmpVec);
                _eigenValues.push_back(J.Value(i));
            }
        }
    }
    if(_eigenValues.size()==3&&_eigenVectors.size()==3) return true;
    return false;

}

std::vector<gp_Vec> PCA::GetEigenVector()
{
    return _eigenVectors;
}
std::vector<float> PCA::GetEigenvalue()
{
    return _eigenValues;
}
gp_Pnt PCA::GetCenterPoint()
{
    return _ptCenter;
}
