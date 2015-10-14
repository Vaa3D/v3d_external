#ifndef MOUSECLICKMANAGER_H
#define MOUSECLICKMANAGER_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QMouseEvent>
#include <QPoint>

// MouseClickManager helps QWidgets distinguish between single clicks and double clicks.
class MouseClickManager : public QObject
{
    Q_OBJECT
public:
    explicit MouseClickManager(QObject *parent = 0);
    // Call these methods when your widget gets the corresponding event
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseDoubleClickEvent(QMouseEvent * event);

signals:
    // Connect to singleClick(QPoint) signal to get clean single click events
    void singleClick(QPoint);
    void possibleSingleClickAlert(); // To help provide instant visual feedback
    void notSingleClick();

public slots:

protected slots:
    void onClickTimerTimedOut();

protected:
    void abortSingleClick();

    QPoint mousePressPosition;
    QTimer singleClickTimer;
    QTime mousePressTime;
    int mousePressInterval; // time between presses in milliseconds
};

#endif // MOUSECLICKMANAGER_H
