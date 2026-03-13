#include <board.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <cctype>
#include <string>
#include <stdexcept>

#include <player.h>

// Gets an integer from the user that falls between 2 integers (with those integers included). Rejects other input
int getIntBetween(int lowerBound, int upperBound, std::string phrase)
{
    int num;
    while (true)
    {
        std::cout << phrase;
        std::cin >> num;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else if (num >= lowerBound && num <= upperBound)
        {
            break;
        }
    }
    return num;
}

// Gets a coordinate from the user
board::coordinate player::getCoordinate(const char* question) const
{
    std::string strCoordinate;
    board::coordinate shipCoordinate;

    while (true)
    {
        std::cout << question;
        std::getline(std::cin >> std::ws, strCoordinate);
        int coordLength = strCoordinate.length();

        // Check the length of the coordinate
        if(coordLength != 3 && coordLength != 2)
        {
            std::cout << "Coordinate must consist of a letter (A-J) and a number (1-10)\n";
            continue;
        }


        // Check the letter coordinate
        if (!std::isalpha(strCoordinate[0]))
        {
            std::cout << "First Coordinate must be Letter \n";
            continue;
        }
        else
        {
            shipCoordinate.y = std::toupper(strCoordinate[0]) - 'A' + 1;
            strCoordinate.erase(0, 1);
            if (shipCoordinate.y > 'J')
            {
                std::cout << "First Coordinate must be (A-J)\n";
                continue;
            }
        }
        
        // Validate the number part of the coordinate
        try
        {
            shipCoordinate.x = std::stoi(strCoordinate);
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "Invalid number Coordinate\n";
            continue;
        }

        // Check if the number coordinate is within range
        if (shipCoordinate.x < 1 || shipCoordinate.x > 10)
        {
            std::cout << "Number Coordinate must be between 1 and 10\n";
            continue;
        }
        break;
    }

    return shipCoordinate;
}

// Gets a direction for the ship from the user
board::direction player::getDirection(const char* question) const
{
    int option = getIntBetween(1, 4, question);
    switch (option)
    {
        case 1: 
            return board::direction::up;
        case 2: 
            return board::direction::down;
        case 3: 
            return board::direction::left;
        case 4: 
            return board::direction::right;
        default:
            std::cerr << "getIntBetween() failed";
            return board::direction::right;
    }
}

// Places the ships onto the board
bool player::placeShips(const char* fileName)
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

    // Only read lines for how ever many ships the 
    for (int i = 0; i < board::NUM_OF_SHIPS; i++)
    {
        char shipName[board::MAX_NAME_LENGTH];
        int shipLength;

        if (!boards.readShipInfo(shipFile, shipName, shipLength))
        {
            return false;
        }

        boards.printBoard(board::playerBoards::shipBoard);
        std::cout << "You are placing your " << shipName << "!\tLength: " << shipLength << '\n';
        board::direction shipDirection = getDirection("How would you like to place your ship? (1.)Up (2.)Down (3.)Left (4.)Right\n");
        board::coordinate shipCoordinate;

        while (true)
        {
            shipCoordinate = getCoordinate("Where would you like to place your ship? (A1 - J10) ");
            if (boards.placeShip(i, shipLength, shipName, shipDirection, shipCoordinate))
            {
                break;
            }
            std::cout << "The space is currently occupied\n";
        }
    }

    shipFile.close();
    return true;
}

// Gets a coordinate from the user to attack
board::attackResult player::attack(board &opponentBoard)
{
    board::coordinate coordinateAttacking;
    do
    {   // Get a coordinate and configure it
        coordinateAttacking = getCoordinate("Where would you like to attack? (A1 - J10) ");
        boards.configureCoordinate(coordinateAttacking);
    }
    while(!boards.spaceAvailable(board::playerBoards::attackBoard, coordinateAttacking));

    board::attackResult result = opponentBoard.attacked(coordinateAttacking);
    boards.updateBoard(board::playerBoards::attackBoard, result, coordinateAttacking);
    return result;
}

// Get the action from the player
board::attackResult player::playerTurn(board &opponentBoard)
{
    while (true)
    {
        int option = getIntBetween(1, 3, "What would you like to do?\n 1. See ship board.\n 2. See attack Board.\n 3. Attack\n");
        switch (option)
        {
            case 1:
                boards.printBoard(board::playerBoards::shipBoard);
                break;
            case 2:
                boards.printBoard(board::playerBoards::attackBoard);
                break;
            case 3:
                return attack(opponentBoard);
            default:
                std::cerr << "Invalid option type";
                break;
        }
    }
}
