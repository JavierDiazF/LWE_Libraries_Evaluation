#include "openfhe.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include "tools.h"

// using namespace lbcrypto;

#define N_POLINOMIOS 10 // En cada test creamos 10 polinomios
#define REPETICIONES 10 // Vamos a hacer 10 repeticiones por test

#define CSV_FILE "statistics.csv"
#define LIBRERIA "openfhe" // Utilizada para saber a qué librería pertenece

template<size_t degree, size_t modulus>
void run_poly(int n_test) {
    // ------------------ Parámetros ------------------
    usint m = 2 * degree; // orden ciclotómico, de 256 es aproximadamente 512 = 256*2
    usint numPrimes = 1; // número de primos en el CRT. Por ahora 1

    // Construcción de parámetros de DCRT
    auto params = std::make_shared<lbcrypto::ILDCRTParams<lbcrypto::BigInteger>>(m, numPrimes, modulus);
    // Generador de números aleatorios con distribución uniforme para rellenar los polinomios
    // ref: https://github.com/openfheorg/openfhe-development/blob/main/benchmark/src/poly-benchmark.h
    lbcrypto::DiscreteUniformGeneratorImpl<lbcrypto::NativeVector> dug; 
    std::chrono::high_resolution_clock::time_point start, finish;
    // ------------------ Crear polinomios ------------------
    std::vector<lbcrypto::DCRTPoly> dcrt_a, dcrt_b, dcrt_c;
    dcrt_a.reserve(N_POLINOMIOS);
    dcrt_b.reserve(N_POLINOMIOS);
    dcrt_c.reserve(N_POLINOMIOS);

    // Inicializo polinomio  c a 0
     for (int i = 0; i < N_POLINOMIOS; i++) {
        // Polinomio con coeficientes cero al poner true
        lbcrypto::DCRTPoly c(params, Format::COEFFICIENT, true);
        dcrt_c.push_back(c);
    }
    // Test 1: Tiempo de creacion de polinomios
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N_POLINOMIOS; i++) {
        // Polinomio con coeficientes aleatorios
        lbcrypto::DCRTPoly a(dug, params, Format::COEFFICIENT);
        lbcrypto::DCRTPoly b(dug, params, Format::COEFFICIENT);

        dcrt_a.push_back(a);
        dcrt_b.push_back(b);
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_polinomio = get_time_us(start, finish, 2*N_POLINOMIOS);

    /*
    La transforrmada NTT va por el formato en el que esté el polinomio
    Si es COEFCIENT, entonces está sin transformar
    Si es EVALUATION, entonces está transformado
    Se puede añadir un: if (a[i].GetFormat() != Format::COEFFICIENT) a[i].SwitchFormat();
    para asegurar que se cambia de formato en el sentido correcto:
        NTT Directa: if (a[i].GetFormat() != Format::COEFFICIENT) a[i].SwitchFormat();
        NTT Inversa: if (a[i].GetFormat() != Format::EVALUATION) a[i].SwitchFormat();
    Dado que es un escenario controlado, sé ccuando están transformados y cuando no, no hace falta
    */

    // Test 2: Tansformada directa NTT
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N_POLINOMIOS; i++) {
        dcrt_a[i].SwitchFormat(); // coef → eval
        dcrt_b[i].SwitchFormat(); // coef → eval
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_ntt = get_time_us(start, finish, 2*N_POLINOMIOS); // Multiplico por 2 porque estamos haciendo dos transformadas de a y b

    // Test 3: Suma NTT
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N_POLINOMIOS; i++) {
        dcrt_c[i] = dcrt_a[i] + dcrt_b[i]; // suma
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_suma = get_time_us(start, finish, N_POLINOMIOS);

    // Test 4: Multiplicacion NTT
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N_POLINOMIOS; i++) {
        dcrt_c[i] = dcrt_a[i] * dcrt_b[i]; // multiplicacion
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_multiplicacion = get_time_us(start, finish, N_POLINOMIOS);

    // Test 5: Transformada inversa NTT
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N_POLINOMIOS; i++) {
        dcrt_a[i].SwitchFormat(); // eval → coef
        dcrt_b[i].SwitchFormat(); // eval → coef
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_intt = get_time_us(start, finish, 2*N_POLINOMIOS);

    // Fin: Escribe resultados en csv
    std::ofstream datos_csv;
    datos_csv.open(CSV_FILE, std::ios::out | std::ios::app);
    datos_csv << LIBRERIA << "," << n_test << "," << modulus << "," << tiempo_polinomio << "," << tiempo_ntt << "," << tiempo_suma << "," << tiempo_multiplicacion << "," << tiempo_intt << "\n";
    datos_csv.close();

}

int main(){
    // Ejecutamos los tests para os tres tamaños
    for (int i=0; i < REPETICIONES; i++){
        run_poly<256, 14>(i);
        run_poly<256, 30>(i);
        run_poly<256, 60>(i);
    }
}