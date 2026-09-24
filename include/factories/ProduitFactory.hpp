#pragma once

#include <string>
#include <memory>
#include <optional>
#include <chrono>
#include <stdexcept>

#include "models/Produit.hpp"

// Types de produits que la fabrique sait construire. Étendre cette liste
// (et le switch de creerProduit) est le seul endroit à toucher pour
// ajouter un nouveau type de produit dans l'application.
enum class TypeProduit
{
    STANDARD,
    PERISSABLE
};

// Centralise la création polymorphe des produits. Sans cette classe, la logique
// "quel type instancier selon quels paramètres" se disperserait dans chaque endroit
// qui crée un produit (menu interactif, chargement depuis un fichier, import CSV...).
class ProduitFactory
{
    public:
        static std::unique_ptr<Produit> creerProduit(
            TypeProduit type,
            const std::string& reference,
            const std::string& nom,
            const std::string& categorie,
            double prixAchat,
            double prixVente,
            int quantiteStock,
            int seuilAlerte,
            std::optional<std::chrono::year_month_day> datePeremption = std::nullopt)
        {
            switch (type)
            {
                case TypeProduit::STANDARD:
                    return std::make_unique<Produit>(reference, nom, categorie, prixAchat, prixVente, quantiteStock, seuilAlerte);

                case TypeProduit::PERISSABLE:
                    if (!datePeremption.has_value())
                        throw std::invalid_argument("Une date de péremption est requise pour un produit périssable.");
                    return std::make_unique<ProduitPerissable>(reference, nom, categorie, prixAchat, prixVente,
                                                                 quantiteStock, seuilAlerte, *datePeremption);
            }
            throw std::invalid_argument("Type de produit inconnu.");
        }

        // Variante à partir d'un tag texte (utile pour le repository, qui lit des
        // chaînes "STANDARD"/"PERISSABLE" depuis le fichier de persistance).
        static TypeProduit typeDepuisTexte(const std::string& texte)
        {
            if (texte == "STANDARD") return TypeProduit::STANDARD;
            if (texte == "PERISSABLE") return TypeProduit::PERISSABLE;
            throw std::invalid_argument("Type de produit inconnu : " + texte);
        }
};