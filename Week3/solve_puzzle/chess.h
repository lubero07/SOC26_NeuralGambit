#ifndef CHESS_H
#define CHESS_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "chess.hpp" // Disservin's library

using namespace chess;
using namespace std;

const int infinity = 1000000;

class ChessEngine {
private:
    // Scoring system for final game states (Checkmate/Draws)
    int get_utility_given_terminal_state(int depth) {
        auto game_over_info = board.isGameOver();
        
        if (game_over_info.first == GameResultReason::CHECKMATE) {
            // The side whose turn it currently is has lost
            if (board.sideToMove() == Color::WHITE) {
                return -100000 + depth; // White is checkmated (Excellent for Black)
            } else {
                return 100000 - depth;  // Black is checkmated (Excellent for White)
            }
        }
        return 0; // Stalemate or structural draw
    }

    int eval() {
        return 0; // Neutral score if look-ahead limit is hit without a checkmate
    }

public:
    Board board;

    void loadFen(const string& fen) {
        board = Board(fen); // Correct alternative constructor pattern for FEN
    }

    // Traditional Minimax Tree Search with Alpha-Beta Pruning
    int minimax_alpha_beta(int depth, int alpha, int beta, vector<Move> &best_moves_vec) {
        auto game_over_info = board.isGameOver();
        
        // Base Case Checkpoints
        if (game_over_info.second != GameResult::NONE) {
            return get_utility_given_terminal_state(depth);
        }
        if (depth == 0) {
            return eval();
        }

        MoveList moves; 
        movegen::legalmoves(moves, board);

        // Maximizing Player Branch (White)
        if (board.sideToMove() == Color::WHITE) {
            int max_value = -infinity;
            
            for (const Move& move : moves) {
                vector<Move> child_moves;
                
                board.makeMove(move);
                int evaluation = minimax_alpha_beta(depth - 1, alpha, beta, child_moves);
                board.unmakeMove(move);

                if (evaluation > max_value) {
                    max_value = evaluation;
                    best_moves_vec.clear();
                    best_moves_vec.push_back(move);
                    best_moves_vec.insert(best_moves_vec.end(), child_moves.begin(), child_moves.end());
                }
                
                alpha = max(alpha, evaluation);
                if (alpha >= beta) {
                    break; // Beta Cutoff
                }
            }
            return max_value;
        } 
        // Minimizing Player Branch (Black)
        else {
            int min_value = infinity;
            
            for (const Move& move : moves) {
                vector<Move> child_moves;
                
                board.makeMove(move);
                int evaluation = minimax_alpha_beta(depth - 1, alpha, beta, child_moves);
                board.unmakeMove(move);

                if (evaluation < min_value) {
                    min_value = evaluation;
                    best_moves_vec.clear();
                    best_moves_vec.push_back(move);
                    best_moves_vec.insert(best_moves_vec.end(), child_moves.begin(), child_moves.end());
                }
                
                beta = min(beta, evaluation);
                if (alpha >= beta) {
                    break; // Alpha Cutoff
                }
            }
            return min_value;
        }
    }
};

#endif // CHESS_H
#endif // CHESS_H
