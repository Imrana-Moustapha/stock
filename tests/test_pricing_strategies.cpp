#include "vendor/catch_amalgamated.hpp"

#include "services/PricingStrategies.hpp"
#include "support/Support.hpp"

#include <stdexcept>

using namespace Support;

TEST_CASE("PrixStandard renvoie le prix de vente inchangé", "[pricing]")
{
    auto p = produit("R1", 10, 3, "Cat", 10.0);   // prixVente = 15.0 (voir Support::produit)
    PrixStandard strategie;
    REQUIRE(strategie.calculerPrix(*p) == Catch::Approx(15.0));
    REQUIRE(strategie.nom() == "Prix standard");
}

TEST_CASE("SoldeNoel applique un pourcentage de remise", "[pricing]")
{
    Produit p("R1", "N", "C", 10, 100, 5, 1);
    SoldeNoel solde(20.0);
    REQUIRE(solde.calculerPrix(p) == Catch::Approx(80.0));

    SoldeNoel gratuit(100.0);
    REQUIRE(gratuit.calculerPrix(p) == Catch::Approx(0.0));

    SoldeNoel aucune(0.0);
    REQUIRE(aucune.calculerPrix(p) == Catch::Approx(100.0));
}

TEST_CASE("SoldeNoel refuse un pourcentage hors de [0, 100]", "[pricing]")
{
    REQUIRE_THROWS_AS(SoldeNoel(-1.0), std::invalid_argument);
    REQUIRE_THROWS_AS(SoldeNoel(101.0), std::invalid_argument);
    REQUIRE_NOTHROW(SoldeNoel(0.0));
    REQUIRE_NOTHROW(SoldeNoel(100.0));
}

TEST_CASE("RemiseFidelite soustrait un montant fixe", "[pricing]")
{
    Produit p("R1", "N", "C", 10, 50, 5, 1);
    RemiseFidelite remise(15.0);
    REQUIRE(remise.calculerPrix(p) == Catch::Approx(35.0));
}

TEST_CASE("RemiseFidelite ne rend jamais un prix négatif", "[pricing]")
{
    Produit p("R1", "N", "C", 10, 20, 5, 1);
    RemiseFidelite remise(1000.0);
    REQUIRE(remise.calculerPrix(p) == Catch::Approx(0.0));
}

TEST_CASE("RemiseFidelite refuse un montant négatif", "[pricing]")
{
    REQUIRE_THROWS_AS(RemiseFidelite(-1.0), std::invalid_argument);
    REQUIRE_NOTHROW(RemiseFidelite(0.0));
}

TEST_CASE("Les stratégies sont interchangeables via l'interface commune", "[pricing]")
{
    Produit p("R1", "N", "C", 10, 100, 5, 1);
    std::vector<std::unique_ptr<IPricingStrategy>> strategies;
    strategies.push_back(std::make_unique<PrixStandard>());
    strategies.push_back(std::make_unique<SoldeNoel>(50.0));
    strategies.push_back(std::make_unique<RemiseFidelite>(30.0));

    std::vector<double> resultats;
    for (const auto& s : strategies)
        resultats.push_back(s->calculerPrix(p));

    REQUIRE(resultats == std::vector<double>{100.0, 50.0, 70.0});
}