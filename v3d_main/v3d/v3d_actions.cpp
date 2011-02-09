#include "v3d_actions.h"

#include <QUrl>
#include <QDesktopServices>
#include <QTextEdit>
#include "v3d_version_info.h"

namespace v3d {

BrowseToWebPageAction::BrowseToWebPageAction(
        const QString& text,
        const QUrl& url,
        QObject* parent)
    : QAction(text, parent) , _url(url)
{
    connect(this, SIGNAL(triggered()),
            this, SLOT(openWebPage()));
}

void BrowseToWebPageAction::openWebPage() {
    bool success = QDesktopServices::openUrl(_url);
    if (! success )
        emit page_open_failed();
}


OpenV3dWebPageAction::OpenV3dWebPageAction(QObject* parent)
        : BrowseToWebPageAction(tr("Go to V3D web site..."),
                QUrl("http://penglab.janelia.org/proj/v3d/"),
                parent)
{}


ShowV3dAboutDialogAction::ShowV3dAboutDialogAction(QWidget* parent)
        : QAction(QIcon(":/pic/help.png"), tr("Help Info and &About"), parent)
{
    setStatusTip(tr("Show help and version information"));
    connect(this, SIGNAL(triggered()), this, SLOT(show_dialog()));
}

void ShowV3dAboutDialogAction::show_dialog() {
    v3d_aboutinfo();
}

} // namespace v3d
