#ifndef CHESS_H
#define CHESS_H

#include <vector>
#include <algorithm>
#include "chess.hpp"

using namespace chess;
using namespace std;

const int infinity = 1000000;

class ChessEngine {
private:
    int get_utility_given_terminal_state(Board& board, int depth) {
        auto game_over_info = board.isGameOver();
        if (game_over_info.first == GameResultReason::CHECKMATE) {
            if (board.sideToMove() == Color::WHITE) {
                return -100000 + depth;
            } else {
                return 100000 - depth;
            }
        }
        return 0;
    }

public:
    // Pure structural Minimax with Alpha-Beta that takes the working board reference directly
    int minimax_alpha_beta(Board& board, int depth, int alpha, int beta, vector<Move>& best_moves_vec) {
        auto game_over_info = board.isGameOver();
        
        if (game_over_info.second != GameResult::NONE) {
            return get_utility_given_terminal_state(board, depth);
        }
        if (depth == 0) {
            return 0;
        }

        // We use a universal auto collection loop here to bypass MvList vs MoveList version issues entirely
        auto moves = MvList();
        movegen::legalmoves(moves, board);

        if (board.sideToMove() == Color::WHITE) {
            int max_value = -infinity;
            for (const auto& move : moves) {
                vector<Move> child_moves;
                board.makeMove(move);
                int evaluation = minimax_alpha_beta(board, depth - 1, alpha, beta, child_moves);
                board.unmakeMove(move);

                if (evaluation > max_value) {
                    max_value = evaluation;
                    best_moves_vec.clear();
                    best_moves_vec.push_back(move);
                    best_moves_vec.insert(best_moves_vec.end(), child_moves.begin(), child_moves.end());
                }
                alpha = max(alpha, evaluation);
                if (alpha >= beta) {
                    break;
                }
            }
            return max_value;
        } 
        else {
            int min_value = infinity;
            for (const auto& move : moves) {
                vector<Move> child_moves;
                board.makeMove(move);
                int evaluation = minimax_alpha_beta(board, depth - 1, alpha, beta, child_moves);
                board.unmakeMove(move);

                if (evaluation < min_value) {
                    min_value = evaluation;
                    best_moves_vec.clear();
                    best_moves_vec.push_back(move);
                    best_moves_vec.insert(best_moves_vec.end(), child_moves.begin(), child_moves.end());
                }
                beta = min(beta, evaluation);
                if (alpha >= beta) {
                    break;
                }
            }
            return min_value;
        }
    }
};

#endif // CHESS_H
