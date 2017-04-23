#include "MainWindow.h"
#include "global.h"
#include <gtkmm.h>
#include <glibmm.h>
#include <gtkmm/messagedialog.h>

// Only use gpio on pis
#if __arm__
#include <pigpio.h>
#endif

#define STRING_GPIO_ERROR_TITLE "Error"
#define STRING_GPIO_ERROR "Unable to initialize GPIO"

#define GPIO_LEFT_BUTTON 18
#define GPIO_RIGHT_BUTTON 17

MainWindow::MainWindow() :
    mScoreboard(&mGame),
    mGame(),
    mInGame(false),
    mIgnoreLeftRelease(false),
    mIgnoreRightRelease(false)
{
    set_title(APP_NAME);
    #ifdef NDEBUG
    fullscreen();
    #else
    set_default_size(853, 480);
    set_position(Gtk::WIN_POS_CENTER);
    #endif
    override_background_color(Gdk::RGBA("000000"));

    #if !__arm__ || !defined NDEBUG
    mScoreboard.signal_button_release_event().connect(sigc::mem_fun(*this, &MainWindow::onMouseRelease));
    mScoreboard.signal_button_press_event().connect(sigc::mem_fun(*this, &MainWindow::onMousePress));
    mScoreboard.set_events(Gdk::EventMask::BUTTON_PRESS_MASK | Gdk::EventMask::BUTTON_RELEASE_MASK);
    #endif
    add(mScoreboard);
    show_all();

    #if __arm__
    if (gpioInitialise() < 0)
    {
        Gtk::MessageDialog dialog(*this, STRING_GPIO_ERROR, false,
            Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
        dialog.set_title(STRING_GPIO_ERROR_TITLE);
        dialog.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
        dialog.run();

        close();
    }
    else
    {
        gpioSetMode(GPIO_LEFT_BUTTON, PI_INPUT);
        gpioSetMode(GPIO_RIGHT_BUTTON, PI_INPUT);
        gpioSetPullUpDown(GPIO_LEFT_BUTTON, PI_PUD_UP);
        gpioSetPullUpDown(GPIO_RIGHT_BUTTON, PI_PUD_UP);
        gpioGlitchFilter(GPIO_LEFT_BUTTON, 100000);
        gpioGlitchFilter(GPIO_RIGHT_BUTTON, 100000);
        gpioSetAlertFuncEx(GPIO_LEFT_BUTTON, GPIOAlert, (void *) this);
        gpioSetAlertFuncEx(GPIO_RIGHT_BUTTON, GPIOAlert, (void *) this);
    }
    #endif

    Glib::RefPtr<Gdk::Cursor> cursor = Gdk::Cursor::create(
        Gdk::Display::get_default(), Gdk::BLANK_CURSOR);
    get_window()->set_cursor(cursor);
}

#if __arm__
MainWindow::~MainWindow()
{
    gpioTerminate();
}
#endif

#if !__arm__ || !defined NDEBUG
bool MainWindow::onMousePress(GdkEventButton *event)
{
    if (event->button == 1)
        input(Game::LEFT, true);
    else if (event->button == 3)
        input(Game::RIGHT, true);

    return true;
}

bool MainWindow::onMouseRelease(GdkEventButton *event)
{
    if (event->button == 1)
        input(Game::LEFT, false);
    else if (event->button == 3)
        input(Game::RIGHT, false);

    return true;
}
#endif
#if __arm__
void MainWindow::GPIOAlert(int gpio, int level, uint32_t tick, void *obj)
{
    (void) tick;

    // We're in a static method for C link
    MainWindow *window = (MainWindow *) obj;

    if (window->is_active())
    {
        if (gpio == GPIO_LEFT_BUTTON)
            window->input(Game::LEFT, level == PI_LOW);
        else if (gpio == GPIO_RIGHT_BUTTON)
            window->input(Game::RIGHT, level == PI_LOW);
    }
}
#endif

void MainWindow::input(Game::Player player, bool pressed)
{
    if (pressed)
    {
        if (mInGame) // We only need to do the hold timers while in-game
        {
            if (player == Game::LEFT)
            {
                /* If for some reason this was pressed twice without release,
                   we kill the existing timer.*/
                if (mLeftHoldConnection.connected())
                    mLeftHoldConnection.disconnect();

                mLeftHoldConnection = Glib::signal_timeout().connect(
                    sigc::bind(sigc::mem_fun(*this, &MainWindow::undoPoint), player),
                    1000);
            }
            else if (player == Game::RIGHT)
            {
                if (mRightHoldConnection.connected())
                    mRightHoldConnection.disconnect();

                mRightHoldConnection = Glib::signal_timeout().connect(
                    sigc::bind(sigc::mem_fun(*this, &MainWindow::undoPoint), player),
                    1000);
            }
        }
    }
    else
    {
        // Kill the button hold timer
        if (player == Game::LEFT)
        {
            if (mLeftHoldConnection.connected())
                mLeftHoldConnection.disconnect();
        }
        else if (player == Game::RIGHT)
        {
            if (mRightHoldConnection.connected())
                mRightHoldConnection.disconnect();
        }

        if (player == Game::LEFT and mIgnoreLeftRelease)
            mIgnoreLeftRelease = false;
        else if (player == Game::RIGHT and mIgnoreRightRelease)
            mIgnoreRightRelease = false;
        else
        {
            // Not in-game, start game
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
                mGame.point(player, 1);

            /* If this method was run on arm, it was run from the GPIO thread.
                You can't successfully invoke queue_draw from there. */
            #if __arm__
            Glib::signal_idle().connect(
                sigc::mem_fun(*this, &MainWindow::drawScoreboard));
            #else
            mScoreboard.queue_draw();
            #endif

            // That move caused the game to end
            if (mGame.state == Game::GAMEOVER)
            {
                mIdleTimeoutConnection = Glib::signal_timeout().connect(
                    sigc::mem_fun(*this, &MainWindow::idleTimeout),
                    60000);
            }
        }
    }
}

#if __arm__
bool MainWindow::drawScoreboard()
{
    mScoreboard.queue_draw();
    return false;
}
#endif

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

bool MainWindow::undoPoint(Game::Player player)
{
    if (mInGame)
    {
        // If we undo a gameover, the timeout timer needs to be canceled.
        if (mGame.state == Game::GAMEOVER && mIdleTimeoutConnection.connected())
            mIdleTimeoutConnection.disconnect();

        if (player == Game::LEFT)
            mIgnoreLeftRelease = true;
        else if (player == Game::RIGHT)
            mIgnoreRightRelease = true;

        mGame.point(player, -1);
        mScoreboard.queue_draw();
    }

    return false;
}
