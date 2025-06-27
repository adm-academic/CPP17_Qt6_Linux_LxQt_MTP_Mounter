#ifndef STATES_H
#define STATES_H
// специализированная машина состояний, которая:
// A. запускается в отдельном потоке и в основном использует блокирующий
//    стиль программирования
// B. отслеживает подключение смартфона
// C. отображает текущее состояние через иконку в трее
// D. при необходимости взаимодействует с GUI

#include <QObject>
#include <QSystemTrayIcon>

// упреждающие объявления классов
class MainWindow;
class States_Machine;
class Disconnected_State;
class Connected_State;
class Mounted_State;
class Finished_State;

// асбтрактный класс состояния
class State: public QObject {
    Q_OBJECT
protected:
    States_Machine *states_machine;
    MainWindow *pointer_to_main_window;

public:
    explicit State(MainWindow *ptr_to_main_window,
                   QObject *parent = nullptr);

    virtual ~State();

    void set_states_machine(States_Machine *new_states_machine);

    virtual void loop() = 0;


signals:
    void sig_change_tray_icon(QString icon_path, QString tool_tip);
    void sig_error(QString err);
};



// класс стэйт-машины
// предполагается что будет запускаться с помощью отдельного потока через QThread
class States_Machine : public QObject
{
    Q_OBJECT
private:
    MainWindow *pointer_to_main_window;


public:
    State *current_state;
    State *requested_state;
    bool must_be_finished;

    Connected_State *connected_state;
    Disconnected_State *disconnected_state;
    Mounted_State *mounted_state;
    Finished_State *finished_state;


public:
    explicit States_Machine( MainWindow *ptr_to_main_window,
                             QObject *parent = nullptr);
    virtual ~States_Machine();

public slots:
    void slot_main_loop();
    void slot_request_finish_main_loop();

signals:
    void sig_work_finished();
    void sig_error(QString err);

};


// класс для состояния disconnected
// интегрирован по переменным с классом States_Machine
class Disconnected_State: public State
{
    Q_OBJECT
public:
    explicit Disconnected_State(MainWindow *ptr_to_main_window,
                                QObject *parent = nullptr);
    void loop() override;
    virtual ~Disconnected_State();


signals:

};


// класс для состояния plugged
// интегрирован по переменным с классом States_Machine
class Connected_State: public State
{
    Q_OBJECT
public:
    explicit Connected_State(MainWindow *ptr_to_main_window,
                           QObject *parent = nullptr);
    void loop() override;
    virtual ~Connected_State();

signals:

};


// класс для состояния mounted
// интегрирован по переменным с классом States_Machine
class Mounted_State: public State
{
    Q_OBJECT
public:
    explicit Mounted_State(MainWindow *ptr_to_main_window,
                           QObject *parent = nullptr);
    void loop() override;
    virtual ~Mounted_State();


signals:
};


// класс для состояния finished
// интегрирован по переменным с классом States_Machine
class Finished_State: public State
{
    Q_OBJECT
public:
    explicit Finished_State(MainWindow *ptr_to_main_window,
                           QObject *parent = nullptr);
    void loop() override;
    virtual ~Finished_State();


signals:
};

























#endif // STATES_H
