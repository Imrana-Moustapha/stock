#include "ui/Console.hpp"
#include "menus/Menus.hpp"
#include "services/Gestionnairestock.hpp"
#include <algorithm>
#include <vector>
#include <utility>

using namespace Couleur;

namespace {

void pause()
{
    std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
    std::cin.get();
}

void afficherValeurStock(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tValeur totale du stock\n" << RESET << "\n";

    std::cout << "\t\t" << GRAS << "Valeur globale : " << RESET
               << gestionnaire.valeurTotaleStock() << "\n\n";

    auto parCategorie = gestionnaire.valeurStockParCategorie();
    if (parCategorie.empty()) {
        std::cout << "\t\t" << JAUNE << "Aucun produit enregistré." << RESET << "\n";
    } else {
        std::cout << "\t\t" << GRAS << "Détail par catégorie :" << RESET << "\n";
        for (const auto& [categorie, valeur] : parCategorie)
            std::cout << "\t\t  - " << categorie << " : " << valeur << "\n";
    }
    pause();
}

void afficherProduitsLesPlusMouvementes(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tProduits les plus mouvementés\n" << RESET << "\n";

    auto compteurs = gestionnaire.nombreMouvementsParProduit();
    if (compteurs.empty()) {
        std::cout << "\t\t" << JAUNE << "Aucun mouvement enregistré pour le moment." << RESET << "\n";
        pause();
        return;
    }

    // Trie par nombre de mouvements décroissant.
    std::vector<std::pair<std::string, int>> tri(compteurs.begin(), compteurs.end());
    std::sort(tri.begin(), tri.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    int rang = 1;
    for (const auto& [reference, nombre] : tri) {
        auto it = std::find_if(gestionnaire.getProduits().begin(), gestionnaire.getProduits().end(),
            [&reference](const auto& p) { return p->getReference() == reference; });
        std::string nom = (it != gestionnaire.getProduits().end()) ? (*it)->getNom() : "(produit supprimé)";
        std::cout << "\t\t" << rang++ << ". " << reference << " (" << nom << ") : "
                   << nombre << " mouvement(s)\n";
    }
    pause();
}

void afficherProduitsDormants(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tProduits dormants (aucun mouvement)\n" << RESET << "\n";

    auto dormants = gestionnaire.produitsDormants();
    if (dormants.empty()) {
        std::cout << "\t\t" << VERT << "Aucun produit dormant : tous ont connu au moins un mouvement." << RESET << "\n";
    } else {
        for (const auto* produit : dormants) {
            std::cout << "\t\t" << JAUNE;
            produit->afficher();
            std::cout << RESET;
        }
    }
    pause();
}

} // namespace anonyme

void sous_menu_statistiques(GestionnaireStock& gestionnaire) {
    int choixSousMenu = 0;
    do {
        clear();
        std::cout << "\n\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << CYAN  << "\t\t||         " << GRAS << "STATISTIQUES & TABLEAUX DE BORD" << RESET << CYAN << "           ||\n" << RESET;
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << BLANC << "\t\t||  1. Valeur totale du stock (globale / categorie)  ||\n";
        std::cout << "\t\t||  2. Consulter les produits les plus mouvementes   ||\n";
        std::cout << "\t\t||  3. Detecter les produits dormants (surstock)     ||\n";
        std::cout << "\t\t||  0. Retour au menu principal                      ||" << RESET << "\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << "\t\t   Votre choix : " << VERT;

        if (!lireChoix(choixSousMenu)) continue;

        switch (choixSousMenu) {
            case 1: afficherValeurStock(gestionnaire); break;
            case 2: afficherProduitsLesPlusMouvementes(gestionnaire); break;
            case 3: afficherProduitsDormants(gestionnaire); break;
            case 0: break;
            default: 
                std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
                std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
                viderBuffer();
                std::cin.get();
        }
    } while (choixSousMenu != 0);
}