#include "common.h"
#include <QDebug>
#include <QObject>
#include <iostream>
#include <string>


Settings *settings;


QString fast_exec(QString command_string)
{
    QProcess process;

    // Switch command locale to American English and run command_string via bash
    process.start("bash", QStringList() << "-c" << "GDM_LANG=en_US.UTF-8 LANG=en_US.UTF-8 && " + command_string );

    if (!process.waitForStarted()) {
        std::wcout << QObject::tr(" Critical error! > ").toStdWString()
                   << QObject::tr("Error when starting the process").toStdWString()
                   << std::endl << std::flush;
        return "";
    }

    if (!process.waitForFinished()) {
        std::wcout << QObject::tr(" Critical error! > ").toStdWString()
        << QObject::tr("Error waiting for process to complete").toStdWString()
        << std::endl << std::flush;
        return "";
    }

    QByteArray result = process.readAllStandardOutput();
    QByteArray error = process.readAllStandardError();

    QString rslt;
    rslt = QString::fromUtf8(result);
    if (!error.isEmpty())
        rslt.append( QString::fromUtf8(error) );

    return rslt;

}
