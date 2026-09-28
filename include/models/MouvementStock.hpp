#pragma once

#include <string>
#include <chrono>
#include <utility>

enum class TypeMouvement
{
    ENTREE,
    SORTIE,
    AJUSTEMENT
};

// Libellé texte d'un type de mouvement (affichage, exports, fichiers de données).
inline const char* libelle(TypeMouvement type)
{
    switch (type) {
        case TypeMouvement::ENTREE:     return "ENTREE";
        case TypeMouvement::SORTIE:     return "SORTIE";
        case TypeMouvement::AJUSTEMENT: return "AJUSTEMENT";
    }
    return "?";
}

class MouvementStock
{
    private:
        int id;
        std::string referenceProduit;
        TypeMouvement type;
        int quantite;
        std::chrono::system_clock::time_point dateHeure;
        std::string auteur;

    public:
        MouvementStock(int idMouv, std::string refProd, TypeMouvement t, int qte,
                        std::chrono::system_clock::time_point dt, std::string aut)
            : id(idMouv), referenceProduit(std::move(refProd)), type(t), quantite(qte),
              dateHeure(dt), auteur(std::move(aut)) {}

        int getId() const { return id; }
        const std::string& getReferenceProduit() const { return referenceProduit; }
        TypeMouvement getType() const { return type; }
        int getQuantite() const { return quantite; }
        const std::chrono::system_clock::time_point& getDateHeure() const { return dateHeure; }
        const std::string& getAuteur() const { return auteur; }
};