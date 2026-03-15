#include <bot.h>
#include <random>
#include <iostream>
#include <cstring>

const board::direction directions[4] = 
    { board::direction::up, board::direction::left, board::direction::down, board::direction::right };
    

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> genCoord(1, 10);
std::uniform_int_distribution<int> genDirection(0, 3);

bot::bot()
{
    m_mode = attackMode::scout;
    m_shipQueueIndex = 0;
    for (int i = 0; i < 5; i++)
    {
        m_shipQueue[i].knowsDirection = false;
        m_shipQueue[i].currentCoordinate = NUL_COORDINATE;
        m_shipQueue[i].startingCoordinate = NUL_COORDINATE;
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
        char shipName[board::MAX_SHIP_NAME_LENGTH];
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
    } while(!boards.isSpaceAvailable(board::playerBoards::attackBoard, coordAttacking));

    board::attackResult result = opponentBoard.attacked(coordAttacking);

    // If we find a ship, we then try to find it's direction
    if (result == board::attackResult::discover)
    {
        m_shipQueue[0].startingCoordinate = coordAttacking;
        m_shipQueue[0].name = opponentBoard.getShipName(coordAttacking);
        m_shipQueueIndex = 0;
        m_mode = attackMode::findDirection;
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
        coordAttacking = m_shipQueue[m_shipQueueIndex].startingCoordinate;
        directionAttacking = randDirection();

        // Changes the coordinate attacking, therefore needs to be ran outside loop condition
        if (!boards.incrementCoordinate(directionAttacking, coordAttacking))
        {
            continue;
        }
    } while (!boards.isSpaceAvailable(board::playerBoards::attackBoard, coordAttacking));

    // Attack the ship and take action with it's result
    board::attackResult result = opponentBoard.attacked(coordAttacking);
    switch (result)
    {
        // switch to attack the new ship
        case board::attackResult::discover: 
            switchToNewShip(opponentBoard.getShipName(coordAttacking), coordAttacking);
            break;

        // Store the direction and the coordinate that was just hit and change m_mode to attack
        case board::attackResult::hit: 
            m_shipQueue[m_shipQueueIndex].shipDirection = directionAttacking;
            m_shipQueue[m_shipQueueIndex].currentCoordinate = coordAttacking;
            m_shipQueue[m_shipQueueIndex].knowsDirection = true;
            m_mode = attackMode::attack;
            break;

        // If it was a ship of length 2, it would've sank.
        case board::attackResult::sank: 
            switchToQueuedShip(opponentBoard, coordAttacking);
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
    board::coordinate coordAttacking = m_shipQueue[m_shipQueueIndex].currentCoordinate;
    board::attackResult result;

    // If we are at the edge of the board or if space is not available, we switch directions and continue attacking because the boat must continue in the other direction
    if (
        !boards.incrementCoordinate(m_shipQueue[m_shipQueueIndex].shipDirection, coordAttacking) || 
        !boards.isSpaceAvailable(board::playerBoards::attackBoard, coordAttacking)
    )
    {
        // switch directions to the opposite one and go back to the beginning of the ship
        m_shipQueue[m_shipQueueIndex].shipDirection = oppositeDirection(m_shipQueue[m_shipQueueIndex].shipDirection);
        coordAttacking = m_shipQueue[m_shipQueueIndex].startingCoordinate;
        boards.incrementCoordinate(m_shipQueue[m_shipQueueIndex].shipDirection, coordAttacking);
    }

    result = opponentBoard.attacked(coordAttacking);

    switch (result)
    {
        // Change direction because the ship must go in the other direction
        case board::attackResult::miss: 
            switchDirection();
            break;

        // Change direction because the ship must go in the other direction and switch to attacking the new ship
        case board::attackResult::discover: 
            switchDirection();
            switchToNewShip(opponentBoard.getShipName(coordAttacking), coordAttacking);
            break;

        // Continue attacking in the same direction
        case board::attackResult::hit: 
            m_shipQueue[m_shipQueueIndex].currentCoordinate = coordAttacking;
            break;

        // Delete the queue info and switch to another queued ship or find another one
        case board::attackResult::sank:
            switchToQueuedShip(opponentBoard, coordAttacking);
            break;
        default:
            break;
    }

    boards.updateBoard(board::playerBoards::attackBoard, result, coordAttacking);
    return result;
}

// Check if the original targeting ship was sank or a ship of length 1 was sank instead
bool bot::originalShipSink(const char* nameOfShipHit) const
{
    if (std::strcmp(m_shipQueue[m_shipQueueIndex].name, nameOfShipHit) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Resets the current element and checks the queue for any ships available. Switches attack mode based on queue
void bot::switchToQueuedShip(board opponentBoard, board::coordinate coordAttacked)
{
    // Check if the original ship was sank, if so switch to the queue, else, switch directions
    if (originalShipSink(opponentBoard.getShipName(coordAttacked)))
    {
        m_shipQueue[m_shipQueueIndex].currentCoordinate = NUL_COORDINATE;
        m_shipQueue[m_shipQueueIndex].startingCoordinate = NUL_COORDINATE;
        m_shipQueue[m_shipQueueIndex].knowsDirection = false;
        
        if (m_shipQueueIndex == 0)
        {
            m_mode = attackMode::scout;
        }
        else
        {
            m_shipQueueIndex--;
            if(m_shipQueue[m_shipQueueIndex].knowsDirection)
            {
                m_mode = attackMode::attack;
            }
            else
            {
                m_mode = attackMode::findDirection;
            }
        }
    }
    else
    {
        switchDirection();
    }
    
}

// Swtich to target the opposite direction of a ship
void bot::switchDirection()
{
    m_shipQueue[m_shipQueueIndex].shipDirection = oppositeDirection(m_shipQueue[m_shipQueueIndex].shipDirection);
    m_shipQueue[m_shipQueueIndex].currentCoordinate =  m_shipQueue[m_shipQueueIndex].startingCoordinate;
}

// Switch to a newly found ship and mark it's first location.
void bot::switchToNewShip(const char* shipName, board::coordinate coordAttacked)
{
    m_shipQueueIndex++;
    m_shipQueue[m_shipQueueIndex].startingCoordinate = coordAttacked;
    m_shipQueue[m_shipQueueIndex].name = shipName;
    m_mode = attackMode::findDirection;
}

board::attackResult bot::botTurn(board& opponentBoard)
{
    switch (m_mode)
    {
        case attackMode::scout: 
            return scout(opponentBoard);
        case attackMode::findDirection: 
            return findDirection(opponentBoard);
        case attackMode::attack: 
            return attack(opponentBoard);
        default:
            std::cerr << "m_mode switch failed";
            return board::attackResult::miss;
    }
}
