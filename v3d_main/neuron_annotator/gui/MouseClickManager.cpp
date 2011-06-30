#include "MouseClickManager.h"
#include <QMouseEvent>
#include <QDebug>

MouseClickManager::MouseClickManager(QObject *parent)
    : QObject(parent)
    , mousePressInterval(5000)
{
    singleClickTimer.setSingleShot(true);
    connect(&singleClickTimer, SIGNAL(timeout()),
            this, SLOT(onClickTimerTimedOut()));
    mousePressTime.start();
}

void MouseClickManager::mousePressEvent(QMouseEvent * event)
{
    // qDebug() << "press";
    abortSingleClick(); // void any pending single click events
    if(event->button() != Qt::LeftButton) return; // want left click only
    mousePressInterval = mousePressTime.elapsed(); // remember time to *previous* click
    mousePressTime.restart();
    // qDebug() << event->pos();
    mousePressPosition = event->pos();
}

void MouseClickManager::mouseReleaseEvent(QMouseEvent * event)
{
    // qDebug() << "release";
    // Could this be a single click event?
    abortSingleClick();
    if(event->button() != Qt::LeftButton) return; // want left click only
    // qDebug() << "left button";
    // qDebug() << "mousePressInterval = " << mousePressInterval;
    if(mousePressInterval < 800) return; // looks more like a double click than like a single click
    int clickInterval = mousePressTime.elapsed(); // milliseconds
    // qDebug() << "clickInterval = " << clickInterval;
    // if (clickInterval < 2) return; // nobody clicks that fast // not sure why this is happening
    if (clickInterval > 1500) return; // too slow, that's not a click
    QPoint dv = event->pos() - mousePressPosition;
    // qDebug() << event->pos() << ", " << mousePressPosition;
    // qDebug() << "drag distance = " << dv.manhattanLength();
    if (dv.manhattanLength() > 2) return; // That's a drag, not a click
    // Got this far?  This might be a clean single click!
    // (as long as no more clicks come too quickly)
    // qDebug() << "might be click";
    singleClickTimer.start(400); // You must survive 400 milliseconds more to be annointed a true click.
    emit possibleSingleClickAlert(); // for instant feedback, if needed
}

void MouseClickManager::mouseDoubleClickEvent(QMouseEvent * event)
{
    // qDebug() << "double click";
    // Seems like we miss one mouse press event when double click occurs
    abortSingleClick();
    mousePressInterval = mousePressTime.elapsed();
    mousePressTime.restart();
}

void MouseClickManager::abortSingleClick()
{
    if (singleClickTimer.isActive()) {
        // qDebug() << "abort single click";
        singleClickTimer.stop();
        emit notSingleClick();
    }
}

void MouseClickManager::onClickTimerTimedOut()
{
    // qDebug() << "signal single click";
    emit singleClick(mousePressPosition);
}
