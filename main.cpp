#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMessageBox>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QDebug>
#include <iostream>
#include <string>

#include "mainwindow.h"
#include "common.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QApplication a( argc, argv );
    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");

    // создадим глобальный объект настроек
    settings = new Settings;

    // настроим консоль, используя параметры настроек
    QString program_locale = settings->get_locale_console_str();
    setlocale( LC_ALL, program_locale.toLatin1().constData() );

    // проинициализуруем объект настроек
    settings->find_basedir_and_changedir();
    settings->load_current_config_file_locale();


    // Дождёмся пока не будет включен системный трей...
    forever{
        if ( QSystemTrayIcon::isSystemTrayAvailable() ) {
            break;
        }
        else{
            QThread::msleep( 1000 );
        }
    };


    /////////////////////////////// синглтон для приложения...
    /// код взят с сайта https://evileg.com/ru/
    QSystemSemaphore semaphore("<uniq id>", 1);  // создаём семафор
    semaphore.acquire(); // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью
    // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
    // поэтому необходимо избавиться от данного мусора
    QSharedMemory nix_fix_shared_memory("<uniq id 2>");
    if (nix_fix_shared_memory.attach())
    {
        nix_fix_shared_memory.detach();
    }
    QSharedMemory sharedMemory("<uniq id 2>");  // Создаём экземпляр разделяемой памяти
    bool is_running;            // переменную для проверки ууже запущенного приложения
    if (sharedMemory.attach())
    {                           // пытаемся присоединить экземпляр разделяемой памяти
        // к уже существующему сегменту
        is_running = true;      // Если успешно, то определяем, что уже есть запущенный экземпляр
    }
    else
    {
        sharedMemory.create(1); // В противном случае выделяем 1 байт памяти
        is_running = false;     // И определяем, что других экземпляров не запущено
    }
    semaphore.release();        // Опускаем семафор
    // Если уже запущен один экземпляр приложения, то сообщаем об этом пользователю
    // и завершаем работу текущего экземпляра приложения
    if (is_running)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(QObject::tr("The application is already running.\n"
                                   "You can only run one instance of the application."));
        msgBox.exec();
        return 1;
    }
    ///////////////////////////////////////////////////////////


    MainWindow w;


    return a.exec();

    // удалим объект настроек
    delete settings;
}
