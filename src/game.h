#ifndef GAME
#define GAME

#include <ctime>

class Game
{
public:
    enum Player
    {
        LEFT,
        RIGHT
    };

    enum State
    {
        PLAYING,
        GAMEOVER,
        GAMEPOINT
    };

    unsigned short leftScore;
    unsigned short rightScore;
    State state;
    Player serving;
    Player servedFirst;
    std::time_t gameStartTime;
    std::time_t gameFinishTime;

    Game();
    void startGame(Player firstServing);
    void point(Player player, unsigned short points);
    int getGameTime();
};

#endif /* end of include guard:GAME */
