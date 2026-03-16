#ifndef BOT_H_
#define BOT_H_

#include <board.h>

class bot
{
    public:
        board boards;
    private:
        const board::coordinate NUL_COORDINATE = {0, 0};
        enum class attackMode {scout, findDirection, attack};
        struct shipInfo
        {
            const char* name;
            bool knowsDirection;
            board::direction shipDirection;
            board::coordinate startingCoordinate, currentCoordinate;
        };

        attackMode m_mode;
        shipInfo m_shipQueue[5];
        int m_shipQueueIndex;

    public:
        bot();
        void placeShips();

        board::attackResult botTurn(board& opponentBoard);

    private:
        bool originalShipSink(const char* nameOfShipHit) const;
        void printResult(board::attackResult result, const char* shipName) const;
        void switchDirection();
        void switchToQueuedShip(board opponentBoard, board::coordinate coordAttacked);
        void switchToNewShip(const char* shipName, board::coordinate coordAttacked);

        board::coordinate randCoord() const;
        board::direction randDirection() const;
        board::direction oppositeDirection(board::direction direction);
        board::attackResult scout(board& opponentBoard);
        board::attackResult findDirection(board& opponentBoard);
        board::attackResult attack(board& opponentBoard);
};

#endif