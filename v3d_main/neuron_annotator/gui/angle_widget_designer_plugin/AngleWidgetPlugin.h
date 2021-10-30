#ifndef ANGLE_WIDGET_PLUGIN_H
#define ANGLE_WIDGET_PLUGIN_H

#include <QObject>
#include <QtDesigner/QDesignerCustomWidgetInterface>
#include <QtPlugin>
#include "../AngleWidget.h"

class AngleWidgetPlugin
    : public QObject
    , public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    AngleWidgetPlugin(QObject *parent = 0) 
        : QObject(parent)
        , m_initialized(false)
    {}

    bool isContainer() const {return false;}
    bool isInitialized() const {return m_initialized;}
    QIcon icon() const {return QIcon();} // TODO
    QString domXml() const {
        return "<ui language=\"c++\">"
                "  <widget class=\"AngleWidget\" "
                "  name=\"angleWidget\" />"
                "</ui>";
    }
    QString group() const {return tr("Input Widgets");}
    QString includeFile() const {return "../neuron_annotator/gui/AngleWidget.h";}
    QString name() const {return "AngleWidget";}
    QString toolTip() const {return tr("Widget used to control angles in degrees");}
    QString whatsThis() const {return tr("This is a widget for setting rotation angles in the NeuronAnnotator");}
    QWidget *createWidget(QWidget *parent) {return new AngleWidget(parent, "RotX");}
    void initialize(QDesignerFormEditorInterface *) {m_initialized = true;}

private:
    bool m_initialized;
};

#endif /* ANGLE_WIDGET_PLUGIN_H */

