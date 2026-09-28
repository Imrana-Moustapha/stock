#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <stdexcept>

#include "models/MouvementStock.hpp"
#include "repositories/IMouvementRepository.hpp"
#include "repositories/FormatTexte.hpp"
#include "exceptions/Exceptions.hpp"

// Persistance du journal des mouvements dans un fichier texte, une ligne par mouvement :
//   id;type;referenceProduit;quantite;epochSecondes;auteur
// L'horodatage est stocké en secondes depuis l'epoch Unix plutôt qu'en texte lisible :
// c'est trivial à reconvertir en std::chrono::system_clock::time_point sans ambiguïté
// de fuseau horaire ou de format, contrairement à une date écrite en toutes lettres.
class FichierTexteMouvementRepository : public IMouvementRepository
{
    private:
        std::string cheminFichier;
        static constexpr char DELIMITEUR = ';';

        static std::string typeVersTexte(TypeMouvement type)
        {
            return libelle(type);
        }

        static TypeMouvement texteVersType(const std::string& texte)
        {
            if (texte == "ENTREE") return TypeMouvement::ENTREE;
            if (texte == "SORTIE") return TypeMouvement::SORTIE;
            if (texte == "AJUSTEMENT") return TypeMouvement::AJUSTEMENT;
            throw FormatFichierInvalideException("type de mouvement inconnu : " + texte);
        }

        static std::string ech(const std::string& texte)
        {
            return FormatTexte::echapper(texte, DELIMITEUR);
        }

    public:
        explicit FichierTexteMouvementRepository(std::string chemin) : cheminFichier(std::move(chemin)) {}

        void sauvegarder(const std::vector<MouvementStock>& historique) override
        {
            std::ofstream fichier(cheminFichier, std::ios::trunc);
            if (!fichier)
                throw FormatFichierInvalideException("impossible d'ouvrir '" + cheminFichier + "' en écriture");

            for (const auto& mvt : historique) {
                auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
                    mvt.getDateHeure().time_since_epoch()).count();

                fichier << mvt.getId() << DELIMITEUR
                        << typeVersTexte(mvt.getType()) << DELIMITEUR
                        << ech(mvt.getReferenceProduit()) << DELIMITEUR
                        << mvt.getQuantite() << DELIMITEUR
                        << epoch << DELIMITEUR
                        << ech(mvt.getAuteur()) << "\n";
            }
        }

        std::vector<MouvementStock> charger() override
        {
            std::vector<MouvementStock> historique;
            std::ifstream fichier(cheminFichier);
            if (!fichier)
                return historique; // Aucun fichier existant : journal vide, pas une erreur.

            std::string ligne;
            while (std::getline(fichier, ligne)) {
                if (ligne.empty()) continue;

                auto champs = FormatTexte::decouper(ligne, DELIMITEUR);
                if (champs.size() != 6)
                    throw FormatFichierInvalideException("ligne mal formée : " + ligne);

                try {
                    long long epoch = std::stoll(champs[4]);
                    auto dateHeure = std::chrono::system_clock::time_point(std::chrono::seconds(epoch));

                    historique.emplace_back(std::stoi(champs[0]), champs[2], texteVersType(champs[1]),
                                             std::stoi(champs[3]), dateHeure, champs[5]);
                } catch (const std::invalid_argument&) {
                    throw FormatFichierInvalideException("valeur numérique invalide (ligne : " + ligne + ")");
                } catch (const std::out_of_range&) {
                    throw FormatFichierInvalideException("valeur numérique hors limites (ligne : " + ligne + ")");
                }
            }
            return historique;
        }
};