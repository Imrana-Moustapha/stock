#include "ui/Console.hpp"
#include "menus/Menus.hpp"
#include "services/Gestionnairestock.hpp"
#include <algorithm>
#include <vector>
#include <cctype>
#include <iterator>

using namespace Couleur;

namespace {

std::string lireTexte(const std::string& invite)
{
    std::string valeur;
    do {
        std::cout << invite;
        std::getline(std::cin, valeur);
    } while (valeur.empty());
    return valeur;
}

int lireEntier(const std::string& invite)
{
    int valeur;
    while (true) {
        std::cout << invite;
        if (std::cin >> valeur) break;
        std::cout << ROUGE << "[!] Veuillez entrer un nombre entier valide." << RESET << "\n";
        viderBuffer();
    }
    viderBuffer();
    return valeur;
}

void pause()
{
    std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
    std::cin.get();
}

std::string versMinuscules(const std::string& texte)
{
    std::string resultat = texte;
    std::transform(resultat.begin(), resultat.end(), resultat.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return resultat;
}

bool contient(const std::string& texte, const std::string& motif)
{
    return versMinuscules(texte).find(versMinuscules(motif)) != std::string::npos;
}

// Vue non-propriétaire sur les produits, pratique pour les algorithmes STL
// (std::copy_if ne peut pas travailler directement sur des unique_ptr).
std::vector<const Produit*> vueProduits(const GestionnaireStock& gestionnaire)
{
    std::vector<const Produit*> vue;
    for (const auto& p : gestionnaire.getProduits())
        vue.push_back(p.get());
    return vue;
}

void afficherResultats(const std::vector<const Produit*>& resultats)
{
    if (resultats.empty()) {
        std::cout << "\t\t" << JAUNE << "Aucun résultat." << RESET << "\n";
        return;
    }
    for (const Produit* produit : resultats) {
        std::cout << "\t\t";
        if (produit->estSousLeSeuil())
            std::cout << ROUGE;
        produit->afficher();
        std::cout << RESET;
    }
}

void rechercherParNomOuReference(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tRecherche par nom ou référence\n" << RESET << "\n";

    std::string motif = lireTexte("\t\tTexte recherché : ");
    auto vue = vueProduits(gestionnaire);

    std::vector<const Produit*> resultats;
    std::copy_if(vue.begin(), vue.end(), std::back_inserter(resultats),
        [&motif](const Produit* p) { return contient(p->getNom(), motif) || contient(p->getReference(), motif); });

    std::cout << "\n\t\t" << resultats.size() << " résultat(s) :\n\n";
    afficherResultats(resultats);
    pause();
}

void filtrerParCategorie(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tFiltrer par catégorie\n" << RESET << "\n";

    std::string categorie = lireTexte("\t\tCatégorie : ");
    auto vue = vueProduits(gestionnaire);

    std::vector<const Produit*> resultats;
    std::copy_if(vue.begin(), vue.end(), std::back_inserter(resultats),
        [&categorie](const Produit* p) { return versMinuscules(p->getCategorie()) == versMinuscules(categorie); });

    std::cout << "\n\t\t" << resultats.size() << " résultat(s) :\n\n";
    afficherResultats(resultats);
    pause();
}

void filtrerPerissablesProchesPeremption(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tPérissables proches de péremption\n" << RESET << "\n";

    int seuilJours = lireEntier("\t\tAfficher les produits périmant dans moins de combien de jours ? : ");
    auto vue = vueProduits(gestionnaire);

    std::vector<const Produit*> resultats;
    std::copy_if(vue.begin(), vue.end(), std::back_inserter(resultats),
        [seuilJours](const Produit* p) {
            const auto* perissable = dynamic_cast<const ProduitPerissable*>(p);
            return perissable != nullptr && perissable->joursAvantPeremption() <= seuilJours;
        });

    std::cout << "\n\t\t" << resultats.size() << " résultat(s) :\n\n";
    afficherResultats(resultats);
    pause();
}

void afficherProduitsSousLeSeuil(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tProduits sous le seuil critique\n" << RESET << "\n";

    auto resultats = gestionnaire.produitsSousLeSeuil();
    std::cout << "\n\t\t" << resultats.size() << " résultat(s) :\n\n";
    afficherResultats(resultats);
    pause();
}

} // namespace anonyme

void sous_menu_recherche(GestionnaireStock& gestionnaire) {
    int choixSousMenu = 0;
    do {
        clear();
        std::cout << "\n\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << CYAN  << "\t\t||              " << GRAS << "RECHERCHE ET FILTRES" << RESET << CYAN << "                 ||\n" << RESET;
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << BLANC << "\t\t||  1. Rechercher par nom ou reference               ||\n";
        std::cout << "\t\t||  2. Filtrer par categorie                         ||\n";
        std::cout << "\t\t||  3. Filtrer perissables proches de peremption     ||\n";
        std::cout << "\t\t||  4. Afficher les produits sous le seuil critique  ||\n";
        std::cout << "\t\t||  0. Retour au menu principal                      ||" << RESET << "\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << "\t\t   Votre choix : " << VERT;

        if (!lireChoix(choixSousMenu)) continue;

        switch (choixSousMenu) {
            case 1: rechercherParNomOuReference(gestionnaire); break;
            case 2: filtrerParCategorie(gestionnaire); break;
            case 3: filtrerPerissablesProchesPeremption(gestionnaire); break;
            case 4: afficherProduitsSousLeSeuil(gestionnaire); break;
            case 0: break;
            default: 
                std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
                std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
                viderBuffer();
                std::cin.get();
        }
    } while (choixSousMenu != 0);
}