#include "ui/Console.hpp"
#include "ui/Saisie.hpp"
#include "menus/Menus.hpp"
#include "services/GestionnaireStock.hpp"
#include "services/GestionnaireCommandes.hpp"
#include <chrono>
#include "utils/DateUtils.hpp"

using namespace Couleur;

namespace {

std::string libelleStatut(StatutCommande statut)
{
    switch (statut) {
        case StatutCommande::EN_COURS: return "EN COURS";
        case StatutCommande::LIVREE:   return "LIVREE";
        case StatutCommande::ANNULEE:  return "ANNULEE";
    }
    return "?";
}

void listerFournisseurs(const GestionnaireCommandes& gc)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tFournisseurs (" << gc.getFournisseurs().size() << ")\n" << RESET << "\n";
    if (gc.getFournisseurs().empty()) {
        std::cout << "\t\t" << JAUNE << "Aucun fournisseur enregistré." << RESET << "\n";
    } else {
        for (const auto& f : gc.getFournisseurs())
            std::cout << "\t\t[" << f.getId() << "] " << f.getNom() << " - " << f.getContact() << " - " << f.getAdresse() << "\n";
    }
    attendreEntree();
}

void ajouterFournisseur(GestionnaireCommandes& gc)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tAjouter un fournisseur\n" << RESET << "\n";
    std::string nom = lireTexte("\t\tNom : ");
    std::string contact = lireTexte("\t\tContact (téléphone/email) : ");
    std::string adresse = lireTexte("\t\tAdresse : ");

    int id = gc.ajouterFournisseur(nom, contact, adresse);
    std::cout << "\n\t\t" << VERT << "[OK] Fournisseur ajouté (id=" << id << ")." << RESET << "\n";
    attendreEntree();
}

void creerCommande(GestionnaireCommandes& gc)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tCréer une commande\n" << RESET << "\n";

    if (gc.getFournisseurs().empty()) {
        std::cout << "\t\t" << ROUGE << "[!] Aucun fournisseur enregistré. Ajoutez-en un d'abord." << RESET << "\n";
        attendreEntree();
        return;
    }

    int idFournisseur = lireEntier("\t\tId du fournisseur : ");
    auto date = lireDate("Date de commande");

    int idCommande = gc.creerCommande(idFournisseur, date);
    std::cout << "\n\t\t" << VERT << "[OK] Commande créée (id=" << idCommande << "). "
               << "Utilisez ensuite \"Ajouter une ligne\" pour y ajouter des produits." << RESET << "\n";
    attendreEntree();
}

void ajouterLigneCommande(GestionnaireCommandes& gc)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tAjouter une ligne à une commande\n" << RESET << "\n";

    int idCommande = lireEntier("\t\tId de la commande : ");
    std::string reference = lireTexte("\t\tRéférence produit : ");
    int quantite = lireEntierStrictementPositif("\t\tQuantité commandée : ");
    double prixUnitaire = lireDoubleNonNegatif("\t\tPrix unitaire : ");

    gc.ajouterLigneCommande(idCommande, reference, quantite, prixUnitaire);
    std::cout << "\n\t\t" << VERT << "[OK] Ligne ajoutée." << RESET << "\n";
    attendreEntree();
}

void listerCommandes(const GestionnaireCommandes& gc)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tCommandes (" << gc.getCommandes().size() << ")\n" << RESET << "\n";
    if (gc.getCommandes().empty()) {
        std::cout << "\t\t" << JAUNE << "Aucune commande enregistrée." << RESET << "\n";
    } else {
        for (const auto& c : gc.getCommandes()) {
            std::cout << "\t\t[" << c.getIdCommande() << "] Fournisseur #" << c.getIdFournisseur()
                       << " | Statut: " << libelleStatut(c.getStatut())
                       << " | " << c.getLignes().size() << " ligne(s)"
                       << " | Total: " << c.getMontantTotal() << "\n";
            for (const auto& ligne : c.getLignes())
                std::cout << "\t\t    - " << ligne.getReferenceProduit() << " x" << ligne.getQuantite()
                           << " @ " << ligne.getPrixUnitaire() << "\n";
        }
    }
    attendreEntree();
}

// Changer le statut d'une commande. Marquer une commande "LIVREE" met à jour
// automatiquement le stock (une ligne de commande = une entrée de stock pour
// le produit concerné) — c'est le point d'intégration entre les deux services.
void changerStatutCommande(GestionnaireStock& gestionnaire, GestionnaireCommandes& gc)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tChanger le statut d'une commande\n" << RESET << "\n";

    int idCommande = lireEntier("\t\tId de la commande : ");
    Commande* commande = gc.trouverCommande(idCommande);
    if (commande == nullptr) {
        std::cout << "\n\t\t" << ROUGE << "[!] Commande introuvable." << RESET << "\n";
        attendreEntree();
        return;
    }

    std::cout << "\t\tStatut actuel : " << libelleStatut(commande->getStatut()) << "\n";
    std::cout << "\t\tNouveau statut : 1. EN_COURS  2. LIVREE  3. ANNULEE\n";
    int choix = lireEntier("\t\tVotre choix : ");

    StatutCommande nouveauStatut;
    switch (choix) {
        case 1: nouveauStatut = StatutCommande::EN_COURS; break;
        case 2: nouveauStatut = StatutCommande::LIVREE; break;
        case 3: nouveauStatut = StatutCommande::ANNULEE; break;
        default:
            std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
            attendreEntree();
            return;
    }

    bool etaitDejaLivree = (commande->getStatut() == StatutCommande::LIVREE);
    gc.changerStatut(idCommande, nouveauStatut);

    if (nouveauStatut == StatutCommande::LIVREE && !etaitDejaLivree) {
        int misAJour = 0, ignores = 0;
        for (const auto& ligne : commande->getLignes()) {
            if (gestionnaire.trouverProduit(ligne.getReferenceProduit()) != nullptr) {
                gestionnaire.ajouterStock(ligne.getReferenceProduit(), ligne.getQuantite(),
                                            "Réception commande #" + std::to_string(idCommande));
                misAJour++;
            } else {
                ignores++;
            }
        }
        std::cout << "\n\t\t" << VERT << "[OK] Commande marquée livrée. Stock mis à jour pour "
                   << misAJour << " ligne(s)." << RESET << "\n";
        if (ignores > 0)
            std::cout << "\t\t" << JAUNE << "[!] " << ignores << " ligne(s) ignorée(s) (produit introuvable)." << RESET << "\n";
    } else {
        std::cout << "\n\t\t" << VERT << "[OK] Statut mis à jour." << RESET << "\n";
    }
    attendreEntree();
}

void genererPropositionReapprovisionnement(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tProposition de réapprovisionnement\n" << RESET << "\n";

    auto sousLeSeuil = gestionnaire.produitsSousLeSeuil();
    if (sousLeSeuil.empty()) {
        std::cout << "\t\t" << VERT << "Aucun produit sous son seuil critique actuellement." << RESET << "\n";
        attendreEntree();
        return;
    }

    std::cout << "\t\tProduits à réapprovisionner (quantité suggérée = 2x le seuil) :\n\n";
    for (const auto* p : sousLeSeuil) {
        int suggestion = (p->getSeuilAlerte() * 2) - p->getQuantiteStock();
        std::cout << "\t\t- " << p->getReference() << " (" << p->getNom() << ") : "
                   << "stock=" << p->getQuantiteStock() << ", seuil=" << p->getSeuilAlerte()
                   << " -> commander " << suggestion << " unité(s)\n";
    }
    std::cout << "\n\t\t" << JAUNE << "Utilisez \"Créer une commande\" puis \"Ajouter une ligne\" pour formaliser ces suggestions." << RESET << "\n";
    attendreEntree();
}

} // namespace anonyme

void sous_menu_commandes(GestionnaireStock& gestionnaire, GestionnaireCommandes& gestionnaireCommandes) {
    int choixSousMenu = 0;
    do {
        clear();
        std::cout << "\n\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << CYAN  << "\t\t||              " << GRAS << "COMMANDES & FOURNISSEURS" << RESET << CYAN << "             ||\n" << RESET;
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << BLANC << "\t\t||  1. Lister les fournisseurs                       ||\n";
        std::cout << "\t\t||  2. Ajouter un fournisseur                        ||\n";
        std::cout << "\t\t||  3. Creer une commande                            ||\n";
        std::cout << "\t\t||  4. Ajouter une ligne a une commande              ||\n";
        std::cout << "\t\t||  5. Changer le statut d'une commande              ||\n";
        std::cout << "\t\t||  6. Lister les commandes                          ||\n";
        std::cout << "\t\t||  7. Generer une proposition de reapprovisionnement||\n";
        std::cout << "\t\t||  0. Retour au menu principal                      ||" << RESET << "\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << "\t\t   Votre choix : " << VERT;

        if (!lireChoix(choixSousMenu)) continue;

        try {
            switch (choixSousMenu) {
                case 1: listerFournisseurs(gestionnaireCommandes); break;
                case 2: ajouterFournisseur(gestionnaireCommandes); break;
                case 3: creerCommande(gestionnaireCommandes); break;
                case 4: ajouterLigneCommande(gestionnaireCommandes); break;
                case 5: changerStatutCommande(gestionnaire, gestionnaireCommandes); break;
                case 6: listerCommandes(gestionnaireCommandes); break;
                case 7: genererPropositionReapprovisionnement(gestionnaire); break;
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