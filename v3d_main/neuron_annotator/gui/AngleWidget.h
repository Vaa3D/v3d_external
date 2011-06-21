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

    AngleWidget(QWidget * parent = NULL, QString angleLabelText = "Rot")
        : QWidget(parent)
    {
        setupUi(this);
        label->setText(angleLabelText);
        // dial and spinbox are already linked to one another, so we only need to
        // connect to the signals of one of them.
        connect(spinBox, SIGNAL(valueChanged(int)),
                this, SIGNAL(angleChanged(int)));
    }

    QString textLabel() const {return label->text();}
    void setTextLabel(QString l) {label->setText(l);}

signals:
    void angleChanged(int angleInDegrees);

public slots:
    void setAngle(qreal angle) {
        while (angle >= 360) angle -= 360.0;
        while (angle < 0) angle += 360.0;
        int intAngle = (int)(angle + 0.4999);
        spinBox->setValue(intAngle);
    }
    void reset() {setAngle(0);}
};

#endif // ANGLEWIDGET_H
