#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>

using namespace std;

const int MAX_PALOS = 20;
const int MAX_JUGADORES = 20;
const int MAX_CARTAS = MAX_PALOS * 13;

class Carta {
private:
    int numero;
    string palo;

public:
    Carta() {
        numero = 1;
        palo = "";
    }

    Carta(int n, string p) {
        numero = n;
        palo = p;
    }

    int valor() {
        if (numero == 1) return 11;
        if (numero >= 11 && numero <= 13) return 10;
        return numero;
    }

    bool esAs() {
        return numero == 1;
    }

    void mostrar() {
        if (numero == 1) cout << "As";
        else if (numero == 11) cout << "J";
        else if (numero == 12) cout << "Q";
        else if (numero == 13) cout << "K";
        else cout << numero;

        cout << " de " << palo;
    }
};

class Configuracion {
public:
    string palos[MAX_PALOS];
    int cantidadPalos;
    int jugadores;
    int meta;
    int cartasPorPalo;

    Configuracion() {
        cantidadPalos = 0;
        jugadores = 0;
        meta = 0;
        cartasPorPalo = 13;
    }

    bool contiene(string texto, string palabra) {
        return texto.find(palabra) != string::npos;
    }

    bool cargar(string nombreArchivo) {
        ifstream archivo(nombreArchivo);

        if (!archivo.is_open()) {
            cout << "Error: no se encontro " << nombreArchivo << ".\n";
            return false;
        }

        string linea;
        while (getline(archivo, linea)) {
            int pos = (int)linea.find(':');

            if (pos != (int)string::npos) {
                string clave = linea.substr(0, pos);
                string valor = linea.substr(pos + 1);

                if (contiene(clave, "palos")) {
                    for (int i = 0; i < (int)valor.size(); i++) {
                        if (valor[i] == '{' || valor[i] == '}' ||
                            valor[i] == ',') {
                            valor[i] = ' ';
                        }
                    }

                    cantidadPalos = 0;
                    stringstream ss(valor);
                    string palo;

                    while (ss >> palo && cantidadPalos < MAX_PALOS) {
                        palos[cantidadPalos] = palo;
                        cantidadPalos++;
                    }
                }
                else if (contiene(clave, "jugadores")) {
                    stringstream ss(valor);
                    int dato;
                    if (ss >> dato && dato >= 1 && dato <= MAX_JUGADORES) {
                        jugadores = dato;
                    }
                    else {
                        cout << "Numero de jugadores invalido. Se conserva el valor anterior.\n";
                    }
                }
                else if (contiene(clave, "ganar")) {
                    stringstream ss(valor);
                    int dato;
                    if (ss >> dato && dato > 0) {
                        meta = dato;
                    }
                    else {
                        cout << "Valor para ganar invalido. Se conserva el valor anterior.\n";
                    }
                }
                else if (contiene(clave, "cartas")) {
                    stringstream ss(valor);
                    int dato;
                    if (ss >> dato && dato >= 1 && dato <= 13) {
                        cartasPorPalo = dato;
                    }
                }
            }
        }

        archivo.close();

        if (cantidadPalos == 0 || jugadores < 1 || meta < 1) {
            cout << "Error: el archivo no contiene una configuracion valida.\n";
            return false;
        }

        return true;
    }
};

class Mazo {
private:
    Carta cartas[MAX_CARTAS];
    int totalCartas;
    int siguiente;

public:
    Mazo(string palos[], int cantidadPalos, int n) {
        totalCartas = 0;

        for (int p = 0; p < cantidadPalos; p++) {
            for (int numero = 1; numero <= n; numero++) {
                if (totalCartas < MAX_CARTAS) {
                    cartas[totalCartas] = Carta(numero, palos[p]);
                    totalCartas++;
                }
            }
        }

        siguiente = 0;
        barajar();
    }

    void barajar() {
        for (int i = totalCartas - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            Carta temp = cartas[i];
            cartas[i] = cartas[j];
            cartas[j] = temp;
        }
        siguiente = 0;
    }

    void reiniciar() {
        barajar();
    }

    Carta repartir() {
        if (siguiente >= totalCartas) {
            reiniciar();
        }

        Carta c = cartas[siguiente];
        siguiente++;
        return c;
    }

    int quedan() {
        return totalCartas - siguiente;
    }

    int total() {
        return totalCartas;
    }
};

class Mano {
private:
    Carta cartas[MAX_CARTAS];
    int cantidad;

public:
    Mano() {
        cantidad = 0;
    }

    void agregar(Carta c) {
        if (cantidad < MAX_CARTAS) {
            cartas[cantidad] = c;
            cantidad++;
        }
    }

    int puntos(int meta) {
        int total = 0;
        int ases = 0;

        for (int i = 0; i < cantidad; i++) {
            total += cartas[i].valor();
            if (cartas[i].esAs()) ases++;
        }

        while (total > meta && ases > 0) {
            total -= 10;
            ases--;
        }

        return total;
    }

    bool esBlackjack(int meta) {
        return cantidad == 2 && puntos(meta) == meta;
    }

    void mostrar(bool ocultarSegunda, int meta) {
        for (int i = 0; i < cantidad; i++) {
            cout << "  - ";
            if (ocultarSegunda && i == 1) cout << "[carta oculta]";
            else cartas[i].mostrar();
            cout << endl;
        }

        if (!ocultarSegunda) {
            cout << "  Total: " << puntos(meta) << endl;
        }
    }
};

void mostrarMesa(Mano manos[], int cantidadJugadores, Mano &crupier,
                 int apuestas[], bool ocultar, int meta) {
    cout << "\nCrupier:\n";
    crupier.mostrar(ocultar, meta);

    for (int i = 0; i < cantidadJugadores; i++) {
        if (apuestas[i] > 0) {
            cout << "Jugador " << i + 1 << ":\n";
            manos[i].mostrar(false, meta);
        }
    }
}

int main() {
    srand((unsigned int)time(0));

    Configuracion config;
    if (!config.cargar("setup.txt")) {  //en esta parte se modifica el nombre del txt
        cout << "El juego no puede iniciar sin una configuracion valida.\n";
        return 1;
    }

    int n = config.jugadores;
    int meta = config.meta;
    int plantaCrupier = meta - 4;

    Mazo mazo(config.palos, config.cantidadPalos, config.cartasPorPalo);

    cout << "=== BLACKJACK (" << meta << ") ===\n";
    cout << "Palos: " << config.cantidadPalos
         << " | Cartas en el mazo: " << mazo.total()
         << " | Jugadores: " << n << "\n";

    int fichas[MAX_JUGADORES];
    for (int i = 0; i < n; i++) fichas[i] = 100;

    char seguir = 's';
    bool hayFichas = true;

    while (seguir == 's' && hayFichas) {
        if (mazo.quedan() < (n + 1) * 5) {
            mazo.reiniciar();
            cout << "\n(Mazo nuevo barajado)\n";
        }

        int apuestas[MAX_JUGADORES] = {0};

        for (int i = 0; i < n; i++) {
            if (fichas[i] > 0) {
                cout << "\nJugador " << i + 1 << " | Fichas: "
                     << fichas[i] << " | Apuesta: ";
                cin >> apuestas[i];

                while (cin.fail() || apuestas[i] < 1 || apuestas[i] > fichas[i]) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "Apuesta invalida. Intenta de nuevo: ";
                    cin >> apuestas[i];
                }
            }
        }

        Mano manos[MAX_JUGADORES];
        Mano crupier;

        for (int vuelta = 0; vuelta < 2; vuelta++) {
            for (int i = 0; i < n; i++) {
                if (apuestas[i] > 0) manos[i].agregar(mazo.repartir());
            }
            crupier.agregar(mazo.repartir());
        }

        mostrarMesa(manos, n, crupier, apuestas, true, meta);

        for (int i = 0; i < n; i++) {
            if (apuestas[i] > 0 &&
                !manos[i].esBlackjack(meta) &&
                !crupier.esBlackjack(meta)) {

                char op = 'p';

                while (op == 'p' && manos[i].puntos(meta) < meta) {
                    cout << "\nJugador " << i + 1
                         << ": (p)edir carta o (s)e plantar? ";
                    cin >> op;
                    op = tolower(op);

                    if (op == 'p') {
                        manos[i].agregar(mazo.repartir());
                        mostrarMesa(manos, n, crupier, apuestas, true, meta);
                    }
                    else if (op != 's') {
                        op = 'p';
                    }
                }
            }
        }

        cout << "\n--- Turno del crupier ---\n";
        while (crupier.puntos(meta) < plantaCrupier) {
            crupier.agregar(mazo.repartir());
        }

        mostrarMesa(manos, n, crupier, apuestas, false, meta);

        int pc = crupier.puntos(meta);
        cout << "\n--- Resultados ---\n";

        for (int i = 0; i < n; i++) {
            if (apuestas[i] > 0) {
                int pj = manos[i].puntos(meta);
                cout << "Jugador " << i + 1 << ": ";

                if (pj > meta) {
                    cout << "Te pasaste. Pierdes " << apuestas[i] << " fichas.\n";
                    fichas[i] -= apuestas[i];
                }
                else if (manos[i].esBlackjack(meta) &&
                         !crupier.esBlackjack(meta)) {
                    int ganancia = apuestas[i] * 3 / 2;
                    cout << "Blackjack! Ganas " << ganancia << " fichas.\n";
                    fichas[i] += ganancia;
                }
                else if (crupier.esBlackjack(meta) &&
                         !manos[i].esBlackjack(meta)) {
                    cout << "El crupier tiene Blackjack. Pierdes "
                         << apuestas[i] << " fichas.\n";
                    fichas[i] -= apuestas[i];
                }
                else if (pc > meta || pj > pc) {
                    cout << "Ganas " << apuestas[i] << " fichas!\n";
                    fichas[i] += apuestas[i];
                }
                else if (pj < pc) {
                    cout << "Gana el crupier. Pierdes "
                         << apuestas[i] << " fichas.\n";
                    fichas[i] -= apuestas[i];
                }
                else {
                    cout << "Empate. Recuperas tu apuesta.\n";
                }
            }
        }

        hayFichas = false;
        for (int i = 0; i < n; i++) {
            if (fichas[i] > 0) hayFichas = true;
        }

        if (hayFichas) {
            cout << "\nOtra ronda? (s/n): ";
            cin >> seguir;
            seguir = tolower(seguir);
        }
    }

    cout << "\n=== FIN DEL JUEGO ===\n";
    for (int i = 0; i < n; i++) {
        cout << "Jugador " << i + 1 << ": " << fichas[i] << " fichas\n";
    }

    return 0;
}