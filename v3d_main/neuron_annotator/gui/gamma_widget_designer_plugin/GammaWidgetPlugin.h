#ifndef GAMMA_WIDGET_PLUGIN_H
#define GAMMA_WIDGET_PLUGIN_H

#include <QObject>
#include <QtDesigner/QDesignerCustomWidgetInterface>
#include <QtPlugin>
#include "../GammaWidget.h"

class GammaWidgetPlugin
    : public QObject
    , public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    GammaWidgetPlugin(QObject *parent = 0)
        : QObject(parent)
        , m_initialized(false)
    {}

    bool isContainer() const {return false;}
    bool isInitialized() const {return m_initialized;}
    QIcon icon() const {return QIcon();} // TODO
    QString domXml() const {
        return "<ui language=\"c++\">"
                "  <widget class=\"GammaWidget\" "
                "  name=\"angleWidget\" />"
                "</ui>";
    }
    QString group() const {return tr("Input Widgets");}
    QString includeFile() const {return "../neuron_annotator/gui/GammaWidget.h";}
    QString name() const {return "GammaWidget";}
    QString toolTip() const {return tr("Widget used to adjust image brightness");}
    QString whatsThis() const {return tr("This is a widget for adjusting viewer brightness in the NeuronAnnotator");}
    QWidget *createWidget(QWidget *parent) {return new GammaWidget(parent);}
    void initialize(QDesignerFormEditorInterface *) {m_initialized = true;}

private:
    bool m_initialized;
};

#endif /* GAMMA_WIDGET_PLUGIN_H */

