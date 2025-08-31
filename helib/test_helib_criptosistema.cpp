#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <helib/helib.h>
#include "tools.h"

#define N_COEF 10 // Coeficiente de los polinomios
#define REPETICIONES 10 // Vamos a hacer 10 repeticiones por test
#define CSV_FILE "he_schemes.csv"
#define LIBRERIA "helib" // Utilizada para saber a qué librería pertenece

int run_bgv(int n_test, int security_level)
{
    // Parámetros
    unsigned long p = 65537;
    unsigned long m = 32768*2; // n de homomorphicstandard
    unsigned long r = 1; // Por defecto
    // Numbero de bits q según la seguridad
    unsigned long bits;
    switch (security_level){
        case 128: bits = 829; break;
        case 192: bits = 573; break;
        case 256: bits = 445; break;
        default:
            std::cerr << "Nivel de seguridad inválido.\n";
            return 1;
      }
    unsigned long c_keyswtich = 1; // nummero de columnas de keyswitch lo dejo por defecto
    std::chrono::high_resolution_clock::time_point start, finish;
    // Creo los polinomios que voy a multiplicar y sumar
    std::vector<int64_t> a = {12,2345,65222,44,5913,65505,65,1987,65520,20};
    std::vector<int64_t> b = {11,3690,65535,35,8765,65490,89,9012,65530,10};
    std::vector<int64_t> c = {5,4321,65100,23,6789,65495,88,1024,65200,95};
    
    // Test 0: Inicializo el contexto
    start = std::chrono::high_resolution_clock::now();
    helib::Context context = helib::ContextBuilder<helib::BGV>()
                                 .m(m)
                                 .p(p)
                                 .r(r)
                                 .bits(bits)
                                 .c(c_keyswtich)
                                 .build();
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_contexto = get_time_us(start, finish, 1);

    // Print the security level
    std::cout << "Security: " << context.securityLevel() << std::endl;

    // Test 1: Generación de claves
    start = std::chrono::high_resolution_clock::now();
    helib::SecKey secret_key(context);
    secret_key.GenSecKey();
    // Crea clave pública
    const helib::PubKey& public_key = secret_key;
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_keygen = get_time_us(start, finish, 1);

    // Genera contextto de cifrado
    const helib::EncryptedArray& ea = context.getEA();

    // Get the number of slot (phi(m))
    long nslots = ea.size();
    std::cout << "Number of slots: " << nslots << std::endl;

    // Codificiación de polinomios a texto plano 
    helib::Ptxt<helib::BGV> plaintext_a(context, a);
    helib::Ptxt<helib::BGV> plaintext_b(context, b);
    helib::Ptxt<helib::BGV> plaintext_c(context, c);

    // Test 2: Cifrado de criptogramas
    start = std::chrono::high_resolution_clock::now();
    helib::Ctxt ciphertext_a(public_key), 
                ciphertext_b(public_key), 
                ciphertext_c(public_key);
    public_key.Encrypt(ciphertext_a, plaintext_a);
    public_key.Encrypt(ciphertext_b, plaintext_b);
    public_key.Encrypt(ciphertext_c, plaintext_c);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_cifrado = get_time_us(start, finish, 3);

    // Test 3: Suma a+b+c
    start = std::chrono::high_resolution_clock::now();
    helib::Ctxt ciphertext_sum(public_key);
    ciphertext_sum = ciphertext_a;
    ciphertext_sum += ciphertext_b;
    ciphertext_sum += ciphertext_c;
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_suma = get_time_us(start, finish, 2);

    // Test 4: Multiplicacion a*b*c
    start = std::chrono::high_resolution_clock::now();
    helib::Ctxt ciphertext_mul(public_key);
    ciphertext_mul = ciphertext_a;
    ciphertext_mul *= ciphertext_b;
    ciphertext_mul *= ciphertext_c;
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_mul = get_time_us(start, finish, 2);

    // Test 5: Descifrado
    start = std::chrono::high_resolution_clock::now();
    helib::Ptxt<helib::BGV> plaintext_suma(context), plaintext_mul(context);
    secret_key.Decrypt(plaintext_suma, ciphertext_sum);
    secret_key.Decrypt(plaintext_mul, ciphertext_mul);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_descifrado = get_time_us(start, finish, 2);

    std::cout << "Polinomio a: [";
    for (int i = 0; i < N_COEF; i++){
        std::cout << plaintext_a[i] << (i == N_COEF-1 ? "]\n" : ", ");
    }
    std::cout << "Polinomio b: [";
    for (int i = 0; i < N_COEF; i++){
        std::cout << plaintext_b[i] << (i == N_COEF-1 ? "]\n" : ", ");
    }
    std::cout << "Polinomio c: ["; 
    for (int i = 0; i < N_COEF; i++){
        std::cout << plaintext_c[i] << (i == N_COEF-1 ? "]\n" : ", ");
    }
    std::cout << "a + b + c: [";
    for (int i = 0; i < N_COEF; i++){
        std::cout << plaintext_suma[i] << (i == N_COEF-1 ? "]\n" : ", ");
    }
    std::cout << "a * b * c: [";
    for (int i = 0; i < N_COEF; i++){
        std::cout <<plaintext_mul[i] << (i == N_COEF-1 ? "]\n" : ", ");
    }
    // Fin: Escribe resultados en csv
    std::ofstream datos_csv;
    datos_csv.open(CSV_FILE, std::ios::out | std::ios::app);
    datos_csv << LIBRERIA << "," << n_test << "," << security_level << "," << tiempo_keygen << "," << tiempo_cifrado << "," << tiempo_suma << "," << tiempo_mul << "," << tiempo_descifrado << "," << tiempo_contexto << "\n";
    datos_csv.close();

    return 0;
}

int main(){
  // Ejecutamos los test para los tres tamaños
    for (int i=0; i< REPETICIONES; i++){
        run_bgv(i, 128);
        run_bgv(i, 192);
        run_bgv(i, 256);
    }
}