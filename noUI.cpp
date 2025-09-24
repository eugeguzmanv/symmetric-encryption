#include <iostream>
#include <string>
#include <vector>
#include <locale>
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

// Función para imprimir matrices
void printMatrix(vector<vector<int>> &matrix) {
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix[i].size(); ++j) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

// Función de cifrado
string cipher(string &palabra, string &clave) {
    int n = palabra.size();
    int m = clave.size();

    // Limitar la clave a 16 caracteres
    if (m > 16) {
        clave = clave.substr(0, 16);
        m = 16;
    }

    int columnas = n / 2 + n % 2;   // En caso de que la longitud del string sea impar
    int filas = 2;

    // Matrices
    vector<vector<int>> palabra_matriz(filas, vector<int>(columnas, 0));
    vector<vector<int>> clave_matriz(filas, vector<int>(columnas, 0));
    vector<vector<int>> cifrado_matriz(filas, vector<int>(columnas, 0));
    vector<vector<int>> transpuesta(columnas, vector<int>(filas, 0));

    // Llenar palabra_matriz
    for (int i = 0; i < n; i++)
        palabra_matriz[i % filas][i / filas] = (int)palabra[i];

    // Llenar clave_matriz (se repite la clave hasta que esta matriz y la anterior tengan la misma cantidad de elementos distintos de cero)
    for (int i = 0; i < n; i++)
        clave_matriz[i % filas][i / filas] = (int)clave[i % m];

    // Sumar cada elemento de palabra_matriz y clave_matriz (confusión)
    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] = palabra_matriz[i][j] + clave_matriz[i][j];

    // XOR de cada elemento de cifrado_matriz con cada elemento de clave_matriz (confusión)
    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] ^= clave_matriz[i][j];

    // Intercambiar columnas espejo (primera y última, segunda y antepenúltima, etc.) (difusión)
    for (int j = 0; j < columnas / 2; j++)
        for (int i = 0; i < filas; i++)
            swap(cifrado_matriz[i][j], cifrado_matriz[i][columnas - 1 - j]);

    // Transpuesta de la matriz (difusión)
    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            transpuesta[j][i] = cifrado_matriz[i][j];

    // Agregar los valores numéricos resultantes a un string
    string resultado;
    for (int j = 0; j < columnas; j++)
        for (int i = 0; i < filas; i++)
            resultado.push_back((unsigned char)transpuesta[j][i]);

    return resultado;
}

// Función de descifrado
string decipher(string &cifrado, string &clave) {
    int n = cifrado.size();
    int m = clave.size();

    if (m > 16) {
        clave = clave.substr(0, 16);
        m = 16;
    }
    
    int columnas = n / 2 + n % 2;
    int filas = 2;

    // Matrices
    vector<vector<int>> transpuesta(columnas, vector<int>(filas, 0));
    vector<vector<int>> cifrado_matriz(filas, vector<int>(columnas, 0));
    vector<vector<int>> clave_matriz(filas, vector<int>(columnas, 0));

    // Reconstruir matriz transpuesta iterando el string cifrado
    for (int i = 0; i < columnas; i++)
        for (int j = 0; j < filas; j++)
            transpuesta[i][j] = (unsigned char)cifrado[i*filas + j];

    // Transponer la matriz (las dos transposiciones se anulan)
    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] = transpuesta[j][i];

    // Intercambiar elementos espejo para obtener la matriz original
    for (int j = 0; j < columnas / 2; j++)
        for (int i = 0; i < filas; i++)
            swap(cifrado_matriz[i][j], cifrado_matriz[i][columnas - 1 - j]);

    // Reconstruir clave_matriz
    for (int i = 0; i < n; i++)     // Usamos n para que la longitud de clave_matriz sea la misma que la del string a descifrar
        clave_matriz[i % filas][i / filas] = (int)clave[i % m];     // Llenar la matriz columna por columna con los caracteres de la clave

    // Anular XOR aplicándolo nuevamente
    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] ^= clave_matriz[i][j];

    // Resta clave (confusión inversa)
    for (int i = 0; i < filas; i++)
        for (int j = 0; j < columnas; j++)
            cifrado_matriz[i][j] = (cifrado_matriz[i][j] - clave_matriz[i][j] + 256) % 256;     // Deshacer la suma de los elementos de clave_matriz y palabra_matriz
                                                                                                // El módulo se asegura de que el valor siempre sea positivo (y de que no interpretemos mal un carácter)

    // Guardar los caracteres obtenidos en un string
    string resultado;
    for (int j = 0; j < columnas; j++) {
        for (int i = 0; i < filas; i++) {
            resultado.push_back((unsigned char)cifrado_matriz[i][j]);
        }
    }

    return resultado;
}



int main() {
    setlocale(LC_ALL,"Spanish");
            
    // "jbrSbpKddkmZcPK7CmYbwDNWFuN1f0C4bYmbFA8L2CgHnCQJ51dmdi3NP6yGVMvVA6rShJrFT0p6tK7DzUCRqE6agvZjwhMbLv6k"; // String de cien caracteres
    string s, key;
    cout << "Enter a string to encrypt: ";
    cin >> s;
    cout << "Enter the key: ";
    cin >> key;
    cout << endl << endl << endl;
    int rounds = 11;
    string temp = s;

    for (int r = 0; r < rounds; r++)
        temp = cipher(temp, key);
    string ciphered = temp;
    string encoded = base64_encode(ciphered);

    cout << "Ciphered (number): ";

    for (char c : encoded)
        cout << (int)(unsigned char)c << " ";   // Convertir los números anteriores en caracteres

    cout << endl << "Ciphered (string): " << encoded << endl << endl;

    string decoded = base64_decode(encoded);
    temp = decoded;
    for (int r = 0; r < rounds; r++)
        temp = decipher(temp, key);
    
    string desciphered = temp;
    cout << "Deciphered: " << desciphered << endl;

    return 0;
}