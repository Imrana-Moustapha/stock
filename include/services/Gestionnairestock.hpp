#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <string>
#include <chrono>
#include <stdexcept>
#include <map>

#include "models/Produit.hpp"
#include "models/MouvementStock.hpp"
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
        std::vector<MouvementStock> historique;
        int prochainIdMouvement = 1;

        // Notifie tous les observateurs enregistrés si le produit est sous son seuil.
        void notifierSiSeuilCritique(const Produit& produit) const
        {
            if (!produit.estSousLeSeuil()) return;
            for (IObservateurStock* observateur : observateurs)
                observateur->notifierSeuilCritique(produit);
        }

        // Enregistre automatiquement chaque mouvement dans le journal d'audit,
        // avec un identifiant croissant et l'horodatage réel de l'opération.
        void enregistrerMouvement(const std::string& reference, TypeMouvement type, int quantite, const std::string& auteur)
        {
            historique.emplace_back(prochainIdMouvement++, reference, type, quantite,
                                     std::chrono::system_clock::now(), auteur);
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

        void ajouterStock(const std::string& reference, int quantite, const std::string& auteur = "Système")
        {
            if (quantite <= 0)
                throw std::invalid_argument("La quantité à ajouter doit être strictement positive.");

            Produit& produit = trouverProduitOuLever(reference);
            produit.setQuantiteStock(produit.getQuantiteStock() + quantite);
            enregistrerMouvement(reference, TypeMouvement::ENTREE, quantite, auteur);
        }

        void retirerStock(const std::string& reference, int quantite, const std::string& auteur = "Système")
        {
            if (quantite <= 0)
                throw std::invalid_argument("La quantité à retirer doit être strictement positive.");

            Produit& produit = trouverProduitOuLever(reference);
            if (produit.getQuantiteStock() < quantite)
                throw StockInsuffisantException(reference, quantite, produit.getQuantiteStock());

            produit.setQuantiteStock(produit.getQuantiteStock() - quantite);
            enregistrerMouvement(reference, TypeMouvement::SORTIE, quantite, auteur);
            notifierSiSeuilCritique(produit);
        }

        // Fixe la quantité en stock à une valeur absolue (ex. après un inventaire physique),
        // plutôt que d'ajouter ou de retirer une quantité relative. L'écart réel est
        // conservé dans le journal pour garder une trace fidèle de l'ajustement.
        void ajusterStock(const std::string& reference, int nouvelleQuantite, const std::string& auteur = "Système")
        {
            if (nouvelleQuantite < 0)
                throw std::invalid_argument("La quantité ajustée ne peut pas être négative.");

            Produit& produit = trouverProduitOuLever(reference);
            int ecart = nouvelleQuantite - produit.getQuantiteStock();
            produit.setQuantiteStock(nouvelleQuantite);
            enregistrerMouvement(reference, TypeMouvement::AJUSTEMENT, ecart, auteur);
            notifierSiSeuilCritique(produit);
        }

        const std::vector<MouvementStock>& getHistorique() const
        {
            return historique;
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

        // Valeur du stock (prix d'achat × quantité) regroupée par catégorie.
        std::map<std::string, double> valeurStockParCategorie() const
        {
            std::map<std::string, double> resultat;
            for (const auto& p : produits)
                resultat[p->getCategorie()] += p->getPrixAchat() * p->getQuantiteStock();
            return resultat;
        }

        // Nombre de mouvements (entrées + sorties + ajustements confondus) par référence
        // de produit, à partir du journal d'audit.
        std::map<std::string, int> nombreMouvementsParProduit() const
        {
            std::map<std::string, int> resultat;
            for (const auto& mvt : historique)
                resultat[mvt.getReferenceProduit()]++;
            return resultat;
        }

        // Produits n'ayant fait l'objet d'aucun mouvement enregistré dans le journal —
        // un indicateur simple de surstock ou d'article à faible rotation.
        std::vector<const Produit*> produitsDormants() const
        {
            auto compteurs = nombreMouvementsParProduit();
            std::vector<const Produit*> resultat;
            for (const auto& p : produits)
                if (compteurs.find(p->getReference()) == compteurs.end())
                    resultat.push_back(p.get());
            return resultat;
        }
};