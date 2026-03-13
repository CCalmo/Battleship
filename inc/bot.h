#ifndef BOT_H_
#define BOT_H_

#include <board.h>

class bot
{
    private:
        const board::coordinate NUL_COORDINATE = {0, 0};
        enum class attackMode {scout, findDirection, attack};
        struct shipInfo
        {
            bool knowsDirection;
            board::direction shipDirection;
            board::coordinate startingCoordinate, currentCoordinate;
        };
        attackMode mode;
        shipInfo shipQueue[5];
        int queueIndex;

        board::direction randDirection() const;
        board::coordinate randCoord() const;

        board::direction oppositeDirection(board::direction direction);

        board::attackResult scout(board& opponentBoard);
        board::attackResult findDirection(board& opponentBoard);
        board::attackResult attack(board& opponentBoard);
    public:
        bot();
        board boards;
        bool placeShips(const char* fileName);
        board::attackResult botTurn(board& opponentBoard);
};

#endif