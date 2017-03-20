#include "MainWindow.h"
#include "global.h"
#include <gtkmm.h>
#include <glibmm.h>
#include <gtkmm/messagedialog.h>

// Only use gpio on pis
#if __arm__
#include <pigpio.h>
#endif

#define GPIO_LEFT_BUTTON 18
#define GPIO_RIGHT_BUTTON 17

MainWindow::MainWindow() :
    mScoreboard(&mGame),
    mGame(),
    mInGame(false)
{
    set_title(APP_NAME);
    #ifdef NDEBUG
    fullscreen();
    #else
    set_default_size(853, 480);
    #endif
    override_background_color(Gdk::RGBA("000000"));

    #if !__arm__ || defined NDEBUG
    mScoreboard.signal_button_release_event().connect(sigc::mem_fun(*this, &MainWindow::onMouseRelease));
    mScoreboard.set_events(Gdk::EventMask::BUTTON_PRESS_MASK | Gdk::EventMask::BUTTON_RELEASE_MASK);
    #endif
    add(mScoreboard);
    show_all();

    Glib::RefPtr<Gdk::Cursor> cursor = Gdk::Cursor::create(Gdk::Display::get_default(), Gdk::BLANK_CURSOR);
    get_window()->set_cursor(cursor);

    #if __arm__
    if (gpioInitialise() == 0)
    {
        gpioSetMode(GPIO_LEFT_BUTTON, PI_INPUT);
        gpioSetMode(GPIO_RIGHT_BUTTON, PI_INPUT);
        gpioSetPullUpDown(GPIO_LEFT_BUTTON, PI_PUD_UP);
        gpioSetPullUpDown(GPIO_RIGHT_BUTTON, PI_PUD_UP);
        gpioGlitchFilter(GPIO_LEFT_BUTTON, 100000);
        gpioGlitchFilter(GPIO_RIGHT_BUTTON, 100000);
        gpioSetAlertFuncEx(GPIO_LEFT_BUTTON, GPIOAlert, (void *) this);
        gpioSetAlertFuncEx(GPIO_RIGHT_BUTTON, buttonChange, (void *) this);
    }
    else
    {
        Gtk::MessageDialog dialog(*this, "Unable to initialize GPIO");
        dialog.set_secondary_text("The scoreboard will now exit.");
        dialog.run();

        close();
    }
    #endif
}

#if __arm__
MainWindow::~MainWindow()
{
    gpioTerminate();
}
#endif

#if !__arm__ || defined NDEBUG
bool MainWindow::onMouseRelease(GdkEventButton *event)
{
    if (event->button == 1)
        input(Game::LEFT);
    else if (event->button == 3)
        input(Game::RIGHT);

    return true;
}
#endif

#if __arm__
void MainWindow::GPIOAlert(int gpio, int level, uint32_t tick, void *obj)
{
    (void) tick;

    // We're in a static method for C link
    MainWindow *window = (MainWindow *) obj;

    if (window->get_visible()) // Don't do anything if the window hasn't shown itself yet.
    {
        if (level == PI_LOW) // Button is pressed down
        {
            if (gpio == GPIO_LEFT_BUTTON)
                window->input(Game::LEFT);
            else if (gpio == GPIO_RIGHT_BUTTON)
                window->input(Game::RIGHT);
        }
    }
}
#endif

void MainWindow::input(Game::Player player)
{
    if (!mInGame || mGame.state == Game::GAMEOVER)
    {
        if (mIdleTimeoutConnection.connected())
            mIdleTimeoutConnection.disconnect();

        mInGame = true;
        mGame.startGame(player);
        mScoreboard.setDrawInstructions(false);

        // Start refresh for timer if not already done so
        if (!mTimerConnection.connected())
            mTimerConnection = Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &MainWindow::updateTimer),
                1000);
    }
    else
    {
        mGame.point(player, 1);
    }

    mScoreboard.queue_draw();

    // That move caused the game to end
    if (mGame.state == Game::GAMEOVER)
    {
        mIdleTimeoutConnection = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &MainWindow::idleTimeout),
            3000);
    }
}

bool MainWindow::updateTimer()
{
    mScoreboard.queue_draw();
    return true;
}

bool MainWindow::idleTimeout()
{
    mInGame = false;
    mScoreboard.setDrawInstructions(true);
    mScoreboard.queue_draw();

    if (mTimerConnection.connected())
        mTimerConnection.disconnect();

    return false;
}
