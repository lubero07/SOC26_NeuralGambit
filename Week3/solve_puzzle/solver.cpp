#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "chess.hpp"
#include "chess.h" // Links directly to your updated engine header

using namespace std;
using namespace chess;

// Lightweight custom text parser targeting puzzle JSON keys without dependencies
string extract_json_value(const string& file_content, const string& key, bool is_string) {
    size_t key_pos = file_content.find("\"" + key + "\"");
    if (key_pos == string::npos) return "";

    size_t colon_pos = file_content.find(":", key_pos);
    if (colon_pos == string::npos) return "";

    if (is_string) {
        size_t start_quote = file_content.find("\"", colon_pos + 1);
        size_t end_quote = file_content.find("\"", start_quote + 1);
        if (start_quote != string::npos && end_quote != string::npos) {
            return file_content.substr(start_quote + 1, end_quote - start_quote - 1);
        }
    } else {
        size_t start_pos = file_content.find_first_not_of(" \t\n\r\",", colon_pos + 1);
        size_t end_pos = file_content.find_first_of(" \t\n\r,}", start_pos);
        if (start_pos != string::npos) {
            return file_content.substr(start_pos, end_pos - start_pos);
        }
    }
    return "";
}

void execute_puzzle_file(const string& relative_path) {
    ifstream file(relative_path);
    if (!file.is_open()) return;

    // Load file stream context into a memory block string
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    string fen_layout = extract_json_value(content, "fen", true);
    string mate_in_str = extract_json_value(content, "mate_in", false);
    
    if (fen_layout.empty() || mate_in_str.empty()) return;

    int moves_to_mate = stoi(mate_in_str);
    int total_search_plies = moves_to_mate * 2; // Turns = full moves * 2

    cout << "-------------------------------------------" << endl;
    cout << "Processing File: " << relative_path << endl;
    cout << "Target Solution: Forced Mate-in-" << moves_to_mate << endl;
    cout << "-------------------------------------------" << endl;

    ChessEngine engine;
    engine.loadFen(fen_layout);

    vector<Move> winning_move_sequence;
    engine.minimax_alpha_beta(total_search_plies, -infinity, infinity, winning_move_sequence);

    cout << "Engine Verified Sequence: " << endl;
    Board display_board = engine.board;
    int step_counter = 1;
    
    for (const auto& individual_move : winning_move_sequence) {
        string uci_format = uci::moveToUci(individual_move);
        
        if (display_board.side_to_move() == Color::WHITE) {
            cout << "  Step " << step_counter << " (White): " << uci_format;
        } else {
            cout << " | (Black Forced Response): " << uci_format << endl;
            step_counter++;
        }
        display_board.make_move(individual_move);
    }
    cout << "\n===========================================\n\n";
}

int main() {
    // Matches the exact folder name inside your fork repository directory
    string puzzle_folder_path = "puzzles"; 

    cout << "===========================================" << endl;
    cout << "   LAUNCHING MATE-IN-N PUZZLE SOLVER BOT   " << endl;
    cout << "===========================================" << endl;

    if (filesystem::exists(puzzle_folder_path) && filesystem::is_directory(puzzle_folder_path)) {
        // Iterate through the target directory layout items dynamically
        for (const auto& entry : filesystem::directory_iterator(puzzle_folder_path)) {
            if (entry.path().extension() == ".json") {
                execute_puzzle_file(entry.path().string());
            }
        }
    } else {
        cerr << "ERROR: Folder path '" << puzzle_folder_path << "' not found." << endl;
    }

    return 0;
}
