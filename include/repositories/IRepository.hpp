#pragma once

#include <vector>
#include <memory>

#include "models/Produit.hpp"

// Interface de persistance (pattern Strategy / inversion des dépendances).
// GestionnaireStock dépend de cette abstraction, jamais d'une implémentation
// concrète : on peut donc remplacer FichierTexteRepository par CsvRepository
// ou une base de données sans toucher à la logique métier.
class IRepository
{
    public:
        virtual ~IRepository() = default;

        virtual void sauvegarder(const std::vector<std::unique_ptr<Produit>>& produits) = 0;
        virtual std::vector<std::unique_ptr<Produit>> charger() = 0;
};