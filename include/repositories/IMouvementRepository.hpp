#pragma once

#include <vector>

#include "models/MouvementStock.hpp"

// Interface de persistance pour le journal d'audit, séparée de IRepository
// (qui persiste les produits) : les mouvements sont un agrégat distinct,
// avec son propre cycle de vie et son propre format de stockage possible.
class IMouvementRepository
{
    public:
        virtual ~IMouvementRepository() = default;

        virtual void sauvegarder(const std::vector<MouvementStock>& historique) = 0;
        virtual std::vector<MouvementStock> charger() = 0;
};