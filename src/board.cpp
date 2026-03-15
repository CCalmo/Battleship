#include <fstream>
#include <iostream>
#include <sstream>

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
    boardInit();
}

// Return the name of the ship that is found at a given coordinate, (returns "EMPTY" if no ship is found)
const char* board::getShipName(coordinate coordinateSearching) const
{
    // Search all ships return the name if one is found
    for (int i = 0; i < NUM_OF_SHIPS; i++)
    {
        for (int j = 0; j < m_playerShips[i].length; j++)
        {
            if (m_playerShips[i].location[j] == coordinateSearching)
            {
                return m_playerShips[i].name;
            }
        }
    }
    return "EMPTY\n";
}

// Check to see if the player has lost
bool board::lost() const
{
    // Go through all the ships and see if at least 1 is still alive
    for (int i = 0; i < NUM_OF_SHIPS; i++)
    {
        if (m_playerShips[i].alive)
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
        if (endOfName > board::MAX_SHIP_NAME_LENGTH)
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

// Check to see if the chosen board is empty
bool board::isSpaceAvailable(playerBoards boardChecking, coordinate locationChecking) const
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
bool board::placeShip(int currentShip, int length, char name[MAX_SHIP_NAME_LENGTH + 1], direction shipDirection, coordinate startingCoord)
{
    configureCoordinate(startingCoord);
    // Store the coordiantes into a temporary array in case the ship is out of bounds.
    for (int i = 0; i < length; i++)
    {
        if (!isSpaceAvailable(playerBoards::shipBoard, startingCoord))
            return false;
        
        m_playerShips[currentShip].location[i] = startingCoord;
        m_playerShips[currentShip].placesHit[i] = false;

        if (!incrementCoordinate(shipDirection, startingCoord) && i + 1 != length)
            return false;
    }
    m_playerShips[currentShip].length = length;
    std::strcpy(m_playerShips[currentShip].name, name);

    printShipOnBoard(currentShip);
    return true;
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
    // Set the default outcome to miss
    board::attackResult result = board::attackResult::miss;

    // Go through all ships and see if any got hit
    for (int shipIndex = 0; shipIndex < NUM_OF_SHIPS; shipIndex++)
    {
        // Skip the current ship if it's already sunk
        if (!m_playerShips[shipIndex].alive)
        {
            continue;
        }

        // If the result is a hit, no need to check the other ships
        result = wasShipHit(locationHit, shipIndex);
        if (result != board::attackResult::miss)
        {
            break;
        }
    }

    updateBoard(playerBoards::shipBoard, result, locationHit);

    // only when the a ship is sank, we check if the player lost
    if (result == board::attackResult::sank && lost())
    {
        result = board::attackResult::opponentDefeated;
    }

    return result;
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

void board::boardInit()
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

// Prints the ship onto the board
void board::printShipOnBoard(int currentShip)
{
    const static coordinate nulCoordinate = {0, 0};
    for (int i = 0; i < MAX_SHIP_LENGTH; i++)
    {
        if (m_playerShips[currentShip].location[i] == nulCoordinate)
        {
            return;
        }
        shipBoard[m_playerShips[currentShip].location[i].y][m_playerShips[currentShip].location[i].x] = '#';
    }
}

// Check if the coordinate that was hit has an occupying ship
board::attackResult board::wasShipHit(coordinate location, int currentShip)
{
    int hitsTaken = 0;
    bool shipWasHit = false;
    attackResult result;

    // Iterate though all the locations on the ship and track how many times it's hit
    for (int i = 0; i < m_playerShips[currentShip].length; i++)
    {
        // The ship is already hit at this location, we skip the check
        if (m_playerShips[currentShip].placesHit[i])
        {
            hitsTaken++;
            continue;
        }

        // The ship was hit at this location so we mark it as hit or discovered
        if (m_playerShips[currentShip].location[i] == location)
        {
            if(!m_playerShips[currentShip].discovered)
            {
                m_playerShips[currentShip].discovered = true;
                result = attackResult::discover;
            }
            else
            {
                result = attackResult::hit;
            }

            m_playerShips[currentShip].placesHit[i] = true;
            shipWasHit = true;
            hitsTaken++;
        }
    }

    // Check if the ship is down or if it never received a hit
    if (m_playerShips[currentShip].length == hitsTaken)
    {
        m_playerShips[currentShip].alive = false;
        result = attackResult::sank;
    }
    else if (!shipWasHit)
    {
        result = attackResult::miss;
    }


    return result;
}
