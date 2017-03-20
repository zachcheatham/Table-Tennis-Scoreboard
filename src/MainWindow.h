#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtkmm/window.h>
#include "Scoreboard.h"
#include "game.h"

class MainWindow : public Gtk::Window
{
public:
    MainWindow();

    // We only use the destructor for gpio
    #if __arm__
    virtual ~MainWindow();
    #endif
protected:
    Scoreboard mScoreboard;

    #if !__arm__ || !defined NDEBUG
    bool onMouseRelease(GdkEventButton *event);
    #endif
    #if __arm__
    bool drawScoreboard();
    #endif
    bool updateTimer();
    bool idleTimeout();

    // Have to have a static method for C link
    #if __arm__
    static void GPIOAlert(int gpio, int level, uint32_t tick, void *obj);
    #endif
private:
    Game mGame;
    bool mInGame;
    sigc::connection mTimerConnection;
    sigc::connection mIdleTimeoutConnection;

    void input(Game::Player player);
};

#endif
