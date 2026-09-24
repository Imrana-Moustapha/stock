#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <string>

#include "models/Produit.hpp"
#include "observers/IObservateurStock.hpp"
#include "exceptions/Exceptions.hpp"
#include "repositories/IRepository.hpp"

// Couche Service : applique les règles métier sur la collection de produits.
// Ne sait rien de l'affichage (couche UI) ni du format de stockage réel —
// elle dépend uniquement de l'abstraction IRepository (inversion des dépendances),
// injectée au constructeur plutôt que construite en dur ici.
class GestionnaireStock
{
    private:
        std::vector<std::unique_ptr<Produit>> produits;
        std::vector<IObservateurStock*> observateurs;
        std::unique_ptr<IRepository> repository;

        // Notifie tous les observateurs enregistrés si le produit est sous son seuil.
        void notifierSiSeuilCritique(const Produit& produit) const
        {
            if (!produit.estSousLeSeuil()) return;
            for (IObservateurStock* observateur : observateurs)
                observateur->notifierSeuilCritique(produit);
        }

    public:
        // Le repository est injecté : GestionnaireStock ne sait pas s'il s'agit
        // d'un FichierTexteRepository, d'un CsvRepository ou d'autre chose.
        explicit GestionnaireStock(std::unique_ptr<IRepository> repo) : repository(std::move(repo)) {}

        // Non copiable (les unique_ptr l'interdisent de toute façon), mais déplaçable.
        GestionnaireStock(const GestionnaireStock&) = delete;
        GestionnaireStock& operator=(const GestionnaireStock&) = delete;
        GestionnaireStock(GestionnaireStock&&) = default;
        GestionnaireStock& operator=(GestionnaireStock&&) = default;

        // --- Persistance (délègue au repository injecté) ---

        void charger()
        {
            produits = repository->charger();
        }

        void sauvegarder() const
        {
            repository->sauvegarder(produits);
        }

        // --- Pattern Observer ---

        void ajouterObservateur(IObservateurStock* observateur)
        {
            observateurs.push_back(observateur);
        }

        // --- Gestion des produits ---

        void ajouterProduit(std::unique_ptr<Produit> produit)
        {
            produits.push_back(std::move(produit));
        }

        // Retourne un pointeur nu (non propriétaire) vers le produit, ou nullptr si absent.
        // Pratique pour les appelants qui veulent juste vérifier l'existence sans lever d'exception.
        Produit* trouverProduit(const std::string& reference)
        {
            auto it = std::find_if(produits.begin(), produits.end(),
                [&reference](const std::unique_ptr<Produit>& p) { return p->getReference() == reference; });
            return (it != produits.end()) ? it->get() : nullptr;
        }

        // Variante qui lève une exception si le produit n'existe pas — utile quand
        // l'absence du produit est une erreur et non un cas normal à gérer.
        Produit& trouverProduitOuLever(const std::string& reference)
        {
            Produit* produit = trouverProduit(reference);
            if (produit == nullptr)
                throw ProduitIntrouvableException(reference);
            return *produit;
        }

        void supprimerProduit(const std::string& reference)
        {
            auto it = std::remove_if(produits.begin(), produits.end(),
                [&reference](const std::unique_ptr<Produit>& p) { return p->getReference() == reference; });
            if (it == produits.end())
                throw ProduitIntrouvableException(reference);
            produits.erase(it, produits.end());
        }

        // --- Mouvements de stock ---

        void ajouterStock(const std::string& reference, int quantite)
        {
            Produit& produit = trouverProduitOuLever(reference);
            produit.setQuantiteStock(produit.getQuantiteStock() + quantite);
        }

        void retirerStock(const std::string& reference, int quantite)
        {
            Produit& produit = trouverProduitOuLever(reference);
            if (produit.getQuantiteStock() < quantite)
                throw StockInsuffisantException(reference, quantite, produit.getQuantiteStock());

            produit.setQuantiteStock(produit.getQuantiteStock() - quantite);
            notifierSiSeuilCritique(produit);
        }

        // --- Lecture et statistiques ---

        const std::vector<std::unique_ptr<Produit>>& getProduits() const
        {
            return produits;
        }

        std::size_t nombreDeProduits() const { return produits.size(); }

        double valeurTotaleStock() const
        {
            return std::accumulate(produits.begin(), produits.end(), 0.0,
                [](double total, const std::unique_ptr<Produit>& p) {
                    return total + p->getPrixAchat() * p->getQuantiteStock();
                });
        }

        // Retourne les références (pointeurs bruts, non propriétaires) des produits
        // actuellement sous leur seuil critique.
        std::vector<const Produit*> produitsSousLeSeuil() const
        {
            std::vector<const Produit*> resultat;
            for (const auto& p : produits)
                if (p->estSousLeSeuil())
                    resultat.push_back(p.get());
            return resultat;
        }
};