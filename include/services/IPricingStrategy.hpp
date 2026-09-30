#pragma once

#include "models/Produit.hpp"

// Interface du pattern Strategy pour la tarification. Permet de faire varier le calcul
// du prix de vente affiché (soldes, remise fidélité...) sans toucher à la classe Produit
// ni à son prixVente stocké, qui reste la référence de base.
class IPricingStrategy
{
    public:
        virtual ~IPricingStrategy() = default;
        virtual double calculerPrix(const Produit& produit) const = 0;
        virtual std::string nom() const = 0;
};