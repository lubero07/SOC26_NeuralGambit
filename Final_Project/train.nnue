import torch
import torch.nn as nn
import torch.optim as optim
import chess
import random

# --- 1. FEATURE EXTRACTION ---
# Represent board as 768 binary inputs (6 piece types x 2 colors x 64 squares)
def board_to_features(board: chess.Board):
    features = torch.zeros(768, dtype=torch.float32)
    
    piece_offset = {
        chess.PAWN: 0,
        chess.KNIGHT: 1,
        chess.BISHOP: 2,
        chess.ROOK: 3,
        chess.QUEEN: 4,
        chess.KING: 5
    }

    for sq in chess.SQUARES:
        piece = board.piece_at(sq)
        if piece:
            # White pieces: 0 to 5, Black pieces: 6 to 11
            color_offset = 0 if piece.color == chess.WHITE else 6
            idx = (color_offset + piece_offset[piece.piece_type]) * 64 + sq
            features[idx] = 1.0

    return features

# --- 2. SIMPLE NNUE ARCHITECTURE ---
# 768 inputs -> 256 hidden layer (Clipped ReLU) -> 1 evaluation output (centipawns)
class MiniNNUE(nn.Module):
    def __init__(self):
        super(MiniNNUE, self).__init__()
        self.fc1 = nn.Linear(768, 256)
        self.fc2 = nn.Linear(256, 1)

    def forward(self, x):
        # Clipped ReLU simulates hardware-friendly integer bounds used in NNUE
        x = torch.clamp(self.fc1(x), 0.0, 1.0)
        x = self.fc2(x)
        return x

# --- 3. DUMMY DATASET GENERATOR ---
def generate_sample_data(num_samples=1000):
    X, Y = [], []
    board = chess.Board()
    
    print(f"Generating {num_samples} sample positions for testing...")
    for _ in range(num_samples):
        if board.is_game_over():
            board.reset()
        
        # Make a random move to explore positions
        moves = list(board.legal_moves)
        if moves:
            board.push(random.choice(moves))
            
        features = board_to_features(board)
        
        # Target valuation in centipawns (basic material heuristic for ground truth)
        white_mat = sum(len(board.pieces(p, chess.WHITE)) * v for p, v in [(1,100), (2,300), (3,325), (4,500), (5,900)])
        black_mat = sum(len(board.pieces(p, chess.BLACK)) * v for p, v in [(1,100), (2,300), (3,325), (4,500), (5,900)])
        target_score = float(white_mat - black_mat) if board.turn == chess.WHITE else float(black_mat - white_mat)

        X.append(features)
        Y.append(torch.tensor([target_score], dtype=torch.float32))

    return torch.stack(X), torch.stack(Y)

# --- 4. TRAINING LOOP ---
if __name__ == "__main__":
    # Generate data
    X, Y = generate_sample_data(2000)

    model = MiniNNUE()
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    print("\nStarting training run...")
    model.train()
    for epoch in range(1, 11):
        optimizer.zero_grad()
        outputs = model(X)
        loss = criterion(outputs, Y)
        loss.backward()
        optimizer.step()
        
        print(f"Epoch [{epoch}/10] - Loss: {loss.item():.2f}")

    print("\nTraining complete! Exporting model weights...")
    
    # Save network weights for future binary conversion
    torch.save(model.state_dict(), "nnue_model.pt")
    print("Saved model checkpoint to 'nnue_model.pt'")
