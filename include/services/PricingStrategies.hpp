#pragma once

#include <algorithm>
#include <stdexcept>

#include "services/IPricingStrategy.hpp"

// Aucune remise : renvoie le prix de vente tel qu'enregistré sur le produit.
class PrixStandard : public IPricingStrategy
{
    public:
        double calculerPrix(const Produit& produit) const override
        {
            return produit.getPrixVente();
        }

        std::string nom() const override { return "Prix standard"; }
};

// Remise proportionnelle (ex. solde saisonnier), en pourcentage du prix de vente.
class SoldeNoel : public IPricingStrategy
{
    private:
        double pourcentage;

    public:
        explicit SoldeNoel(double pourcentageRemise) : pourcentage(pourcentageRemise)
        {
            if (pourcentage < 0 || pourcentage > 100)
                throw std::invalid_argument("Le pourcentage de remise doit être entre 0 et 100.");
        }

        double calculerPrix(const Produit& produit) const override
        {
            return produit.getPrixVente() * (1.0 - pourcentage / 100.0);
        }

        std::string nom() const override { return "Solde (-" + std::to_string(static_cast<int>(pourcentage)) + "%)"; }
};

// Remise fixe (montant absolu), plafonnée pour ne jamais rendre le prix négatif.
class RemiseFidelite : public IPricingStrategy
{
    private:
        double montant;

    public:
        explicit RemiseFidelite(double montantRemise) : montant(montantRemise)
        {
            if (montant < 0)
                throw std::invalid_argument("Le montant de la remise ne peut pas être négatif.");
        }

        double calculerPrix(const Produit& produit) const override
        {
            return std::max(0.0, produit.getPrixVente() - montant);
        }

        std::string nom() const override { return "Remise fidélité (-" + std::to_string(montant) + ")"; }
};