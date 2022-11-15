/*
** EPITECH PROJECT, 2022
** Board.hpp
** File description:
** Board.hpp
*/

#pragma once

#include <string>
#include <vector>

class Board {
    enum CellState { EMPTY, FIRST_PLAYER, SECOND_PLAYER };
    public:
        Board();
        ~Board() = default;

        /* Getter */
        bool empty() const;
        bool isGameStarted() const;

        /* Setter */
        void setBoard(std::size_t size);
        void setPos(CellState cell, std::size_t x, std::size_t y);

    private:
        std::vector<std::vector<CellState>> _board;
        bool _gameStarted;
};
