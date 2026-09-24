#include "menus/Menus.hpp"
#include "ui/Console.hpp"

using namespace Couleur;

void menu_principal(GestionnaireStock &gestionnaire)
{
    int choixPrincipal = 0;

    do
    {
        clear();

        std::cout << "\n\n";
        std::cout << JAUNE << "\t\t=======================================================\n"
                  << RESET;
        std::cout << CYAN << "\t\t||           " << GRAS << "SYSTEME DE GESTION DE STOCK v1.0" << RESET << CYAN << "        ||\n"
                  << RESET;
        std::cout << JAUNE << "\t\t=======================================================\n"
                  << RESET;
        std::cout << BLANC << "\t\t||  1. Gestion des Produits                          ||\n";
        std::cout << "\t\t||  2. Mouvements de Stock (Entrees/Sorties)         ||\n";
        std::cout << "\t\t||  3. Recherche et Filtres                          ||\n";
        std::cout << "\t\t||  4. Commandes et Fournisseurs                     ||\n";
        std::cout << "\t\t||  5. Statistiques et Tableaux de bord              ||\n";
        std::cout << "\t\t||  6. Configuration et Administration [Admin]       ||\n";
        std::cout << "\t\t||  0. Quitter                                       ||" << RESET << "\n";
        std::cout << JAUNE << "\t\t=======================================================\n"
                  << RESET;
        std::cout << "\t\t   Votre choix : " << VERT;

        if (!lireChoix(choixPrincipal))
            continue;

        switch (choixPrincipal)
        {
        case 1:
            sous_menu_produits(gestionnaire);
            break;
        case 2:
            sous_menu_mouvements();
            break;
        case 3:
            sous_menu_recherche();
            break;
        case 4:
            sous_menu_commandes();
            break;
        case 5:
            sous_menu_statistiques();
            break;
        case 6:
            sous_menu_administration();
            break;
        case 0:
            clear();
            std::cout << "\n\t\t" << JAUNE << "=======================================\n";
            std::cout << "\t\t  Fermeture de l'application. Au revoir !\n";
            std::cout << "\t\t=======================================" << RESET << "\n\n";
            break;
        default:
            std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide. Veuillez reessayer." << RESET << "\n";
            std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
            viderBuffer();
            std::cin.get();
        }
    } while (choixPrincipal != 0);
}