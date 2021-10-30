#ifndef ANGLEWIDGET_H
#define ANGLEWIDGET_H

#include <QWidget>
#include "ui_AngleWidget.h"

/* Include QDesignerExportWidget only if we're exporting
   the widget as a designer plugin.
*/
#ifdef _DESIGNER_EXPORT
#include <QDesignerExportWidget>
#else
#define QDESIGNER_WIDGET_EXPORT
#endif

class QDESIGNER_WIDGET_EXPORT AngleWidget : public QWidget, public Ui::AngleWidget
{
    Q_OBJECT

public:

    /* These properties are accessible by Qt Designer */
    Q_PROPERTY(QString textLabel READ textLabel WRITE setTextLabel);

    explicit AngleWidget(QWidget * parentObject = NULL, QString angleLabelText = "Rot");
    QString textLabel() const;
    void setTextLabel(QString l);

signals:
    void angleChanged(int angleInDegrees);

public slots:
    void setAngle(qreal angle);
    void reset();
};

#endif // ANGLEWIDGET_H
