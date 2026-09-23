#pragma once

#include <string>
#include <iostream>
#include <chrono>
#include <utility>

class Produit
{
    protected:
        std::string reference;
        std::string nom;
        std::string categorie;
        double prixAchat;
        double prixVente;
        int quantiteStock;
        int seuilAlerte;

    public:
        Produit(std::string ref, std::string n, std::string cat, double pA, double pV, int qte, int seuil)
            : reference(std::move(ref)), nom(std::move(n)), categorie(std::move(cat)),
              prixAchat(pA), prixVente(pV), quantiteStock(qte), seuilAlerte(seuil) {}

        virtual ~Produit() = default;

        // Getters par référence constante : évite une copie de string à chaque appel,
        // ce qui compte dès qu'on trie ou filtre des centaines de produits.
        const std::string& getReference() const { return reference; }
        const std::string& getNom() const { return nom; }
        const std::string& getCategorie() const { return categorie; }
        double getPrixAchat() const { return prixAchat; }
        double getPrixVente() const { return prixVente; }
        int getQuantiteStock() const { return quantiteStock; }
        int getSeuilAlerte() const { return seuilAlerte; }

        // Setters
        void setQuantiteStock(int qte) { quantiteStock = qte; }

        bool estSousLeSeuil() const { return quantiteStock <= seuilAlerte; }

        virtual void afficher() const
        {
            std::cout << "Ref: " << reference << " | Nom: " << nom
                      << " | Stock: " << quantiteStock << " | Seuil: " << seuilAlerte << "\n";
        }
};

class ProduitPerissable : public Produit
{
    private:
        std::chrono::year_month_day datePeremption;

    public:
        ProduitPerissable(std::string ref, std::string n, std::string cat, double pA, double pV, int qte, int seuil,
                           std::chrono::year_month_day dateP)
            : Produit(std::move(ref), std::move(n), std::move(cat), pA, pV, qte, seuil),
              datePeremption(dateP) {}

        const std::chrono::year_month_day& getDatePeremption() const { return datePeremption; }

        // Nombre de jours restants avant péremption par rapport à une date donnée
        // (aujourd'hui par défaut). Un chrono::year_month_day se compare et se soustrait
        // correctement, contrairement à un tri lexicographique sur "JJ/MM/AAAA".
        long joursAvantPeremption(std::chrono::year_month_day aujourdHui =
            std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())}) const
        {
            using namespace std::chrono;
            sys_days cible = sys_days{datePeremption};
            sys_days depart = sys_days{aujourdHui};
            return (cible - depart).count();
        }

        void afficher() const override
        {
            Produit::afficher();
            std::cout << "   -> Date de peremption : "
                      << static_cast<unsigned>(datePeremption.day()) << "/"
                      << static_cast<unsigned>(datePeremption.month()) << "/"
                      << static_cast<int>(datePeremption.year()) << "\n";
        }
};