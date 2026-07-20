#pragma once
#include "chess.hpp"
#include "nnue_eval.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <vector>

using namespace chess;
inline NNUEEvaluator nnue;
// Time control stuff
inline int max_time = 1000; 
inline auto start_time = std::chrono::high_resolution_clock::now();
inline bool stop_search = false;

// --- TRANSPOSITION TABLE (MEMORY BANK) ---
struct TTEntry {
    uint64_t key;   // Unique board hash (zobrist)
    int depth;      // How deep we searched this position
    int score;      // The evaluation score we found
    Move best_move; // The best move from this position
};

const int TT_SIZE = 1048576;
inline TTEntry transposition_table[TT_SIZE];

// Corrected evaluation function: relative to side to move
inline int evaluate(const Board& board) {
    return nnue.evaluate(board);
}

// --- QUIESCENCE SEARCH (SAFETY NET) ---
inline int quiescence(Board& board, int alpha, int beta) {
    if (stop_search) return 0;

    static int q_nodes = 0;
    q_nodes++;
    if (q_nodes % 1024 == 0) {
        auto current_time = std::chrono::high_resolution_clock::now();
        int time_passed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (time_passed >= max_time) {
            stop_search = true;
            return 0;
        }
    }

    int stand_pat = evaluate(board);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    Movelist moves;
    movegen::legalmoves(moves, board);

    std::vector<Move> captures;
    for (int i = 0; i < moves.size(); i++) {
        if (board.isCapture(moves[i])) {
            captures.push_back(moves[i]);
        }
    }

    std::sort(captures.begin(), captures.end(), [&](const Move& a, const Move& b) {
        return board.isCapture(a) > board.isCapture(b);
    });

    for (size_t i = 0; i < captures.size(); i++) {
        board.makeMove(captures[i]);
        int score = -quiescence(board, -beta, -alpha);
        board.unmakeMove(captures[i]);

        if (stop_search) return 0;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

inline int negamax(Board& board, int depth, int alpha, int beta, Move& best_move) {
    if (stop_search) return 0;
    
    static int nodes = 0;
    nodes++;
    if (nodes % 1024 == 0) {
        auto current_time = std::chrono::high_resolution_clock::now();
        int time_passed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (time_passed >= max_time) {
            stop_search = true;
            return 0;
        }
    }

    uint64_t hash = board.zobrist();
    int tt_index = hash % TT_SIZE;
    TTEntry entry = transposition_table[tt_index];

    if (entry.key == hash && entry.depth >= depth) {
        best_move = entry.best_move;
        return entry.score;
    }

    if (depth == 0) {
        return quiescence(board, alpha, beta);
    }
    
    Movelist moves;
    movegen::legalmoves(moves, board);
    
    if (moves.size() == 0) {
        // Correct checkmate score: adjust for depth so it prefers faster mates
        if (board.inCheck()) return -100000 + (20 - depth); 
        return 0; 
    }
    
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        return board.isCapture(a) > board.isCapture(b);
    });
    
    int best_score = -1000000;
    Move local_best = moves[0];
    
    for (int i = 0; i < moves.size(); i++) {
        board.makeMove(moves[i]);
        Move dummy;
        int score = -negamax(board, depth - 1, -beta, -alpha, dummy);
        board.unmakeMove(moves[i]);
        
        if (stop_search) return 0; 
        
        if (score > best_score) {
            best_score = score;
            local_best = moves[i];
        }
        
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            break; 
        }
    }
    
    best_move = local_best;

    if (!stop_search) {
        transposition_table[tt_index] = { hash, depth, best_score, best_move };
    }

    return best_score;
}

// Loop through depths and output info for Cutechess UI
inline Move iterative_deepening(Board& board) {
    start_time = std::chrono::high_resolution_clock::now();
    stop_search = false;
    
    Move absolute_best;
    Movelist moves;
    movegen::legalmoves(moves, board);
    if (moves.size() > 0) absolute_best = moves[0]; 

    for (int depth = 1; depth <= 20; depth++) {
        Move current_best;
        int score = negamax(board, depth, -1000000, 1000000, current_best);
        
        if (!stop_search) {
            absolute_best = current_best;
            
            // Print UCI info string so Cutechess populates the sidebar!
            auto now = std::chrono::high_resolution_clock::now();
            int ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
            std::cout << "info depth " << depth 
                      << " score cp " << score 
                      << " time " << ms 
                      << " pv " << uci::moveToUci(absolute_best) << std::endl;
        } else {
            break; 
        }
    }
    
    return absolute_best;
}
