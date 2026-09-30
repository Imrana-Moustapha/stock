#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "menus/Menus.hpp"
#include "ui/Console.hpp"
#include "ui/Saisie.hpp"
#include "services/GestionnaireStock.hpp"
#include "services/GestionnaireCommandes.hpp"
#include "repositories/FichierTexteRepository.hpp"
#include "repositories/FichierTexteMouvementRepository.hpp"
#include "repositories/FichierTexteFournisseurRepository.hpp"
#include "repositories/FichierTexteCommandeRepository.hpp"
#include "observers/Logger.hpp"

// Fonction pour charger un fichier .env et injecter les variables dans l'environnement système
void loadEnv(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        // Optionnel : si le fichier n'est pas critique, on peut juste l'ignorer ou afficher un avertissement
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignorer les lignes vides ou les commentaires
        if (line.empty() || line[0] == '#') continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            #ifdef _WIN32
                _putenv_s(key.c_str(), value.c_str());
            #else
                setenv(key.c_str(), value.c_str(), 1);
            #endif
        }
    }
}

int main() {
    // Charger le fichier .env dès le démarrage du programme
    loadEnv(".env");

    // Affichage des variables du .env dans la console
    std::cout << "=======================================\n";
    std::cout << "      CONFIGURATION CHARGEE DU .env    \n";
    std::cout << "=======================================\n";
    
    if (const char* dbUrl = std::getenv("DATABASE_URL")) {
        std::cout << "DATABASE_URL : " << dbUrl << "\n";
    }
    if (const char* port = std::getenv("PORT")) {
        std::cout << "PORT         : " << port << "\n";
    }
    if (const char* apiKey = std::getenv("API_KEY")) {
        std::cout << "API_KEY      : " << apiKey << "\n";
    }
    std::cout << "=======================================\n\n";

    // Gestion du répertoire de données
    std::string dataDir = "data/";
    if (const char* envPath = std::getenv("DATA_PATH")) {
        dataDir = envPath;
    }

    GestionnaireStock gestionnaire(
        std::make_unique<FichierTexteRepository>(dataDir + "stock.txt"),
        std::make_unique<FichierTexteMouvementRepository>(dataDir + "mouvements.txt"));

    GestionnaireCommandes gestionnaireCommandes(
        std::make_unique<FichierTexteFournisseurRepository>(dataDir + "fournisseurs.txt"),
        std::make_unique<FichierTexteCommandeRepository>(dataDir + "commandes.txt", dataDir + "commande_lignes.txt"));

    try {
        gestionnaire.charger();
        gestionnaireCommandes.charger();
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
        menu_principal(gestionnaire, gestionnaireCommandes);
    } catch (const EntreeInterrompue&) {
        std::cout << "\n" << Couleur::ROUGE << "[!] Entrée interrompue. Fermeture de l'application."
                  << Couleur::RESET << "\n";
        code = 1;
    }

    gestionnaire.sauvegarder();
    return code;
}