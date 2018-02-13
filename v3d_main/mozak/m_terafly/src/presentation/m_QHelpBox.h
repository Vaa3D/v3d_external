#ifndef QHELPBOX_H
#define QHELPBOX_H

#include "m_theader.h"

#include "m_CPlugin.h"

class teramanager::QHelpBox : public QWidget
{
    Q_OBJECT

    private:

        QLabel *helpBox;
        QLabel *helpIcon;
        QLabel *backgroundPanel;
        QWidget *parent;

    public:

        QHelpBox(QWidget *_parent);

        void setText(std::string text);
       // void setFixedHeight(int h){helpBox->setFixedHeight(h); backgroundPanel->setFixedHeight(h);}

        void setIconSize(int w, int h){helpIcon->setPixmap(helpIcon->pixmap()->scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));}


    signals:

    public slots:

};

#endif // QHELPBOX_H
