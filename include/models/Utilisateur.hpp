#pragma once

#include <string>
#include <utility>
#include <stdexcept>

#include "utils/Hachage.hpp"

enum class Role
{
    ADMIN,
    GESTIONNAIRE,
    MAGASINIER
};

inline const char* libelle(Role role)
{
    switch (role) {
        case Role::ADMIN:        return "ADMIN";
        case Role::GESTIONNAIRE: return "GESTIONNAIRE";
        case Role::MAGASINIER:   return "MAGASINIER";
    }
    return "?";
}

inline Role roleDepuisTexte(const std::string& texte)
{
    if (texte == "ADMIN")        return Role::ADMIN;
    if (texte == "GESTIONNAIRE") return Role::GESTIONNAIRE;
    if (texte == "MAGASINIER")   return Role::MAGASINIER;
    throw std::invalid_argument("Rôle inconnu : " + texte);
}

class Utilisateur
{
    private:
        int id;
        std::string username;
        std::string passwordHash;   // "sel$hash" — voir utils/Hachage.hpp ; jamais le mot de passe en clair
        Role role;

    public:
        // Constructeur réservé au repository : reconstruit un utilisateur à partir
        // d'un hash déjà calculé, sans jamais repasser par le mot de passe en clair.
        Utilisateur(int idU, std::string user, std::string pwdHashDejaCalcule, Role r)
            : id(idU), username(std::move(user)), passwordHash(std::move(pwdHashDejaCalcule)), role(r) {}

        int getId() const { return id; }
        const std::string& getUsername() const { return username; }
        const std::string& getPasswordHash() const { return passwordHash; }
        Role getRole() const { return role; }

        // Compare un mot de passe EN CLAIR (jamais un hash) au hash stocké.
        bool verifierMotDePasse(const std::string& motDePasseTente) const
        {
            return Hachage::verifierMotDePasse(motDePasseTente, passwordHash);
        }

        void changerMotDePasse(const std::string& nouveauMotDePasse)
        {
            passwordHash = Hachage::hacherNouveauMotDePasse(nouveauMotDePasse);
        }
};