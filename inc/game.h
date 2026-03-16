#ifndef GAME_H_
#define GAME_H_

#include <optional>

#include <player.h>
#include <bot.h>

class game
{
    private:
        bool m_running;
        std::optional<player> m_player;
        std::optional<bot> m_bot;
    public:
        game();
        int run();
    private: 
        bool init();
        bool constructPlayers();
        void update();
        void DEBUG();
};

#endif