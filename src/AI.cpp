/*
** EPITECH PROJECT, 2022
** AI.cpp
** File description:
** AI.cpp
*/

#include "AI.hpp"
#include "BrainCommand.hpp"
#include "define.hpp"
#include "Printer.hpp"

#include <algorithm>
#include <cmath>
#include <future>
#include <thread>
#include <tuple>

void AI::turn(Board &board, std::size_t playerX, std::size_t playerY)
{
    std::size_t x = 0, y = 0;
    std::pair<std::size_t, std::size_t> nbFieldCell = board.countFieldCell();

    if (nbFieldCell.first == 0 && nbFieldCell.second == 0) {
        x = std::rand()%static_cast<int>(std::ceil(DEFAULT_BOARD_SIZE/3)) + static_cast<int>(std::ceil(DEFAULT_BOARD_SIZE/3));
        y = std::rand()%static_cast<int>(std::ceil(DEFAULT_BOARD_SIZE/3)) + static_cast<int>(std::ceil(DEFAULT_BOARD_SIZE/3));
        std::srand(time(NULL));
    } else if (nbFieldCell.first == 0 && nbFieldCell.second == 1) {
        _start(board, x, y, playerX, playerY);
    } else {
        _turn(board, x, y);
    }
    board.setPos(Board::CellState::FIRST_PLAYER, x, y);
    // board.printBoard();
    Printer::print(x, ",", y);
}

void AI::_start(Board &board, std::size_t &x, std::size_t &y, std::size_t playerX, std::size_t playerY)
{
    std::vector<int> possibleX = {-1, 0, 1}, possibleY = {-1, 0, 1};
    std::srand(time(NULL));

    if (playerX == 0)
        possibleX.erase(std::find(possibleX.begin(), possibleX.end(), -1));
    if (playerX == DEFAULT_BOARD_SIZE - 1)
        possibleX.erase(std::find(possibleX.begin(), possibleX.end(), 1));
    if (playerY == 0)
        possibleY.erase(std::find(possibleY.begin(), possibleY.end(), -1));
    if (playerY == DEFAULT_BOARD_SIZE - 1)
        possibleY.erase(std::find(possibleY.begin(), possibleY.end(), 1));
    x = playerX + possibleX.at(std::rand()%possibleX.size());
    y = playerY + possibleY.at(std::rand()%possibleY.size());
    while (!board.setPos(Board::CellState::FIRST_PLAYER, x, y)) {
        x = playerX + possibleX.at(std::rand()%possibleX.size());
        y = playerY + possibleY.at(std::rand()%possibleY.size());
    }
}

void AI::_turn(Board &board, std::size_t &x, std::size_t &y)
{
    bool def = false;
    std::size_t defX = 0, defY = 0, defSize = 0;
    std::vector<Board::CellAttribute> line = {};
    std::vector<Board::Direction> directions = {Board::Direction::HORIZONTAL, Board::Direction::VERTICAL,
                                                Board::Direction::LEFTTORIGHT, Board::Direction::RIGHTTOLEFT};

    for (std::size_t i = 0; i < board.getBoard().size(); i++) {
        for (std::size_t j = 0; j < board.getBoard().at(i).size(); j++) {
            for (auto &dir : directions) {
                line = board.getLineWithMidCell(dir, j, i);
                if (line.size() < 5)
                    continue;
                for (std::size_t lineIndex = 0; line.size() - lineIndex > 5; lineIndex++) {
                    if (_attack(x, y, line, lineIndex) == true)
                        return;
                    if (defSize < 4) {
                        if (def == true) {
                            _defend(defX, defY, defSize, line, lineIndex);
                        } else {
                            def = _defend(defX, defY, defSize, line, lineIndex);
                        }
                    }
                }
            }
        }
    }
    if (def == true) {
        x = defX;
        y = defY;
    } else {
        _exploration(board, x, y);
    }
}

bool AI::_attack(std::size_t &x, std::size_t &y, std::vector<Board::CellAttribute> line, std::size_t lineIndex)
{
    for (auto &pattern : Board::attackPattern) {
        for (std::size_t ptnIndex = 0; ptnIndex < 5; ptnIndex++) {
            if (line.at(lineIndex + ptnIndex).field != pattern.at(ptnIndex))
                break;
            if (line.at(lineIndex + ptnIndex).field == Board::CellState::EMPTY) {
                x = line.at(lineIndex + ptnIndex).posX;
                y = line.at(lineIndex + ptnIndex).posY;
            }
            if (ptnIndex == 4)
                return (true);
        }
    }
    return (false);
}

bool AI::_defend(std::size_t &defX, std::size_t &defY, std::size_t &defSize, std::vector<Board::CellAttribute> line, std::size_t lineIndex)
{
    for (auto &pattern : Board::defensePattern) {
        std::size_t emptyX = 0, emptyY = 0, tmpDefSize = 0;
        for (std::size_t ptnIndex = 0; ptnIndex < 5; ptnIndex++) {
            if (line.at(lineIndex + ptnIndex).field != pattern.at(ptnIndex))
                break;
            if (line.at(lineIndex + ptnIndex).field == Board::CellState::EMPTY) {
                emptyX = line.at(lineIndex + ptnIndex).posX;
                emptyY = line.at(lineIndex + ptnIndex).posY;
            }
            if (line.at(lineIndex + ptnIndex).field == Board::CellState::SECOND_PLAYER)
                tmpDefSize++;
            if (ptnIndex == 4) {
                defX = emptyX;
                defY = emptyY;
                defSize = tmpDefSize;
                return (true);
            }
        }
    }
    return (false);
}

void AI::_exploration(Board &board, std::size_t &x, std::size_t &y)
{
    int score = -9999;
    std::future<std::tuple<int, std::size_t, std::size_t>> f[DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE];

    // Exploration a un de profondeur
    for (std::size_t i = 0; i < board.getBoard().size(); i++) {
        for (std::size_t j = 0; j < board.getBoard().at(i).size(); j++) {
            board.resetPredictionBoard();
            f[(i * DEFAULT_BOARD_SIZE) + j] = std::async(std::launch::async, &AI::_threadFunc, board, i, j);
        }
    }
    for (auto &it : f)
        it.wait();
    for (auto &it : f) {
        std::tuple<int, std::size_t, std::size_t> results = it.get();
        if (std::get<0>(results) > score) {
            score = std::get<0>(results);
            x = std::get<1>(results);
            y = std::get<2>(results);
        }
    }
}

// void AI::_exploration(Board &board, std::size_t &x, std::size_t &y)
// {
//     int score = -9999;
//     std::future<std::tuple<int, std::size_t, std::size_t>> f[8];
//     Board::CellAttribute lastPos = board.getLastPos().first;
//     std::vector<Board::CellAttribute> possiblePos = {
//             {lastPos.posX - 1, lastPos.posY - 1, lastPos.field}, {lastPos.posX, lastPos.posY - 1, lastPos.field},
//             {lastPos.posX + 1, lastPos.posY - 1, lastPos.field}, {lastPos.posX + 1, lastPos.posY, lastPos.field},
//             {lastPos.posX + 1, lastPos.posY + 1, lastPos.field}, {lastPos.posX, lastPos.posY + 1, lastPos.field},
//             {lastPos.posX - 1, lastPos.posY + 1, lastPos.field}, {lastPos.posX - 1, lastPos.posY, lastPos.field}
//         };

//     // Exploration a un de profondeur
//     for (std::size_t posIndex = 0; posIndex < possiblePos.size(); posIndex++) {
//         board.resetPredictionBoard();
//         f[posIndex] = std::async(std::launch::async, &AI::_threadFunc, board, possiblePos.at(posIndex).posY, possiblePos.at(posIndex).posX);
//     }
//     for (auto &it : f)
//         it.wait();
//     for (auto &it : f) {
//         std::tuple<int, std::size_t, std::size_t> results = it.get();
//         if (std::get<0>(results) > score) {
//             score = std::get<0>(results);
//             x = std::get<1>(results);
//             y = std::get<2>(results);
//         }
//     }
// }

std::tuple<int, std::size_t, std::size_t> AI::_threadFunc(Board board, std::size_t i, std::size_t j)
{
    if (board.setPredictionPos(Board::CellState::FIRST_PLAYER, j, i) == false)
        return (std::make_tuple(-9999, j, i));
    return (std::make_tuple(board.evaluation(), j, i));
    // return (std::make_tuple(AI::_playerExploration(board, j, i), j, i));
}
