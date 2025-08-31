// Se parte del ejemplo disponible en https://github.com/quarkslab/NFLlib

#include <cstddef>
#include <gmpxx.h>
#include <chrono>
#include <iostream>
#include <nfl.hpp>
#include <thread>
#include <vector>
#include <fstream>
#include "tools.h"

#define N_COEF 16 // Coeficiente de los polinomios y grado del polinomio
#define REPETICIONES 10 // Vamos a hacer 10 repeticiones por test
#define CSV_FILE "he_schemes.csv"
#define LIBRERIA "nfllib" // Utilizada para saber a qué librería pertenece
#define SEC_LEVEL 192
#define MODULUS_Q 573 // Es el parámetro para seguridad 256 definido por el homomorphic standard

/// Los parámetros se definen en el namespace y luego se llama a #include <fv.hpp>
namespace FV {
namespace params {
using poly_t = nfl::poly_from_modulus<uint64_t, N_COEF, MODULUS_Q>;
template <typename T>
struct plaintextModulus;
template <>
struct plaintextModulus<mpz_class> {
  static mpz_class value() {
    return mpz_class("65537");
  }
};
using gauss_struct = nfl::gaussian<uint16_t, uint64_t, 2>;
using gauss_t = nfl::FastGaussianNoise<uint16_t, uint64_t, 2>;
gauss_t fg_prng_sk(8.0, 128, 1 << 14);
gauss_t fg_prng_evk(8.0, 128, 1 << 14);
gauss_t fg_prng_pk(8.0, 128, 1 << 14);
gauss_t fg_prng_enc(8.0, 128, 1 << 14);
}
}  // namespace FV::params
#include "FV.hpp"

int run_bfv(int n_test, int security_level) {
    // Semilla para valores deterministas
    srand(0);
    std::chrono::high_resolution_clock::time_point start, finish;
    // Creo los polinomios que voy a multiplicar
    FV::params::poly_p polinomios[3];
    
    polinomios[0] = {12,2345,65222,44,5913,65505,65,1987,65520,20,0,0,0,0,0,0}; // a
    polinomios[1] = {11,3690,65535,35,8765,65490,89,9012,65530,10,0,0,0,0,0,0}; // b
    polinomios[2] = {5,4321,65100,23,6789,65495,88,1024,65200,95,0,0,0,0,0,0};  // c

    // Test 1: Generación de claves
    start = std::chrono::high_resolution_clock::now();
    FV::sk_t secret_key;
    FV::evk_t evaluation_key(secret_key, 32);
    FV::pk_t public_key(secret_key, evaluation_key);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_keygen = get_time_us(start, finish, 1);

    // Test 2: Cifrado
    start = std::chrono::high_resolution_clock::now();
    std::array<FV::ciphertext_t, 3> texto_cifrado;
    FV::encrypt_poly(texto_cifrado[0], public_key, polinomios[0]);
    FV::encrypt_poly(texto_cifrado[1], public_key, polinomios[1]);
    FV::encrypt_poly(texto_cifrado[2], public_key, polinomios[2]);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_cifrado = get_time_us(start, finish, 3);

    // Test 3: Suma a+b+c
    start = std::chrono::high_resolution_clock::now();
    FV::ciphertext_t suma_abc = texto_cifrado[0] + texto_cifrado[1];
    suma_abc = suma_abc + texto_cifrado[2];
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_suma = get_time_us(start, finish, 2);

    // Test 4: Multiplicacion a*b*c
    start = std::chrono::high_resolution_clock::now();
    FV::ciphertext_t mul_abc = texto_cifrado[0] * texto_cifrado[1];
    mul_abc = mul_abc * texto_cifrado[2];
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_mul = get_time_us(start, finish, 2);

    // Inicializamos polinomios de descifrado
    std::array<mpz_t, N_COEF> plaintext_suma, plaintext_mul;
    
    for (size_t i = 0; i < N_COEF; i++) {
        mpz_inits(plaintext_suma[i], plaintext_mul[i], nullptr);
    }
    
    // Test 5: Descifrado
    start = std::chrono::high_resolution_clock::now();
    FV::decrypt_poly(plaintext_suma, secret_key, public_key, suma_abc);
    FV::decrypt_poly(plaintext_mul, secret_key, public_key, mul_abc);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_descifrado = get_time_us(start, finish, 2);

    std::array<mpz_t, N_COEF> polinomio_a, polinomio_b, polinomio_c;
    
    for (size_t i = 0; i < N_COEF; i++) {
        mpz_inits(polinomio_a[i], polinomio_b[i], polinomio_c[i], nullptr);
    }
    
    FV::decrypt_poly(polinomio_a, secret_key, public_key, texto_cifrado[0]);
    FV::decrypt_poly(polinomio_b, secret_key, public_key, texto_cifrado[1]);
    FV::decrypt_poly(polinomio_c, secret_key, public_key, texto_cifrado[2]);
    
    std::cout << "Polinomio a: [";
    for (int i = 0; i < N_COEF; i++){
        std::cout << mpz_class(polinomio_a[i]).get_str() << (i == N_COEF-1 ? "]\n" : ", ");
    }
    std::cout << "Polinomio b: [";
    for (int i = 0; i < N_COEF; i++){
        std::cout << mpz_class(polinomio_b[i]).get_str() << (i == N_COEF-1 ? "]\n" : ", ");
    }
    
    std::cout << "Polinomio c: ["; 
    for (int i = 0; i < N_COEF; i++){
        std::cout << mpz_class(polinomio_c[i]).get_str() << (i == N_COEF-1 ? "]\n" : ", ");
    }
    std::cout << "a + b + c: [";
    for (int i = 0; i < N_COEF; i++){
        std::cout << mpz_class(plaintext_suma[i]).get_str() << (i == N_COEF-1 ? "]\n" : ", ");
    }
    std::cout << "a * b * c: [";
    for (int i = 0; i < N_COEF; i++){
        std::cout << mpz_class(plaintext_mul[i]).get_str() << (i == N_COEF-1 ? "]\n" : ", ");
    }
    
    for (size_t i = 0; i < N_COEF; i++) {
        mpz_clear(polinomio_a[i]);
        mpz_clear(polinomio_b[i]);
        mpz_clear(polinomio_c[i]);
        mpz_clear(plaintext_suma[i]);
        mpz_clear(plaintext_mul[i]);
    }
    
    // Fin: Escribe resultados en csv
    std::ofstream datos_csv;
    datos_csv.open(CSV_FILE, std::ios::out | std::ios::app);
    datos_csv << LIBRERIA << "," << n_test << "," << security_level << "," << tiempo_keygen << "," << tiempo_cifrado << "," << tiempo_suma << "," << tiempo_mul << "," << tiempo_descifrado << "," << "\n"; //Pongo dos commas porque este no veo que inicialize contttexto
    datos_csv.close();

    return 0;
}

int main(){
    for (int i=0; i< REPETICIONES; i++){
        run_bfv(i, SEC_LEVEL);
    }
}