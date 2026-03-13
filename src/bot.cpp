#include <bot.h>
#include <random>
#include <iostream>

const board::direction directions[4] = 
    { board::direction::up, board::direction::left, board::direction::down, board::direction::right };
    

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> genCoord(1, 10);
std::uniform_int_distribution<int> genDirection(0, 3);

bot::bot()
{
    mode = attackMode::scout;
    queueIndex = 0;
    for (int i = 0; i < 5; i++)
    {
        shipQueue[i].knowsDirection = false;
        shipQueue[i].currentCoordinate = NUL_COORDINATE;
        shipQueue[i].startingCoordinate = NUL_COORDINATE;
    }
}

board::direction bot::randDirection() const
{
    return directions[genDirection(gen)];
}

board::coordinate bot::randCoord() const
{
    return {genCoord(gen), genCoord(gen)};
}

bool bot::placeShips(const char* fileName)
{
    std::ifstream shipFile;
    std::string firstLine;
    shipFile.open(fileName);
    if (!shipFile.is_open())
    {
        std::cerr << "Unable to open ship.csv file";
        return false;
    }

    // Disgard the first line of the file
    getline(shipFile, firstLine);
    for (int i = 0; i < board::NUM_OF_SHIPS; i++)
    {
        char shipName[board::MAX_NAME_LENGTH];
        int shipLength;

        if (!boards.readShipInfo(shipFile, shipName, shipLength))
        {
            return false;
        }

        while (!boards.placeShip(i, shipLength, shipName, randDirection(), randCoord()));
    }

    shipFile.close();
    return true;
}

// Change to the opposite direction
board::direction bot::oppositeDirection(board::direction direction)
{
    return static_cast<board::direction>(-static_cast<int>(direction));
}

// Attack a random coordinate until a ship is found. A bot only scouts when there are no ships in queue
board::attackResult bot::scout(board& opponentBoard)
{
    board::coordinate coordAttacking;

    // Get a random coordinate and attack it
    do
    {
        coordAttacking = randCoord();
        boards.configureCoordinate(coordAttacking);
    } while(!boards.spaceAvailable(board::playerBoards
        ::attackBoard, coordAttacking));

    board::attackResult result = opponentBoard.attacked(coordAttacking);

    // If we find a ship, we then try to find it's direction
    if (result == board::attackResult::discover)
    {
        shipQueue[0].startingCoordinate = coordAttacking;
        queueIndex = 0;
        mode = attackMode::findDirection;
    }
    boards.updateBoard(board::playerBoards::attackBoard, result, coordAttacking);
    return result;
}

// We only know where the ship was hit and are trying to find its direction
board::attackResult bot::findDirection(board& opponentBoard)
{
    board::coordinate coordAttacking;
    board::direction directionAttacking;

    // Generate a random direction to attack the ship from
    do
    {
        coordAttacking = shipQueue[queueIndex].startingCoordinate;
        directionAttacking = randDirection();

        // Changes the coordinate attacking, therefore needs to be ran outside loop condition
        if (!boards.incrementCoordinate(directionAttacking, coordAttacking))
        {
            continue;
        }
    } while (!boards.spaceAvailable(board::playerBoards
        ::attackBoard, coordAttacking));

    // Attack the ship and take action with it's result
    board::attackResult result = opponentBoard.attacked(coordAttacking);
    switch (result)
    {
        // The mode will remain find direction, it will just change it to the newly found ship
        case board::attackResult::discover: 
            queueIndex++;
            shipQueue[queueIndex].startingCoordinate = coordAttacking;
            break;

        // Store the direction and the coordinate that was just hit and change mode to attack
        case board::attackResult::hit: 
            shipQueue[queueIndex].shipDirection = directionAttacking;
            shipQueue[queueIndex].currentCoordinate = coordAttacking;
            shipQueue[queueIndex].knowsDirection = true;
            mode = attackMode::attack;
            break;

        // If it was a ship of length 2, it would've sank. We delete the queue info and switch to either scout to another ship
        case board::attackResult::sank: 
            shipQueue[queueIndex].currentCoordinate = NUL_COORDINATE;
            shipQueue[queueIndex].startingCoordinate = NUL_COORDINATE;
            if (queueIndex == 0)
            {
                mode = attackMode::scout;
            }
            else
            {
                queueIndex--;
                if (shipQueue[queueIndex].knowsDirection)
                {
                    mode = attackMode::attack;
                }
                else
                {
                    mode = attackMode::findDirection;
                }
            }
            break;
        default:
            break;
    }

    boards.updateBoard(board::playerBoards::attackBoard, result, coordAttacking);
    return result;
}

// We know the direction of the ship, and are attacking in that direction
board::attackResult bot::attack(board& opponentBoard)
{
    board::coordinate coordAttacking = shipQueue[queueIndex].currentCoordinate;
    board::attackResult result;

    // If we are at the edge of the board or if space is not available, we switch directions and continue attacking because the boat must continue in the other direction
    if (
        !boards.incrementCoordinate(shipQueue[queueIndex].shipDirection, coordAttacking) || 
        !boards.spaceAvailable(board::playerBoards
            ::attackBoard, coordAttacking)
    )
    {
        // switch directions to the opposite one and go back to the beginning of the ship
        shipQueue[queueIndex].shipDirection = oppositeDirection(shipQueue[queueIndex].shipDirection);
        coordAttacking = shipQueue[queueIndex].startingCoordinate;
        boards.incrementCoordinate(shipQueue[queueIndex].shipDirection, coordAttacking);
    }

    result = opponentBoard.attacked(coordAttacking);

    switch (result)
    {
        // Change direction because the ship must go in the other direction
        case board::attackResult::miss: 
            shipQueue[queueIndex].shipDirection = oppositeDirection(shipQueue[queueIndex].shipDirection);
            shipQueue[queueIndex].currentCoordinate = shipQueue[queueIndex].startingCoordinate;
            break;

        // Change direction because the ship must go in the other direction and switch to attacking the new ship
        case board::attackResult::discover: 
            shipQueue[queueIndex].shipDirection = oppositeDirection(shipQueue[queueIndex].shipDirection);
            shipQueue[queueIndex].currentCoordinate =  shipQueue[queueIndex].startingCoordinate;
            queueIndex++;
            shipQueue[queueIndex].startingCoordinate = coordAttacking;
            mode = attackMode::findDirection;
            break;

        // Continue attacking in the same direction
        case board::attackResult::hit: 
            shipQueue[queueIndex].currentCoordinate = coordAttacking;
            break;

        // Delete the queue info and switch to another queued ship or find another one
        case board::attackResult::sank: 
            shipQueue[queueIndex].currentCoordinate = NUL_COORDINATE;
            shipQueue[queueIndex].startingCoordinate = NUL_COORDINATE;
            shipQueue[queueIndex].knowsDirection = false;
            if (queueIndex == 0)
            {
                mode = attackMode::scout;
            }
            else
            {
                queueIndex--;
                if(shipQueue[queueIndex].knowsDirection)
                {
                    mode = attackMode::attack;
                }
                else
                {
                    mode = attackMode::findDirection;
                }
            }
            break;
        default:
            break;
    }

    boards.updateBoard(board::playerBoards::attackBoard, result, coordAttacking);
    return result;
}

board::attackResult bot::botTurn(board& opponentBoard)
{
    switch (mode)
    {
        case attackMode::scout: 
            return scout(opponentBoard);
        case attackMode::findDirection: 
            return findDirection(opponentBoard);
        case attackMode::attack: 
            return attack(opponentBoard);
        default:
            std::cerr << "mode switch failed";
            return board::attackResult::miss;
    }
}
