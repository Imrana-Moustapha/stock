#pragma once

#include <string>
#include <utility>

class Fournisseur
{
    private:
        int id;
        std::string nom;
        std::string contact;
        std::string adresse;

    public:
        Fournisseur(int idF, std::string n, std::string c, std::string ad)
            : id(idF), nom(std::move(n)), contact(std::move(c)), adresse(std::move(ad)) {}

        int getId() const { return id; }
        const std::string& getNom() const { return nom; }
        const std::string& getContact() const { return contact; }
        const std::string& getAdresse() const { return adresse; }
};