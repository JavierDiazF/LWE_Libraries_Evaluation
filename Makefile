CXX = clang++
# NFLlib
CXXFLAGS_NFLlib = -std=c++11 -O2 -I$(HOME)/nfllib/include -I/usr/local/opt/gmp/include -I./include
LDFLAGS_NFLlib = -L$(HOME)/nfllib/lib -L/usr/local/opt/gmp/lib -Wl,-rpath,$(HOME)/nfllib/lib 
LIBS_NFLlib = -lnfllib -lgmp -lmpfr
TARGET_NFLlib = test_nfllib
SRC_NFLlib = nfllib/test_nfllib.cpp
# Seguridad 128
TARGET_FV_NFLlib_128 = test_nfllib_criptosistema_128
SRC_FV_NFLlib_128 = nfllib/test_nfllib_criptosistema_128.cpp
# Seguridad 192
TARGET_FV_NFLlib_192 = test_nfllib_criptosistema_192
SRC_FV_NFLlib_192 = nfllib/test_nfllib_criptosistema_192.cpp
# Seguridad 256
TARGET_FV_NFLlib_256 = test_nfllib_criptosistema_256
SRC_FV_NFLlib_256 = nfllib/test_nfllib_criptosistema_256.cpp

# OpenFHE
# Compilando con el CMakeList.txt de OpenFHE
BUILDDIR_OpenFHE = build_openfhe
DIR_OpenFHE = ../openfhe
SRC_OpenFHE = openfhe/test_openfhe.cpp openfhe/test_openfhe_criptosistema.cpp
CMakeList_OpenFHE = openfhe/CMakeLists.txt

# HElib
CMAKEFLAGS_HElib = -Dhelib_DIR=$(HOME)/TFM_Cripto_Librerias/HElib_install/helib_pack/share/cmake/helib
BUILDDIR_HElib = build_helib
DIR_HElib = ../helib
SRC_Helib = helib/test_helib.cpp helib/test_helib_criptosistema.cpp helib/prueba.cpp
CMakeList_HElib = helib/CMakeLists.txt

all: nfllib openfhe helib

nfllib: $(SRC_NFLlib) $(SRC_FV_NFLlib_128) $(SRC_FV_NFLlib_192) $(SRC_FV_NFLlib_256)
	$(CXX) $(CXXFLAGS_NFLlib) -o $(TARGET_NFLlib) $(SRC_NFLlib) $(LDFLAGS_NFLlib) $(LIBS_NFLlib)
	$(CXX) $(CXXFLAGS_NFLlib) -o $(TARGET_FV_NFLlib_128) $(SRC_FV_NFLlib_128) $(LDFLAGS_NFLlib) $(LIBS_NFLlib)
	$(CXX) $(CXXFLAGS_NFLlib) -o $(TARGET_FV_NFLlib_192) $(SRC_FV_NFLlib_192) $(LDFLAGS_NFLlib) $(LIBS_NFLlib)
	$(CXX) $(CXXFLAGS_NFLlib) -o $(TARGET_FV_NFLlib_256) $(SRC_FV_NFLlib_256) $(LDFLAGS_NFLlib) $(LIBS_NFLlib)
	
openfhe: $(SRC_OpenFHE) $(CMakeList_OpenFHE)
	mkdir -p $(BUILDDIR_OpenFHE)
	cd $(BUILDDIR_OpenFHE) && cmake $(DIR_OpenFHE)
	cd $(BUILDDIR_OpenFHE) && make
	mv $(BUILDDIR_OpenFHE)/test_* .
# Para compilar por mi cuenta	
# $(CXX) $(CXXFLAGS_OpenFHE) -o $(TARGET_OpenFHE) $(SRC_OpenFHE) $(LDFLAGS_OpenFHE) $(LIBS_OpenFHE)

helib: $(SRC_Helib) $(CMakeList_HElib)
	mkdir -p $(BUILDDIR_HElib)
	cd $(BUILDDIR_HElib) && cmake $(CMAKEFLAGS_HElib) $(DIR_HElib)
	cd $(BUILDDIR_HElib) && make
	mv $(BUILDDIR_HElib)/bin/test_* .

clean:
	rm -f test_*
	rm -f *.csv
	rm -rf $(BUILDDIR_OpenFHE)
	rm -rf $(BUILDDIR_HElib)
	rm -rf *.log