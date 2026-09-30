#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>

#include "models/Fournisseur.hpp"
#include "models/Commande.hpp"
#include "repositories/IFournisseurRepository.hpp"
#include "repositories/ICommandeRepository.hpp"

// Service dédié aux fournisseurs et aux commandes — séparé de GestionnaireStock
// pour respecter la responsabilité unique (un agrégat, un service). Persisté via
// deux repositories injectés, sur le même principe que GestionnaireStock
// (produits + mouvements) : sauvegarde automatique après chaque opération.
class GestionnaireCommandes
{
    private:
        std::vector<Fournisseur> fournisseurs;
        std::vector<Commande> commandes;
        int prochainIdFournisseur = 1;
        int prochainIdCommande = 1;

        std::unique_ptr<IFournisseurRepository> repositoryFournisseurs;
        std::unique_ptr<ICommandeRepository> repositoryCommandes;

    public:
        GestionnaireCommandes(std::unique_ptr<IFournisseurRepository> repoFournisseurs,
                               std::unique_ptr<ICommandeRepository> repoCommandes)
            : repositoryFournisseurs(std::move(repoFournisseurs)),
              repositoryCommandes(std::move(repoCommandes)) {}

        GestionnaireCommandes(const GestionnaireCommandes&) = delete;
        GestionnaireCommandes& operator=(const GestionnaireCommandes&) = delete;
        GestionnaireCommandes(GestionnaireCommandes&&) = default;
        GestionnaireCommandes& operator=(GestionnaireCommandes&&) = default;

        // --- Persistance ---

        void charger()
        {
            auto fournisseursCharges = repositoryFournisseurs->charger();
            auto commandesChargees = repositoryCommandes->charger();

            prochainIdFournisseur = 1;
            for (const auto& f : fournisseursCharges)
                prochainIdFournisseur = std::max(prochainIdFournisseur, f.getId() + 1);

            prochainIdCommande = 1;
            for (const auto& c : commandesChargees)
                prochainIdCommande = std::max(prochainIdCommande, c.getIdCommande() + 1);

            fournisseurs = std::move(fournisseursCharges);
            commandes = std::move(commandesChargees);
        }

        void sauvegarder() const
        {
            repositoryFournisseurs->sauvegarder(fournisseurs);
            repositoryCommandes->sauvegarder(commandes);
        }

        // --- Fournisseurs ---

        int ajouterFournisseur(std::string nom, std::string contact, std::string adresse)
        {
            int id = prochainIdFournisseur++;
            fournisseurs.emplace_back(id, std::move(nom), std::move(contact), std::move(adresse));
            sauvegarder();
            return id;
        }

        const std::vector<Fournisseur>& getFournisseurs() const { return fournisseurs; }

        const Fournisseur* trouverFournisseur(int id) const
        {
            auto it = std::find_if(fournisseurs.begin(), fournisseurs.end(),
                [id](const Fournisseur& f) { return f.getId() == id; });
            return (it != fournisseurs.end()) ? &(*it) : nullptr;
        }

        // --- Commandes ---

        int creerCommande(int idFournisseur, std::chrono::year_month_day date)
        {
            if (trouverFournisseur(idFournisseur) == nullptr)
                throw std::invalid_argument("Fournisseur introuvable (id=" + std::to_string(idFournisseur) + ")");
            if (!date.ok())
                throw std::invalid_argument("Date de commande invalide.");

            int id = prochainIdCommande++;
            commandes.emplace_back(id, idFournisseur, date, StatutCommande::EN_COURS);
            sauvegarder();
            return id;
        }

        Commande* trouverCommande(int id)
        {
            auto it = std::find_if(commandes.begin(), commandes.end(),
                [id](const Commande& c) { return c.getIdCommande() == id; });
            return (it != commandes.end()) ? &(*it) : nullptr;
        }

        const std::vector<Commande>& getCommandes() const { return commandes; }

        void ajouterLigneCommande(int idCommande, const std::string& referenceProduit, int quantite, double prixUnitaire)
        {
            Commande* commande = trouverCommande(idCommande);
            if (commande == nullptr)
                throw std::invalid_argument("Commande introuvable (id=" + std::to_string(idCommande) + ")");
            if (quantite <= 0)
                throw std::invalid_argument("La quantité doit être strictement positive.");
            if (prixUnitaire < 0)
                throw std::invalid_argument("Le prix unitaire ne peut pas être négatif.");

            commande->ajouterLigne(LigneCommande(referenceProduit, quantite, prixUnitaire));
            sauvegarder();
        }

        void changerStatut(int idCommande, StatutCommande nouveauStatut)
        {
            Commande* commande = trouverCommande(idCommande);
            if (commande == nullptr)
                throw std::invalid_argument("Commande introuvable (id=" + std::to_string(idCommande) + ")");
            commande->setStatut(nouveauStatut);
            sauvegarder();
        }
};