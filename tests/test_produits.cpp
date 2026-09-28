#include "vendor/catch_amalgamated.hpp"

#include "factories/ProduitFactory.hpp"
#include "models/Produit.hpp"
#include "support/Support.hpp"

#include <stdexcept>

using Support::date;

// ---------------------------------------------------------------------------------------
// Modèle
// ---------------------------------------------------------------------------------------

TEST_CASE("Produit : accesseurs et modification", "[produit]")
{
    Produit p("R1", "Clavier", "Info", 10.0, 15.0, 8, 3);
    REQUIRE(p.getReference() == "R1");
    REQUIRE(p.getNom() == "Clavier");
    REQUIRE(p.getCategorie() == "Info");
    REQUIRE(p.getQuantiteStock() == 8);

    p.setNom("Clavier mécanique");
    p.setCategorie("Périphériques");
    p.setPrixAchat(20.0);
    p.setPrixVente(30.0);
    p.setSeuilAlerte(5);
    REQUIRE(p.getNom() == "Clavier mécanique");
    REQUIRE(p.getCategorie() == "Périphériques");
    REQUIRE(p.getPrixAchat() == Catch::Approx(20.0));
    REQUIRE(p.getPrixVente() == Catch::Approx(30.0));
    REQUIRE(p.getSeuilAlerte() == 5);
}

TEST_CASE("Produit::estSousLeSeuil vaut vrai dès que le stock atteint le seuil", "[produit]")
{
    Produit p("R1", "X", "C", 1, 2, 4, 3);
    REQUIRE_FALSE(p.estSousLeSeuil());   // 4 > 3
    p.setQuantiteStock(3);
    REQUIRE(p.estSousLeSeuil());         // 3 == seuil : déjà en alerte
    p.setQuantiteStock(0);
    REQUIRE(p.estSousLeSeuil());
}

TEST_CASE("ProduitPerissable refuse une date qui n'existe pas", "[produit][dates]")
{
    REQUIRE_THROWS_AS(ProduitPerissable("R", "N", "C", 1, 2, 3, 1, date(2026, 2, 31)), std::invalid_argument);
    REQUIRE_THROWS_AS(ProduitPerissable("R", "N", "C", 1, 2, 3, 1, date(2026, 13, 5)), std::invalid_argument);

    ProduitPerissable p("R", "N", "C", 1, 2, 3, 1, date(2026, 10, 15));
    REQUIRE_THROWS_AS(p.setDatePeremption(date(2026, 4, 31)), std::invalid_argument);
    REQUIRE(p.getDatePeremption() == date(2026, 10, 15));   // inchangée après le refus

    p.setDatePeremption(date(2027, 1, 1));
    REQUIRE(p.getDatePeremption() == date(2027, 1, 1));
}

TEST_CASE("ProduitPerissable::joursAvantPeremption", "[produit][dates]")
{
    ProduitPerissable p("R", "N", "C", 1, 2, 3, 1, date(2026, 10, 15));
    REQUIRE(p.joursAvantPeremption(date(2026, 10, 1)) == 14);
    REQUIRE(p.joursAvantPeremption(date(2026, 10, 15)) == 0);
    REQUIRE(p.joursAvantPeremption(date(2026, 10, 20)) == -5);   // déjà périmé
    REQUIRE(p.joursAvantPeremption(date(2026, 2, 28)) == 229);   // traverse des mois de longueurs différentes
}

// ---------------------------------------------------------------------------------------
// Fabrique
// ---------------------------------------------------------------------------------------

TEST_CASE("ProduitFactory crée le bon type de produit", "[factory]")
{
    auto standard = ProduitFactory::creerProduit(TypeProduit::STANDARD, "R1", "Nom", "Cat", 1, 2, 3, 1);
    REQUIRE(dynamic_cast<ProduitPerissable*>(standard.get()) == nullptr);
    REQUIRE(standard->getReference() == "R1");

    auto perissable = ProduitFactory::creerProduit(TypeProduit::PERISSABLE, "R2", "Nom", "Cat", 1, 2, 3, 1,
                                                    date(2026, 10, 15));
    auto* p = dynamic_cast<ProduitPerissable*>(perissable.get());
    REQUIRE(p != nullptr);
    REQUIRE(p->getDatePeremption() == date(2026, 10, 15));
}

TEST_CASE("ProduitFactory exige une date pour un périssable", "[factory]")
{
    REQUIRE_THROWS_AS(ProduitFactory::creerProduit(TypeProduit::PERISSABLE, "R", "N", "C", 1, 2, 3, 1),
                      std::invalid_argument);
}

TEST_CASE("ProduitFactory refuse une date impossible", "[factory][dates]")
{
    REQUIRE_THROWS_AS(ProduitFactory::creerProduit(TypeProduit::PERISSABLE, "R", "N", "C", 1, 2, 3, 1,
                                                    date(2026, 2, 31)),
                      std::invalid_argument);
}

TEST_CASE("ProduitFactory refuse les valeurs incohérentes", "[factory]")
{
    using T = TypeProduit;
    REQUIRE_THROWS_AS(ProduitFactory::creerProduit(T::STANDARD, "", "N", "C", 1, 2, 3, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(ProduitFactory::creerProduit(T::STANDARD, "R", "", "C", 1, 2, 3, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(ProduitFactory::creerProduit(T::STANDARD, "R", "N", "C", -1, 2, 3, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(ProduitFactory::creerProduit(T::STANDARD, "R", "N", "C", 1, -2, 3, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(ProduitFactory::creerProduit(T::STANDARD, "R", "N", "C", 1, 2, -3, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(ProduitFactory::creerProduit(T::STANDARD, "R", "N", "C", 1, 2, 3, -1), std::invalid_argument);

    // 0 est une valeur légitime (produit gratuit, stock épuisé, pas de seuil)
    REQUIRE_NOTHROW(ProduitFactory::creerProduit(T::STANDARD, "R", "N", "C", 0, 0, 0, 0));
}

TEST_CASE("ProduitFactory::typeDepuisTexte", "[factory]")
{
    REQUIRE(ProduitFactory::typeDepuisTexte("STANDARD") == TypeProduit::STANDARD);
    REQUIRE(ProduitFactory::typeDepuisTexte("PERISSABLE") == TypeProduit::PERISSABLE);
    REQUIRE_THROWS_AS(ProduitFactory::typeDepuisTexte("standard"), std::invalid_argument);   // sensible à la casse
    REQUIRE_THROWS_AS(ProduitFactory::typeDepuisTexte("ELECTRONIQUE"), std::invalid_argument);
}