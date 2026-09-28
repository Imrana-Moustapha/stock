#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

// Utilitaires de dates partagés (menus, repositories, import CSV).
// Centralise la validation pour qu'aucune date invalide n'entre dans le système,
// quelle que soit la porte d'entrée (saisie, fichier de données, import).
namespace DateUtils
{
    // Construit une date en vérifiant les bornes AVANT de convertir vers les types
    // de <chrono> : month et day sont stockés sur un octet, donc month{257} devient
    // silencieusement le mois 1. Retourne nullopt si la date n'existe pas
    // (mois 13, 31 février, 29 février d'une année non bissextile...).
    inline std::optional<std::chrono::year_month_day> creer(int annee, int mois, int jour)
    {
        if (annee < 1900 || annee > 2200) return std::nullopt;
        if (mois < 1 || mois > 12) return std::nullopt;
        if (jour < 1 || jour > 31) return std::nullopt;

        std::chrono::year_month_day date{std::chrono::year{annee},
                                          std::chrono::month{static_cast<unsigned>(mois)},
                                          std::chrono::day{static_cast<unsigned>(jour)}};
        if (!date.ok()) return std::nullopt;
        return date;
    }

    // Format AAAA-MM-JJ, utilisé dans les fichiers de données et les exports CSV.
    inline std::string formaterIso(const std::chrono::year_month_day& date)
    {
        std::ostringstream oss;
        oss << static_cast<int>(date.year()) << "-"
            << std::setw(2) << std::setfill('0') << static_cast<unsigned>(date.month()) << "-"
            << std::setw(2) << std::setfill('0') << static_cast<unsigned>(date.day());
        return oss.str();
    }

    // Lit une date AAAA-MM-JJ. Retourne nullopt si le format ou la date est invalide.
    inline std::optional<std::chrono::year_month_day> parserIso(const std::string& texte)
    {
        int annee, mois, jour;
        char s1, s2;
        std::istringstream iss(texte);
        if (!(iss >> annee >> s1 >> mois >> s2 >> jour) || s1 != '-' || s2 != '-')
            return std::nullopt;
        char reste;
        if (iss >> reste) return std::nullopt; // caractères parasites après la date
        return creer(annee, mois, jour);
    }

    // Format JJ/MM/AAAA HH:MM:SS en heure locale, pour l'affichage et les exports.
    inline std::string formaterDateHeure(const std::chrono::system_clock::time_point& tp)
    {
        std::time_t temps = std::chrono::system_clock::to_time_t(tp);
        std::tm local{};
#if defined(_WIN32)
        localtime_s(&local, &temps);
#else
        localtime_r(&temps, &local);
#endif
        std::ostringstream oss;
        oss << std::put_time(&local, "%d/%m/%Y %H:%M:%S");
        return oss.str();
    }
}