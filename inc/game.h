#ifndef GAME_H_
#define GAME_H_

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
        void update();
        void DEBUG();
};

#endif