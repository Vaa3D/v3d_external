#ifndef FRAGMENT_GALLERY_WIDGET_PLUGIN_H
#define FRAGMENT_GALLERY_WIDGET_PLUGIN_H

#include <QObject>
#include <QtDesigner/QDesignerCustomWidgetInterface>
#include <QtPlugin>
#include "../FragmentGalleryWidget.h"

class FragmentGalleryWidgetPlugin
    : public QObject
    , public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    FragmentGalleryWidgetPlugin(QObject *parent = 0) 
        : QObject(parent)
        , m_initialized(false)
    {}

    bool isContainer() const {return false;}
    bool isInitialized() const {return m_initialized;}
    QIcon icon() const {return QIcon();} // TODO
    QString domXml() const {
        return "<ui language=\"c++\">"
                "  <widget class=\"FragmentGalleryWidget\" "
                "  name=\"fragmentGalleryWidget\" />"
                "</ui>";
    }
    QString group() const {return tr("Containers");}
    QString includeFile() const {return "../neuron_annotator/gui/FragmentGalleryWidget.h";}
    QString name() const {return "FragmentGalleryWidget";}
    QString toolTip() const {return tr("Widget used to display a collection neuron fragment thumbnails");}
    QString whatsThis() const {return tr("This is a widget for displaying neuron fragment thumbnails in NeuronAnnotator");}
    QWidget *createWidget(QWidget *parent) {return new FragmentGalleryWidget(parent);}
    void initialize(QDesignerFormEditorInterface *) {m_initialized = true;}

private:
    bool m_initialized;
};

#endif /* FRAGMENT_GALLERY_WIDGET_PLUGIN_H */

