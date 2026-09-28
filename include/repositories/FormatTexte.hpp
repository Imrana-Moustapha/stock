#pragma once

#include <string>
#include <vector>

#include "exceptions/Exceptions.hpp"

// Échappement/découpage partagés par les repositories à fichier texte délimité.
// Sans échappement, un ';' dans un nom de produit ou dans l'auteur d'un mouvement
// décalerait toutes les colonnes et rendrait la ligne illisible au rechargement.
// Règle : le délimiteur et le caractère d'échappement '\' sont précédés d'un '\'.
namespace FormatTexte
{
    inline constexpr char ECHAPPEMENT = '\\';

    inline std::string echapper(const std::string& texte, char delimiteur)
    {
        std::string resultat;
        resultat.reserve(texte.size());
        for (char c : texte) {
            if (c == ECHAPPEMENT || c == delimiteur)
                resultat += ECHAPPEMENT;
            resultat += c;
        }
        return resultat;
    }

    inline std::vector<std::string> decouper(const std::string& ligne, char delimiteur)
    {
        std::vector<std::string> champs;
        std::string courant;
        bool echappe = false;

        for (char c : ligne) {
            if (echappe) {
                courant += c;
                echappe = false;
            } else if (c == ECHAPPEMENT) {
                echappe = true;
            } else if (c == delimiteur) {
                champs.push_back(courant);
                courant.clear();
            } else {
                courant += c;
            }
        }
        if (echappe)
            throw FormatFichierInvalideException("séquence d'échappement incomplète : " + ligne);

        champs.push_back(courant);
        return champs;
    }
}