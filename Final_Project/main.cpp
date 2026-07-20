#include <iostream>
#include "chess.hpp"
#include <string>
#include <sstream>
#include "search.hpp"
#include "nnue_eval.hpp"


using namespace chess;
using namespace std;

int main() {
    nnue.load_model("model.nnue"); // Load weights once on startup
    Board board; // initialize chess board 
    string line;
    
    while (getline(cin, line)) {
        stringstream ss(line);
        string command;
        ss >> command;
        
        if (command == "uci") {
            cout << "id name MyFirstChessBot" << endl;
            cout << "id author Nivin" << endl;
            cout << "uciok" << endl;
        }
        else if (command == "isready") {
            cout << "readyok" << endl;
        }
        else if (command == "position") {
            string type;
            ss >> type;
            if (type == "startpos") {
                board = Board(); // reset to start position
            }
            
            string extra;
            ss >> extra;
            if (extra == "moves") {
                string move_str;
                while (ss >> move_str) {
                    Move m = uci::uciToMove(board, move_str);
                    board.makeMove(m);
                }
            }
        }
        else if (command == "go") {        
            int wtime = 0, btime = 0;
            string arg;
            
            while (ss >> arg) {
                if (arg == "wtime") ss >> wtime;
                else if (arg == "btime") ss >> btime;
            }

            int current_player_time = (board.sideToMove() == Color::WHITE) ? wtime : btime;
            
            // Spend roughly 1/40th of our total remaining time allocation
            // Fall back to 1000ms if no clock parameter is provided
            if (current_player_time > 0) {
                max_time = current_player_time / 40;
            } else {
                max_time = 1000;
            }

            // Run time-capped iterative deepening search
            Move best_move = iterative_deepening(board);
            
            cout << "bestmove " << uci::moveToUci(best_move) << endl;
        }
        else if (command == "quit") {
            break;
        }
    }

    return 0;
}
