cmake_minimum_required(VERSION 3.14)
project(cpp)

set(CMAKE_CXX_STANDARD 23)

# Rediriger l'exécutable compilé vers le dossier bin/ à la racine
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# Inclure le dossier 'include' pour que le compilateur trouve tous vos .hpp
include_directories(include)

# Récupérer automatiquement tous les fichiers .cpp dans src/ (et ses sous-dossiers)
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Nom de l'exécutable aligné sur celui produit par le Makefile (bin/mon_programme) :
# le même nom, quel que soit l'outil de build utilisé pour compiler.
add_executable(mon_programme ${SOURCES})