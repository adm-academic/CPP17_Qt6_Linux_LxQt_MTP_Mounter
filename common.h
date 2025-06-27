#ifndef COMMON_H
#define COMMON_H
// общий повсеместно используемый в приложении код и переменные

#include <QString>
#include <QProcess>
#include <settings.h>

// функция для выполнения команды операционной системы от текущего пользователя.
// использует блокирующий подход
// объединяет вывод stdout и stderr
QString fast_exec(QString command_string);

extern Settings *settings;

#endif // COMMON_H
