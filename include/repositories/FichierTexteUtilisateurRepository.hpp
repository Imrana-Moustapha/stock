#pragma once

#include <string>
#include <fstream>
#include <stdexcept>

#include "repositories/IUtilisateurRepository.hpp"
#include "repositories/FormatTexte.hpp"
#include "exceptions/Exceptions.hpp"

// Format : id;username;passwordHash;role
// passwordHash est déjà "sel$hash" (voir utils/Hachage.hpp) : jamais un mot de
// passe en clair n'est écrit sur disque.
class FichierTexteUtilisateurRepository : public IUtilisateurRepository
{
    private:
        std::string cheminFichier;
        static constexpr char DELIMITEUR = ';';

        static std::string ech(const std::string& t) { return FormatTexte::echapper(t, DELIMITEUR); }

    public:
        explicit FichierTexteUtilisateurRepository(std::string chemin) : cheminFichier(std::move(chemin)) {}

        void sauvegarder(const std::vector<Utilisateur>& utilisateurs) override
        {
            std::ofstream fichier(cheminFichier, std::ios::trunc);
            if (!fichier)
                throw FormatFichierInvalideException("impossible d'ouvrir '" + cheminFichier + "' en écriture");

            for (const auto& u : utilisateurs)
                fichier << u.getId() << DELIMITEUR << ech(u.getUsername()) << DELIMITEUR
                        << ech(u.getPasswordHash()) << DELIMITEUR << libelle(u.getRole()) << "\n";
        }

        std::vector<Utilisateur> charger() override
        {
            std::vector<Utilisateur> utilisateurs;
            std::ifstream fichier(cheminFichier);
            if (!fichier) return utilisateurs;

            std::string ligne;
            while (std::getline(fichier, ligne)) {
                if (ligne.empty()) continue;
                auto champs = FormatTexte::decouper(ligne, DELIMITEUR);
                if (champs.size() != 4)
                    throw FormatFichierInvalideException("ligne d'utilisateur mal formée : " + ligne);

                try {
                    utilisateurs.emplace_back(std::stoi(champs[0]), champs[1], champs[2], roleDepuisTexte(champs[3]));
                } catch (const std::invalid_argument& e) {
                    throw FormatFichierInvalideException(std::string(e.what()) + " (ligne : " + ligne + ")");
                } catch (const std::out_of_range&) {
                    throw FormatFichierInvalideException("id d'utilisateur hors limites (ligne : " + ligne + ")");
                }
            }
            return utilisateurs;
        }
};