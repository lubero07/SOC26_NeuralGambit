import torch
import struct
import numpy as np
from train_nnue import MiniNNUE

def export_to_binary():
    # 1. Load trained PyTorch weights
    model = MiniNNUE()
    model.load_state_dict(torch.load("nnue_model.pt"))
    model.eval()

    # 2. Extract weights & biases
    fc1_weight = model.fc1.weight.detach().numpy()  # Shape: (256, 768)
    fc1_bias = model.fc1.bias.detach().numpy()      # Shape: (256,)
    fc2_weight = model.fc2.weight.detach().numpy()  # Shape: (1, 256)
    fc2_bias = model.fc2.bias.detach().numpy()      # Shape: (1,)

    # 3. Quantize floats to 16-bit integers for C++ arithmetic
    SCALE = 127.0  # Scaling factor for int16 precision
    fc1_w_int = np.clip(fc1_weight * SCALE, -32768, 32767).astype(np.int16)
    fc1_b_int = np.clip(fc1_bias * SCALE, -32768, 32767).astype(np.int16)
    fc2_w_int = np.clip(fc2_weight * SCALE, -32768, 32767).astype(np.int16)
    fc2_b_int = np.clip(fc2_bias * SCALE, -32768, 32767).astype(np.int16)

    # 4. Write directly to binary file: `model.nnue`
    with open("model.nnue", "wb") as f:
        # Magic header ID for verification in C++
        f.write(struct.pack("<I", 0x45554E4E)) # "NNUE" in hex

        # Dimensions: inputs (768), hidden (256), outputs (1)
        f.write(struct.pack("<III", 768, 256, 1))

        # Raw binary weight buffers
        f.write(fc1_w_int.tobytes())
        f.write(fc1_b_int.tobytes())
        f.write(fc2_w_int.tobytes())
        f.write(fc2_b_int.tobytes())

    print("Successfully exported quantized binary model to 'model.nnue'!")

if __name__ == "__main__":
    export_to_binary()
