add_executable(${PROJECT_NAME} 
    board.cpp 
    bot.cpp 
    player.cpp 
    main.cpp
    game.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/inc
)