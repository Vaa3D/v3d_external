#ifndef COMPARTMENTMAPCOMBOBOX_H
#define COMPARTMENTMAPCOMBOBOX_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>
#include <QtGui>

// Define a class for selecting compartments
// interactive select compartment
class CompartmentMapComboBox: public QComboBox
{
    Q_OBJECT

public:
    CompartmentMapComboBox(QWidget *widget = 0);
    virtual ~CompartmentMapComboBox();
    bool eventFilter(QObject *object, QEvent *event);
    virtual void paintEvent(QPaintEvent *);
    void SetDisplayText(QString text);
    QString GetDisplayText() const;
    virtual void wheelEvent ( QWheelEvent * e );
    
private:
    QString m_DisplayText;
    int indexSelected;
};


#endif // COMPARTMENTMAPCOMBOBOX_H
