#ifndef ZOOMSPINBOX_H
#define ZOOMSPINBOX_H

#include <QSpinBox>

// ZoomSpinBox shows a positive floating point zoom value rather than an integer
// Underlying integer is log(double_val) * 100
class ZoomSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit ZoomSpinBox(QWidget *parent = 0);

signals:
    void zoomValueChanged(qreal zoomValue);

public slots:
    void setZoomValue(qreal doubleValue);
    void onValueChanged(int intValue);
    void reset();

protected:
    virtual QString textFromValue(int value) const;
    virtual int valueFromText(const QString & text) const;
    virtual QValidator::State validate(QString & text, int & pos) const;
    static int intValueFromDoubleValue(qreal doubleValue);
    static qreal doubleValueFromIntValue(int intValue);
    QDoubleValidator validator;
};

#endif // ZOOMSPINBOX_H
