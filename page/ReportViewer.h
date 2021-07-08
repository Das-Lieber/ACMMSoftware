#pragma once

#include <QGraphicsView>

class LB_ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit LB_ImageViewer(QWidget *parent = nullptr);
    ~LB_ImageViewer();

    void SetPixmap(const QPixmap &map);

protected:

    virtual void resizeEvent(QResizeEvent * event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    QPixmap m_pixmap;
    QGraphicsPixmapItem *m_mapItem;
    QGraphicsScene *m_mapScene;

};