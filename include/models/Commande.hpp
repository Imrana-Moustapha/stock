#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <utility>
#include <stdexcept>

enum class StatutCommande
{
    EN_COURS,
    LIVREE,
    ANNULEE
};

// Libellé texte d'un statut (affichage, exports, fichiers de données).
inline const char* libelle(StatutCommande statut)
{
    switch (statut) {
        case StatutCommande::EN_COURS: return "EN_COURS";
        case StatutCommande::LIVREE:   return "LIVREE";
        case StatutCommande::ANNULEE:  return "ANNULEE";
    }
    return "?";
}

inline StatutCommande statutDepuisTexte(const std::string& texte)
{
    if (texte == "EN_COURS") return StatutCommande::EN_COURS;
    if (texte == "LIVREE")   return StatutCommande::LIVREE;
    if (texte == "ANNULEE")  return StatutCommande::ANNULEE;
    throw std::invalid_argument("Statut de commande inconnu : " + texte);
}

// Une commande fournisseur doit savoir CE QUI a été commandé, pas seulement son montant :
// sans ça, impossible de mettre à jour le stock automatiquement à la réception.
class LigneCommande
{
    private:
        std::string referenceProduit;
        int quantite;
        double prixUnitaire;

    public:
        LigneCommande(std::string refProduit, int qte, double prixU)
            : referenceProduit(std::move(refProduit)), quantite(qte), prixUnitaire(prixU) {}

        const std::string& getReferenceProduit() const { return referenceProduit; }
        int getQuantite() const { return quantite; }
        double getPrixUnitaire() const { return prixUnitaire; }
        double getSousTotal() const { return quantite * prixUnitaire; }
};

class Commande
{
    private:
        int idCommande;
        int idFournisseur;
        std::chrono::year_month_day dateCommande;
        StatutCommande statut;
        std::vector<LigneCommande> lignes;

    public:
        Commande(int idC, int idF, std::chrono::year_month_day date, StatutCommande stat)
            : idCommande(idC), idFournisseur(idF), dateCommande(date), statut(stat) {}

        int getIdCommande() const { return idCommande; }
        int getIdFournisseur() const { return idFournisseur; }
        const std::chrono::year_month_day& getDateCommande() const { return dateCommande; }
        StatutCommande getStatut() const { return statut; }
        void setStatut(StatutCommande s) { statut = s; }

        void ajouterLigne(LigneCommande ligne) { lignes.push_back(std::move(ligne)); }
        const std::vector<LigneCommande>& getLignes() const { return lignes; }

        // Le montant total est toujours recalculé à partir des lignes, jamais stocké en dur,
        // pour éviter toute désynchronisation entre l'affichage et le contenu réel de la commande.
        double getMontantTotal() const
        {
            double total = 0.0;
            for (const auto& ligne : lignes)
                total += ligne.getSousTotal();
            return total;
        }
};