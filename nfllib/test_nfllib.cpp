#include <iostream>
#include <fstream>
#include <iterator>
#include <nfl.hpp>
#include <chrono>
#include "tools.h" // Archivo con funciones para agilizar los tests

#define N_POLINOMIOS 10 // En cada test creamos 10 polinomios
#define REPETICIONES 10 // Vamos a hacer 10 repeticiones por test

#define CSV_FILE "statistics.csv"
#define LIBRERIA "nfllib" // Utilizada para saber a qué librería pertenece

template<size_t degree, size_t modulus, class T>
void run_poly(int n_test) {

    // Definir el tipo de polinomio (parámetros: grado y tamaño q)
    using poly_t = nfl::poly_from_modulus<T, degree, modulus>;
    std::chrono::high_resolution_clock::time_point start, finish;

    // Creamos un array de n Repeticiones alineado 32 bytes para asegurar el funcionamiento de AVX y SSE
    poly_t *polinomio_a = alloc_aligned<poly_t, 32>(N_POLINOMIOS), // Polinomio original sobre el que se van a hacer las operaciones básicas
           *polinomio_b = alloc_aligned<poly_t, 32>(N_POLINOMIOS), // Polinomio que voy a sumar y multiplicar
           *polinomio_c = alloc_aligned<poly_t, 32>(N_POLINOMIOS); // Polinomio donde voy a guardar el restultado de las sumas y restas
    if ((((uintptr_t)polinomio_a % 32) != 0) || 
        (((uintptr_t)polinomio_b % 32) != 0) || 
        (((uintptr_t)polinomio_b % 32) != 0)) {
        printf("Error: Puntero no aineado\n");
        exit(1);
    }
    // Inicializamos los array a ceros
    std::fill(polinomio_a, polinomio_a + N_POLINOMIOS, 0);
    std::fill(polinomio_b, polinomio_b + N_POLINOMIOS, 0);
    std::fill(polinomio_c, polinomio_c + N_POLINOMIOS, 0);

    // Test 1: Tiempo de creación de polinomios
    start = std::chrono::high_resolution_clock::now();
    std::fill(polinomio_a, polinomio_a + N_POLINOMIOS, nfl::uniform());
    std::fill(polinomio_b, polinomio_b + N_POLINOMIOS, nfl::uniform());
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_polinomio = get_time_us(start, finish, 2*N_POLINOMIOS);
    
    // Test 2: Transformada directa NTT
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i <N_POLINOMIOS; i++){
        polinomio_a[i].ntt_pow_phi();
        polinomio_b[i].ntt_pow_phi();
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_ntt = get_time_us(start, finish, 2*N_POLINOMIOS); // Multiplico por 2 porque estamos haciendo dos transformadas de a y b

    // Test 3: Suma NTT
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i <N_POLINOMIOS; i++){
        polinomio_c[i] = polinomio_a[i] + polinomio_b[i];
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_suma = get_time_us(start, finish, N_POLINOMIOS);

    // Test 4: Multiplicación NTT
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i <N_POLINOMIOS; i++){
        polinomio_c[i] = polinomio_a[i] * polinomio_b[i];
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_multiplicacion = get_time_us(start, finish, N_POLINOMIOS);

    // Test 5: Transformada inversa NTT
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i <N_POLINOMIOS; i++){
        polinomio_a[i].invntt_pow_invphi();
        polinomio_b[i].invntt_pow_invphi();
    }
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_intt = get_time_us(start, finish, 2*N_POLINOMIOS);

    // Fin: Escribe resultados en csv
    std::ofstream datos_csv;
    datos_csv.open(CSV_FILE, std::ios::out | std::ios::app);
    datos_csv << LIBRERIA << "," << n_test << "," << modulus << "," << tiempo_polinomio << "," << tiempo_ntt << "," << tiempo_suma << "," << tiempo_multiplicacion << "," << tiempo_intt << "\n";
    datos_csv.close();
    
    // Libero el espacio reservado
    free_aligned(REPETICIONES, polinomio_a);
    free_aligned(REPETICIONES, polinomio_b);
    free_aligned(REPETICIONES, polinomio_c);
}

int main(){
    // Ejecutamos los test para los tres tamaños
    for (int i=0; i< REPETICIONES; i++){
        run_poly<256, 14, uint16_t>(i);
        run_poly<256, 30, uint32_t>(i);
        run_poly<256, 62, uint64_t>(i);
    }

}