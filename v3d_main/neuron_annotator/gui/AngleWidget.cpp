#include "AngleWidget.h"

/* explicit */
AngleWidget::AngleWidget(QWidget * parentObject /* = NULL */, QString angleLabelText /* = "Rot" */)
    : QWidget(parentObject)
{
    setupUi(this);
    label->setText(angleLabelText);
    // dial and spinbox are already linked to one another, so we only need to
    // connect to the signals of one of them.
    connect(spinBox, SIGNAL(valueChanged(int)),
            this, SIGNAL(angleChanged(int)));
}

QString AngleWidget::textLabel() const {return label->text();}

void AngleWidget::setTextLabel(QString l) {label->setText(l);}

void AngleWidget::setAngle(qreal angle)
{
    while (angle >= 360) angle -= 360.0;
    while (angle < 0) angle += 360.0;
    int intAngle = (int)(angle + 0.4999);
    spinBox->setValue(intAngle);
}

void AngleWidget::reset() {setAngle(0);}


