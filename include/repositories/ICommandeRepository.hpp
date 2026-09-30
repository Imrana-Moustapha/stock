#pragma once

#include <vector>

#include "models/Commande.hpp"

class ICommandeRepository
{
    public:
        virtual ~ICommandeRepository() = default;
        virtual void sauvegarder(const std::vector<Commande>& commandes) = 0;
        virtual std::vector<Commande> charger() = 0;
};