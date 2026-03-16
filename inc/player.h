#ifndef PLAYER_H_
#define PLAYER_H_

#include <board.h>

class player
{
    private:
        void printResult(board::attackResult result, const char* shipName) const;
        board::coordinate getCoordinate(const char* question) const;
        board::direction getDirection(const char* question) const;
        board::attackResult attack(board &opponentBoard);

    public:
        board boards;

        void placeShips();
        board::attackResult playerTurn(board &opponentBoard);
};

#endif