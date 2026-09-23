#pragma once

class Produit;

// Interface du pattern Observer. Toute classe qui veut être notifiée
// lorsqu'un produit passe sous son seuil critique implémente ce contrat
// (ex. Logger, GestionnaireAlertes) et s'enregistre auprès du GestionnaireStock.
class IObservateurStock
{
    public:
        virtual ~IObservateurStock() = default;
        virtual void notifierSeuilCritique(const Produit& produit) = 0;
};