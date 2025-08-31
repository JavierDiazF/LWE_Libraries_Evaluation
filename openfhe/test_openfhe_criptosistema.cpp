#include "openfhe.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include "tools.h"

#define N_COEF 10 // Coeficiente de los polinomios
#define REPETICIONES 10 // Vamos a hacer 10 repeticiones por test
#define CSV_FILE "he_schemes.csv"
#define LIBRERIA "openfhe" // Utilizada para saber a qué librería pertenece

//using namespace lbcrypto;

int run_bgv(int n_test, lbcrypto::SecurityLevel security_level) {

    // Parámetros Los comentados es que no se usan en OpenFHE, los define internamente
    // int n = 32768; // Es la dimensión del anillo
    // int q; // Número de bits de primos q
    int p = 65537; // Es el modulo de texto plano
    int multDepth = 2; // Número de multiplicaciones que se pueden hacer
    int sec_level;
    std::chrono::high_resolution_clock::time_point start, finish;
    switch(security_level){
      case lbcrypto::HEStd_128_quantum:
          sec_level = 128;
          break;
      case lbcrypto::HEStd_192_quantum:
          sec_level = 192;
          break;
      case lbcrypto::HEStd_256_quantum:
          sec_level = 256;
          break;
      default:
          std::cout << "Error: Nivel de seguridad no disponible\n";
          return -1;
    }
    // Creo los polinomios que voy a multiplicar y sumar
    std::vector<int64_t> a = {12,2345,65222,44,5913,65505,65,1987,65520,20};
    std::vector<int64_t> b = {11,3690,65535,35,8765,65490,89,9012,65530,10};
    std::vector<int64_t> c = {5,4321,65100,23,6789,65495,88,1024,65200,95};

    // Test 0: Crea contexto
    start = std::chrono::high_resolution_clock::now();
    lbcrypto::CCParams<lbcrypto::CryptoContextBGVRNS> parameters;
    parameters.SetMultiplicativeDepth(multDepth);
    parameters.SetPlaintextModulus(p);
    parameters.SetSecurityLevel(security_level);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_contexto= get_time_us(start, finish, 1);

    lbcrypto::CryptoContext<lbcrypto::DCRTPoly> cryptoContext = lbcrypto::GenCryptoContext(parameters);
    cryptoContext->Enable(lbcrypto::PKE);
    cryptoContext->Enable(lbcrypto::LEVELEDSHE);

    // Test 1: Generación de claves
    lbcrypto::KeyPair<lbcrypto::DCRTPoly> keyPair;
    start = std::chrono::high_resolution_clock::now();
    keyPair = cryptoContext->KeyGen();
    //Creo clave de evaluación. Si no no funciona
    cryptoContext->EvalMultKeysGen(keyPair.secretKey);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_keygen = get_time_us(start, finish, 1); // Pongo 1 porque aunque hago dos claves, son un único set, sk y de evaluación

    // Codifiación de los polinomios a texto plano
    lbcrypto::Plaintext plaintext_a = cryptoContext->MakePackedPlaintext(a);
    lbcrypto::Plaintext plaintext_b = cryptoContext->MakePackedPlaintext(b);
    lbcrypto::Plaintext plaintext_c = cryptoContext->MakePackedPlaintext(c);

    // Test 2: Cifrado de los criptogramas
    start = std::chrono::high_resolution_clock::now();
    auto ciphertext_a = cryptoContext->Encrypt(keyPair.publicKey, plaintext_a);
    auto ciphertext_b = cryptoContext->Encrypt(keyPair.publicKey, plaintext_b);
    auto ciphertext_c = cryptoContext->Encrypt(keyPair.publicKey, plaintext_c);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_cifrado = get_time_us(start, finish, 3);

    // Test 3: Suma a+b+c
    start = std::chrono::high_resolution_clock::now();
    auto suma_ab = cryptoContext->EvalAdd(ciphertext_a, ciphertext_b);
    auto suma_abc = cryptoContext->EvalAdd(suma_ab, ciphertext_c);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_suma = get_time_us(start, finish, 2);

    // Test 4: Multiplicación a*b*c
    start = std::chrono::high_resolution_clock::now();
    auto mul_ab = cryptoContext->EvalMult(ciphertext_a, ciphertext_b);
    auto mul_abc = cryptoContext->EvalMult(mul_ab, ciphertext_c);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_mul = get_time_us(start, finish, 2);

    // Test 5: Descifrado
    start = std::chrono::high_resolution_clock::now();
    lbcrypto::Plaintext plaintext_suma, plaintext_mul;
    cryptoContext->Decrypt(keyPair.secretKey, suma_abc, &plaintext_suma);
    cryptoContext->Decrypt(keyPair.secretKey, mul_abc, &plaintext_mul);
    finish = std::chrono::high_resolution_clock::now();
    double tiempo_descifrado = get_time_us(start, finish, 2);

    std::cout << "Polinomio a: " << plaintext_a << std::endl;
    std::cout << "Polinomio b: " << plaintext_b << std::endl;
    std::cout << "Polinomio c: " << plaintext_c << std::endl;
    std::cout << "a + b + c: " << plaintext_suma << std::endl;
    std::cout << "a * b * c: " << plaintext_mul << std::endl;

    // Fin: Escribe resultados en csv
    std::ofstream datos_csv;
    datos_csv.open(CSV_FILE, std::ios::out | std::ios::app);
    datos_csv << LIBRERIA << "," << n_test << "," << sec_level << "," << tiempo_keygen << "," << tiempo_cifrado << "," << tiempo_suma << "," << tiempo_mul << "," << tiempo_descifrado << "," << tiempo_contexto << "\n";
    datos_csv.close();

    return 0;
}

int main(){
  for(int i = 0; i<REPETICIONES; i++){
      run_bgv(i, lbcrypto::HEStd_128_quantum);
      run_bgv(i, lbcrypto::HEStd_192_quantum);
      run_bgv(i, lbcrypto::HEStd_256_quantum);
  }
}