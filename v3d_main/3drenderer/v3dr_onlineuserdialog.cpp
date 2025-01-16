#include "v3dr_onlineuserdialog.h"
#include <QVBoxLayout>

V3dr_onlineusersDialog::V3dr_onlineusersDialog(QPoint topLeftPoint, QWidget *parent)
    : QWidget{parent}
{
    init();
    createLayout(topLeftPoint);
}

void V3dr_onlineusersDialog::createLayout(QPoint topLeftPoint){
    userListWidget = new QListWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(userListWidget);
    this->setLayout(layout);
    this->move(topLeftPoint.x() + 65, topLeftPoint.y() + 40);
    // 设置窗口的最小大小为推荐的最小大小
    QSize size = this->minimumSizeHint();
    this->resize(size.width() + 85, size.height() + 25);
    this->show();
}

void V3dr_onlineusersDialog::init(){
    this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    this->setWindowTitle("Online Users");
}

void V3dr_onlineusersDialog::setUserList(std::vector<QString> users){
    userList = users;
    userListWidget->clear();
    QFont font;
    font.setPointSize(11);
    for (int i = 0; i < users.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(users[i]);
        item->setFont(font);
        userListWidget->addItem(item);
    }
}
