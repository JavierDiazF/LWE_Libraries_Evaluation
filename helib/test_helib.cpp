#include <helib/helib.h>
#include <NTL/ZZX.h>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include "tools.h"

#define N_POLINOMIOS 10 // En cada test creamos 10 polinomios
#define REPETICIONES 10 // Vamos a hacer 10 repeticiones por test

#define CSV_FILE "statistics.csv"
#define LIBRERIA "helib" // Utilizada para saber a qué librería pertenece

template<size_t modulus> 
int run_poly(int n_test) {
    // Parámetros similares a NFLlib
    long degree = 256;        // grado polinomio
    long p = 65537;           // Móulo plaintext estándar
    long c = 1;               // columns de la mmatriz Key-Switching matrix 
    long r = 1;               // potencia de p
    long m = 512;             // orden ciclotómico, phi(512) >= 256 (grado)

    // Construir contexto BGV
    auto context = helib::ContextBuilder<helib::BGV>()
                       .m(m)
                       .p(p)
                       .r(r)
                       .c(c)
                       .bits(modulus) // tamaño de primos CRT
                       .build();

    // Para los tiempos
    std::chrono::high_resolution_clock::time_point start, finish;

    // Crear polinomios NTL
    std::vector<NTL::ZZX> polinomio_a, polinomio_b;
    polinomio_a.reserve(N_POLINOMIOS);
    polinomio_b.reserve(N_POLINOMIOS);
    // Creo distribución uniforme para generar los polinomios
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> dist(0, 1000);

    NTL::ZZX polinomio_a_t, polinomio_b_t; // Creo polinomios temporales que voy metiendo en el vector
    polinomio_a_t.SetLength(degree+1);
    polinomio_b_t.SetLength(degree+1);

    // Test 1: Tiempo de creación de polinomios
    start = std::chrono::high_resolution_clock::now();
    for (int k=0; k< N_POLINOMIOS; ++k){
        for (int i=0; i<=degree; i++){
            polinomio_a_t[i] = dist(gen);
            polinomio_b_t[i] = dist(gen);
        }
        polinomio_a.push_back(polinomio_a_t);
        polinomio_b.push_back(polinomio_b_t);
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_polinomio = get_time_us(start, finish, N_POLINOMIOS);
    
    // IndexSet para el polinomio
    helib::IndexSet allPrimes = context.getCtxtPrimes();
    // Convertir a DoubleCRT (aplica NTT/convolución)
    std::vector<helib::DoubleCRT> dcrt_a, dcrt_b;
    dcrt_a.reserve(N_POLINOMIOS);
    dcrt_b.reserve(N_POLINOMIOS);

    //Test 2: Transformada diercta NTT (En este caso es NTT y RNS)
    start = std::chrono::steady_clock::now();
    for (int i = 0; i< N_POLINOMIOS; i++){
        helib::DoubleCRT dcrt_a_t(polinomio_a[i], context, allPrimes);
        helib::DoubleCRT dcrt_b_t(polinomio_b[i], context, allPrimes);

        dcrt_a.push_back(dcrt_a_t);
        dcrt_b.push_back(dcrt_b_t);
    }
    finish = std::chrono::steady_clock::now();
    double tiempo_ntt = get_time_us(start, finish, 2*N_POLINOMIOS);

    // Test 3: Suma NTT
    std::vector<helib::DoubleCRT> dcrt_c = dcrt_a;
    start = std::chrono::steady_clock::now();
    for (int i=0; i< N_POLINOMIOS; i++){
        dcrt_c[i] += dcrt_b[i];
    }
    finish = std::chrono::steady_clock::now();
    double tiempo_suma = get_time_us(start, finish, N_POLINOMIOS);

    // Test 4: Multiplicación NTT
    dcrt_c = dcrt_a;
    start = std::chrono::steady_clock::now();
    for (int i=0; i< N_POLINOMIOS; i++){
        dcrt_c[i] *= dcrt_b[i];
    }
    finish = std::chrono::steady_clock::now();
    double tiempo_multiplicacion = get_time_us(start, finish, N_POLINOMIOS);

    // Test 5: Transformada inversa NTT
    start = std::chrono::steady_clock::now();
    for (int i=0; i< N_POLINOMIOS; i++){
        dcrt_a[i].toPoly(polinomio_a_t);
        dcrt_b[i].toPoly(polinomio_b_t);

        polinomio_a[i] = polinomio_a_t;
        polinomio_b[i] = polinomio_b_t;
    }
    finish = std::chrono::steady_clock::now();
    double tiempo_intt = get_time_us(start, finish, 2*N_POLINOMIOS);

    // Fin: Escribe resultados en csv
    std::ofstream datos_csv;
    datos_csv.open(CSV_FILE, std::ios::out | std::ios::app);
    datos_csv << LIBRERIA << "," << n_test << "," << modulus << "," << tiempo_polinomio << "," << tiempo_ntt << "," << tiempo_suma << "," << tiempo_multiplicacion << "," << tiempo_intt << "\n";
    datos_csv.close();

    return 0;
}

int main(){
    // Ejecutamos los test para los tres tamaños
    for (int i=0; i< REPETICIONES; i++){
        run_poly<14>(i);
        run_poly<30>(i);
        run_poly<62>(i);
    }
}
