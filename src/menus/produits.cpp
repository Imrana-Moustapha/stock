#include "ui/Console.hpp"
#include "menus/Menus.hpp"
#include "services/Gestionnairestock.hpp"
#include "exceptions/Exceptions.hpp"
#include "factories/ProduitFactory.hpp"
#include <limits>

using namespace Couleur;

namespace
{

    // Lit une ligne de texte non vide. viderBuffer() élimine le '\n' laissé par
    // le >> précédent (celui du choix de menu) avant le premier getline.
    std::string lireTexte(const std::string &invite)
    {
        std::string valeur;
        do
        {
            std::cout << invite;
            std::getline(std::cin, valeur);
        } while (valeur.empty());
        return valeur;
    }

    double lireDouble(const std::string &invite)
    {
        double valeur;
        while (true)
        {
            std::cout << invite;
            if (std::cin >> valeur)
                break;
            std::cout << ROUGE << "[!] Veuillez entrer un nombre valide." << RESET << "\n";
            viderBuffer();
        }
        viderBuffer(); // laisse la ligne propre pour un éventuel getline suivant
        return valeur;
    }

    int lireEntier(const std::string &invite)
    {
        int valeur;
        while (true)
        {
            std::cout << invite;
            if (std::cin >> valeur)
                break;
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

    void afficherListeProduits(const GestionnaireStock &gestionnaire)
    {
        clear();
        std::cout << "\n\n"
                  << CYAN << GRAS << "\t\tListe des produits (" << gestionnaire.nombreDeProduits() << ")\n"
                  << RESET << "\n";

        if (gestionnaire.nombreDeProduits() == 0)
        {
            std::cout << "\t\t" << JAUNE << "Aucun produit enregistré pour le moment." << RESET << "\n";
        }
        else
        {
            for (const auto &produit : gestionnaire.getProduits())
            {
                std::cout << "\t\t";
                if (produit->estSousLeSeuil())
                    std::cout << ROUGE;
                produit->afficher();
                std::cout << RESET;
            }
        }
        pause();
    }

    void ajouterProduitInteractif(GestionnaireStock &gestionnaire)
    {
        clear();
        std::cout << "\n\n"
                  << CYAN << GRAS << "\t\tAjouter un produit\n"
                  << RESET << "\n";

        std::cout << "\t\tType de produit :\n";
        std::cout << "\t\t  1. Standard\n";
        std::cout << "\t\t  2. Périssable\n";
        int typeChoisi = lireEntier("\t\tVotre choix : ");

        if (typeChoisi != 1 && typeChoisi != 2)
        {
            std::cout << "\n\t\t" << ROUGE << "[!] Type invalide, ajout annulé." << RESET << "\n";
            pause();
            return;
        }

        std::string reference = lireTexte("\t\tRéférence : ");

        if (gestionnaire.trouverProduit(reference) != nullptr)
        {
            std::cout << "\n\t\t" << ROUGE << "[!] Un produit avec cette référence existe déjà." << RESET << "\n";
            pause();
            return;
        }

        std::string nom = lireTexte("\t\tNom : ");
        std::string categorie = lireTexte("\t\tCatégorie : ");
        double prixAchat = lireDouble("\t\tPrix d'achat : ");
        double prixVente = lireDouble("\t\tPrix de vente : ");
        int quantite = lireEntier("\t\tQuantité initiale en stock : ");
        int seuil = lireEntier("\t\tSeuil d'alerte : ");

        if (typeChoisi == 1)
        {
            gestionnaire.ajouterProduit(ProduitFactory::creerProduit(
                TypeProduit::STANDARD, reference, nom, categorie, prixAchat, prixVente, quantite, seuil));
        }
        else
        {
            int annee = lireEntier("\t\tDate de péremption - année (AAAA) : ");
            int mois = lireEntier("\t\tDate de péremption - mois (1-12) : ");
            int jour = lireEntier("\t\tDate de péremption - jour (1-31) : ");

            auto date = std::chrono::year{annee} / std::chrono::month{static_cast<unsigned>(mois)} / std::chrono::day{static_cast<unsigned>(jour)};

            gestionnaire.ajouterProduit(ProduitFactory::creerProduit(
                TypeProduit::PERISSABLE, reference, nom, categorie, prixAchat, prixVente, quantite, seuil, date));
        }

        std::cout << "\n\t\t" << VERT << "[OK] Produit '" << nom << "' ajouté avec succès." << RESET << "\n";
        pause();
    }

    void modifierProduitInteractif(GestionnaireStock &gestionnaire)
    {
        clear();
        std::cout << "\n\n"
                  << CYAN << GRAS << "\t\tModifier un produit\n"
                  << RESET << "\n";

        std::string reference = lireTexte("\t\tRéférence du produit à modifier : ");
        Produit *produit = gestionnaire.trouverProduit(reference);

        if (produit == nullptr)
        {
            std::cout << "\n\t\t" << ROUGE << "[!] Aucun produit avec cette référence." << RESET << "\n";
            pause();
            return;
        }

        auto *perissable = dynamic_cast<ProduitPerissable *>(produit);

        bool continuer = true;
        while (continuer)
        {
            clear();
            std::cout << "\n\n"
                      << CYAN << GRAS << "\t\tModifier : " << produit->getNom() << RESET << "\n\n";
            produit->afficher();
            std::cout << "\n\t\tChamp à modifier :\n";
            std::cout << "\t\t  1. Nom (" << produit->getNom() << ")\n";
            std::cout << "\t\t  2. Catégorie (" << produit->getCategorie() << ")\n";
            std::cout << "\t\t  3. Prix d'achat (" << produit->getPrixAchat() << ")\n";
            std::cout << "\t\t  4. Prix de vente (" << produit->getPrixVente() << ")\n";
            std::cout << "\t\t  5. Seuil d'alerte (" << produit->getSeuilAlerte() << ")\n";
            if (perissable != nullptr)
                std::cout << "\t\t  6. Date de péremption\n";
            std::cout << "\t\t  0. Terminer\n";

            int choix = lireEntier("\t\tVotre choix : ");

            switch (choix)
            {
            case 1:
                produit->setNom(lireTexte("\t\tNouveau nom : "));
                break;
            case 2:
                produit->setCategorie(lireTexte("\t\tNouvelle catégorie : "));
                break;
            case 3:
                produit->setPrixAchat(lireDouble("\t\tNouveau prix d'achat : "));
                break;
            case 4:
                produit->setPrixVente(lireDouble("\t\tNouveau prix de vente : "));
                break;
            case 5:
                produit->setSeuilAlerte(lireEntier("\t\tNouveau seuil d'alerte : "));
                break;
            case 6:
                if (perissable != nullptr)
                {
                    int annee = lireEntier("\t\tNouvelle date - année (AAAA) : ");
                    int mois = lireEntier("\t\tNouvelle date - mois (1-12) : ");
                    int jour = lireEntier("\t\tNouvelle date - jour (1-31) : ");
                    perissable->setDatePeremption(
                        std::chrono::year{annee} / std::chrono::month{static_cast<unsigned>(mois)} / std::chrono::day{static_cast<unsigned>(jour)});
                }
                break;
            case 0:
                continuer = false;
                break;
            default:
                std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
            }
        }

        std::cout << "\n\t\t" << VERT << "[OK] Produit mis à jour." << RESET << "\n";
        pause();
    }

    void supprimerProduitInteractif(GestionnaireStock &gestionnaire)
    {
        clear();
        std::cout << "\n\n"
                  << CYAN << GRAS << "\t\tSupprimer un produit\n"
                  << RESET << "\n";

        std::string reference = lireTexte("\t\tRéférence du produit à supprimer : ");
        Produit *produit = gestionnaire.trouverProduit(reference);

        if (produit == nullptr)
        {
            std::cout << "\n\t\t" << ROUGE << "[!] Aucun produit avec cette référence." << RESET << "\n";
            pause();
            return;
        }

        std::cout << "\n\t\t";
        produit->afficher();
        std::string confirmation = lireTexte("\n\t\tConfirmer la suppression ? (o/n) : ");

        if (confirmation == "o" || confirmation == "O")
        {
            gestionnaire.supprimerProduit(reference); // ne peut pas lever ici : on vient de vérifier l'existence
            std::cout << "\n\t\t" << VERT << "[OK] Produit supprimé." << RESET << "\n";
        }
        else
        {
            std::cout << "\n\t\t" << JAUNE << "Suppression annulée." << RESET << "\n";
        }
        pause();
    }

} // namespace anonyme

void sous_menu_produits(GestionnaireStock &gestionnaire)
{
    int choixSousMenu = 0;
    do
    {
        clear();
        std::cout << "\n\n";
        std::cout << JAUNE << "\t\t=======================================================\n"
                  << RESET;
        std::cout << CYAN << "\t\t||              " << GRAS << "GESTION DES PRODUITS" << RESET << CYAN << "                 ||\n"
                  << RESET;
        std::cout << JAUNE << "\t\t=======================================================\n"
                  << RESET;
        std::cout << BLANC << "\t\t||  1. Lister tous les produits                      ||\n";
        std::cout << "\t\t||  2. Ajouter un produit (Standard, Perissable, etc.)||\n";
        std::cout << "\t\t||  3. Modifier un produit                           ||\n";
        std::cout << "\t\t||  4. Supprimer un produit                          ||\n";
        std::cout << "\t\t||  0. Retour au menu principal                      ||" << RESET << "\n";
        std::cout << JAUNE << "\t\t=======================================================\n"
                  << RESET;
        std::cout << "\t\t   Votre choix : " << VERT;

        if (!lireChoix(choixSousMenu))
            continue;

        try
        {
            switch (choixSousMenu)
            {
            case 1:
                afficherListeProduits(gestionnaire);
                break;
            case 2:
                ajouterProduitInteractif(gestionnaire);
                break;
            case 3:
                modifierProduitInteractif(gestionnaire);
                break;
            case 4:
                supprimerProduitInteractif(gestionnaire);
                break;
            case 0:
                break;
            default:
                std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
                std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
                viderBuffer();
                std::cin.get();
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "\n\t\t" << ROUGE << "[!] Erreur : " << e.what() << RESET << "\n";
            std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
            std::cin.get();
        }
    } while (choixSousMenu != 0);
}