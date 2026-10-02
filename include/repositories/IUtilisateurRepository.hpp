#pragma once

#include <vector>

#include "models/Utilisateur.hpp"

class IUtilisateurRepository
{
    public:
        virtual ~IUtilisateurRepository() = default;
        virtual void sauvegarder(const std::vector<Utilisateur>& utilisateurs) = 0;
        virtual std::vector<Utilisateur> charger() = 0;
};