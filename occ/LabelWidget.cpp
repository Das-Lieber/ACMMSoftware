#include "LabelWidget.h"

#include <QLabel>

#include "occview.h"

LabelWidget::LabelWidget(QWidget *parent)
    : QWidget(parent)
    ,mHasInitTol(false)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setStyleSheet("background-color:#FFFFFF;");

    pLayMain = new QVBoxLayout;

    m_pLabPix = new QLabel;
    m_pLabText = new QLabel(tr(""));

    QHBoxLayout *m_pLayLabel = new QHBoxLayout;
    m_pLayLabel->addWidget(m_pLabPix);
    m_pLayLabel->addWidget(m_pLabText);

    pLayMain->addLayout(m_pLayLabel);
    pLayMain->setSpacing(0);
    pLayMain->setContentsMargins(0, 0, 0, 0);
    pLayMain->setMargin(0);

    setLayout(pLayMain);
    isDisplayed = false;

    m_pLayValue = new QHBoxLayout;
    m_pLabNM = new QLabel;
    m_pLabMS = new QLabel;    
    m_pLabMSTitle = new QLabel;
    m_pLabDV = new QLabel;
    m_pLabDVTitle = new QLabel;

    m_pLabDVTitle->setText("偏差:");
    m_pLabMSTitle->setText("实测值:");
}

void LabelWidget::setInfo(const GeometryType &type)
{
    switch(type) {
    case GeometryType::UNKNOWN:
        break;
    case GeometryType::LINE:
    {
        m_pLabPix->setPixmap(QPixmap(":/res/tolerance/Linearity.png").scaled(20,20));
        m_pLabText->setText("直线度");
        break;
    }
    case GeometryType::PLANE:
    {
        m_pLabPix->setPixmap(QPixmap(":/res/tolerance/Planarity.png").scaled(20,20));
        m_pLabText->setText("平面度");
        break;
    }
    case GeometryType::CIRCLE:
    {
        m_pLabPix->setPixmap(QPixmap(":/res/tolerance/Circularity.png").scaled(20,20));
        m_pLabText->setText("圆度");
        break;
    }
    case GeometryType::SPHERE:
    {
        m_pLabPix->setPixmap(QPixmap(":/res/tolerance/Sphericity.png").scaled(20,20));
        m_pLabText->setText("球度");
        break;
    }
    case GeometryType::CYLINDER:
    {
        m_pLabPix->setPixmap(QPixmap(":/res/tolerance/Cylindricity.png").scaled(20,20));
        m_pLabText->setText("圆柱度");
        break;
    }
    case GeometryType::CONE:
    {
        m_pLabPix->setPixmap(QPixmap(":/res/tolerance/Conicity.png").scaled(20,20));
        m_pLabText->setText("锥度");
        break;
    }
    case GeometryType::DISTANCE:
    {
        m_pLabPix->setPixmap(QPixmap(":/res/tolerance/Position.png").scaled(20,20));
        m_pLabText->setText("距离");
        break;
    }
    }
    m_pLabText->setText(m_pLabText->text().append(QString::number(m_id)));
}

void LabelWidget::setData(double MS, double minE, double maxE, double NM)
{
    if( qFuzzyCompare(minE,maxE) ) //未传入有效的公差值
        return;

    double DV = MS-NM; // deviation

    if(!qFuzzyIsNull(NM))//理论值不为0，位置公差
    {
        m_pLabNM->setText("理论值:"+QString::number(NM,'f'));
        m_pLabDV->setText(QString::number(DV,'f'));
        m_pLabMS->setText(QString::number(MS,'f'));
        // 根据公差带设置底色
        if(0.95*minE<DV && DV<0.95*maxE)
            m_pLabDV->setStyleSheet("background-color:#00FF00;");
        else if(MS>1.05*maxE || MS<1.05*minE)
            m_pLabDV->setStyleSheet("background-color:#FF0000;");
        else
            m_pLabDV->setStyleSheet("background-color:#FFFF00;");

        if(!mHasInitTol)
        {
            // 1.第二行添加理论值
            pLayMain->addWidget(HSpacer());
            pLayMain->addWidget(m_pLabNM);
            pLayMain->addWidget(HSpacer());

            // 3.第三行第一列添加偏差
            QVBoxLayout *singleDV = new QVBoxLayout;
            singleDV->addWidget(m_pLabDVTitle);
            singleDV->addWidget(HSpacer());
            singleDV->addWidget(m_pLabDV);
            m_pLayValue->addLayout(singleDV);

            // 2.第三行第二列添加实测值
            QVBoxLayout *singleMS = new QVBoxLayout;
            singleMS->addWidget(m_pLabMSTitle);
            singleMS->addWidget(HSpacer());
            singleMS->addWidget(m_pLabMS);
            m_pLayValue->addWidget(VSpacer());
            m_pLayValue->addLayout(singleMS);
        }
    }
    else {
        // 形状公差
        m_pLabMS->setText(QString::number(MS,'f'));
        if(0.95*minE<DV && DV<0.95*maxE)
            m_pLabMS->setStyleSheet("background-color:#00FF00;");
        else if(MS>1.05*maxE || MS<1.05*minE)
            m_pLabMS->setStyleSheet("background-color:#FF0000;");
        else
            m_pLabMS->setStyleSheet("background-color:#FFFF00;");

        if(!mHasInitTol)
        {
            pLayMain->addWidget(HSpacer());

            QVBoxLayout *singleMS = new QVBoxLayout;
            singleMS->addWidget(m_pLabMSTitle);
            singleMS->addWidget(HSpacer());
            singleMS->addWidget(m_pLabMS);
            m_pLayValue->addLayout(singleMS);
        }
    }

    pLayMain->addLayout(m_pLayValue);
    mHasInitTol = true;
}

QString LabelWidget::Title() const
{
    return m_pLabText->text();
}

void LabelWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMoveing = true;
        mLastMousePosition = event->globalPos();
    }
}
void LabelWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        occView *pParentWidget = qobject_cast<occView *>(parent());
        int parentWidth = pParentWidget->width();
        int parentHeight = pParentWidget->height();
        QPoint gPos = pParentWidget->mapToGlobal(QPoint(0, 0));
        bool bInRange = (event->globalPos().x() > gPos.x() && event->globalPos().x() < (gPos.x() + parentWidth))
                && (event->globalPos().y() > gPos.y() && event->globalPos().y() < (gPos.y() + parentHeight)
                && ( QApplication::keyboardModifiers () != Qt::ControlModifier));
        if (bInRange)
        {
            move(pos() + (event->globalPos()- mLastMousePosition));
            mLastMousePosition  = event->globalPos();
        }
    }
}
void LabelWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMoveing = false;
    }
}
