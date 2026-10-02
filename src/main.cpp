#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "menus/Menus.hpp"
#include "ui/Console.hpp"
#include "ui/Saisie.hpp"
#include "models/Utilisateur.hpp"
#include "services/GestionnaireStock.hpp"
#include "services/GestionnaireCommandes.hpp"
#include "services/GestionnaireUtilisateurs.hpp"
#include "repositories/FichierTexteRepository.hpp"
#include "repositories/FichierTexteMouvementRepository.hpp"
#include "repositories/FichierTexteFournisseurRepository.hpp"
#include "repositories/FichierTexteCommandeRepository.hpp"
#include "repositories/FichierTexteUtilisateurRepository.hpp"
#include "observers/Logger.hpp"

using namespace Couleur;

namespace {

std::string rogner(const std::string& texte)
{
    const char* espaces = " \t\r\n";
    auto debut = texte.find_first_not_of(espaces);
    if (debut == std::string::npos) return "";
    auto fin = texte.find_last_not_of(espaces);
    std::string resultat = texte.substr(debut, fin - debut + 1);

    // Retire des guillemets englobants : VAR="valeur" ou VAR='valeur'
    if (resultat.size() >= 2 &&
        ((resultat.front() == '"' && resultat.back() == '"') ||
         (resultat.front() == '\'' && resultat.back() == '\'')))
        resultat = resultat.substr(1, resultat.size() - 2);

    return resultat;
}

// Charge un fichier .env dans l'environnement du processus. Ne sert ici qu'à de la
// configuration (chemin des données) et à l'amorçage du tout premier compte admin —
// jamais à stocker durablement des identifiants : voir GestionnaireUtilisateurs,
// qui persiste les comptes avec un mot de passe haché, pas en clair dans un fichier texte.
void chargerEnv(const std::string& chemin)
{
    std::ifstream fichier(chemin);
    if (!fichier) return; // Fichier absent : pas une erreur, juste pas de config à charger.

    std::string ligne;
    while (std::getline(fichier, ligne)) {
        std::string ligneRognee = rogner(ligne);
        if (ligneRognee.empty() || ligneRognee[0] == '#') continue;

        auto separateur = ligneRognee.find('=');
        if (separateur == std::string::npos) continue;

        std::string cle = rogner(ligneRognee.substr(0, separateur));
        std::string valeur = rogner(ligneRognee.substr(separateur + 1));
        if (cle.empty()) continue;

#if defined(_WIN32)
        _putenv_s(cle.c_str(), valeur.c_str());
#else
        setenv(cle.c_str(), valeur.c_str(), 1);
#endif
    }
}

std::string env(const char* nom, const std::string& defaut = "")
{
    const char* valeur = std::getenv(nom);
    return (valeur != nullptr) ? std::string(valeur) : defaut;
}

// S'il n'existe encore aucun utilisateur, crée le premier compte ADMIN : soit à
// partir des variables d'amorçage du .env (pratique pour Docker ou un déploiement
// automatisé), soit en le demandant interactivement sinon. Dans les deux cas, seul
// le mot de passe haché est écrit sur disque — jamais le mot de passe en clair.
void amorcerPremierAdmin(GestionnaireUtilisateurs& gestionnaireUtilisateurs)
{
    if (!gestionnaireUtilisateurs.estVide()) return;

    std::cout << JAUNE << "Aucun compte utilisateur trouvé : création du premier compte administrateur." << RESET << "\n";

    std::string nomBootstrap = env("ADMIN_BOOTSTRAP_USERNAME");
    std::string mdpBootstrap = env("ADMIN_BOOTSTRAP_PASSWORD");

    if (!nomBootstrap.empty() && !mdpBootstrap.empty()) {
        gestionnaireUtilisateurs.ajouterUtilisateur(nomBootstrap, mdpBootstrap, Role::ADMIN);
        std::cout << VERT << "[OK] Compte '" << nomBootstrap << "' créé depuis .env (ADMIN_BOOTSTRAP_*)." << RESET << "\n\n";
        return;
    }

    std::cout << "Aucun ADMIN_BOOTSTRAP_USERNAME/PASSWORD dans .env : création manuelle.\n";
    while (true) {
        std::string nom = lireTexte("Nom d'utilisateur admin : ");
        std::string mdp = lireMotDePasse("Mot de passe (4 caractères minimum) : ");
        try {
            gestionnaireUtilisateurs.ajouterUtilisateur(nom, mdp, Role::ADMIN);
            std::cout << VERT << "[OK] Compte '" << nom << "' créé." << RESET << "\n\n";
            return;
        } catch (const std::exception& e) {
            std::cout << ROUGE << "[!] " << e.what() << RESET << "\n";
        }
    }
}

// Boucle de connexion : redemande en cas d'échec, jusqu'à succès ou fin de flux
// (EntreeInterrompue, propagée telle quelle jusqu'à main()).
//
// Retourne une COPIE, volontairement : GestionnaireUtilisateurs::authentifier()
// renvoie un pointeur vers un élément de son std::vector interne, qui peut se
// réallouer si un utilisateur est ajouté pendant la session (ex. depuis le menu
// Administration). Garder une référence vers cet élément exposerait une référence
// pendante dès la première réallocation ; Utilisateur est une petite valeur sans
// ressource à gérer, la copier ne coûte rien et règle le problème à la racine.
Utilisateur connecter(GestionnaireUtilisateurs& gestionnaireUtilisateurs)
{
    while (true) {
        std::cout << "\n" << CYAN << GRAS << "Connexion" << RESET << "\n";
        std::string nom = lireTexte("Nom d'utilisateur : ");
        std::string mdp = lireMotDePasse("Mot de passe : ");

        const Utilisateur* u = gestionnaireUtilisateurs.authentifier(nom, mdp);
        if (u != nullptr) return *u;

        std::cout << ROUGE << "[!] Identifiant ou mot de passe incorrect." << RESET << "\n";
    }
}

} // namespace anonyme

int main() {
    chargerEnv(".env");

    std::string dataDir = env("DATA_PATH", "data/");
    if (!dataDir.empty() && dataDir.back() != '/')
        dataDir += '/';

    GestionnaireStock gestionnaire(
        std::make_unique<FichierTexteRepository>(dataDir + "stock.txt"),
        std::make_unique<FichierTexteMouvementRepository>(dataDir + "mouvements.txt"));

    GestionnaireCommandes gestionnaireCommandes(
        std::make_unique<FichierTexteFournisseurRepository>(dataDir + "fournisseurs.txt"),
        std::make_unique<FichierTexteCommandeRepository>(dataDir + "commandes.txt", dataDir + "commande_lignes.txt"));

    GestionnaireUtilisateurs gestionnaireUtilisateurs(
        std::make_unique<FichierTexteUtilisateurRepository>(dataDir + "utilisateurs.txt"));

    // Si les données ne peuvent pas être lues (fichier corrompu, doublons...), on s'arrête
    // AVANT toute opération : continuer avec un état vide ferait écraser les fichiers
    // existants par la sauvegarde automatique, et ferait perdre les données.
    try {
        gestionnaire.charger();
        gestionnaireCommandes.charger();
        gestionnaireUtilisateurs.charger();
    } catch (const std::exception& e) {
        std::cerr << "Impossible de charger les données : " << e.what() << "\n"
                  << "Le programme s'arrête pour ne pas écraser vos fichiers dans " << dataDir << ".\n"
                  << "Corrigez ou déplacez le fichier concerné, puis relancez.\n";
        return 1;
    }

    Logger logger(dataDir + "alertes.log");
    gestionnaire.ajouterObservateur(&logger);

    int code = 0;
    try {
        amorcerPremierAdmin(gestionnaireUtilisateurs);
        Utilisateur utilisateurConnecte = connecter(gestionnaireUtilisateurs);

        menu_principal(gestionnaire, gestionnaireCommandes, gestionnaireUtilisateurs, utilisateurConnecte);
    } catch (const EntreeInterrompue&) {
        std::cout << "\n" << Couleur::ROUGE << "[!] Entrée interrompue. Fermeture de l'application."
                  << Couleur::RESET << "\n";
        code = 1;
    }

    gestionnaire.sauvegarder();
    return code;
}