#include "settings.h"
#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QFileInfo>

Settings::Settings(QObject *parent)
    : QObject{parent}
{
    this->locale_console_str = "en_US.UTF-8"; // "ru_RU.UTF-8";
}

QString Settings::get_locale_console_str()
{
    return this->locale_console_str;
}

QString Settings::get_GUI_locale_str()
{

    QString locale_str;
    QSettings settings( "MTP_Mounter.conf", QSettings::IniFormat );
    settings.beginGroup( "Locale" );
    locale_str = settings.value( "Locale", "" ).toString();
    settings.endGroup();
    return locale_str;

}

void Settings::set_GUI_locale_str( QString locale_str )
{

    QSettings settings( "MTP_Mounter.conf", QSettings::IniFormat );
    settings.beginGroup( "Locale" );
    settings.setValue( "Locale", locale_str );
    settings.endGroup();

}


void Settings::load_current_config_file_locale(){

    if ( this->get_GUI_locale_str()=="" ){
        this->translator.load(nullptr);
        qApp->installTranslator( &(this->translator) );
    }
    else if ( this->get_GUI_locale_str()=="en_US" ){
        this->translator.load(nullptr);
        qApp->installTranslator( &(this->translator) );
    }
    else if (this->get_GUI_locale_str()=="ru_RU"){
        const QString baseName = "CPP17_Qt6_Linux_LxQt_MTP_Mounter_ru_RU.qm";
        this->translator.load(nullptr);
        if ( this->translator.load( baseName ) ) {
            qApp->installTranslator( &(this->translator) );
        }
        return;
    };

}


void Settings::find_basedir_and_changedir(){

    QDir::setCurrent( QApplication::applicationDirPath() );

    if ( fileExists("MTP_Mounter.conf") )
        return;

    if ( fileExists("../../MTP_Mounter.conf") )
        QDir::setCurrent ( "../../" );

}


bool Settings::fileExists(QString path) {

    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        return true;
    } else {
        return false;
    }

}
