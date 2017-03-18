// Only use gpio on pis
#if __arm__
#include <pigpio.h>
#endif

#include <cmath>
#include <gtk/gtk.h>
#include <iostream>
#include "game.h"

#define APP_NAME "Table Tennis Scoreboard"
#define APP_PACKAGE "me.zachcheatham.table-tennis-scoreboard"
#define STRING_INSTRUCTIONS {"FIRST SERVER", "PRESS BUTTON", "TO START"}
#define STRING_INSTRUCTION_LINES 3
#define STRING_WINNER "WINNER"
#define STRING_LOSER "LOSER"
#define STRING_SERVING "SERVING"

#define VERSION_SIZE 12
#define VERSION_PADDING 15
#define INSTRUCTIONS_SIZE_FACTOR 0.125
#define TIMER_POSITION_FACTOR 0.03333333333
#define TIMER_SIZE_FACTOR 0.19
#define SCORE_SIZE_FACTOR 0.45
#define LABEL_SIZE_FACTOR 0.13

#define FONT "Ozone"

#define DEFAULT_COLOR 1, 0.8, 0
#define GAMEPOINT_COLOR 1, 0.549019608, 0
#define WINNING_COLOR 0, 1, 0
#define LOSING_COLOR 1, 0, 0

#define LEFT_BUTTON 18
#define RIGHT_BUTTON 17

Game *game;
GtkWidget *drawingArea;
bool ready = false;
bool inGame = false;

void draw_multiline_text(cairo_t *cr, const char **text,
    unsigned short lines, double x, double y)
{
    cairo_text_extents_t textExtents;
    unsigned short i;
    double lineHeight;
    double height;
    double verticalSpacing;
    double topY;
    double currentX;
    double currentY;

    cairo_text_extents(cr, text[0], &textExtents);

    lineHeight = textExtents.height;
    verticalSpacing = lineHeight * 0.33;
    height = lineHeight * lines + verticalSpacing * (lines - 1);
    topY = y - height / 2;

    for (i = 0; i < lines; i++)
    {
        cairo_text_extents(cr, text[i], &textExtents);

        currentX = x - textExtents.width / 2;
        currentY = topY + lineHeight * (i+1) + verticalSpacing * i;

        cairo_move_to(cr, currentX, currentY);
        cairo_show_text(cr, text[i]);
    }
}

void draw_text_centered(cairo_t *cr, const char *text, double x, double y)
{
    cairo_text_extents_t textExtents;

    cairo_text_extents(cr, text, &textExtents);

    cairo_move_to(cr, x - textExtents.width / 2, y + textExtents.height / 2);
    cairo_show_text(cr, text);
}

gboolean draw(GtkWidget *widget, cairo_t *cr)
{
    cairo_text_extents_t textExtents;
    double halfX;
    guint windowWidth;
    guint windowHeight;

    windowWidth = gtk_widget_get_allocated_width(widget);
    windowHeight = gtk_widget_get_allocated_height(widget);
    halfX = windowWidth / 2;

    // Background
    cairo_rectangle(cr, 0, 0, windowWidth, windowHeight);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_fill(cr);

    cairo_select_font_face(cr, FONT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    if (inGame)
    {
        char text[8];
        unsigned short timerSeconds;
        unsigned short timerMinutes;
        double timerY;
        double horzLineY;
        double scoreY;
        double labelY;

        // Timer
        timerSeconds = game->getGameTime();
        timerMinutes = std::floor(timerSeconds / 60);
        timerSeconds %= 60;
        sprintf(text, "%02d:%02d", timerMinutes, timerSeconds);

        cairo_set_font_size(cr, windowHeight * TIMER_SIZE_FACTOR);
        cairo_text_extents(cr, text, &textExtents);
        timerY = windowHeight * TIMER_POSITION_FACTOR + textExtents.height;
        cairo_move_to(cr, halfX - textExtents.width / 2, timerY);
        if (game->state == Game::GAMEPOINT)
            cairo_set_source_rgb(cr, GAMEPOINT_COLOR);
        else
            cairo_set_source_rgb(cr, DEFAULT_COLOR);
        cairo_show_text(cr, text);

        // Dividers
        cairo_set_line_width(cr, 2);

        horzLineY = timerY + windowHeight * TIMER_POSITION_FACTOR;
        cairo_move_to(cr, 0, horzLineY);
        cairo_line_to(cr, windowWidth, horzLineY);
        cairo_stroke(cr);

        cairo_line_to(cr, halfX, horzLineY);
        cairo_line_to(cr, halfX, windowHeight);
        cairo_stroke(cr);

        // Left score
        sprintf(text, "%02hu", game->leftScore);
        cairo_set_font_size(cr, windowHeight * SCORE_SIZE_FACTOR);
        cairo_text_extents(cr, text, &textExtents);
        scoreY = windowHeight / 2 + textExtents.height / 2;
        cairo_move_to(cr, halfX / 2 - textExtents.width / 2, scoreY);
        if (game->state == Game::GAMEOVER)
        {
            if (game->leftScore > game->rightScore)
                cairo_set_source_rgb(cr, WINNING_COLOR);
            else
                cairo_set_source_rgb(cr, LOSING_COLOR);
        }
        cairo_show_text(cr, text);

        // Right score
        sprintf(text, "%02hu", game->rightScore);
        cairo_text_extents(cr, text, &textExtents);
        cairo_move_to(cr, halfX * 1.5 - textExtents.width / 2, scoreY);
        if (game->state == Game::GAMEOVER)
        {
            if (game->leftScore < game->rightScore)
                cairo_set_source_rgb(cr, WINNING_COLOR);
            else
                cairo_set_source_rgb(cr, LOSING_COLOR);
        }
        cairo_show_text(cr, text);

        // Left label
        cairo_set_font_size(cr, windowHeight * LABEL_SIZE_FACTOR);
        labelY = scoreY + (windowHeight - scoreY) / 2;
        if (game->state == Game::GAMEOVER)
        {
            if (game->leftScore > game->rightScore)
            {
                cairo_set_source_rgb(cr, WINNING_COLOR);
                draw_text_centered(cr, STRING_WINNER, halfX / 2, labelY);
            }
            else
            {
                cairo_set_source_rgb(cr, LOSING_COLOR);
                draw_text_centered(cr, STRING_LOSER, halfX / 2, labelY);
            }
        }
        else if (game->serving == Game::LEFT)
        {
            draw_text_centered(cr, STRING_SERVING, halfX / 2, labelY);
        }

        // Right label
        if (game->state == Game::GAMEOVER)
        {
            if (game->leftScore < game->rightScore)
            {
                cairo_set_source_rgb(cr, WINNING_COLOR);
                draw_text_centered(cr, STRING_WINNER, halfX * 1.5, labelY);
            }
            else
            {
                cairo_set_source_rgb(cr, LOSING_COLOR);
                draw_text_centered(cr, STRING_LOSER, halfX * 1.5, labelY);
            }
        }
        else if (game->serving == Game::RIGHT)
        {
            draw_text_centered(cr, STRING_SERVING, halfX * 1.5, labelY);
        }
    }
    else
    {
        const char *instructions[] = STRING_INSTRUCTIONS;

        cairo_set_source_rgb(cr, DEFAULT_COLOR);
        cairo_set_font_size(cr, windowWidth * INSTRUCTIONS_SIZE_FACTOR);
        draw_multiline_text(cr, instructions, STRING_INSTRUCTION_LINES, halfX, windowHeight / 2);

        cairo_set_font_size(cr, VERSION_SIZE);
        cairo_text_extents(cr, GIT_VERSION, &textExtents);
        cairo_move_to(cr, windowWidth - textExtents.width - VERSION_PADDING,
            windowHeight - VERSION_PADDING);
        cairo_show_text(cr, GIT_VERSION);
    }

    return FALSE;
}

/* We need a wrapper to pass to g_idle_add in the below function
   so that we can return false....*/
#if __arm__
gboolean updateGUI()
{
    gtk_widget_queue_draw(drawingArea);
    return FALSE;
}
#endif

void input(Game::Player player)
{
    if (!inGame or game->state == Game::GAMEOVER)
    {
        game->startGame(player);
        inGame = true;
    }
    else
        game->point(player, 1);

    /* If this was run on arm, it was run from the GPIO thread.
       You can't successfully invoke gtk_widget_queue_draw from there. */
    #if __arm__
    g_idle_add((GSourceFunc)updateGUI, NULL);
    #else
    gtk_widget_queue_draw(drawingArea);
    #endif
}

#if __arm__
void buttonChange(int gpio, int level)
{
    if (ready)
    {
        if (level == PI_LOW)
        {
            if (gpio == LEFT_BUTTON)
                input(Game::LEFT);
            else if (gpio == RIGHT_BUTTON)
                input(Game::RIGHT);
        }
    }
}
#endif
#if !__arm__ || defined NDEBUG
gboolean mouseClick(GtkWidget *widget, GdkEventButton *event)
{
    (void)widget;

    if (event->button == 1)
        input(Game::LEFT);
    else if (event->button == 3)
        input(Game::RIGHT);

    return TRUE;
}
#endif

gint timerTick(gpointer data)
{
    (void) data;
    gtk_widget_queue_draw(drawingArea);
    return 1;
}

void windowActivated(GtkApplication* app)
{
    GtkWidget *window;
    GdkCursor *blankCursor;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW (window), APP_NAME);
    #ifdef NDEBUG
    gtk_window_fullscreen(GTK_WINDOW(window));
    #else
    gtk_window_set_default_size(GTK_WINDOW (window), 853, 480);
    #endif

    drawingArea = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(draw), NULL);
    #if !__arm__ || defined NDEBUG
    g_signal_connect(G_OBJECT(drawingArea), "button-release-event", G_CALLBACK(mouseClick), NULL);
    gtk_widget_set_events(drawingArea, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    #endif
    gtk_container_add(GTK_CONTAINER(window), drawingArea);
    gtk_widget_show_all(window);

    blankCursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_BLANK_CURSOR);
	gdk_window_set_cursor(gtk_widget_get_window(window), blankCursor);

    g_timeout_add(1000, timerTick, NULL);
    ready = true;
}

int main (int argc, char **argv)
{
    GtkApplication *app;
    int status;

    #if __arm__
    if (gpioInitialise() < 0)
        return 1;

    gpioSetMode(LEFT_BUTTON, PI_INPUT);
    gpioSetMode(RIGHT_BUTTON, PI_INPUT);
    gpioSetPullUpDown(LEFT_BUTTON, PI_PUD_UP);
    gpioSetPullUpDown(RIGHT_BUTTON, PI_PUD_UP);
    gpioGlitchFilter(LEFT_BUTTON, 100000);
    gpioGlitchFilter(RIGHT_BUTTON, 100000);
    gpioSetAlertFunc(LEFT_BUTTON, buttonChange);
    gpioSetAlertFunc(RIGHT_BUTTON, buttonChange);
    #endif

    game = new Game();

    app = gtk_application_new(APP_PACKAGE, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(windowActivated), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    #if __arm__
    gpioTerminate();
    #endif

    delete game;
    return status;
}
