#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <stdexcept>

#include "models/Produit.hpp"
#include "repositories/IRepository.hpp"
#include "repositories/FormatTexte.hpp"
#include "exceptions/Exceptions.hpp"
#include "factories/ProduitFactory.hpp"
#include "utils/DateUtils.hpp"

// Persistance des produits dans un fichier texte, une ligne par produit,
// champs séparés par ';' (le ';' et le '\' présents dans les données sont échappés). Format :
//   STANDARD;reference;nom;categorie;prixAchat;prixVente;quantiteStock;seuilAlerte
//   PERISSABLE;reference;nom;categorie;prixAchat;prixVente;quantiteStock;seuilAlerte;AAAA-MM-JJ
//   ELECTRONIQUE;reference;nom;categorie;prixAchat;prixVente;quantiteStock;seuilAlerte;dureeGarantieMois;numeroSerie
class FichierTexteRepository : public IRepository
{
    private:
        std::string cheminFichier;
        static constexpr char DELIMITEUR = ';';

        static std::string ech(const std::string& texte)
        {
            return FormatTexte::echapper(texte, DELIMITEUR);
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
                const auto* perissable = dynamic_cast<const ProduitPerissable*>(p.get());
                const auto* electronique = dynamic_cast<const ProduitElectronique*>(p.get());

                std::string type = perissable ? "PERISSABLE" : electronique ? "ELECTRONIQUE" : "STANDARD";

                fichier << type << DELIMITEUR
                        << ech(p->getReference()) << DELIMITEUR
                        << ech(p->getNom()) << DELIMITEUR
                        << ech(p->getCategorie()) << DELIMITEUR
                        << p->getPrixAchat() << DELIMITEUR
                        << p->getPrixVente() << DELIMITEUR
                        << p->getQuantiteStock() << DELIMITEUR
                        << p->getSeuilAlerte();

                if (perissable)
                    fichier << DELIMITEUR << DateUtils::formaterIso(perissable->getDatePeremption());
                else if (electronique)
                    fichier << DELIMITEUR << electronique->getDureeGarantieMois()
                            << DELIMITEUR << ech(electronique->getNumeroSerie());
                fichier << "\n";
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

                std::vector<std::string> champs = FormatTexte::decouper(ligne, DELIMITEUR);

                try {
                    TypeProduit type = ProduitFactory::typeDepuisTexte(champs.at(0));
                    std::optional<std::chrono::year_month_day> date;
                    std::optional<int> dureeGarantie;
                    std::optional<std::string> numeroSerie;

                    if (type == TypeProduit::STANDARD && champs.size() == 8) {
                        // pas de champ supplémentaire
                    } else if (type == TypeProduit::PERISSABLE && champs.size() == 9) {
                        date = DateUtils::parserIso(champs[8]);
                        if (!date.has_value())
                            throw std::invalid_argument("date invalide : " + champs[8]);
                    } else if (type == TypeProduit::ELECTRONIQUE && champs.size() == 10) {
                        dureeGarantie = std::stoi(champs[8]);
                        numeroSerie = champs[9];
                    } else {
                        throw std::invalid_argument("nombre de colonnes inattendu");
                    }

                    produits.push_back(ProduitFactory::creerProduit(
                        type, champs[1], champs[2], champs[3],
                        std::stod(champs[4]), std::stod(champs[5]),
                        std::stoi(champs[6]), std::stoi(champs[7]), date, dureeGarantie, numeroSerie));

                } catch (const std::invalid_argument& e) {
                    throw FormatFichierInvalideException(std::string(e.what()) + " (ligne : " + ligne + ")");
                } catch (const std::out_of_range& e) {
                    throw FormatFichierInvalideException("valeur numérique hors limites (ligne : " + ligne + ")");
                }
            }
            return produits;
        }
};