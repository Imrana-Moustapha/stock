#pragma once

#include <string>
#include <vector>

// CSV conforme à l'usage courant (RFC 4180, une ligne à la fois) :
// un champ contenant une virgule, un guillemet ou un saut de ligne est entouré de
// guillemets, et les guillemets internes sont doublés. Sans ça, un nom de produit
// comme « Câble USB, 2 m » décalerait toutes les colonnes de la ligne.
namespace Csv
{
    inline std::string echapper(const std::string& champ)
    {
        if (champ.find_first_of(",\"\n\r") == std::string::npos)
            return champ;

        std::string resultat = "\"";
        for (char c : champ) {
            if (c == '"') resultat += '"';
            resultat += c;
        }
        resultat += '"';
        return resultat;
    }

    inline std::vector<std::string> decouper(const std::string& ligne)
    {
        std::vector<std::string> champs;
        std::string courant;
        bool entreGuillemets = false;

        for (std::size_t i = 0; i < ligne.size(); ++i) {
            char c = ligne[i];
            if (entreGuillemets) {
                if (c == '"') {
                    if (i + 1 < ligne.size() && ligne[i + 1] == '"') { courant += '"'; ++i; }
                    else entreGuillemets = false;
                } else {
                    courant += c;
                }
            } else if (c == '"') {
                entreGuillemets = true;
            } else if (c == ',') {
                champs.push_back(courant);
                courant.clear();
            } else if (c != '\r') {   // tolère les fins de ligne Windows
                courant += c;
            }
        }
        champs.push_back(courant);
        return champs;
    }
}