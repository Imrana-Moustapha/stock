#include "vendor/catch_amalgamated.hpp"

#include "ui/Saisie.hpp"
#include "support/Support.hpp"

using Support::FluxSimules;
using Support::compter;

// Ces tests remplacent std::cin/std::cout le temps de chaque TEST_CASE (via FluxSimules) :
// aucune saisie clavier réelle n'est nécessaire pour les exécuter.

TEST_CASE("lireTexte ignore les lignes vides et retire les espaces", "[saisie]")
{
    FluxSimules flux("\n   \n  Bonjour  \n");
    REQUIRE(lireTexte("> ") == "Bonjour");
    REQUIRE(compter(flux.affiche(), "> ") == 3);   // reproposé après chaque ligne vide
}

TEST_CASE("lireEntier redemande tant que ce n'est pas un entier", "[saisie]")
{
    FluxSimules flux("abc\n12.5\n \n42\n");
    REQUIRE(lireEntier("> ") == 42);
    REQUIRE(flux.affiche().find("nombre entier valide") != std::string::npos);
}

TEST_CASE("lireEntier refuse un nombre suivi de texte parasite", "[saisie]")
{
    FluxSimules flux("12abc\n7\n");
    REQUIRE(lireEntier("> ") == 7);
}

TEST_CASE("lireEntierNonNegatif accepte zéro mais refuse les négatifs", "[saisie]")
{
    {
        FluxSimules flux("0\n");
        REQUIRE(lireEntierNonNegatif("> ") == 0);
    }
    {
        FluxSimules flux("-5\n3\n");
        REQUIRE(lireEntierNonNegatif("> ") == 3);
    }
}

TEST_CASE("lireEntierStrictementPositif refuse zéro et les négatifs", "[saisie]")
{
    FluxSimules flux("0\n-1\n5\n");
    REQUIRE(lireEntierStrictementPositif("> ") == 5);
}

TEST_CASE("lireDouble accepte le point et la virgule décimale", "[saisie]")
{
    {
        FluxSimules flux("1.5\n");
        REQUIRE(lireDouble("> ") == Catch::Approx(1.5));
    }
    {
        FluxSimules flux("1,5\n");
        REQUIRE(lireDouble("> ") == Catch::Approx(1.5));
    }
}

TEST_CASE("lireDouble redemande sur une saisie invalide", "[saisie]")
{
    FluxSimules flux("abc\n\n3.25\n");
    REQUIRE(lireDouble("> ") == Catch::Approx(3.25));
}

TEST_CASE("lireDoubleNonNegatif refuse les valeurs négatives", "[saisie]")
{
    FluxSimules flux("-1.5\n2.0\n");
    REQUIRE(lireDoubleNonNegatif("> ") == Catch::Approx(2.0));
}

TEST_CASE("lireDate redemande tant que la date n'existe pas", "[saisie]")
{
    // 31 février puis mois 13, avant une date valide
    FluxSimules flux("2026\n2\n31\n2026\n13\n1\n2026\n10\n15\n");
    auto d = lireDate("Date");
    REQUIRE(d == Support::date(2026, 10, 15));
    REQUIRE(compter(flux.affiche(), "n'existe pas") == 2);
}

TEST_CASE("lireChoix lit un entier valide et retourne true", "[saisie]")
{
    FluxSimules flux("4\n");
    int choix = -1;
    REQUIRE(lireChoix(choix));
    REQUIRE(choix == 4);
}

TEST_CASE("lireChoix retourne false sur une saisie invalide", "[saisie]")
{
    FluxSimules flux("abc\n\n");   // la ligne vide simule l'appui sur Entrée demandé par attendreEntree()
    int choix = -1;
    REQUIRE_FALSE(lireChoix(choix));
    REQUIRE(flux.affiche().find("Erreur de saisie") != std::string::npos);
}

TEST_CASE("Une entrée standard fermée lève EntreeInterrompue plutôt que de boucler", "[saisie]")
{
    FluxSimules flux("");   // flux vide : EOF immédiat
    REQUIRE_THROWS_AS(lireTexte("> "), EntreeInterrompue);
}

TEST_CASE("EntreeInterrompue est levée même après des tentatives invalides", "[saisie]")
{
    FluxSimules flux("abc\nxyz\n");   // deux tentatives invalides, puis EOF
    REQUIRE_THROWS_AS(lireEntier("> "), EntreeInterrompue);
}