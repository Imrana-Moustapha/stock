#pragma once

#include <string>
#include <fstream>
#include <map>
#include <stdexcept>

#include "repositories/ICommandeRepository.hpp"
#include "repositories/FormatTexte.hpp"
#include "utils/DateUtils.hpp"
#include "exceptions/Exceptions.hpp"

// Deux fichiers, comme deux tables reliées par idCommande :
//   commandes.txt      : id;idFournisseur;dateCommande(AAAA-MM-JJ);statut
//   commande_lignes.txt : idCommande;referenceProduit;quantite;prixUnitaire
// Séparer les lignes de commande dans leur propre fichier évite d'avoir à imbriquer
// un délimiteur dans un autre pour représenter une liste de longueur variable.
class FichierTexteCommandeRepository : public ICommandeRepository
{
    private:
        std::string cheminCommandes;
        std::string cheminLignes;
        static constexpr char DELIMITEUR = ';';

        static std::string ech(const std::string& t) { return FormatTexte::echapper(t, DELIMITEUR); }

    public:
        FichierTexteCommandeRepository(std::string cheminCmd, std::string cheminLgn)
            : cheminCommandes(std::move(cheminCmd)), cheminLignes(std::move(cheminLgn)) {}

        void sauvegarder(const std::vector<Commande>& commandes) override
        {
            std::ofstream fCmd(cheminCommandes, std::ios::trunc);
            if (!fCmd)
                throw FormatFichierInvalideException("impossible d'ouvrir '" + cheminCommandes + "' en écriture");
            std::ofstream fLgn(cheminLignes, std::ios::trunc);
            if (!fLgn)
                throw FormatFichierInvalideException("impossible d'ouvrir '" + cheminLignes + "' en écriture");

            for (const auto& c : commandes) {
                fCmd << c.getIdCommande() << DELIMITEUR << c.getIdFournisseur() << DELIMITEUR
                     << DateUtils::formaterIso(c.getDateCommande()) << DELIMITEUR
                     << libelle(c.getStatut()) << "\n";

                for (const auto& l : c.getLignes())
                    fLgn << c.getIdCommande() << DELIMITEUR << ech(l.getReferenceProduit()) << DELIMITEUR
                         << l.getQuantite() << DELIMITEUR << l.getPrixUnitaire() << "\n";
            }
        }

        std::vector<Commande> charger() override
        {
            std::vector<Commande> commandes;
            std::ifstream fCmd(cheminCommandes);
            if (!fCmd) return commandes; // Aucun fichier : aucune commande, pas une erreur.

            std::string ligne;
            while (std::getline(fCmd, ligne)) {
                if (ligne.empty()) continue;
                auto champs = FormatTexte::decouper(ligne, DELIMITEUR);
                if (champs.size() != 4)
                    throw FormatFichierInvalideException("ligne de commande mal formée : " + ligne);

                try {
                    auto date = DateUtils::parserIso(champs[2]);
                    if (!date.has_value())
                        throw std::invalid_argument("date invalide : " + champs[2]);

                    commandes.emplace_back(std::stoi(champs[0]), std::stoi(champs[1]), *date,
                                            statutDepuisTexte(champs[3]));
                } catch (const std::invalid_argument& e) {
                    throw FormatFichierInvalideException(std::string(e.what()) + " (ligne : " + ligne + ")");
                } catch (const std::out_of_range&) {
                    throw FormatFichierInvalideException("valeur numérique hors limites (ligne : " + ligne + ")");
                }
            }

            // Index par id pour rattacher chaque ligne à sa commande en une passe.
            std::map<int, Commande*> parId;
            for (auto& c : commandes)
                parId[c.getIdCommande()] = &c;

            std::ifstream fLgn(cheminLignes);
            if (!fLgn) return commandes; // Des commandes sans aucune ligne, c'est possible et valide.

            while (std::getline(fLgn, ligne)) {
                if (ligne.empty()) continue;
                auto champs = FormatTexte::decouper(ligne, DELIMITEUR);
                if (champs.size() != 4)
                    throw FormatFichierInvalideException("ligne de commande_lignes mal formée : " + ligne);

                try {
                    int idCommande = std::stoi(champs[0]);
                    auto it = parId.find(idCommande);
                    if (it == parId.end())
                        throw FormatFichierInvalideException("ligne orpheline, commande #" +
                                                              std::to_string(idCommande) + " introuvable");

                    it->second->ajouterLigne(LigneCommande(champs[1], std::stoi(champs[2]), std::stod(champs[3])));
                } catch (const std::invalid_argument&) {
                    throw FormatFichierInvalideException("valeur numérique invalide (ligne : " + ligne + ")");
                } catch (const std::out_of_range&) {
                    throw FormatFichierInvalideException("valeur numérique hors limites (ligne : " + ligne + ")");
                }
            }
            return commandes;
        }
};