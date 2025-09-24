#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <locale>
#include <sstream>

using namespace std;

static const string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

string base64_encode(const string &in) {
    string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

string base64_decode(const string &in) {
    vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
    string out;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

string cipher(string &palabra, string &clave) {
    int n = palabra.size();
    int m = clave.size();
    if (m == 0 || n == 0) return palabra;

    if (m > 16) {
        clave = clave.substr(0, 16);
        m = 16;
    }

    int columnas = n / 2 + n % 2;
    int filas = 2;

    vector<vector<int>> palabra_matriz(filas, vector<int>(columnas, 0));
    vector<vector<int>> clave_matriz(filas, vector<int>(columnas, 0));
    vector<vector<int>> cifrado_matriz(filas, vector<int>(columnas, 0));
    vector<vector<int>> transpuesta(columnas, vector<int>(filas, 0));

    for (int i = 0; i < n; i++)
        palabra_matriz[i % filas][i / filas] = (int)palabra[i];

    for (int i = 0; i < n; i++)
        clave_matriz[i % filas][i / filas] = (int)clave[i % m];

    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] = palabra_matriz[i][j] + clave_matriz[i][j];

    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] ^= clave_matriz[i][j];

    for (int j = 0; j < columnas / 2; j++)
        for (int i = 0; i < filas; i++)
            swap(cifrado_matriz[i][j], cifrado_matriz[i][columnas - 1 - j]);

    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            transpuesta[j][i] = cifrado_matriz[i][j];

    string resultado;
    for (int j = 0; j < columnas; j++)
        for (int i = 0; i < filas; i++)
            resultado.push_back((unsigned char)transpuesta[j][i]);

    return resultado;
}

string decipher(string &cifrado, string &clave) {
    int n = cifrado.size();
    int m = clave.size();
    if (m == 0 || n == 0) return cifrado;

    if (m > 16) {
        clave = clave.substr(0, 16);
        m = 16;
    }
    
    int columnas = n / 2 + n % 2;
    int filas = 2;

    vector<vector<int>> transpuesta(columnas, vector<int>(filas, 0));
    vector<vector<int>> cifrado_matriz(filas, vector<int>(columnas, 0));
    vector<vector<int>> clave_matriz(filas, vector<int>(columnas, 0));
    
    for (int i = 0; i < columnas; i++)
        for (int j = 0; j < filas; j++)
            if ((i * filas + j) < n)
                transpuesta[i][j] = (unsigned char)cifrado[i*filas + j];

    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] = transpuesta[j][i];

    for (int j = 0; j < columnas / 2; j++)
        for (int i = 0; i < filas; i++)
            swap(cifrado_matriz[i][j], cifrado_matriz[i][columnas - 1 - j]);

    for (int i = 0; i < n; i++)
        clave_matriz[i % filas][i / filas] = (int)clave[i % m];

    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] ^= clave_matriz[i][j];

    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] = (cifrado_matriz[i][j] - clave_matriz[i][j] + 256) % 256;

    string resultado;
    for (int j = 0; j < columnas; j++) {
        for (int i = 0; i < filas; i++) {
            if ((j * filas + i) < n) {
                resultado.push_back((unsigned char)cifrado_matriz[i][j]);
            }
        }
    }

    return resultado;
}


int main() {
    setlocale(LC_ALL, "Spanish");

    sf::RenderWindow window(sf::VideoMode({900u, 600u}), "Simple Cipher UI");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded = false;
    const char* candidates[] = {
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\tahoma.ttf",
        "C:\\Windows\\Fonts\\calibri.ttf"
    };
    for (const char* p : candidates) {
        if (font.openFromFile(p)) {
            fontLoaded = true;
            break;
        }
    }

    sf::Text *title = nullptr, *prompt = nullptr, *inputText = nullptr, *statusText = nullptr,
             *cipherNumberText = nullptr, *cipherStringText = nullptr, *decipherStatusText = nullptr, *decipheredText = nullptr,
             *keyPrompt = nullptr, *keyText = nullptr;

    string inputString;
    string keyInput;
    
    if (fontLoaded) {
        title = new sf::Text(font, "Cipher/Decipher Tool", 20);
        title->setPosition({10.f, 10.f});
        title->setFillColor(sf::Color::White);

        prompt = new sf::Text(font, "Message:", 20);
        prompt->setPosition({10.f, 50.f});
        prompt->setFillColor(sf::Color::White);

        inputText = new sf::Text(font, "", 22);
        inputText->setPosition({10.f, 80.f});
        inputText->setFillColor(sf::Color::Yellow);

        keyPrompt = new sf::Text(font, "Key:", 18);
        keyPrompt->setPosition({10.f, 120.f});
        keyPrompt->setFillColor(sf::Color::White);

        keyText = new sf::Text(font, "", 20);
        keyText->setPosition({10.f, 145.f});
        keyText->setFillColor(sf::Color::White);

        statusText = new sf::Text(font, "", 20);
        statusText->setPosition({10.f, 190.f});
        statusText->setFillColor(sf::Color::Cyan);

        cipherNumberText = new sf::Text(font, "", 16);
        cipherNumberText->setPosition({10.f, 220.f});
        cipherNumberText->setFillColor(sf::Color::White);

        cipherStringText = new sf::Text(font, "", 18);
        cipherStringText->setPosition({10.f, 340.f});
        cipherStringText->setFillColor(sf::Color::White);

        decipherStatusText = new sf::Text(font, "", 20);
        decipherStatusText->setPosition({10.f, 380.f});
        decipherStatusText->setFillColor(sf::Color::Cyan);

        decipheredText = new sf::Text(font, "", 18);
        decipheredText->setPosition({10.f, 410.f});
        decipheredText->setFillColor(sf::Color::Green);
    } else {
        cerr << "No system font found. Text rendering is disabled.\n";
    }

    enum class UIState { Input, ShowingResult };
    UIState state = UIState::Input;

    enum class ActiveField { Message, Key };
    ActiveField activeField = ActiveField::Message;

    int rounds = 11;

    while (window.isOpen()) {
        while (auto evOpt = window.pollEvent()) {
            auto &event = *evOpt;

            if (event.is<sf::Event::Closed>())
                window.close();

            if (state == UIState::Input) {
                if (auto te = event.getIf<sf::Event::TextEntered>()) {
                    uint32_t unicode = te->unicode;
                    if (unicode >= 32 && unicode < 127) {
                        if (activeField == ActiveField::Message) {
                            inputString += static_cast<char>(unicode);
                            if (inputText) inputText->setString(inputString);
                        } else {
                            keyInput += static_cast<char>(unicode);
                            if (keyText) keyText->setString(keyInput);
                        }
                    }
                }
                
                if (auto ke = event.getIf<sf::Event::KeyPressed>()) {
                    if (ke->code == sf::Keyboard::Key::Backspace) {
                        if (activeField == ActiveField::Message && !inputString.empty()) {
                            inputString.pop_back();
                            if (inputText) inputText->setString(inputString);
                        } else if (activeField == ActiveField::Key && !keyInput.empty()) {
                            keyInput.pop_back();
                            if (keyText) keyText->setString(keyInput);
                        }
                    }
                    else if (ke->code == sf::Keyboard::Key::Tab) {
                        if (activeField == ActiveField::Message) {
                            activeField = ActiveField::Key;
                            if (inputText) inputText->setFillColor(sf::Color::White);
                            if (keyText) keyText->setFillColor(sf::Color::Yellow);
                        } else {
                            activeField = ActiveField::Message;
                            if (inputText) inputText->setFillColor(sf::Color::Yellow);
                            if (keyText) keyText->setFillColor(sf::Color::White);
                        }
                    }
                    else if (ke->code == sf::Keyboard::Key::Enter) {
                        if (inputString.empty() || keyInput.empty()) {
                             if (statusText) statusText->setString("Error: Message and Key cannot be empty!");
                             continue;
                        }
                        
                        if (statusText) statusText->setString("Ciphering...");
                        
                        string temp = inputString;
                        string key_copy = keyInput;
                        for (int r = 0; r < rounds; r++)
                            temp = cipher(temp, key_copy);
                        string ciphered = temp;
                        
                        string encoded = base64_encode(ciphered);
                        
                        std::ostringstream oss;
                        for (unsigned char c : encoded) oss << int(c) << " ";
                        string cipherNumbers = oss.str();
                        
                        if (decipherStatusText) decipherStatusText->setString("Verifying by deciphering...");
                        string decoded = base64_decode(encoded);
                        temp = decoded;
                        key_copy = keyInput;
                        for (int r = 0; r < rounds; r++)
                            temp = decipher(temp, key_copy);
                        string deciphered_result = temp;

                        if (statusText) statusText->setString("Ciphering Complete!");
                        if (cipherNumberText) cipherNumberText->setString("Ciphered (numeric): " + cipherNumbers);
                        if (cipherStringText) cipherStringText->setString("Ciphered (string): " + encoded);
                        if (decipherStatusText) decipherStatusText->setString("Deciphering Verification:");
                        if (decipheredText) decipheredText->setString("Deciphered: " + deciphered_result);
                        
                        state = UIState::ShowingResult;
                    }
                    else if (ke->code == sf::Keyboard::Key::F2) {
                         if (inputString.empty() || keyInput.empty()) {
                             if (statusText) statusText->setString("Error: Message and Key cannot be empty!");
                             continue;
                        }
                        
                        string s = inputString;
                        auto is_base64_like = [&](const string &t) {
                            if (t.empty()) return false;
                            for (unsigned char ch : t) {
                                if (ch == '=') continue;
                                if (base64_chars.find(ch) == string::npos) return false;
                            }
                            return true;
                        };
                        
                        string bytesInput = is_base64_like(s) ? base64_decode(s) : s;
                        
                        string temp = bytesInput;
                        string key_copy = keyInput;
                        for (int r = 0; r < rounds; r++)
                            temp = decipher(temp, key_copy);
                        string deciphered_result = temp;
                        
                        if (decipherStatusText) decipherStatusText->setString("Deciphering Complete!");
                        if (cipherNumberText) cipherNumberText->setString("");
                        if (cipherStringText) cipherStringText->setString("Input (string): " + inputString);
                        if (decipheredText) decipheredText->setString("Deciphered: " + deciphered_result);
                        
                        state = UIState::ShowingResult;
                    }
                }
            } else {
                if (auto ke = event.getIf<sf::Event::KeyPressed>()) {
                    if (ke->code == sf::Keyboard::Key::Escape) {
                        inputString.clear();
                        if (inputText) inputText->setString("");
                        if (statusText) statusText->setString("");
                        if (cipherNumberText) cipherNumberText->setString("");
                        if (cipherStringText) cipherStringText->setString("");
                        if (decipherStatusText) decipherStatusText->setString("");
                        if (decipheredText) decipheredText->setString("");
                        state = UIState::Input;
                    }
                }
            }
        }

        window.clear(sf::Color(40, 40, 45));
        
        if (title) window.draw(*title);
        if (prompt) window.draw(*prompt);
        if (inputText) window.draw(*inputText);
        if (keyPrompt) window.draw(*keyPrompt);
        if (keyText) window.draw(*keyText);

        if (state == UIState::ShowingResult) {
            if (statusText) window.draw(*statusText);
            if (cipherNumberText) window.draw(*cipherNumberText);
            if (cipherStringText) window.draw(*cipherStringText);
            if (decipherStatusText) window.draw(*decipherStatusText);
            if (decipheredText) window.draw(*decipheredText);
        } else {
             if (statusText) window.draw(*statusText);
        }

        if (fontLoaded) {
            string hint_text = (state == UIState::Input)
                ? "Type message/key (Tab to switch). Enter=Cipher, F2=Decipher."
                : "Press Esc to start over.";
            sf::Text hint(font, hint_text, 14);
            hint.setPosition({10.f, 570.f});
            hint.setFillColor({200, 200, 200});
            window.draw(hint);
        }

        window.display();
    }
    
    delete title;
    delete prompt;
    delete inputText;
    delete statusText;
    delete cipherNumberText;
    delete cipherStringText;
    delete decipherStatusText;
    delete decipheredText;
    delete keyPrompt;
    delete keyText;

    return 0;
}