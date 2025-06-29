#ifndef MAINWINDOW_H
#define MAINWINDOW_H
// главное окно GUI и иконка системного трея

#include <QMainWindow>
#include <QEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMessageBox>
#include <QThread>
#include <QMutex>
#include "states.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private: /// здесь храним отдельный тред, стэйт-машину и мьютексы для доступа
    QThread *state_machine_thread;
    States_Machine *states_machine;
    QMutex mutex;

public:
    Ui::MainWindow *ui;
    QSystemTrayIcon *tray_icon;
    QMenu *menu;

private:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

public slots:
    void slot_center_this_Window();
    void slot_onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void slot_on_thread_error(QString err);
    void slot_change_tray_icon(QString icon_path, QString tool_tip);
    void slot_check_all_dependencies();

signals:
    void sig_request_finish_threaded_state_machine();

private slots:
    void on_pb_About_Qt_clicked();
    void on_pb_sync_clicked();
    void on_rb_US_toggled(bool checked);
    void on_rb_RU_toggled(bool checked);
};
#endif // MAINWINDOW_H
