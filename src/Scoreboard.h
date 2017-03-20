#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <gtkmm/drawingarea.h>
#include "game.h"

class Scoreboard : public Gtk::DrawingArea
{
public:
    Scoreboard(Game *game);
    void setDrawInstructions(bool drawInstructions);
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
private:
    bool mInstructions;
    Game *mGame;

    void drawBoard(const Cairo::RefPtr<Cairo::Context>& cr,
        const double w, const double h);
    static void drawInstructions(const Cairo::RefPtr<Cairo::Context>& cr,
        const double h, const double w);
    static void drawTextCentered(const Cairo::RefPtr<Cairo::Context>& cr,
        const char *text, const double x, const double y);
    static void drawTextMultilineCentered(const Cairo::RefPtr<Cairo::Context>& cr,
        const char **text, const unsigned short lines, const double x, const double y);
};

#endif
