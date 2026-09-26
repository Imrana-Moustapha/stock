#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>

#include "models/Fournisseur.hpp"
#include "models/Commande.hpp"

// Service dédié aux fournisseurs et aux commandes — séparé de GestionnaireStock
// pour respecter la responsabilité unique (un agrégat, un service). Pas encore
// de persistance sur disque (comme la gestion des utilisateurs) : c'est une
// limitation connue, à combler par un futur CommandeRepository suivant le même
// pattern IRepository que FichierTexteRepository.
class GestionnaireCommandes
{
    private:
        std::vector<Fournisseur> fournisseurs;
        std::vector<Commande> commandes;
        int prochainIdFournisseur = 1;
        int prochainIdCommande = 1;

    public:
        // --- Fournisseurs ---

        int ajouterFournisseur(std::string nom, std::string contact, std::string adresse)
        {
            int id = prochainIdFournisseur++;
            fournisseurs.emplace_back(id, std::move(nom), std::move(contact), std::move(adresse));
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

            int id = prochainIdCommande++;
            commandes.emplace_back(id, idFournisseur, date, StatutCommande::EN_COURS);
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
            commande->ajouterLigne(LigneCommande(referenceProduit, quantite, prixUnitaire));
        }

        void changerStatut(int idCommande, StatutCommande nouveauStatut)
        {
            Commande* commande = trouverCommande(idCommande);
            if (commande == nullptr)
                throw std::invalid_argument("Commande introuvable (id=" + std::to_string(idCommande) + ")");
            commande->setStatut(nouveauStatut);
        }
};