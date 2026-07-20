#ifndef NNUE_EVAL_HPP
#define NNUE_EVAL_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include "chess.hpp"

class NNUEEvaluator {
private:
    static constexpr uint32_t MAGIC_HEADER = 0x45554E4E; // "NNUE"
    static constexpr int INPUT_SIZE = 768;
    static constexpr int HIDDEN_SIZE = 256;

    std::vector<int16_t> fc1_weight;
    std::vector<int16_t> fc1_bias;  
    std::vector<int16_t> fc2_weight;
    std::vector<int16_t> fc2_bias;  

    bool loaded_successfully = false;

    int piece_to_offset(chess::PieceType pt) {
        switch (pt.internal()) {
            case chess::PieceType::PAWN:   return 0;
            case chess::PieceType::KNIGHT: return 1;
            case chess::PieceType::BISHOP: return 2;
            case chess::PieceType::ROOK:   return 3;
            case chess::PieceType::QUEEN:  return 4;
            case chess::PieceType::KING:   return 5;
            default: return 0;
        }
    }

public:
    NNUEEvaluator() = default;

    bool load_model(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "NNUE Error: Cannot open file " << filename << std::endl;
            return false;
        }

        uint32_t magic;
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (magic != MAGIC_HEADER) {
            std::cerr << "NNUE Error: Invalid magic header!" << std::endl;
            return false;
        }

        uint32_t in_dim, hid_dim, out_dim;
        file.read(reinterpret_cast<char*>(&in_dim), sizeof(in_dim));
        file.read(reinterpret_cast<char*>(&hid_dim), sizeof(hid_dim));
        file.read(reinterpret_cast<char*>(&out_dim), sizeof(out_dim));

        if (in_dim != INPUT_SIZE || hid_dim != HIDDEN_SIZE || out_dim != 1) {
            std::cerr << "NNUE Error: Architecture mismatch!" << std::endl;
            return false;
        }

        fc1_weight.resize(HIDDEN_SIZE * INPUT_SIZE);
        fc1_bias.resize(HIDDEN_SIZE);
        fc2_weight.resize(HIDDEN_SIZE);
        fc2_bias.resize(1);

        file.read(reinterpret_cast<char*>(fc1_weight.data()), fc1_weight.size() * sizeof(int16_t));
        file.read(reinterpret_cast<char*>(fc1_bias.data()), fc1_bias.size() * sizeof(int16_t));
        file.read(reinterpret_cast<char*>(fc2_weight.data()), fc2_weight.size() * sizeof(int16_t));
        file.read(reinterpret_cast<char*>(fc2_bias.data()), fc2_bias.size() * sizeof(int16_t));

        loaded_successfully = true;
        std::cout << "NNUE model successfully loaded from " << filename << std::endl;
        return true;
    }

    int evaluate(const chess::Board& board) {
        if (!loaded_successfully) {
            return 0;
        }

        std::vector<int32_t> hidden_layer(HIDDEN_SIZE);
        for (int i = 0; i < HIDDEN_SIZE; ++i) {
            hidden_layer[i] = fc1_bias[i];
        }

        for (int sq = 0; sq < 64; ++sq) {
            chess::Square square = chess::Square(sq);
            chess::Piece piece = board.at(square);

            if (piece != chess::Piece::NONE) {
                int color_offset = (piece.color() == chess::Color::WHITE) ? 0 : 6;
                int feature_idx = (color_offset + piece_to_offset(piece.type())) * 64 + sq;

                for (int h = 0; h < HIDDEN_SIZE; ++h) {
                    hidden_layer[h] += fc1_weight[h * INPUT_SIZE + feature_idx];
                }
            }
        }

        int32_t output = fc2_bias[0];
        for (int h = 0; h < HIDDEN_SIZE; ++h) {
            int32_t activated = std::clamp(hidden_layer[h], 0, 127);
            output += activated * fc2_weight[h];
        }

        int score = output / 127;
        return (board.sideToMove() == chess::Color::WHITE) ? score : -score;
    }
};

#endif // NNUE_EVAL_HPP
