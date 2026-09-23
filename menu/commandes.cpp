#include "../include/header.hpp"

using namespace Couleur;

void sous_menu_commandes() {
    int choixSousMenu = 0;
    do {
        clear();
        std::cout << "\n\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << CYAN  << "\t\t||              " << GRAS << "COMMANDES & FOURNISSEURS" << RESET << CYAN << "             ||\n" << RESET;
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << BLANC << "\t\t||  1. Lister les fournisseurs                       ||\n";
        std::cout << "\t\t||  2. Generer une proposition de reapprovisionnement||\n";
        std::cout << "\t\t||  3. Suivre le statut des commandes en cours       ||\n";
        std::cout << "\t\t||  0. Retour au menu principal                      ||" << RESET << "\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << "\t\t   Votre choix : " << VERT;

        if (!lireChoix(choixSousMenu)) continue;

        switch (choixSousMenu) {
            case 1: break;
            case 2: break;
            case 3: break;
            case 0: break;
            default: 
                std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
                std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
                viderBuffer();
                std::cin.get();
        }
    } while (choixSousMenu != 0);
}