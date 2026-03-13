#ifndef BOARD_H_
#define BOARD_H_

#include <fstream>

class board
{
    public:
        static constexpr int BOARD_ROWS = 11;
        static constexpr int ROW_LENGTH = 45;
        static constexpr int MAX_NAME_LENGTH = 20;
        static constexpr int MAX_SHIP_LENGTH = 5;
        static constexpr int NUM_OF_SHIPS = 5;

        // Values are equal to how many spaces a coordinate needs to be shifted to move in that direction
        enum class direction {up = -1, down = 1, left = -4, right = 4};

        enum class attackResult {discover, hit, sank, miss};

        // Ship board is the board where your ships are kept and reflects the results of the opponents attack. 
        // Attack Board is the board that reflects the results of your attacks 
        enum class playerBoards {shipBoard, attackBoard};

        struct coordinate 
        {
            int x;
            int y;

            bool operator==(const coordinate& comparing) const;
            bool operator>(const coordinate& comparing) const;
            bool operator<(const coordinate& comparing) const;

            coordinate operator+=(const direction& comparing);
            coordinate operator-=(const direction& comparing);
            coordinate operator+(const direction& comparing) const;
        };
    private:
        static constexpr int EMPTY_ROW_SPACE = 4;
        static constexpr int EMPTY_COLUMN_SPACE = 1;

        struct ship
        {
            int length;
            bool alive;
            bool discovered;
            bool placesHit[MAX_SHIP_LENGTH];
            char name[MAX_NAME_LENGTH + 1];
            coordinate location[MAX_SHIP_LENGTH];

            ship();
        };

        // There are 3 char's per row, first index is y, second is x
        char shipBoard[BOARD_ROWS][ROW_LENGTH];
        char attackBoard[BOARD_ROWS][ROW_LENGTH];

        ship playerShips[NUM_OF_SHIPS];

        attackResult wasShipHit(coordinate location, int shipIndex);

    public:
        board();
        bool lost() const;
        bool readShipInfo(std::ifstream &file, char *shipName, int &shipLength) const;
        bool spaceAvailable(playerBoards boardChecking, coordinate locationChecking) const;
        bool incrementCoordinate(direction shipDirection, coordinate &coordinateIncrementing) const;
        bool placeShip(int shipIndex, int length, char name[MAX_NAME_LENGTH + 1], direction directionFacing, coordinate startingCoord);
        void printShip(int shipIndex);
        void updateBoard(playerBoards board, attackResult result, coordinate coordinateAttacked);
        void printBoard(playerBoards boardPrinting) const;
        void configureCoordinate(coordinate &coordinateConfiguring) const;
        attackResult attacked(coordinate locationHit);
};

#endif