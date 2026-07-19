#include <iostream>
#include "chess.hpp"
#include <string>
#include <sstream>

using namespace chess;
using namespace std;

int main(){
        
    Board board; // initialize chess board 
    string line;
    while(getline(cin,line)){
        stringstream ss(line);
        string command;
        ss >> command;
        if(command == "uci"){
            cout << "id name MyFirstChessBot" << endl;
            cout << "id author Nivin" << endl;
            cout << "uciok" << endl;
        }
        else if(command == "isready"){
            cout << "readyok" << endl;
        }
        else if(command == "position"){
            string type;
            ss >> type;
            if(type == "startpos"){
                board = Board();
            }
            string extra;
            ss >> extra;
            if(extra == "moves"){
                string move_str;
                while(ss >> move_str){
                    Move m = uci::uciToMove(board, move_str);
                    board.makeMove(m);
                }
            }
            // FIX 1: Removed the visual text board printout so the GUI doesn't get confused.
        }
        else if(command == "go"){        
            Movelist moves; // stores the moves
            movegen::legalmoves(moves, board); // generates legal moves
            if (moves.size() > 0) {
                // FIX 2: Changed to strictly output "bestmove <move>" format
                cout << "bestmove " << uci::moveToUci(moves[0]) << endl; 
            } else {
                cout << "bestmove 0000" << endl; // Safeguard if no moves are left
            }
        }
        
        else if(command == "quit"){
            break;
        }
    }

    return 0;
}    
