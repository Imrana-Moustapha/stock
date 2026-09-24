#pragma once

#include <exception>
#include <string>
#include <utility>

// Exception levée lorsqu'une sortie de stock demande plus que la quantité disponible.
class StockInsuffisantException : public std::exception
{
    private:
        std::string message;

    public:
        StockInsuffisantException(const std::string& reference, int demande, int disponible)
            : message("Stock insuffisant pour '" + reference + "' : demande=" + std::to_string(demande)
                       + ", disponible=" + std::to_string(disponible)) {}

        const char* what() const noexcept override { return message.c_str(); }
};

// Exception levée lorsqu'un produit référencé n'existe pas dans le stock.
class ProduitIntrouvableException : public std::exception
{
    private:
        std::string message;

    public:
        explicit ProduitIntrouvableException(const std::string& reference)
            : message("Produit introuvable : " + reference) {}

        const char* what() const noexcept override { return message.c_str(); }
};

// Exception levée lors d'un échec de lecture/écriture d'un fichier de persistance
// (format inattendu, colonnes manquantes, fichier corrompu...).
class FormatFichierInvalideException : public std::exception
{
    private:
        std::string message;

    public:
        explicit FormatFichierInvalideException(std::string detail)
            : message("Format de fichier invalide : " + std::move(detail)) {}

        const char* what() const noexcept override { return message.c_str(); }
};