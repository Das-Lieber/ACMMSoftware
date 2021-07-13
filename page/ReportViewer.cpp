#include "ReportViewer.h"

#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

LB_ImageViewer::LB_ImageViewer(QWidget *parent)
    : QGraphicsView(parent),
      m_mapItem(nullptr)
{
    m_mapScene = new QGraphicsScene(this);
    this->setScene(m_mapScene);
    this->setBackgroundBrush(QBrush(QColor(128,128,128,255),Qt::SolidPattern));
    this->setDragMode(NoDrag);
}

LB_ImageViewer::~LB_ImageViewer()
{
    if(m_mapItem) {
        delete m_mapItem;
        m_mapItem = nullptr;
    }
}

void LB_ImageViewer::SetPixmap(const QPixmap &map)
{
    m_pixmap = map;

    if(!m_mapItem) {
        m_mapItem = m_mapScene->addPixmap(m_pixmap);
    }
    else {
        m_mapItem->setPixmap(map);
    }

    m_mapScene->setSceneRect(m_pixmap.rect());
    this->setDragMode(ScrollHandDrag);
}

void LB_ImageViewer::resizeEvent(QResizeEvent *event)
{
    if(!m_mapItem)
        return;

    QGraphicsView::resizeEvent(event);
    this->centerOn(m_mapItem);
}

void LB_ImageViewer::wheelEvent(QWheelEvent *event)
{
    if(!m_mapItem)
        return;

    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    qreal factor = 1.1;

    if(event->delta() < 0)
        factor = 1.0 / factor;

    this->scale(factor,factor);
    event->accept();
}
