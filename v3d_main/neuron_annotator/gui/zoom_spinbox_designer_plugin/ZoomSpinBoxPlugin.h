#ifndef ZOOM_SPINBOX_PLUGIN_H
#define ZOOM_SPINBOX_PLUGIN_H

#include <QObject>
#include <QtDesigner/QDesignerCustomWidgetInterface>
#include <QtPlugin>
#include "../ZoomSpinBox.h"

class ZoomSpinBoxPlugin
    : public QObject
    , public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    ZoomSpinBoxPlugin(QObject *parent = 0)
        : QObject(parent)
        , m_initialized(false)
    {}

    bool isContainer() const {return false;}
    bool isInitialized() const {return m_initialized;}
    QIcon icon() const {return QIcon();} // TODO
    QString domXml() const {
        return "<ui language=\"c++\">"
                "  <widget class=\"ZoomSpinBox\" "
                "  name=\"zoomSpinBox\" />"
                "</ui>";
    }
    QString group() const {return tr("Input Widgets");}
    QString includeFile() const {return "../neuron_annotator/gui/ZoomSpinBox.h";}
    QString name() const {return "ZoomSpinBox";}
    QString toolTip() const {return tr("Widget used to control relative zoom level");}
    QString whatsThis() const {return tr("This is a widget for setting zoom level in the NeuronAnnotator viewers");}
    QWidget *createWidget(QWidget *parent) {return new ZoomSpinBox(parent);}
    void initialize(QDesignerFormEditorInterface *) {m_initialized = true;}

private:
    bool m_initialized;
};

#endif /* ZOOM_SPINBOX_PLUGIN_H */

