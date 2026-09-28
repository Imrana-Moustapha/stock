#pragma once

#include <string>
#include <chrono>

// Levée quand l'entrée standard est fermée (Ctrl+D, fichier épuisé...) : plus aucune
// saisie ne viendra, continuer à en redemander bouclerait indéfiniment.
// Volontairement PAS dérivée de std::exception : les blocs catch (const std::exception&)
// des menus afficheraient une erreur puis reboucleraient. Seul main() l'attrape.
struct EntreeInterrompue {};

// Toutes les fonctions lisent des lignes entières sur std::cin (jamais de mélange
// operator>> / getline) : il ne reste donc jamais de '\n' en attente d'une lecture
// précédente. Les erreurs de saisie sont signalées à l'utilisateur, qui est
// re-sollicité ; seule la fin de flux lève EntreeInterrompue.

std::string lireLigne();

// Texte non vide (espaces de début/fin retirés).
std::string lireTexte(const std::string& invite);

int lireEntier(const std::string& invite);
int lireEntierNonNegatif(const std::string& invite);          // >= 0
int lireEntierStrictementPositif(const std::string& invite);  // > 0

// Accepte la virgule ou le point comme séparateur décimal (« 1,5 » ou « 1.5 »).
double lireDouble(const std::string& invite);
double lireDoubleNonNegatif(const std::string& invite);       // >= 0

// Redemande année/mois/jour tant que la date n'existe pas.
std::chrono::year_month_day lireDate(const std::string& libelle);

// Choix de menu. Retourne false (après avoir signalé l'erreur) si la saisie n'est pas un entier.
bool lireChoix(int& choix);

// « Appuyez sur Entrée pour continuer... » (ne s'appelle pas pause : ce nom est déjà pris par POSIX)
void attendreEntree();