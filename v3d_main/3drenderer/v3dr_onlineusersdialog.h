
#ifndef V3DR_ONLINEUSERSDIALOG_H
#define V3DR_ONLINEUSERSDIALOG_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>

class V3dr_onlineusersDialog : public QWidget
{
    Q_OBJECT
public:
    V3dr_onlineusersDialog(QPoint topLeftPoint, QWidget *parent = nullptr);
    void setUserList(std::vector<QString> users);
    std::vector<QString> getUserList(){return userList;}

private:
    QListWidget* userListWidget;
    std::vector<QString> userList;

    void init();
    void createLayout(QPoint topLeftPoint);

};

#endif // V3DR_ONLINEUSERSDIALOG_H
