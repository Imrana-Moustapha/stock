#pragma once

#include <iostream>
#include <limits>
#include <string_view>

// Déclaration anticipée : header.hpp n'a besoin que d'une référence,
// pas de la définition complète de la classe (évite un couplage inutile
// et accélère la compilation des fichiers qui n'utilisent pas GestionnaireStock).
class GestionnaireStock;

// Codes de couleur ANSI regroupés dans un namespace typé plutôt qu'en macros #define :
// évite les collisions de noms globaux et bénéficie du typage std::string_view.
namespace Couleur
{
    inline constexpr std::string_view RESET  = "\033[0m";
    inline constexpr std::string_view CYAN   = "\033[36m";
    inline constexpr std::string_view JAUNE  = "\033[33m";
    inline constexpr std::string_view VERT   = "\033[32m";
    inline constexpr std::string_view ROUGE  = "\033[31m";
    inline constexpr std::string_view BLANC  = "\033[37m";
    inline constexpr std::string_view GRAS   = "\033[1m";
}

// Prototypes des fonctions utilitaires et des menus
void viderBuffer();
void clear();

// Lit un choix de menu dans choix. Retourne false en cas de saisie invalide
// (et affiche alors le message d'erreur standard) : centralise ce qui était
// dupliqué dans chaque sous-menu.
bool lireChoix(int& choix);

void menu_principal(GestionnaireStock& gestionnaire);
void sous_menu_produits(GestionnaireStock& gestionnaire);
void sous_menu_mouvements();
void sous_menu_recherche();
void sous_menu_commandes();
void sous_menu_statistiques();
void sous_menu_administration();