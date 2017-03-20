#include "global.h"
#include "Scoreboard.h"
#include <cairomm/context.h>

#define FONT "Ozone"

#define STRING_INSTRUCTIONS {"FIRST SERVER", "PRESS BUTTON", "TO START"}
#define STRING_INSTRUCTIONS_LINES 3
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

#define DEFAULT_COLOR 1, 0.8, 0
#define GAMEPOINT_COLOR 1, 0.549019608, 0
#define WINNING_COLOR 0, 1, 0
#define LOSING_COLOR 1, 0, 0

Scoreboard::Scoreboard(Game *game) : mInstructions(true), mGame(game)
{
}

bool Scoreboard::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    const int windowWidth = get_allocated_width();
    const int windowHeight = get_allocated_height();

    cr->select_font_face(FONT, Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);

    if (mInstructions)
        drawInstructions(cr, windowWidth, windowHeight);
    else
        drawBoard(cr, windowWidth, windowHeight);

    return true;
}

void Scoreboard::drawBoard(const Cairo::RefPtr<Cairo::Context>& cr,
    const double w, const double h)
{
    Cairo::TextExtents textExtents;
    char text[8];
    unsigned short timerSeconds;
    unsigned short timerMinutes;
    double timerY;
    double horzLineY;
    double scoreY;
    double labelY;
    double halfX = w / 2;

    if (mGame->state == Game::GAMEPOINT)
        cr->set_source_rgb(GAMEPOINT_COLOR);
    else
        cr->set_source_rgb(DEFAULT_COLOR);

    // Timer
    timerSeconds = mGame->getGameTime();
    timerMinutes = std::floor(timerSeconds / 60);
    timerSeconds %= 60;
    sprintf(text, "%02d:%02d", timerMinutes, timerSeconds);

    cr->set_font_size(h * TIMER_SIZE_FACTOR);
    cr->get_text_extents(text, textExtents);
    timerY = h * TIMER_POSITION_FACTOR + textExtents.height;
    cr->move_to(halfX - textExtents.width / 2, timerY);
    if (mGame->state == Game::GAMEPOINT)
        cr->set_source_rgb(GAMEPOINT_COLOR);
    else
        cr->set_source_rgb(DEFAULT_COLOR);
    cr->show_text(text);

    // Dividers
    cr->set_line_width(2);

    horzLineY = timerY + h * TIMER_POSITION_FACTOR;
    cr->move_to(0, horzLineY);
    cr->line_to(w, horzLineY);
    cr->stroke();

    cr->move_to(halfX, horzLineY);
    cr->line_to(halfX, h);
    cr->stroke();

    // Left score
    sprintf(text, "%02hu", mGame->leftScore);
    cr->set_font_size(h * SCORE_SIZE_FACTOR);
    cr->get_text_extents(text, textExtents);
    scoreY = h / 2 + textExtents.height / 2;
    cr->move_to(halfX / 2 - textExtents.width / 2, scoreY);
    if (mGame->state == Game::GAMEOVER)
    {
        if (mGame->leftScore > mGame->rightScore)
            cr->set_source_rgb(WINNING_COLOR);
        else
            cr->set_source_rgb(LOSING_COLOR);
    }
    cr->show_text(text);

    // Right score
    sprintf(text, "%02hu", mGame->rightScore);
    cr->set_font_size(h * SCORE_SIZE_FACTOR);
    cr->get_text_extents(text, textExtents);
    scoreY = h / 2 + textExtents.height / 2;
    cr->move_to(halfX * 1.5 - textExtents.width / 2, scoreY);
    if (mGame->state == Game::GAMEOVER)
    {
        if (mGame->leftScore < mGame->rightScore)
            cr->set_source_rgb(WINNING_COLOR);
        else
            cr->set_source_rgb(LOSING_COLOR);
    }
    cr->show_text(text);

    // Left label
    cr->set_font_size(h * LABEL_SIZE_FACTOR);
    labelY = scoreY + (h - scoreY) / 2;
    if (mGame->state == Game::GAMEOVER)
    {
        if (mGame->leftScore > mGame->rightScore)
        {
            cr->set_source_rgb(WINNING_COLOR);
            drawTextCentered(cr, STRING_WINNER, halfX / 2, labelY);
        }
        else
        {
            cr->set_source_rgb(LOSING_COLOR);
            drawTextCentered(cr, STRING_LOSER, halfX / 2, labelY);
        }
    }
    else if (mGame->serving == Game::LEFT)
    {
        drawTextCentered(cr, STRING_SERVING, halfX / 2, labelY);
    }

    // Right label
    if (mGame->state == Game::GAMEOVER)
    {
        if (mGame->leftScore < mGame->rightScore)
        {
            cr->set_source_rgb(WINNING_COLOR);
            drawTextCentered(cr, STRING_WINNER, halfX * 1.5, labelY);
        }
        else
        {
            cr->set_source_rgb(LOSING_COLOR);
            drawTextCentered(cr, STRING_LOSER, halfX * 1.5, labelY);
        }
    }
    else if (mGame->serving == Game::RIGHT)
    {
        drawTextCentered(cr, STRING_SERVING, halfX * 1.5, labelY);
    }
}

void Scoreboard::drawInstructions(const Cairo::RefPtr<Cairo::Context>& cr,
    const double w, const double h)
{
    Cairo::TextExtents textExtents;
    const char *instructions[] = STRING_INSTRUCTIONS;

    cr->set_source_rgb(DEFAULT_COLOR);
    cr->set_font_size(w * INSTRUCTIONS_SIZE_FACTOR);
    drawTextMultilineCentered(cr, instructions, STRING_INSTRUCTIONS_LINES,
        w/2, h/2);

    cr->set_font_size(VERSION_SIZE);
    cr->get_text_extents(GIT_VERSION, textExtents);
    cr->move_to(w - textExtents.width - VERSION_PADDING,
        h - VERSION_PADDING);
    cr->show_text(GIT_VERSION);
}

void Scoreboard::drawTextMultilineCentered(const Cairo::RefPtr<Cairo::Context>& cr,
    const char **text, const unsigned short lines, const double x, const double y)
{
    Cairo::TextExtents textExtents;
    unsigned short i;
    double lineHeight;
    double height;
    double verticalSpacing;
    double topY;
    double currentX;
    double currentY;

    cr->get_text_extents(text[0], textExtents);
    lineHeight = textExtents.height;
    verticalSpacing = lineHeight * 0.33;
    height = lineHeight * lines +
             verticalSpacing * (lines - 1);
    topY = y - height / 2;

    for (i = 0; i < lines; i++)
    {
        cr->get_text_extents(text[i], textExtents);
        currentX = x - textExtents.width / 2;
        currentY = topY + lineHeight * (i+1) + verticalSpacing * i;

        cr->move_to(currentX, currentY);
        cr->show_text(text[i]);
    }
}

void Scoreboard::drawTextCentered(const Cairo::RefPtr<Cairo::Context>& cr,
    const char *text, const double x, const double y)
{
    Cairo::TextExtents textExtents;

    cr->get_text_extents(text, textExtents);
    cr->move_to(x - textExtents.width / 2, y + textExtents.height / 2);
    cr->show_text(text);
}

void Scoreboard::setDrawInstructions(bool draw)
{
    mInstructions = draw;
}
