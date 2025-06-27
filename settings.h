#ifndef SETTINGS_H
#define SETTINGS_H
// класс для работы с настройками приложения

#include <QObject>
#include <QString>
#include <QTranslator>

class Settings : public QObject
{
    Q_OBJECT
private:
    QString locale_console_str;

public:
    explicit Settings(QObject *parent = nullptr);
    QString get_locale_console_str();
    QString get_GUI_locale_str();
    void set_GUI_locale_str( QString locale_str );

    void load_current_config_file_locale();

    bool fileExists(QString path);
    void find_basedir_and_changedir();

signals:

private:
    QTranslator translator;
};

#endif // SETTINGS_H
