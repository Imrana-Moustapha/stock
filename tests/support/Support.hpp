#pragma once

// Socle commun des tests : dépôts en mémoire (aucun fichier réel touché), observateur
// enregistreur, dossier temporaire, et simulation du clavier pour les fonctions de saisie.

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "factories/ProduitFactory.hpp"
#include "observers/IObservateurStock.hpp"
#include "repositories/IMouvementRepository.hpp"
#include "repositories/IRepository.hpp"
#include "services/GestionnaireStock.hpp"

namespace Support
{
    // ---- Produits de test -------------------------------------------------------------

    inline std::unique_ptr<Produit> cloner(const Produit& p)
    {
        if (const auto* per = dynamic_cast<const ProduitPerissable*>(&p))
            return ProduitFactory::creerProduit(TypeProduit::PERISSABLE, per->getReference(), per->getNom(),
                per->getCategorie(), per->getPrixAchat(), per->getPrixVente(), per->getQuantiteStock(),
                per->getSeuilAlerte(), per->getDatePeremption());
        return ProduitFactory::creerProduit(TypeProduit::STANDARD, p.getReference(), p.getNom(),
            p.getCategorie(), p.getPrixAchat(), p.getPrixVente(), p.getQuantiteStock(), p.getSeuilAlerte());
    }

    inline std::unique_ptr<Produit> produit(const std::string& ref, int qte = 10, int seuil = 3,
                                            const std::string& categorie = "Cat", double prixAchat = 10.0)
    {
        return ProduitFactory::creerProduit(TypeProduit::STANDARD, ref, "Produit " + ref, categorie,
                                            prixAchat, prixAchat * 1.5, qte, seuil);
    }

    inline std::chrono::year_month_day date(int a, int m, int j)
    {
        return std::chrono::year{a} / std::chrono::month{static_cast<unsigned>(m)}
                                    / std::chrono::day{static_cast<unsigned>(j)};
    }

    // ---- Dépôts en mémoire ------------------------------------------------------------

    class DepotProduitsMemoire : public IRepository
    {
        public:
            std::vector<std::unique_ptr<Produit>> disque;
            int nbSauvegardes = 0;

            void sauvegarder(const std::vector<std::unique_ptr<Produit>>& produits) override
            {
                disque.clear();
                for (const auto& p : produits) disque.push_back(cloner(*p));
                ++nbSauvegardes;
            }

            std::vector<std::unique_ptr<Produit>> charger() override
            {
                std::vector<std::unique_ptr<Produit>> copie;
                for (const auto& p : disque) copie.push_back(cloner(*p));
                return copie;
            }
    };

    class DepotMouvementsMemoire : public IMouvementRepository
    {
        public:
            std::vector<MouvementStock> disque;
            int nbSauvegardes = 0;

            void sauvegarder(const std::vector<MouvementStock>& historique) override
            {
                disque = historique;
                ++nbSauvegardes;
            }

            std::vector<MouvementStock> charger() override { return disque; }
    };

    // Un GestionnaireStock branché sur des dépôts en mémoire, avec accès à ces dépôts
    // pour vérifier ce qui a été (ou non) sauvegardé.
    struct BancDeTest
    {
        DepotProduitsMemoire* depotProduits = nullptr;
        DepotMouvementsMemoire* depotMouvements = nullptr;
        GestionnaireStock gestionnaire;

        BancDeTest() : gestionnaire(creer(depotProduits, depotMouvements)) {}

        int sauvegardes() const { return depotProduits->nbSauvegardes; }

        private:
            static GestionnaireStock creer(DepotProduitsMemoire*& dp, DepotMouvementsMemoire*& dm)
            {
                auto p = std::make_unique<DepotProduitsMemoire>();
                auto m = std::make_unique<DepotMouvementsMemoire>();
                dp = p.get();
                dm = m.get();
                return GestionnaireStock(std::move(p), std::move(m));
            }
    };

    class ObservateurEnregistreur : public IObservateurStock
    {
        public:
            std::vector<std::string> references;
            void notifierSeuilCritique(const Produit& p) override { references.push_back(p.getReference()); }
    };

    // ---- Fichiers ---------------------------------------------------------------------

    class DossierTemporaire
    {
        private:
            std::filesystem::path chemin;

        public:
            DossierTemporaire()
            {
                static int compteur = 0;
                auto horloge = std::chrono::steady_clock::now().time_since_epoch().count();
                chemin = std::filesystem::temp_directory_path()
                         / ("stock_tests_" + std::to_string(horloge) + "_" + std::to_string(compteur++));
                std::filesystem::create_directories(chemin);
            }

            ~DossierTemporaire()
            {
                std::error_code ignore;
                std::filesystem::remove_all(chemin, ignore);
            }

            DossierTemporaire(const DossierTemporaire&) = delete;
            DossierTemporaire& operator=(const DossierTemporaire&) = delete;

            std::string fichier(const std::string& nom) const { return (chemin / nom).string(); }
    };

    inline void ecrire(const std::string& chemin, const std::string& contenu)
    {
        std::ofstream(chemin, std::ios::trunc) << contenu;
    }

    inline std::string lire(const std::string& chemin)
    {
        std::ifstream f(chemin);
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    // ---- Clavier / écran simulés ------------------------------------------------------

    // Redirige std::cin vers un texte prédéfini et capture tout ce qui est écrit sur std::cout,
    // le temps de la vie de l'objet.
    class FluxSimules
    {
        private:
            std::istringstream entree;
            std::ostringstream sortie;
            std::streambuf* ancienneEntree;
            std::streambuf* ancienneSortie;

        public:
            explicit FluxSimules(const std::string& saisie)
                : entree(saisie),
                  ancienneEntree(std::cin.rdbuf(entree.rdbuf())),
                  ancienneSortie(std::cout.rdbuf(sortie.rdbuf())) {}

            ~FluxSimules()
            {
                std::cin.rdbuf(ancienneEntree);
                std::cout.rdbuf(ancienneSortie);
            }

            FluxSimules(const FluxSimules&) = delete;
            FluxSimules& operator=(const FluxSimules&) = delete;

            std::string affiche() const { return sortie.str(); }
    };

    inline int compter(const std::string& texte, const std::string& motif)
    {
        int n = 0;
        for (auto pos = texte.find(motif); pos != std::string::npos; pos = texte.find(motif, pos + motif.size()))
            ++n;
        return n;
    }
}