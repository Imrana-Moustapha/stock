#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>

#include "models/Utilisateur.hpp"
#include "repositories/IUtilisateurRepository.hpp"
#include "exceptions/Exceptions.hpp"

// Service dédié à l'authentification et aux comptes utilisateurs. Même principe
// que les autres services : repository injecté, sauvegarde automatique après
// chaque opération qui modifie l'état.
class GestionnaireUtilisateurs
{
    private:
        std::vector<Utilisateur> utilisateurs;
        int prochainId = 1;
        std::unique_ptr<IUtilisateurRepository> repository;

    public:
        explicit GestionnaireUtilisateurs(std::unique_ptr<IUtilisateurRepository> repo)
            : repository(std::move(repo)) {}

        GestionnaireUtilisateurs(const GestionnaireUtilisateurs&) = delete;
        GestionnaireUtilisateurs& operator=(const GestionnaireUtilisateurs&) = delete;
        GestionnaireUtilisateurs(GestionnaireUtilisateurs&&) = default;
        GestionnaireUtilisateurs& operator=(GestionnaireUtilisateurs&&) = default;

        void charger()
        {
            utilisateurs = repository->charger();
            prochainId = 1;
            for (const auto& u : utilisateurs)
                prochainId = std::max(prochainId, u.getId() + 1);
        }

        void sauvegarder() const { repository->sauvegarder(utilisateurs); }

        bool estVide() const { return utilisateurs.empty(); }
        const std::vector<Utilisateur>& getUtilisateurs() const { return utilisateurs; }

        const Utilisateur* trouverParUsername(const std::string& username) const
        {
            auto it = std::find_if(utilisateurs.begin(), utilisateurs.end(),
                [&username](const Utilisateur& u) { return u.getUsername() == username; });
            return (it != utilisateurs.end()) ? &(*it) : nullptr;
        }

        // Le mot de passe est pris EN CLAIR ici, haché avant d'être stocké — jamais
        // l'inverse. C'est la seule fonction de ce service qui voit un mot de passe lisible.
        int ajouterUtilisateur(const std::string& username, const std::string& motDePasseEnClair, Role role)
        {
            if (username.empty())
                throw std::invalid_argument("Le nom d'utilisateur ne peut pas être vide.");
            if (motDePasseEnClair.size() < 4)
                throw std::invalid_argument("Le mot de passe doit contenir au moins 4 caractères.");
            if (trouverParUsername(username) != nullptr)
                throw std::invalid_argument("Le nom d'utilisateur '" + username + "' existe déjà.");

            int id = prochainId++;
            utilisateurs.emplace_back(id, username, Hachage::hacherNouveauMotDePasse(motDePasseEnClair), role);
            sauvegarder();
            return id;
        }

        // Retourne l'utilisateur si le couple identifiant/mot de passe est correct,
        // nullptr sinon — sans jamais préciser lequel des deux est fautif (éviter de
        // révéler si un nom d'utilisateur existe).
        const Utilisateur* authentifier(const std::string& username, const std::string& motDePasse) const
        {
            const Utilisateur* u = trouverParUsername(username);
            if (u == nullptr || !u->verifierMotDePasse(motDePasse))
                return nullptr;
            return u;
        }
};