#include "vendor/catch_amalgamated.hpp"

#include "utils/DateUtils.hpp"

using Catch::Matchers::Matches;

TEST_CASE("DateUtils::creer accepte une date qui existe", "[dates]")
{
    auto d = DateUtils::creer(2026, 10, 15);
    REQUIRE(d.has_value());
    REQUIRE(static_cast<int>(d->year()) == 2026);
    REQUIRE(static_cast<unsigned>(d->month()) == 10);
    REQUIRE(static_cast<unsigned>(d->day()) == 15);
}

TEST_CASE("DateUtils::creer gère les années bissextiles", "[dates]")
{
    REQUIRE(DateUtils::creer(2028, 2, 29).has_value());        // divisible par 4
    REQUIRE(DateUtils::creer(2000, 2, 29).has_value());        // divisible par 400
    REQUIRE_FALSE(DateUtils::creer(2026, 2, 29).has_value());  // année ordinaire
    REQUIRE_FALSE(DateUtils::creer(2100, 2, 29).has_value());  // divisible par 100 mais pas par 400
}

TEST_CASE("DateUtils::creer refuse les dates impossibles", "[dates]")
{
    auto [annee, mois, jour] = GENERATE(table<int, int, int>({
        {2026, 13, 1},    // mois 13
        {2026, 0, 1},     // mois 0
        {2026, 1, 0},     // jour 0
        {2026, 1, 32},    // jour 32
        {2026, 2, 30},    // 30 février
        {2026, 2, 31},    // 31 février
        {2026, 4, 31},    // avril n'a que 30 jours
        {2026, 257, 5},   // ne doit PAS être tronqué en mois 1 (month est stocké sur un octet)
        {2026, 5, 257},   // ne doit PAS être tronqué en jour 1
        {1899, 1, 1},     // hors de la plage acceptée
        {2201, 1, 1},
    }));

    INFO("date testée : " << annee << "-" << mois << "-" << jour);
    REQUIRE_FALSE(DateUtils::creer(annee, mois, jour).has_value());
}

TEST_CASE("DateUtils::parserIso lit le format AAAA-MM-JJ", "[dates]")
{
    auto d = DateUtils::parserIso("2026-10-15");
    REQUIRE(d.has_value());
    REQUIRE(*d == DateUtils::creer(2026, 10, 15).value());

    REQUIRE(DateUtils::parserIso("2026-1-5").has_value());   // zéros de tête facultatifs
}

TEST_CASE("DateUtils::parserIso refuse les textes invalides", "[dates]")
{
    auto texte = GENERATE(as<std::string>{},
        "", "abc", "2026", "2026-10", "2026/10/15", "2026-13-01", "2026-02-31", "2026-10-15x", "15-10-2026x");

    INFO("texte testé : '" << texte << "'");
    REQUIRE_FALSE(DateUtils::parserIso(texte).has_value());
}

TEST_CASE("DateUtils::formaterIso complète avec des zéros et fait l'aller-retour", "[dates]")
{
    auto d = DateUtils::creer(2026, 3, 5).value();
    REQUIRE(DateUtils::formaterIso(d) == "2026-03-05");
    REQUIRE(DateUtils::parserIso(DateUtils::formaterIso(d)).value() == d);
}

TEST_CASE("DateUtils::formaterDateHeure produit JJ/MM/AAAA HH:MM:SS", "[dates]")
{
    auto texte = DateUtils::formaterDateHeure(std::chrono::system_clock::now());
    REQUIRE_THAT(texte, Matches("[0-9]{2}/[0-9]{2}/[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}"));
}