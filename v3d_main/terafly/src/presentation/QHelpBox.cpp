#include "QHelpBox.h"

using namespace terafly;

QHelpBox::QHelpBox(QWidget *_parent) : QWidget(_parent)
{
    parent = _parent;

    backgroundPanel = new QLabel();
    backgroundPanel->setStyleSheet("border: 1px solid; border-color: rgb(71,127,249); background-color: rgb(245,245,245); margin-top:0px; "
                                               "margin-bottom:0px; margin-left: 0px; padding-right: 4px; padding-top:4px; padding-bottom:4px; padding-left:60px;"
                           "");

    helpBox = new QLabel();

    #ifdef Q_OS_LINUX
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(9);
    helpBox->setFont(tinyFont);
    #endif

    helpBox->setStyleSheet("margin-top:0px; margin-bottom:0px; margin-left: 0px; padding-right: 4px; padding-top:4px; padding-bottom:4px; padding-left:60px; text-align:justify;");
    helpBox->setWordWrap(true);    
    helpBox->setTextFormat(Qt::RichText);
    helpBox->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    helpBox->setOpenExternalLinks(true);
    QPixmap pixmap(":/icons/help2.png");
    helpIcon = new QLabel();
    helpIcon->setPixmap(pixmap.scaled(50,50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    helpIcon->setStyleSheet("margin-left: 5px");

    QStackedLayout *helpLayout = new QStackedLayout();
    helpLayout->addWidget(backgroundPanel);
    helpLayout->addWidget(helpIcon);
    helpLayout->addWidget(helpBox);
    helpLayout->setStackingMode(QStackedLayout::StackAll);
    setLayout(helpLayout);

    #ifdef Q_OS_MAC
    this->setFixedHeight(90);
    #endif
    #ifdef Q_OS_WIN32
    this->setFixedHeight(120);
    #endif
    #ifdef Q_OS_LINUX
    this->setFixedHeight(120);
    #endif

}

void QHelpBox::setText(std::string text)
{
    helpBox->setText(QString("<html><p style=\"text-align:justify;\">").append(text.c_str()).append("</p></html>"));
}
