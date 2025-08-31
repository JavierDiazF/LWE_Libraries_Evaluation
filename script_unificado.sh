#!/bin/sh

polinomios_csv="statistics.csv"
esquemas_csv="he_schemes.csv"
cabeceras_poly=("Libreria,Iteracion,Tamano_Mod,T_polinomio,T_NTT,T_suma,T_multiplicacion,T_INTT")
cabeceras_scheme=("Libreria,Iteracion,Sec_Level,T_keygen,T_cifardo,T_suma,T_multiplicacion,T_descifrado,T_context")
tests_librerias=("./test_nfllib" "./test_openfhe" "./test_helib" "./test_nfllib_criptosistema_128" "./test_nfllib_criptosistema_192" "./test_nfllib_criptosistema_256" "./test_openfhe_criptosistema" "./test_helib_criptosistema")

echo $cabeceras_poly > $polinomios_csv
echo $cabeceras_scheme > $esquemas_csv

for libreria in ${tests_librerias[@]}; do 
    $libreria
done