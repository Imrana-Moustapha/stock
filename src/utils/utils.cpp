#include "../../include/header.hpp"

using namespace Couleur;

void viderBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void clear()
{
    // Séquence ANSI : efface l'écran et replace le curseur en haut à gauche.
    // Évite std::system("clear"/"cls"), plus lent et dépendant du shell.
    std::cout << "\033[2J\033[1;1H";
}

bool lireChoix(int& choix)
{
    if (!(std::cin >> choix)) {
        std::cout << RESET;
        std::cout << "\n\t\t" << ROUGE << "[!] Erreur de saisie." << RESET << "\n";
        viderBuffer();
        std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
        std::cin.get();
        return false;
    }
    std::cout << RESET;
    return true;
}