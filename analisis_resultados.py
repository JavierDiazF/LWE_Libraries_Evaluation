import pandas as pd

stats = ["mean", "std"] # Las estadísticas que vamos a sacar
# Polinomials results headers
statistics_csv = "analisis_datos/statistics_backup.csv"
statistics_time_labels = ["T_polinomio", "T_NTT", "T_suma", "T_multiplicacion", "T_INTT"]
statistics_group_labels = ["Libreria", "Tamano_Mod"]
# HE results headers
he_scheme_csv = "analisis_datos/he_schemes_backup.csv"
he_scheme_time_labels = ["T_keygen", "T_cifardo", "T_suma", "T_multiplicacion", "T_descifrado", "T_context"]
he_scheme_group_labels = ["Libreria", "Sec_Level"]
# Cargamos datos
df_polynomials = pd.read_csv(statistics_csv)
df_fheschemes = pd.read_csv(he_scheme_csv)
# Convertimos los tiempos a float si no lo están
df_polynomials[statistics_time_labels] = df_polynomials[statistics_time_labels].astype(float)
df_fheschemes[he_scheme_time_labels] = df_fheschemes[he_scheme_time_labels].astype(float)

# Agrupamos por librería y tamaño de módulo/seguridad
df_polynomials_grouped = df_polynomials.groupby(statistics_group_labels)
df_fheschemes_grouped = df_fheschemes.groupby(he_scheme_group_labels)

# Calculamos media y desviación estándar
polynomial_stats = df_polynomials_grouped[statistics_time_labels].agg(stats)
fheschemes_stats = df_fheschemes_grouped[he_scheme_time_labels].agg(stats)

# Mostramos los resultados
print("Estadísticas de los polinomios")
print(polynomial_stats)
print("Estadísticas de los esquemas FHE")
print(fheschemes_stats)

# Si quieres guardarlo en un CSV
polynomial_stats.to_csv("estadisticos_polinomios.csv")
fheschemes_stats.to_csv("estadisticos_esquemasFHE.csv")