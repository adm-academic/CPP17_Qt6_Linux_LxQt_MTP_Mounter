#include "states.h"
#include <QApplication>
#include "mainwindow.h"
#include <QDebug>
#include <iostream>
#include <string>
#include <QThread>
#include "common.h"


///=============== States_Machine ====================
States_Machine::States_Machine( MainWindow *ptr_to_main_window,
                                QObject *parent )
    : QObject{parent}
{

    // если папки для монтирования не существуюет - то создадим её !
    if ( fast_exec("file ~/MTP").trimmed().contains("No such file or directory") ){
        fast_exec("mkdir ~/MTP");
    };

    this->pointer_to_main_window = ptr_to_main_window;
    this->must_be_finished = false;

    this->disconnected_state = new Disconnected_State(this->pointer_to_main_window,this);
    this->disconnected_state->set_states_machine(this);
    connect(this->disconnected_state, &Disconnected_State::sig_change_tray_icon,
            this->pointer_to_main_window, &MainWindow::slot_change_tray_icon);
    connect(this->disconnected_state,&Disconnected_State::sig_error,
            this, &States_Machine::sig_error);

    this->connected_state = new Connected_State(this->pointer_to_main_window,this);
    this->connected_state->set_states_machine(this);
    connect(this->connected_state,&Connected_State::sig_change_tray_icon ,
            this->pointer_to_main_window, &MainWindow::slot_change_tray_icon);
    connect(this->connected_state,&Connected_State::sig_error,
            this, &States_Machine::sig_error);

    this->mounted_state = new Mounted_State(this->pointer_to_main_window,this);
    this->mounted_state->set_states_machine(this);
    connect(this->mounted_state,&Mounted_State::sig_change_tray_icon ,
            this->pointer_to_main_window, &MainWindow::slot_change_tray_icon);
    connect(this->mounted_state,&Mounted_State::sig_error,
            this, &States_Machine::sig_error);


    this->finished_state = new Finished_State(this->pointer_to_main_window,this);
    this->finished_state->set_states_machine(this);
    /// . . .


    this->current_state = this->disconnected_state;
    this->requested_state = nullptr;

}


States_Machine::~States_Machine()
{
    this->current_state = nullptr;
    this->requested_state = nullptr;

    delete this->disconnected_state;
    this->disconnected_state = nullptr;
    delete this->connected_state;
    this->connected_state = nullptr;
    delete this->mounted_state;
    this->mounted_state = nullptr;

    fast_exec("sync");
    fast_exec("fusermount -u ~/MTP");

}

void States_Machine::slot_request_finish_main_loop()
{
    this->must_be_finished = true;
}


void States_Machine::slot_main_loop()
{
    forever{ // в нашем отдельном потоке сделаем бесконечный цикл...

        this->current_state->loop(); // запустим цикл очередного состояния машины состояний

        if ( this->must_be_finished )
            break;

        if ( this->requested_state != nullptr ){
            this->current_state = this->requested_state;
            this->requested_state = nullptr;
        };

        if ( this->must_be_finished )
            break;

        this->thread()->msleep(500); // сделаем паузу для снижения нагрузки на CPU
        QApplication::processEvents(); // обработаем все события этого потока
    };

    emit this->sig_work_finished();
}


///=============== State ====================
State::State(MainWindow *ptr_to_main_window,
             QObject *parent)
    : QObject{parent}
{
    this->pointer_to_main_window = ptr_to_main_window;
}

State::~State()
{

}

void State::set_states_machine(States_Machine *new_states_machine)
{
    this->states_machine = new_states_machine;
}



///=============== Disconnected_State ====================
Disconnected_State::Disconnected_State(MainWindow *ptr_to_main_window,
                                       QObject *parent) : State(ptr_to_main_window,parent)
{

}


void Disconnected_State::loop()
{
    emit this->sig_change_tray_icon( ":/images/state_D.png",
                                    QObject::tr("MTP state - disconnected!") );

    forever{
        if ( this->states_machine->must_be_finished )
            return;

        QString check_mtp_connection = fast_exec("jmtpfs -l | wc -l").trimmed();
        if (check_mtp_connection != "1" ){ // есть подключенные MTP устройства
            this->states_machine->requested_state = this->states_machine->connected_state;
            return;
        }
        this->thread()->msleep(1000);
        QApplication::processEvents();
    };

}

Disconnected_State::~Disconnected_State()
{

}


///=============== Connected_State ====================
Connected_State::Connected_State(MainWindow *ptr_to_main_window,
                             QObject *parent) : State(ptr_to_main_window,parent)
{

}

void Connected_State::loop()
{
    emit this->sig_change_tray_icon( ":/images/state_C.png",
                                     QObject::tr("MTP state - Connected, but not mounted!"));
    forever{
        if ( this->states_machine->must_be_finished )
            return;

        QString check_mtp_connection = fast_exec("jmtpfs -l | wc -l").trimmed();
        if ( check_mtp_connection == "1" ){ // нет подключенных MTP устройств
            this->states_machine->requested_state = this->states_machine->disconnected_state;
            return;
        }
        else if (check_mtp_connection != "1" ){ // есть подключенные MTP устройства
            // если существует папка для монтирования
            if ( fast_exec("file ~/MTP").trimmed().contains("directory") ){
                // эта папка сейчас НЕ примонтирована
                if ( fast_exec("mountpoint ~/MTP").trimmed().contains("is not a mountpoint") ){
                    // в этой папке есть файлы !
                    if ( ! fast_exec("ls ~/MTP").trimmed().isEmpty() ){
                        /// ! в интерфейсе показать ошибку, в папку с файлами монтировать не будем !
                        emit this->sig_error( QObject::tr(
                            "Files found in MTP directory! Can't mount device!") );
                        this->thread()->msleep(1000);
                        QApplication::processEvents();
                        continue;
                    }
                    // в этой папке нету файлов
                    else if( fast_exec("ls ~/MTP").trimmed().isEmpty() ){
                        // дадим команду на монтирование папки
                        fast_exec("jmtpfs ~/MTP");
                        // если папка не примонтировалась - выдадим ошибку
                        if ( ! fast_exec("mountpoint ~/MTP").trimmed().contains("is a mountpoint") ){
                            /// выдадим ошибку и идём на следующую итерацию цикла forever
                            std::wcout << QObject::tr("Сan't mount the folder!").toStdWString()
                                       << std::endl << std::flush;
                            this->thread()->msleep(1000);
                            QApplication::processEvents();
                            continue;
                        }
                        // если папка примонтировалась - переключимся на другое состояние
                        else if ( fast_exec("mountpoint ~/MTP").trimmed().contains("is a mountpoint") ){
                            this->states_machine->requested_state = this->states_machine->mounted_state;
                            return;
                        }
                    }
                }
                // эта папка сейчас примонтирована !!!!
                else if ( fast_exec("mountpoint ~/MTP").trimmed().contains("is a mountpoint") ){
                    // в этой папке не отображаются никакие файлы, значит монтирование не успешно!
                    if ( fast_exec("ls ~/MTP").trimmed().isEmpty() ){
                        /// !!!в интерфейсе показать ошибку
                        this->thread()->msleep(1000);
                        QApplication::processEvents();
                        continue;
                    }
                    // в этой папке отображаются файлы
                    else if ( ! fast_exec("ls ~/MTP").trimmed().isEmpty() ){
                        // тогда переключимся на состояние - монтировано!
                        this->states_machine->requested_state = this->states_machine->mounted_state;
                        return;
                    }
                }
                // ошибка ввода-вывода при монтировании папки, скорее всего на устройстве
                // не включен режим MTP
                else if ( fast_exec("mountpoint ~/MTP").trimmed().contains("Input/output error") ){
                    // отмонтируем папку !
                    fast_exec("fusermount -u ~/MTP");
                    // дадим команду на монтирование папки
                    fast_exec("jmtpfs ~/MTP");
                    // эта папка сейчас примонтирована !!!!
                    if ( fast_exec("mountpoint ~/MTP").trimmed().contains("is a mountpoint") ){
                        // в этой папке отображаются файлы
                        if ( ! fast_exec("ls ~/MTP").trimmed().isEmpty() ){
                            // тогда переключимся на состояние - монтировано!
                            this->states_machine->requested_state = this->states_machine->mounted_state;
                            return;
                        }
                    }
                }
            }
        }
        this->thread()->msleep(1000);
        QApplication::processEvents();
    };
}

Connected_State::~Connected_State()
{

}




///=============== Mounted_State ====================
Mounted_State::Mounted_State(MainWindow *ptr_to_main_window,
                             QObject *parent) : State(ptr_to_main_window,parent)
{

}

void Mounted_State::loop()
{
    emit this->sig_change_tray_icon( ":/images/state_M.png",
                                     QObject::tr("MTP state - Mounted !") );
    forever{
        if ( this->states_machine->must_be_finished )
            return;

        // проверим подключение устройства MTP - если 1 - то ничего не подключено
        if ( fast_exec("jmtpfs -l | wc -l").trimmed() == "1" ){
            fast_exec("fusermount -u ~/MTP");
            // переведём в состояние "не подключено"
            this->states_machine->requested_state = this->states_machine->disconnected_state;
            return;
        }

        QString ls_MTP_dir = fast_exec("ls ~/MTP").trimmed();
        // ввод-вывод в папку монтирования не проходит
        if ( ls_MTP_dir.contains( "Input/output error" ) ){
            this->states_machine->requested_state = this->states_machine->connected_state;
            return;
        }

        // в папке монтирования не отображаются файлы
        else if ( ls_MTP_dir.isEmpty() ){
            // проверим подключение устройства MTP - если 1 - то ничего не подключено
            if ( fast_exec("jmtpfs -l | wc -l").trimmed() == "1" ){
                fast_exec("fusermount -u ~/MTP");
                // переведём в состояние "не подключено"
                this->states_machine->requested_state = this->states_machine->disconnected_state;
                return;
            }
            // иначе в состояние plugged
            else if ( fast_exec("jmtpfs -l | wc -l").trimmed() != "1" ){
                this->states_machine->requested_state = this->states_machine->connected_state;
                return;
            }
        }

        this->thread()->msleep(1000);
        QApplication::processEvents();
    };
}

Mounted_State::~Mounted_State()
{

}


///=============== Finished_State ====================
Finished_State::Finished_State(MainWindow *ptr_to_main_window, QObject *parent)
                                                : State(ptr_to_main_window,parent)
{

}

void Finished_State::loop()
{

}

Finished_State::~Finished_State()
{

}
