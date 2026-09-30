#pragma once

#include <string>
#include <fstream>
#include <stdexcept>

#include "repositories/IFournisseurRepository.hpp"
#include "repositories/FormatTexte.hpp"
#include "exceptions/Exceptions.hpp"

// Format : id;nom;contact;adresse (';' et '\' échappés dans les champs texte).
class FichierTexteFournisseurRepository : public IFournisseurRepository
{
    private:
        std::string cheminFichier;
        static constexpr char DELIMITEUR = ';';

        static std::string ech(const std::string& t) { return FormatTexte::echapper(t, DELIMITEUR); }

    public:
        explicit FichierTexteFournisseurRepository(std::string chemin) : cheminFichier(std::move(chemin)) {}

        void sauvegarder(const std::vector<Fournisseur>& fournisseurs) override
        {
            std::ofstream fichier(cheminFichier, std::ios::trunc);
            if (!fichier)
                throw FormatFichierInvalideException("impossible d'ouvrir '" + cheminFichier + "' en écriture");

            for (const auto& f : fournisseurs)
                fichier << f.getId() << DELIMITEUR << ech(f.getNom()) << DELIMITEUR
                        << ech(f.getContact()) << DELIMITEUR << ech(f.getAdresse()) << "\n";
        }

        std::vector<Fournisseur> charger() override
        {
            std::vector<Fournisseur> fournisseurs;
            std::ifstream fichier(cheminFichier);
            if (!fichier) return fournisseurs;

            std::string ligne;
            while (std::getline(fichier, ligne)) {
                if (ligne.empty()) continue;
                auto champs = FormatTexte::decouper(ligne, DELIMITEUR);
                if (champs.size() != 4)
                    throw FormatFichierInvalideException("ligne de fournisseur mal formée : " + ligne);

                try {
                    fournisseurs.emplace_back(std::stoi(champs[0]), champs[1], champs[2], champs[3]);
                } catch (const std::invalid_argument&) {
                    throw FormatFichierInvalideException("id de fournisseur invalide (ligne : " + ligne + ")");
                } catch (const std::out_of_range&) {
                    throw FormatFichierInvalideException("id de fournisseur hors limites (ligne : " + ligne + ")");
                }
            }
            return fournisseurs;
        }
};