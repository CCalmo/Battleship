#include <player.h>
#include <bot.h>

#include <iostream>
#include <filesystem>

void printResult(board::attackResult result);

int main()
{
    const char* shipInfoDirectory = "ships/ships.csv";
    player player;
    bot bot;

    // Let both players place ships or exit if something goes wrong
    if (!player.placeShips(shipInfoDirectory) || !bot.placeShips(shipInfoDirectory))
    {
        return -1;
    }

    // Futher Improvements:
    // Refactor the file input and reduce redundace in the player and bot.cpp
    // When you discover a ship, make you so that it is known which ship you found
    // Move the the logic of losing in the attack function in board.cpp

    while(true)
    {
        board::attackResult playerResult = player.playerTurn(bot.boards);
        printResult(playerResult);
        if (playerResult == board::attackResult::opponentDefeated)
        {
            std::cout << "You Won!\n";
            break;
        }

        board::attackResult botResult = bot.botTurn(player.boards);
        printResult(botResult);
        if(botResult == board::attackResult::opponentDefeated)
        {
            std::cout << "You lost!\n";
            break;
        }
        
        // /* DEBUG
        std::cout << "\t\tATTACK BOARDS:\n";
        player.boards.printBoard(board::playerBoards::attackBoard);
        bot.boards.printBoard(board::playerBoards::attackBoard);
        std::cout << "\t\tSHIP BOARDS:\n";
        player.boards.printBoard(board::playerBoards::shipBoard);
        bot.boards.printBoard(board::playerBoards::shipBoard);
        // */
    }

    return 0;
}

// Prints the corresponding result
void printResult(board::attackResult result)
{
    switch(result)
    {
        case board::attackResult::discover: std::cout << "Discovered\n"; break;
        case board::attackResult::hit: std::cout << "hit\n"; break;
        case board::attackResult::sank: std::cout << "sank\n"; break;
        case board::attackResult::miss: std::cout << "missed\n"; break;
        case board::attackResult::opponentDefeated: std::cout << "Defeated\n"; break;
    }
}