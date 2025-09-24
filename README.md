# C++ Cipher Tool

A C++ application for encrypting and decrypting text with a user-provided key.

## Requirements

- C++17 Compiler (e.g., g++)
- **SFML 3.0 or higher** (for the GUI version only)

## How to Run

There are two versions of the program.

### 1. GUI Version (main.cpp)

This version provides a graphical user interface.

**To Compile, run this command:**
g++ main.cpp -o cipher-app -lsfml-graphics -lsfml-window -lsfml-system

**To Run the application:**
./cipher-app

**Controls:**
- **Tab:** Switch between Message and Key fields.
- **Enter:** Encrypt the message.
- **F2:** Decrypt the message.
- **Esc:** Reset the interface.

### 2. Terminal Version (noUI.cpp)

This is a command-line version for systems without SFML.

**To Compile, run this command:**
g++ noUI.cpp -o cipher-cli

**To Run the application:**
./cipher-cli
