#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>

class QLabel;

enum class GeometryType {
    UNKNOWN,
    LINE,
    PLANE,
    CIRCLE,
    SPHERE,
    CYLINDER,
    CONE,
    DISTANCE
};

class LabelWidget : public QWidget
{
    friend class MeasureItem;
    Q_OBJECT
public:
    explicit LabelWidget(QWidget *parent);    

    bool getisDisplayed(){return isDisplayed;}
    void setisDisplayed(bool value) {isDisplayed = value;}

    QString Title() const;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:

    int m_id = -1;
    bool isDisplayed;

    QLabel                      *m_pLabPix;
    QLabel                      *m_pLabText;
    QLabel                      *m_pLabNM;//图纸的标注值
    QLabel                      *m_pLabMSTitle;
    QLabel                      *m_pLabMS;//实际值
    QLabel                      *m_pLabDVTitle;
    QLabel                      *m_pLabDV;//偏差值 

    QHBoxLayout                 *m_pLayValue;
    QVBoxLayout                 *pLayMain;

    bool                        mHasInitTol;

    bool                        mMoveing;
    QPoint                      mLastMousePosition ;

    void setID(int id) {
        m_id = id;
    }
    void setInfo(const GeometryType &type);
    void setData(double MS, double minE, double maxE, double NM=0);

    QWidget* HSpacer() {
        QWidget *aHSpacer = new QWidget;
        aHSpacer->setFixedHeight(1);
        aHSpacer->setStyleSheet("background:#000000");
        return aHSpacer;
    }

    QWidget* VSpacer() {
        QWidget *aVSpacer = new QWidget;
        aVSpacer->setFixedWidth(1);
        aVSpacer->setStyleSheet("background:#000000");
        return aVSpacer;
    }
};

#endif // LABELWIDGET_H
