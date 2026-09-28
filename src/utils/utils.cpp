#include "ui/Console.hpp"

void clear()
{
    // Séquence ANSI : efface l'écran et replace le curseur en haut à gauche.
    // Évite std::system("clear"/"cls"), plus lent et dépendant du shell.
    std::cout << "\033[2J\033[1;1H";
}