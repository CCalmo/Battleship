#include <fstream>
#include <iostream>

#include <board.h>

// Returns true if both x and y coordinates are the same
bool board::coordinate::operator==(const coordinate& comparing) const
{
    return x == comparing.x && y == comparing.y;
}

// If any one of the coordinates are greater, returns true
bool board::coordinate::operator>(const coordinate& comparing) const
{
    return x > comparing.x || y > comparing.y;
}

// If any one of the coodinates are less, returns true
bool board::coordinate::operator<(const coordinate& comparing) const
{
    return x < comparing.x || y < comparing.y;
}

// add the direction to the correct axis of the original coordinate
board::coordinate board::coordinate::operator+=(const direction& comparing)
{
    if (comparing == direction::up || comparing == direction::down)
    {
        y += static_cast<int>(comparing);
    }
    else
    {
        x += static_cast<int>(comparing);
    }
    return {x, y};
}

// subtract the direction to the correct axis of the original coordinate
board::coordinate board::coordinate::operator-=(const direction& comparing)
{
    if (comparing == direction::up || comparing == direction::down)
    {
        y -= static_cast<int>(comparing);
    }
    else
    {
        x -= static_cast<int>(comparing);
    }
    return {x, y};
}

// add the correct direction to the correct axis
board::coordinate board::coordinate::operator+(const direction& comparing) const
{
    if (comparing == direction::up || comparing == direction::down)
    {
       return { x, y + static_cast<int>(comparing) };
    }
    else
    {
        return { x, y + static_cast<int>(comparing) };
    }
}

board::ship::ship()
{
    alive = true;
    discovered = false;
    for (int i = 0; i < MAX_SHIP_LENGTH; i++)
    {
        location[i] = {0, 0};
        placesHit[i] = true;
    }
}

// Constructor, Initializes the board
board::board()
{
    constexpr int COLUMN_LABEL_INDEX = 1;
    char * const firstShipRow = shipBoard[0];
    char * const firstAttackRow = attackBoard[0];
    int asciiNum = '1';

    // Print the row for the column numbers
    for (int x = 0; x < ROW_LENGTH; x++)
    {
        // Print the row seperators
        if ((x + 1) % 4 == 0)
        {
            firstShipRow[x] = '|';
            firstAttackRow[x] = '|';
        }
        // Print the numbers for each column
        else if ((x + 1) % 2 == 0 && x != COLUMN_LABEL_INDEX)
        {
            // Print double digit columns
            if (asciiNum > '9')
            {
                int columnNumber = asciiNum - '0';

                // Tens place
                firstShipRow[x] = '0' + columnNumber / 10;
                firstAttackRow[x] = '0' + columnNumber / 10;
                x++;
                // Ones place
                firstShipRow[x] = '0' + columnNumber % 10;
                firstAttackRow[x] = '0' + columnNumber % 10;
            }
            // Print single digit column numbers
            else
            {
                firstShipRow[x] = static_cast<char>(asciiNum);
                firstAttackRow[x] = static_cast<char>(asciiNum);
            }
            asciiNum++;
        }
        // Else, it's an empty space
        else
        {
            firstShipRow[x] = ' ';
            firstAttackRow[x] = ' ';
        }
    }

    // Print the rest of the rows for the board
    for (int y = 1; y < BOARD_ROWS; y++)
    {
        for (int x = 0; x < ROW_LENGTH; x++)
        {
            // Print the letter for the first column
            if (x == COLUMN_LABEL_INDEX)
            {
                char rowLabel = 'A' + y - 1;
                shipBoard[y][x] = rowLabel;
                attackBoard[y][x] = rowLabel;
            }
            // Print the row seperator
            else if ((x + 1) % 4 == 0)
            {
                shipBoard[y][x] = '|';
                attackBoard[y][x] = '|';
            }
            // Print the water for the attack board
            else if ((x + 1) % 2 == 0)
            {
                shipBoard[y][x] = ' ';
                attackBoard[y][x] = '~';
            }
            // Else, it's just an empty space
            else 
            {
                shipBoard[y][x] = ' ';
                attackBoard[y][x] = ' ';
            }
        }
    }
}

// Check to see if the player has lost
bool board::lost() const
{
    // Go through all the ships and see if at least 1 is still alive
    for (int i = 0; i < NUM_OF_SHIPS; i++)
    {
        if (playerShips[i].alive)
        {
            return false;
        }
    }
    return true;
}

// Read one line at a time from the ship csv file
bool board::readShipInfo(std::ifstream &file, char *shipName, int &shipLength) const
{
    std::string fileLine;

    // The line in the file has two values, the name and the length
    while (true)
    {
        // If we reached the end of the file, return false.
        if (file.eof())
        {
            return false;
        }
        // Read a line from the file
        getline(file, fileLine);

        int endOfName = fileLine.find(',');
        int numIndex = fileLine.find(',') + 1;

        // If the name is longer than the max length, skip it
        if (endOfName > board::MAX_NAME_LENGTH)
        {
            continue;
        }

        // If after the file name the value is not a number or it's higher than the max length, skip it.
        if(!std::isdigit(fileLine[numIndex]) || fileLine[numIndex] - '0' > board::MAX_SHIP_LENGTH)
        {
            continue;
        }

        // Copy the name and length of the ship
        for (int i = 0; i < endOfName; i++)
        {
            shipName[i] = fileLine[i];
        }
        shipName[endOfName] = '\0';
        shipLength = fileLine[numIndex] - '0';
        break;
    }
    return true;
}

// Check to see if the chosen board is empty at a configured coordinate
bool board::spaceAvailable(playerBoards boardChecking, coordinate locationChecking) const
{
    const char (*board)[ROW_LENGTH] = nullptr;
    char emptyCharacter;
    switch (boardChecking)
    {
    case playerBoards::shipBoard:
        board = shipBoard;
        emptyCharacter = ' ';
        break;
    case playerBoards::attackBoard:
        board = attackBoard;
        emptyCharacter = '~';
        break;
    default:
        std::cerr << "Invalid Mode, select mode from the playerBoards enum class";
        return false;
    } 

    return board[locationChecking.y][locationChecking.x] == emptyCharacter;
}

// Prints the given board.
void board::printBoard(playerBoards boardPrinting) const
{
    const char (*board)[ROW_LENGTH] = nullptr;
    switch (boardPrinting)
    {
    case playerBoards::shipBoard:
        board = shipBoard;
        break;
    case playerBoards::attackBoard:
        board = attackBoard;
        break;
    default:
        std::cerr << "Invalid Mode, select mode from the playerBoards enum class";
        return;
    }
    // Go through each row in each column and print out the character
    for (int y = 0; y < BOARD_ROWS; y++)
    {
        for (int x = 0; x < ROW_LENGTH; x++)
        {
            std::cout << board[y][x];
        }
        std::cout << '\n';
    }
    std::cout << "\n\n";
}

// Configures the X axis of the coordiante
void board::configureCoordinate(coordinate &coordinateConfiguring) const
{
    coordinateConfiguring.x = coordinateConfiguring.x * EMPTY_ROW_SPACE + EMPTY_COLUMN_SPACE;
}

// Increment the coordinates based on the given direction and checks if they are within the board. Reverts the increment if outside bounds
bool board::incrementCoordinate(direction shipDirection, coordinate &coordinateIncrementing) const
{
    constexpr coordinate LOWER_BOUND = {5, 1};
    constexpr coordinate UPPER_BOUND = {41, 10};

    coordinateIncrementing += shipDirection;

    if (coordinateIncrementing < LOWER_BOUND || coordinateIncrementing > UPPER_BOUND)
    {
        coordinateIncrementing -= shipDirection;
        return false;
    }
    return true;
}

// Returns true if you can place the ship and edits the ship's info
bool board::placeShip(int currentShip, int length, char name[MAX_NAME_LENGTH + 1], direction shipDirection, coordinate startingCoord)
{
    configureCoordinate(startingCoord);

    // Store the coordiantes into a temporary array in case the ship is out of bounds.
    for (int i = 0; i < length; i++)
    {
        if (!spaceAvailable(playerBoards::shipBoard, startingCoord))
            return false;
        
        playerShips[currentShip].location[i] = startingCoord;
        playerShips[currentShip].placesHit[i] = false;

        if (!incrementCoordinate(shipDirection, startingCoord) && i + 1 != length)
            return false;
    }
    playerShips[currentShip].length = length;
    std::strcpy(playerShips[currentShip].name, name);

    printShip(currentShip);
    return true;
}

// Prints the ship onto the board
void board::printShip(int currentShip)
{
    const static coordinate nulCoordinate = {0, 0};
    for (int i = 0; i < MAX_SHIP_LENGTH; i++)
    {
        if (playerShips[currentShip].location[i] == nulCoordinate)
        {
            return;
        }
        shipBoard[playerShips[currentShip].location[i].y][playerShips[currentShip].location[i].x] = '#';
    }
}

// Check if the coordinate that was hit has an occupying ship
board::attackResult board::wasShipHit(coordinate location, int currentShip)
{
    int HitsTaken = 0;
    bool hit = false, sank = false, discovered = false;

    // Iterate though all the locations on the ship and see if it occupyies the coordinate attacked
    for (int i = 0; i < playerShips[currentShip].length; i++)
    {
        // The ship is already hit at this location so we just keep track of the hits
        if (playerShips[currentShip].placesHit[i])
        {
            HitsTaken++;
            continue;
        }

        // The ship was hit at this location so we mark it as hit or discovered
        if (playerShips[currentShip].location[i] == location)
        {
            hit = true;
            HitsTaken++;
            playerShips[currentShip].placesHit[i] = true;

            if (!playerShips[currentShip].discovered)
            {
                playerShips[currentShip].discovered = true;
                discovered = true;
            }
        }
    }

    // If the hits is equal to the hits taken, it's down
    if (playerShips[currentShip].length == HitsTaken)
    {
        playerShips[currentShip].alive = false;
        sank = true;
    }

    if (hit)
    {
        if (discovered)
        {
            return attackResult::discover;
        }
        else if (sank)
        {
            return attackResult::sank;
        }
        else
        {
            return attackResult::hit;
        }
    }
    // There was no hit
    return attackResult::miss;
}

// Updates a board to reflect the result of the either a received attack or attack done
void board::updateBoard(playerBoards board, attackResult result, coordinate coordinateAttacked)
{
    // Get the correct pointer to the board we are going to edit
    char (*boardUpdating)[ROW_LENGTH] = nullptr;
    if (board == playerBoards::shipBoard)
    {
        boardUpdating = shipBoard;
    }
    else
    {
        boardUpdating = attackBoard;
    }

   if (result == board::attackResult::miss)
    {
        boardUpdating[coordinateAttacked.y][coordinateAttacked.x] = 'O';
    }
    else
    {
        boardUpdating[coordinateAttacked.y][coordinateAttacked.x] = 'X';
    }
}

// Return a result based on an attack at a given configured coordinate
board::attackResult board::attacked(coordinate locationHit)
{
    board::attackResult result;
    for (int i = 0; i < NUM_OF_SHIPS; i++)
    {
        // Ship the current ship if it's already sank
        if (!playerShips[i].alive)
        {
            continue;
        }

        // If the result is not a miss, break out of this loop as we found the ship that was hit.
        // (if there was none, the result should be miss in the end)
        result = wasShipHit(locationHit, i);
        if (result != board::attackResult::miss)
        {
            break;
        }
    }

    updateBoard(playerBoards::shipBoard, result, locationHit);
    return result;
}
