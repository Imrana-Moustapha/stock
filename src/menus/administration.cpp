#include "ui/Console.hpp"
#include "menus/Menus.hpp"
#include <cstdlib>
#include "services/Gestionnairestock.hpp"
#include "factories/ProduitFactory.hpp"
#include "models/Utilisateur.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>

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
        if (std::cin.eof()) {
            std::cout << "\n" << ROUGE << "[!] Entrée interrompue. Fermeture de l'application." << RESET << "\n";
            std::exit(1);
        }
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

std::vector<std::string> decouperCsv(const std::string& ligne)
{
    std::vector<std::string> champs;
    std::stringstream ss(ligne);
    std::string champ;
    while (std::getline(ss, champ, ','))
        champs.push_back(champ);
    return champs;
}

std::string formaterDateHeure(const std::chrono::system_clock::time_point& tp)
{
    auto tempsC = std::chrono::system_clock::to_time_t(tp);
    std::tm tmLocal{};
#if defined(_WIN32)
    localtime_s(&tmLocal, &tempsC);
#else
    localtime_r(&tempsC, &tmLocal);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmLocal, "%d/%m/%Y %H:%M:%S");
    return oss.str();
}

// Import massif : chaque ligne (après l'en-tête) est traitée indépendamment.
// Une ligne invalide ne bloque pas les suivantes ; un rapport d'erreurs est
// affiché à la fin plutôt que d'interrompre l'import au premier problème.
void importerProduitsCsv(GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tImport massif de produits (CSV)\n" << RESET << "\n";
    std::cout << "\t\tFormat attendu (avec en-tête) :\n";
    std::cout << "\t\t  type,reference,nom,categorie,prixAchat,prixVente,quantite,seuil,datePeremption\n";
    std::cout << "\t\t  type = STANDARD ou PERISSABLE ; datePeremption (AAAA-MM-JJ) uniquement si périssable.\n\n";

    std::string chemin = lireTexte("\t\tChemin du fichier CSV : ");
    std::ifstream fichier(chemin);
    if (!fichier) {
        std::cout << "\n\t\t" << ROUGE << "[!] Impossible d'ouvrir ce fichier." << RESET << "\n";
        pause();
        return;
    }

    std::string ligne;
    std::getline(fichier, ligne); // en-tête ignoré

    int numeroLigne = 1;
    int succes = 0;
    std::vector<std::string> erreurs;

    while (std::getline(fichier, ligne)) {
        numeroLigne++;
        if (ligne.empty()) continue;

        try {
            auto champs = decouperCsv(ligne);
            if (champs.size() < 8)
                throw std::invalid_argument("nombre de colonnes insuffisant");
            for (int i = 1; i <= 7; ++i)
                if (champs[i].empty())
                    throw std::invalid_argument("colonne obligatoire vide (position " + std::to_string(i + 1) + ")");

            TypeProduit type = ProduitFactory::typeDepuisTexte(champs[0]);
            std::optional<std::chrono::year_month_day> date;

            if (type == TypeProduit::PERISSABLE) {
                if (champs.size() < 9) throw std::invalid_argument("date de péremption manquante");
                int a, m, j; char s1, s2;
                std::istringstream iss(champs[8]);
                if (!(iss >> a >> s1 >> m >> s2 >> j) || s1 != '-' || s2 != '-')
                    throw std::invalid_argument("date de péremption invalide");
                date = std::chrono::year{a} / std::chrono::month{static_cast<unsigned>(m)} / std::chrono::day{static_cast<unsigned>(j)};
            }

            gestionnaire.ajouterProduit(ProduitFactory::creerProduit(
                type, champs[1], champs[2], champs[3],
                std::stod(champs[4]), std::stod(champs[5]),
                std::stoi(champs[6]), std::stoi(champs[7]), date));

            succes++;
        } catch (const std::exception& e) {
            erreurs.push_back("Ligne " + std::to_string(numeroLigne) + " : " + e.what());
        }
    }

    std::cout << "\n\t\t" << VERT << "[OK] " << succes << " produit(s) importé(s)." << RESET << "\n";
    if (!erreurs.empty()) {
        std::cout << "\t\t" << ROUGE << erreurs.size() << " ligne(s) rejetée(s) :" << RESET << "\n";
        for (const auto& e : erreurs)
            std::cout << "\t\t  - " << e << "\n";
    }
    pause();
}

void exporterCatalogueCsv(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tExport du catalogue (CSV)\n" << RESET << "\n";

    std::string chemin = lireTexte("\t\tChemin du fichier CSV de sortie : ");
    std::ofstream fichier(chemin, std::ios::trunc);
    if (!fichier) {
        std::cout << "\n\t\t" << ROUGE << "[!] Impossible d'écrire ce fichier." << RESET << "\n";
        pause();
        return;
    }

    fichier << "reference,nom,categorie,prixAchat,prixVente,quantiteStock,seuilAlerte\n";
    for (const auto& p : gestionnaire.getProduits()) {
        fichier << p->getReference() << ',' << p->getNom() << ',' << p->getCategorie() << ','
                << p->getPrixAchat() << ',' << p->getPrixVente() << ','
                << p->getQuantiteStock() << ',' << p->getSeuilAlerte() << "\n";
    }

    std::cout << "\n\t\t" << VERT << "[OK] " << gestionnaire.nombreDeProduits() << " produit(s) exporté(s) vers " << chemin << RESET << "\n";
    pause();
}

void exporterHistoriqueCsv(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tExport de l'historique des mouvements (CSV)\n" << RESET << "\n";

    std::string chemin = lireTexte("\t\tChemin du fichier CSV de sortie : ");
    std::ofstream fichier(chemin, std::ios::trunc);
    if (!fichier) {
        std::cout << "\n\t\t" << ROUGE << "[!] Impossible d'écrire ce fichier." << RESET << "\n";
        pause();
        return;
    }

    fichier << "id,dateHeure,reference,quantite,auteur\n";
    for (const auto& mvt : gestionnaire.getHistorique()) {
        fichier << mvt.getId() << ',' << formaterDateHeure(mvt.getDateHeure()) << ','
                << mvt.getReferenceProduit() << ',' << mvt.getQuantite() << ',' << mvt.getAuteur() << "\n";
    }

    std::cout << "\n\t\t" << VERT << "[OK] " << gestionnaire.getHistorique().size() << " mouvement(s) exporté(s) vers " << chemin << RESET << "\n";
    pause();
}

void menuExport(const GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tExport des données\n" << RESET << "\n";
    std::cout << "\t\t1. Catalogue de produits\n";
    std::cout << "\t\t2. Historique des mouvements\n";
    int choix = lireEntier("\t\tVotre choix : ");

    if (choix == 1) exporterCatalogueCsv(gestionnaire);
    else if (choix == 2) exporterHistoriqueCsv(gestionnaire);
    else { std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n"; pause(); }
}

void configurerSeuils(GestionnaireStock& gestionnaire)
{
    clear();
    std::cout << "\n\n" << CYAN << GRAS << "\t\tConfigurer les seuils d'alerte\n" << RESET << "\n";
    std::cout << "\t\t" << JAUNE << "Cette action applique le même seuil à TOUS les produits existants." << RESET << "\n";
    std::cout << "\t\tPour un seul produit, utilise plutôt Gestion des Produits > Modifier.\n\n";

    int seuil = lireEntier("\t\tNouveau seuil d'alerte (pour tous les produits) : ");
    std::string confirmation = lireTexte("\t\tConfirmer ? (o/n) : ");

    if (confirmation == "o" || confirmation == "O") {
        gestionnaire.definirSeuilPourTous(seuil);
        std::cout << "\n\t\t" << VERT << "[OK] Seuil appliqué à " << gestionnaire.nombreDeProduits() << " produit(s)." << RESET << "\n";
    } else {
        std::cout << "\n\t\t" << JAUNE << "Action annulée." << RESET << "\n";
    }
    pause();
}

// Gestion des utilisateurs : liste maintenue en mémoire pour la durée de la session
// uniquement (aucune persistance sur disque pour l'instant, contrairement aux produits
// qui passent par IRepository). C'est une limitation connue, à combler par un
// UtilisateurRepository dédié le jour où l'authentification sera implémentée.
void gererUtilisateurs()
{
    static std::vector<Utilisateur> utilisateurs;
    static int prochainId = 1;

    int choix = 0;
    do {
        clear();
        std::cout << "\n\n" << CYAN << GRAS << "\t\tGestion des utilisateurs (session courante)\n" << RESET << "\n";
        std::cout << "\t\t" << JAUNE << "Note : liste non persistée, réinitialisée à chaque redémarrage." << RESET << "\n\n";
        std::cout << "\t\t1. Lister les utilisateurs\n";
        std::cout << "\t\t2. Ajouter un utilisateur\n";
        std::cout << "\t\t0. Retour\n";
        choix = lireEntier("\t\tVotre choix : ");

        if (choix == 1) {
            clear();
            std::cout << "\n\n" << CYAN << GRAS << "\t\tUtilisateurs (" << utilisateurs.size() << ")\n" << RESET << "\n";
            if (utilisateurs.empty()) {
                std::cout << "\t\t" << JAUNE << "Aucun utilisateur créé pour l'instant." << RESET << "\n";
            } else {
                for (const auto& u : utilisateurs) {
                    std::string role = (u.getRole() == Role::ADMIN) ? "ADMIN"
                                      : (u.getRole() == Role::GESTIONNAIRE) ? "GESTIONNAIRE" : "MAGASINIER";
                    std::cout << "\t\t- " << u.getUsername() << " (" << role << ")\n";
                }
            }
            pause();
        } else if (choix == 2) {
            clear();
            std::cout << "\n\n" << CYAN << GRAS << "\t\tAjouter un utilisateur\n" << RESET << "\n";
            std::string nom = lireTexte("\t\tNom d'utilisateur : ");
            std::cout << "\t\tRôle : 1. ADMIN  2. GESTIONNAIRE  3. MAGASINIER\n";
            int r = lireEntier("\t\tVotre choix : ");
            Role role = (r == 1) ? Role::ADMIN : (r == 2) ? Role::GESTIONNAIRE : Role::MAGASINIER;

            // Mot de passe non géré ici (pas d'authentification implémentée) :
            // un hash vide est un espace réservé, jamais utilisable en l'état.
            utilisateurs.emplace_back(prochainId++, nom, "", role);
            std::cout << "\n\t\t" << VERT << "[OK] Utilisateur ajouté." << RESET << "\n";
            pause();
        } else if (choix != 0) {
            std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
            pause();
        }
    } while (choix != 0);
}

} // namespace anonyme

void sous_menu_administration(GestionnaireStock& gestionnaire) {
    int choixSousMenu = 0;
    do {
        clear();
        std::cout << "\n\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << CYAN  << "\t\t||         " << GRAS << "ADMINISTRATION & CONFIGURATION" << RESET << CYAN << "          ||\n" << RESET;
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << BLANC << "\t\t||  1. Import massif de produits (CSV)               ||\n";
        std::cout << "\t\t||  2. Export des donnees (Catalogue / Historique)   ||\n";
        std::cout << "\t\t||  3. Configurer les seuils d'alerte                ||\n";
        std::cout << "\t\t||  4. Gestion des utilisateurs                      ||\n";
        std::cout << "\t\t||  0. Retour au menu principal                      ||" << RESET << "\n";
        std::cout << JAUNE << "\t\t=======================================================\n" << RESET;
        std::cout << "\t\t   Votre choix : " << VERT;

        if (!lireChoix(choixSousMenu)) continue;

        try {
            switch (choixSousMenu) {
                case 1: importerProduitsCsv(gestionnaire); break;
                case 2: menuExport(gestionnaire); break;
                case 3: configurerSeuils(gestionnaire); break;
                case 4: gererUtilisateurs(); break;
                case 0: break;
                default: 
                    std::cout << "\n\t\t" << ROUGE << "[!] Choix invalide." << RESET << "\n";
                    std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
                    viderBuffer();
                    std::cin.get();
            }
        } catch (const std::exception& e) {
            std::cout << "\n\t\t" << ROUGE << "[!] Erreur : " << e.what() << RESET << "\n";
            std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
            std::cin.get();
        }
    } while (choixSousMenu != 0);
}