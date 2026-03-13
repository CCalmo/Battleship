#ifndef PLAYER_H_
#define PLAYER_H_

#include <board.h>

class player
{
    private:
        board::coordinate getCoordinate(const char* question) const;
        board::direction getDirection(const char* question) const;
        board::attackResult attack(board &opponentBoard);
    public:
        board boards;
        bool placeShips(const char* fileName);
        board::attackResult playerTurn(board &opponentBoard);
};

#endif