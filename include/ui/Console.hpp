#pragma once

#include <iostream>
#include <string_view>

// Codes de couleur ANSI regroupés dans un namespace typé plutôt qu'en macros #define :
// évite les collisions de noms globaux et bénéficie du typage std::string_view.
namespace Couleur
{
    inline constexpr std::string_view RESET  = "\033[0m";
    inline constexpr std::string_view CYAN   = "\033[36m";
    inline constexpr std::string_view JAUNE  = "\033[33m";
    inline constexpr std::string_view VERT   = "\033[32m";
    inline constexpr std::string_view ROUGE  = "\033[31m";
    inline constexpr std::string_view BLANC  = "\033[37m";
    inline constexpr std::string_view GRAS   = "\033[1m";
}

// Efface l'écran (séquence ANSI, sans passer par le shell).
void clear();