#include <iostream>
#include <optional>

#include <player.h>
#include <bot.h>
#include <game.h>

game::game()
{
    m_running = true;
}

int game::run()
{
    if (init())
    {
        return -1;
    }

    while(m_running)
    {
        board::attackResult playerResult = m_player->playerTurn(m_bot->boards);
        if (playerResult == board::attackResult::opponentDefeated)
        {
            std::cout << "You Won!\n";
            m_running = false;
            break;
        }

        board::attackResult botResult = m_bot->botTurn(m_player->boards);
        if(botResult == board::attackResult::opponentDefeated)
        {
            std::cout << "You lost!\n";
            m_running = false;
            break;
        }

        DEBUG();
    }

    return 0;
}

bool game::init()
{
    try
    {
        m_player.emplace();
        m_bot.emplace();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return false;
    }
}

void game::update()
{
    return;
}

// Prints the boards | DEBUG ONLY
void game::DEBUG()
{
    std::cout << "\t\tATTACK BOARDS:\n";
    m_player->boards.printBoard(board::playerBoards::attackBoard);
    m_bot->boards.printBoard(board::playerBoards::attackBoard);
    std::cout << "\t\tSHIP BOARDS:\n";
    m_player->boards.printBoard(board::playerBoards::shipBoard);
    m_bot->boards.printBoard(board::playerBoards::shipBoard);
}