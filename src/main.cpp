#include <iostream>
#include "menus/Menus.hpp"
#include "ui/Console.hpp"
#include "ui/Saisie.hpp"
#include "services/GestionnaireStock.hpp"
#include "services/GestionnaireCommandes.hpp"
#include "repositories/FichierTexteRepository.hpp"
#include "repositories/FichierTexteMouvementRepository.hpp"
#include "observers/Logger.hpp"

int main() {
    GestionnaireStock gestionnaire(
        std::make_unique<FichierTexteRepository>("data/stock.txt"),
        std::make_unique<FichierTexteMouvementRepository>("data/mouvements.txt"));

    // Si les données ne peuvent pas être lues (fichier corrompu, doublons...), on s'arrête
    // AVANT toute opération : continuer avec un stock vide ferait écraser le fichier
    // existant par la sauvegarde automatique, et ferait perdre les données.
    try {
        gestionnaire.charger();
    } catch (const std::exception& e) {
        std::cerr << "Impossible de charger les données : " << e.what() << "\n"
                  << "Le programme s'arrête pour ne pas écraser vos fichiers (data/stock.txt, data/mouvements.txt).\n"
                  << "Corrigez ou déplacez le fichier concerné, puis relancez.\n";
        return 1;
    }

    GestionnaireCommandes gestionnaireCommandes;

    Logger logger("data/alertes.log");
    gestionnaire.ajouterObservateur(&logger);

    int code = 0;
    try {
        menu_principal(gestionnaire, gestionnaireCommandes);
    } catch (const EntreeInterrompue&) {
        // Entrée standard fermée (Ctrl+D, fichier de saisie épuisé) : on quitte proprement.
        std::cout << "\n" << Couleur::ROUGE << "[!] Entrée interrompue. Fermeture de l'application."
                  << Couleur::RESET << "\n";
        code = 1;
    }

    gestionnaire.sauvegarder();
    return code;
}