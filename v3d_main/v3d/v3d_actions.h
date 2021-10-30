#ifndef V3D_ACTIONS_H_
#define V3D_ACTIONS_H_

// The purpose of this file is to decouple V3D action objects from
// the heavily weighted V3D MainWindow object.
// There remains a lot of work to do to complete this separation.

#include <QAction>
#include <QUrl>

namespace v3d {

/// Opens a web page in the user's web browser
class BrowseToWebPageAction : public QAction
{
    Q_OBJECT
public:
    BrowseToWebPageAction(const QString& text, const QUrl& url, QObject* parent);
signals:
    void page_open_failed();
private slots:
    void openWebPage();
private:
    QUrl _url;
};


/// Opens the main V3D web page in the user's web browser
class OpenV3dWebPageAction : public BrowseToWebPageAction
{
public:
    OpenV3dWebPageAction(QObject* parent);
};


/// Show the "About V3D" dialog
class ShowV3dAboutDialogAction : public QAction
{
    Q_OBJECT
public:
    ShowV3dAboutDialogAction(QWidget* parent);
private slots:
    void show_dialog();
};


} // namespace v3d

#endif /* V3D_ACTIONS_H_ */
