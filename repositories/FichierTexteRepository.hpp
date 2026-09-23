#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <chrono>
#include <iomanip>

#include "models/Produit.hpp"
#include "repositories/IRepository.hpp"
#include "exceptions/Exceptions.hpp"

// Persistance des produits dans un fichier texte, une ligne par produit,
// champs séparés par ';'. Format :
//   STANDARD;reference;nom;categorie;prixAchat;prixVente;quantiteStock;seuilAlerte
//   PERISSABLE;reference;nom;categorie;prixAchat;prixVente;quantiteStock;seuilAlerte;AAAA-MM-JJ
class FichierTexteRepository : public IRepository
{
    private:
        std::string cheminFichier;
        static constexpr char DELIMITEUR = ';';

        static std::string formaterDate(const std::chrono::year_month_day& date)
        {
            std::ostringstream oss;
            oss << static_cast<int>(date.year()) << "-"
                << std::setw(2) << std::setfill('0') << static_cast<unsigned>(date.month()) << "-"
                << std::setw(2) << std::setfill('0') << static_cast<unsigned>(date.day());
            return oss.str();
        }

        static std::chrono::year_month_day parserDate(const std::string& texte)
        {
            int annee, mois, jour;
            char separateur1, separateur2;
            std::istringstream iss(texte);
            if (!(iss >> annee >> separateur1 >> mois >> separateur2 >> jour) || separateur1 != '-' || separateur2 != '-')
                throw FormatFichierInvalideException("date invalide : " + texte);

            return std::chrono::year{annee} / std::chrono::month{static_cast<unsigned>(mois)}
                                             / std::chrono::day{static_cast<unsigned>(jour)};
        }

        static std::vector<std::string> decouper(const std::string& ligne)
        {
            std::vector<std::string> champs;
            std::stringstream ss(ligne);
            std::string champ;
            while (std::getline(ss, champ, DELIMITEUR))
                champs.push_back(champ);
            return champs;
        }

    public:
        explicit FichierTexteRepository(std::string chemin) : cheminFichier(std::move(chemin)) {}

        void sauvegarder(const std::vector<std::unique_ptr<Produit>>& produits) override
        {
            std::ofstream fichier(cheminFichier, std::ios::trunc);
            if (!fichier)
                throw FormatFichierInvalideException("impossible d'ouvrir '" + cheminFichier + "' en écriture");

            for (const auto& p : produits)
            {
                if (const auto* perissable = dynamic_cast<const ProduitPerissable*>(p.get()))
                {
                    fichier << "PERISSABLE" << DELIMITEUR
                            << perissable->getReference() << DELIMITEUR
                            << perissable->getNom() << DELIMITEUR
                            << perissable->getCategorie() << DELIMITEUR
                            << perissable->getPrixAchat() << DELIMITEUR
                            << perissable->getPrixVente() << DELIMITEUR
                            << perissable->getQuantiteStock() << DELIMITEUR
                            << perissable->getSeuilAlerte() << DELIMITEUR
                            << formaterDate(perissable->getDatePeremption()) << "\n";
                }
                else
                {
                    fichier << "STANDARD" << DELIMITEUR
                            << p->getReference() << DELIMITEUR
                            << p->getNom() << DELIMITEUR
                            << p->getCategorie() << DELIMITEUR
                            << p->getPrixAchat() << DELIMITEUR
                            << p->getPrixVente() << DELIMITEUR
                            << p->getQuantiteStock() << DELIMITEUR
                            << p->getSeuilAlerte() << "\n";
                }
            }
        }

        std::vector<std::unique_ptr<Produit>> charger() override
        {
            std::vector<std::unique_ptr<Produit>> produits;
            std::ifstream fichier(cheminFichier);
            if (!fichier)
                return produits; // Aucun fichier existant : stock vide, pas une erreur.

            std::string ligne;
            while (std::getline(fichier, ligne))
            {
                if (ligne.empty()) continue;

                std::vector<std::string> champs = decouper(ligne);
                const std::string& type = champs.at(0);

                if (type == "STANDARD" && champs.size() == 8)
                {
                    produits.push_back(std::make_unique<Produit>(
                        champs[1], champs[2], champs[3],
                        std::stod(champs[4]), std::stod(champs[5]),
                        std::stoi(champs[6]), std::stoi(champs[7])));
                }
                else if (type == "PERISSABLE" && champs.size() == 9)
                {
                    produits.push_back(std::make_unique<ProduitPerissable>(
                        champs[1], champs[2], champs[3],
                        std::stod(champs[4]), std::stod(champs[5]),
                        std::stoi(champs[6]), std::stoi(champs[7]),
                        parserDate(champs[8])));
                }
                else
                {
                    throw FormatFichierInvalideException("ligne mal formée : " + ligne);
                }
            }
            return produits;
        }
};