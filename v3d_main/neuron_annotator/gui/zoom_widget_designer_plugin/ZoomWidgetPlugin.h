#ifndef ZOOM_WIDGET_PLUGIN_H
#define ZOOM_WIDGET_PLUGIN_H

#include <QObject>
#include <QtDesigner/QDesignerCustomWidgetInterface>
#include <QtPlugin>
#include "../ZoomWidget.h"

class ZoomWidgetPlugin
    : public QObject
    , public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    ZoomWidgetPlugin(QObject *parent = 0)
        : QObject(parent)
        , m_initialized(false)
    {}

    bool isContainer() const {return false;}
    bool isInitialized() const {return m_initialized;}
    QIcon icon() const {return QIcon();} // TODO
    QString domXml() const {
        return "<ui language=\"c++\">"
                "  <widget class=\"ZoomWidget\" "
                "  name=\"zoomWidget\" />"
                "</ui>";
    }
    QString group() const {return tr("Input Widgets");}
    QString includeFile() const {return "../neuron_annotator/gui/ZoomWidget.h";}
    QString name() const {return "ZoomWidget";}
    QString toolTip() const {return tr("Widget used to control relative zoom level");}
    QString whatsThis() const {return tr("This is a widget for setting zoom level in the NeuronAnnotator viewers");}
    QWidget *createWidget(QWidget *parent) {return new ZoomWidget(parent);}
    void initialize(QDesignerFormEditorInterface *) {m_initialized = true;}

private:
    bool m_initialized;
};

#endif /* ZOOM_WIDGET_PLUGIN_H */

