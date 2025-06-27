#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>
#include <QMessageBox>
#include <iostream>
#include <string>
#include "common.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    this->slot_center_this_Window();

    this->tray_icon = new QSystemTrayIcon();
    this->tray_icon->setIcon( QIcon(":/images/state_D.png") );
    this->tray_icon->setToolTip( this->windowTitle() );
    this->tray_icon->show();

    this->menu = new QMenu();
    QAction *showAction = new QAction(tr("Show"), menu);
    menu->addAction(showAction);
    connect(showAction,&QAction::triggered,this,&MainWindow::showNormal);
    //---
    QAction *quitAction = new QAction(tr("Quit"), menu);
    menu->addAction(quitAction);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    //---
    tray_icon->setContextMenu(menu);

    connect(tray_icon, &QSystemTrayIcon::activated, this, &MainWindow::slot_onTrayIconActivated);



    // фактически в отдельном потоке исполняется только слот Thread_Worker::slot_do_work()
    // но благодаря особому подходу мы имеем работоспособную подсистему сигналов-слотов,
    // и цикл событий
    this->state_machine_thread = new QThread; // создадим управляющий объект потока
    this->states_machine = new States_Machine(this,nullptr);
    this->states_machine->moveToThread(this->state_machine_thread); // переместим воркер в управляющий объект потока

    // далее соединим между собой сигналы-слоты объектов управления потоком, воркера, эти обеспечивая
    // коррекную работу через сигналы-слоты
    connect(this, &MainWindow::sig_request_finish_threaded_state_machine,
            this->states_machine,&States_Machine::slot_request_finish_main_loop ); // запрос на коррекную остановку потока стейт-машины

    connect(this->states_machine,&States_Machine::sig_error,
            this, &MainWindow::slot_on_thread_error); // при ошибках внутри отдельного потока, вызываем слот обработки в потоке GUI

    connect(this->state_machine_thread,&QThread::started,
            this->states_machine, &States_Machine::slot_main_loop); // после старта QThread запустим рабочий слот

    connect(this->states_machine,&States_Machine::sig_work_finished,
            this->state_machine_thread,&QThread::quit); // завершаем тред после сигнала от воркера

    this->state_machine_thread->start(); // старутем все эти хитрые объекты и в результате главный цикл
                                         // стейт-машины в отдельном потоке

}

MainWindow::~MainWindow()
{
    emit this->sig_request_finish_threaded_state_machine(); // испутим сигнал, по которому вся наша
                                                            // шаткая система потоков должна
                                                            // коррекно остановить свою работу...

    forever{ // вполне себе легальный метод ожидания завершения управляющего класса потока QThread
             // подразумевается, что после завершения QThread сам слот в котором исполняется
             // эффективный код потока уже завершён
        QApplication::processEvents();
        if ( this->state_machine_thread->isFinished() ){
            break;
        }
        else{
            this->thread()->msleep(300);
        }
    }

    delete this->states_machine;
    delete this->state_machine_thread;
    delete this->ui;

}

void MainWindow::slot_center_this_Window() {
    QRect screenGeometry = qApp->primaryScreen()->geometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    hide();
    event->ignore();
}

void MainWindow::changeEvent(QEvent *event){

    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            this->hide();
            return;
        } else if (isMaximized()) {
            // Window has been maximized
        } else {
            // Window has been restored or hidden
        }
    };

    // сделаем смену локали во время исполнения программы
    if (event->type() == QEvent::LanguageChange){
        this->ui->retranslateUi(this);
    };

    QMainWindow::changeEvent(event); // Important: Call the base class implementation

}


void MainWindow::slot_onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        // This indicates a left-click (or equivalent platform-specific trigger)
        this->showNormal();
    } else if (reason == QSystemTrayIcon::DoubleClick) {
        // Handle double-click if needed
        this->showNormal();
    }
    // Other reasons like Context or MiddleClick can also be handled
}

#include <iostream>
#include <string>
void  MainWindow::slot_on_thread_error(QString err)
{
    std::wcout << std::endl
               << tr("Error while executing state machine: ").toStdWString()
               << std::endl
               << L"\t" << err.toStdWString()
               << std::endl
               << std::flush;

    QMessageBox::critical(nullptr, QObject::tr("MTP Mounter"),
                          QObject::tr("Error while executing state machine:")
                                      + "\n \n"
                                      + err
                          );

}

void MainWindow::slot_change_tray_icon(QString icon_path, QString tool_tip)
{

    this->tray_icon->setIcon( QIcon(icon_path) );
    this->tray_icon->setToolTip( tool_tip );

}

void MainWindow::on_pb_About_Qt_clicked()
{
    QApplication::aboutQt();
}


void MainWindow::on_pb_sync_clicked()
{
    fast_exec("sync");
}


void MainWindow::on_rb_US_toggled(bool checked)
{
    if ( checked ){
        settings->set_GUI_locale_str( this->ui->rb_US->text() );
        settings->load_current_config_file_locale();
    };
}


void MainWindow::on_rb_RU_toggled(bool checked)
{
    if ( checked ){
        settings->set_GUI_locale_str( this->ui->rb_RU->text() );
        settings->load_current_config_file_locale();
    };
}

