#define MAX_SCORE 11
#define SERVES 2
#define WIN_DIFFERENCE 2

#include "game.h"
#include <cmath>
#include <algorithm>

Game::Game()
{
    leftScore = 0;
    rightScore = 0;
    state = State::PLAYING;
}

void Game::startGame(Player firstServing)
{
    leftScore = 0;
    rightScore = 0;
    state = State::PLAYING;
    serving = firstServing;
    servedFirst = firstServing;
    gameStartTime = std::time(nullptr);
}

void Game::point(Player player, short points)
{
    unsigned short winningScore;

    // Add to score
    if (player == Player::LEFT)
    {
        if (points > 0 || leftScore >= abs(points))
            leftScore += points;
    }
    else
    {
        if (points > 0 || rightScore >= abs(points))
            rightScore += points;
    }

    // Check for winner
    winningScore = std::max(leftScore, rightScore);
    if (winningScore >= MAX_SCORE - 1) // We're not normal anymore
    {
        unsigned short losingScore = std::min(leftScore, rightScore);

        // Gameover
        if (winningScore >= MAX_SCORE && winningScore - losingScore >= WIN_DIFFERENCE)
        {
            state = State::GAMEOVER;
            gameFinishTime = std::time(nullptr);
        }
        // Game point
        else
        {
            state = State::GAMEPOINT;
        }
    }
    else
    {
        state = State::PLAYING;
    }

    // Figure out current server
    if (state != State::GAMEOVER)
    {
        // Game point server
        if (state == State::GAMEPOINT && leftScore != rightScore)
        {
            if (leftScore < rightScore)
                serving = Player::LEFT;
            else
                serving = Player::RIGHT;
        }
        else
        {
            unsigned short totalPoints = rightScore  + leftScore;
            bool isFirstServing = ((unsigned short) std::floor(totalPoints / 2) % 2 == 0);

            if (servedFirst == LEFT)
                serving = isFirstServing ? LEFT : RIGHT;
            else
                serving = isFirstServing ? RIGHT : LEFT;
        }
    }
}

int Game::getGameTime()
{
    if (state == State::GAMEOVER)
    {
        return gameFinishTime - gameStartTime;
    }
    else
    {
        return std::time(nullptr) - gameStartTime;
    }
}
