#include "ui/Console.hpp"
#include "ui/Saisie.hpp"
#include "utils/DateUtils.hpp"
#include "menus/Menus.hpp"
#include "services/GestionnaireStock.hpp"
#include "exceptions/Exceptions.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace Couleur;

namespace {

void enregistrerEntree(GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tEntrée de stock\n" << RESET << "\n";

    std::string reference = lireTexte("\t\tRéférence du produit : ");
    if (gestionnaire.trouverProduit(reference) == nullptr) {
        std::cout << "\n\t\t" << ROUGE << "[!] Produit introuvable." << RESET << "\n";
        attendreEntree();
        return;
    }

    int quantite = lireEntier("\t\tQuantité à ajouter : ");
    std::string auteur = lireTexte("\t\tAuteur / motif : ");

    gestionnaire.ajouterStock(reference, quantite, auteur);
    std::cout << "\n\t\t" << VERT << "[OK] Entrée enregistrée." << RESET << "\n";
    attendreEntree();
}

void enregistrerSortie(GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tSortie de stock\n" << RESET << "\n";

    std::string reference = lireTexte("\t\tRéférence du produit : ");
    if (gestionnaire.trouverProduit(reference) == nullptr) {
        std::cout << "\n\t\t" << ROUGE << "[!] Produit introuvable." << RESET << "\n";
        attendreEntree();
        return;
    }

    int quantite = lireEntier("\t\tQuantité à retirer : ");
    std::string auteur = lireTexte("\t\tAuteur / motif : ");

    gestionnaire.retirerStock(reference, quantite, auteur);
    std::cout << "\n\t\t" << VERT << "[OK] Sortie enregistrée." << RESET << "\n";
    attendreEntree();
}

void enregistrerAjustement(GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tAjustement de stock\n" << RESET << "\n";

    std::string reference = lireTexte("\t\tRéférence du produit : ");
    Produit* produit = gestionnaire.trouverProduit(reference);
    if (produit == nullptr) {
        std::cout << "\n\t\t" << ROUGE << "[!] Produit introuvable." << RESET << "\n";
        attendreEntree();
        return;
    }

    std::cout << "\t\tQuantité actuelle en stock : " << produit->getQuantiteStock() << "\n";
    int nouvelleQuantite = lireEntier("\t\tNouvelle quantité (après inventaire) : ");
    std::string auteur = lireTexte("\t\tAuteur / motif : ");

    gestionnaire.ajusterStock(reference, nouvelleQuantite, auteur);
    std::cout << "\n\t\t" << VERT << "[OK] Ajustement enregistré." << RESET << "\n";
    attendreEntree();
}

void consulterJournal(const GestionnaireStock& gestionnaire)
{
    clear();
    const auto& historique = gestionnaire.getHistorique();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tJournal des transactions (" << historique.size() << ")\n" << RESET << "\n";

    if (historique.empty()) {
        std::cout << "\t\t" << JAUNE << "Aucun mouvement enregistré pour le moment." << RESET << "\n";
    } else {
        for (const auto& mvt : historique) {
            std::cout << "\t\t[" << DateUtils::formaterDateHeure(mvt.getDateHeure()) << "] "
                      << libelle(mvt.getType()) << " | Ref: " << mvt.getReferenceProduit()
                      << " | Qte: " << mvt.getQuantite() << " | Par: " << mvt.getAuteur() << "\n";
        }
    }
    attendreEntree();
}

} // namespace anonyme

void sous_menu_mouvements(GestionnaireStock& gestionnaire) {
    int choixSousMenu = 0;
    do {
        clear();
        std::cout << "\n\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << CYAN  << "\t\t||           " << GRAS << "MOUVEMENTS DE STOCK & AUDIT" << RESET << CYAN << "             ||\n" << RESET;
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << BLANC << "\t\t||  1. Enregistrer une entree en stock               ||\n";
        std::cout << "\t\t||  2. Enregistrer une sortie de stock               ||\n";
        std::cout << "\t\t||  3. Ajustement de stock                           ||\n";
        std::cout << "\t\t||  4. Consulter le journal des transactions (Audit) ||\n";
        std::cout << "\t\t||  0. Retour au menu principal                      ||" << RESET << "\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << "\t\t   Votre choix : " << VERT;

        if (!lireChoix(choixSousMenu)) continue;

        try {
            switch (choixSousMenu) {
                case 1: enregistrerEntree(gestionnaire); break;
                case 2: enregistrerSortie(gestionnaire); break;
                case 3: enregistrerAjustement(gestionnaire); break;
                case 4: consulterJournal(gestionnaire); break;
                case 0: break;
                default:
                    std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
                    attendreEntree();
            }
        } catch (const std::exception& e) {
            std::cout << "\n\t\t" << ROUGE << "[!] Erreur : " << e.what() << RESET << "\n";
            attendreEntree();
        }
    } while (choixSousMenu != 0);
}