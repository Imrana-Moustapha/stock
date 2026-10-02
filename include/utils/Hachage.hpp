#pragma once

#include <random>
#include <sstream>
#include <string>

#include "utils/Sha256.hpp"

// Hachage de mots de passe avec sel aléatoire, construit sur SHA-256.
// Le format stocké est "sel$hash" : le sel est nécessaire pour vérifier un mot de
// passe plus tard, donc il doit être conservé à côté du hash (il n'a pas besoin
// d'être secret, seul le mot de passe doit rester inconnu).
//
// Limite assumée : SHA-256 seul est rapide à calculer, donc moins résistant qu'un
// algorithme conçu pour les mots de passe (bcrypt, Argon2) face à une attaque par
// force brute hors ligne sur une fuite de la base. Suffisant pour la portée de ce
// projet (pas de dépendance externe comme OpenSSL à installer) ; à remplacer par
// une bibliothèque dédiée si ce système devait gérer des comptes réels en production.
namespace Hachage
{
    inline std::string genererSel(int longueurOctets = 16)
    {
        std::random_device source;
        std::ostringstream oss;
        for (int i = 0; i < longueurOctets; ++i)
            oss << std::hex << (source() % 256);
        return oss.str();
    }

    // "sel$hash", prêt à être stocké tel quel (dans Utilisateur::passwordHash, par exemple).
    inline std::string hacherMotDePasse(const std::string& motDePasse, const std::string& sel)
    {
        return sel + "$" + Sha256::hacher(sel + motDePasse);
    }

    inline std::string hacherNouveauMotDePasse(const std::string& motDePasse)
    {
        return hacherMotDePasse(motDePasse, genererSel());
    }

    // Recalcule le hash avec le sel stocké et compare : ne révèle jamais le mot de
    // passe en clair, seulement si la tentative correspond.
    inline bool verifierMotDePasse(const std::string& motDePasseTente, const std::string& hashStocke)
    {
        auto separateur = hashStocke.find('$');
        if (separateur == std::string::npos) return false;

        std::string sel = hashStocke.substr(0, separateur);
        return hacherMotDePasse(motDePasseTente, sel) == hashStocke;
    }
}