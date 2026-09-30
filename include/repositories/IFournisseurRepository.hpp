#pragma once

#include <vector>

#include "models/Fournisseur.hpp"

class IFournisseurRepository
{
    public:
        virtual ~IFournisseurRepository() = default;
        virtual void sauvegarder(const std::vector<Fournisseur>& fournisseurs) = 0;
        virtual std::vector<Fournisseur> charger() = 0;
};