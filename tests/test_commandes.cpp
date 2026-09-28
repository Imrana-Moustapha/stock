#include "vendor/catch_amalgamated.hpp"

#include "services/GestionnaireCommandes.hpp"
#include "support/Support.hpp"

#include <stdexcept>

using Support::date;

TEST_CASE("GestionnaireCommandes : ajouter et retrouver un fournisseur", "[commandes]")
{
    GestionnaireCommandes gc;
    int id = gc.ajouterFournisseur("FournTech", "01 02 03", "12 rue X");

    REQUIRE(id == 1);
    REQUIRE(gc.getFournisseurs().size() == 1);
    const auto* f = gc.trouverFournisseur(id);
    REQUIRE(f != nullptr);
    REQUIRE(f->getNom() == "FournTech");
    REQUIRE(gc.trouverFournisseur(999) == nullptr);
}

TEST_CASE("GestionnaireCommandes : les id de fournisseurs s'incrémentent", "[commandes]")
{
    GestionnaireCommandes gc;
    int a = gc.ajouterFournisseur("A", "x", "x");
    int b = gc.ajouterFournisseur("B", "x", "x");
    REQUIRE(b == a + 1);
}

TEST_CASE("GestionnaireCommandes refuse une commande vers un fournisseur inconnu", "[commandes]")
{
    GestionnaireCommandes gc;
    REQUIRE_THROWS_AS(gc.creerCommande(1, date(2026, 1, 1)), std::invalid_argument);
}

TEST_CASE("GestionnaireCommandes refuse une date de commande invalide", "[commandes][dates]")
{
    GestionnaireCommandes gc;
    int f = gc.ajouterFournisseur("A", "x", "x");
    REQUIRE_THROWS_AS(gc.creerCommande(f, Support::date(2026, 2, 30)), std::invalid_argument);
}

TEST_CASE("GestionnaireCommandes : créer une commande et lui ajouter des lignes", "[commandes]")
{
    GestionnaireCommandes gc;
    int f = gc.ajouterFournisseur("A", "x", "x");
    int idCmd = gc.creerCommande(f, date(2026, 1, 1));

    gc.ajouterLigneCommande(idCmd, "R1", 10, 5.0);
    gc.ajouterLigneCommande(idCmd, "R2", 3, 2.0);

    Commande* cmd = gc.trouverCommande(idCmd);
    REQUIRE(cmd != nullptr);
    REQUIRE(cmd->getLignes().size() == 2);
    REQUIRE(cmd->getMontantTotal() == Catch::Approx(56.0));   // 10*5 + 3*2
    REQUIRE(cmd->getStatut() == StatutCommande::EN_COURS);
}

TEST_CASE("GestionnaireCommandes refuse une ligne sur une commande inconnue", "[commandes]")
{
    GestionnaireCommandes gc;
    REQUIRE_THROWS_AS(gc.ajouterLigneCommande(1, "R1", 1, 1.0), std::invalid_argument);
}

TEST_CASE("GestionnaireCommandes : changer le statut d'une commande", "[commandes]")
{
    GestionnaireCommandes gc;
    int f = gc.ajouterFournisseur("A", "x", "x");
    int idCmd = gc.creerCommande(f, date(2026, 1, 1));

    gc.changerStatut(idCmd, StatutCommande::LIVREE);
    REQUIRE(gc.trouverCommande(idCmd)->getStatut() == StatutCommande::LIVREE);

    REQUIRE_THROWS_AS(gc.changerStatut(999, StatutCommande::ANNULEE), std::invalid_argument);
}

TEST_CASE("GestionnaireCommandes : une commande sans ligne a un total de zéro", "[commandes]")
{
    GestionnaireCommandes gc;
    int f = gc.ajouterFournisseur("A", "x", "x");
    int idCmd = gc.creerCommande(f, date(2026, 1, 1));

    REQUIRE(gc.trouverCommande(idCmd)->getMontantTotal() == Catch::Approx(0.0));
}