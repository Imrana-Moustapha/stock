#include "vendor/catch_amalgamated.hpp"

#include "support/Support.hpp"

#include <stdexcept>

using namespace Support;

// ---------------------------------------------------------------------------------------
// Produits
// ---------------------------------------------------------------------------------------

TEST_CASE("GestionnaireStock : ajouter un produit le sauvegarde", "[stock]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;

    g.ajouterProduit(produit("R1"));

    REQUIRE(g.nombreDeProduits() == 1);
    REQUIRE(g.trouverProduit("R1") != nullptr);
    REQUIRE(banc.sauvegardes() == 1);
    REQUIRE(banc.depotProduits->disque.size() == 1);
}

TEST_CASE("GestionnaireStock refuse une référence déjà utilisée", "[stock]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1"));
    int avant = banc.sauvegardes();

    REQUIRE_THROWS_AS(g.ajouterProduit(produit("R1")), ProduitDejaExistantException);

    REQUIRE(g.nombreDeProduits() == 1);
    REQUIRE(banc.sauvegardes() == avant);   // rien n'a été écrit
}

TEST_CASE("GestionnaireStock refuse un produit nul", "[stock]")
{
    BancDeTest banc;
    REQUIRE_THROWS_AS(banc.gestionnaire.ajouterProduit(nullptr), std::invalid_argument);
}

TEST_CASE("GestionnaireStock : trouver un produit", "[stock]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1"));

    REQUIRE(g.trouverProduit("R1")->getReference() == "R1");
    REQUIRE(g.trouverProduit("ABSENT") == nullptr);
    REQUIRE(g.trouverProduitOuLever("R1").getReference() == "R1");
    REQUIRE_THROWS_AS(g.trouverProduitOuLever("ABSENT"), ProduitIntrouvableException);
}

TEST_CASE("GestionnaireStock : supprimer un produit", "[stock]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1"));
    g.ajouterProduit(produit("R2"));

    g.supprimerProduit("R1");
    REQUIRE(g.nombreDeProduits() == 1);
    REQUIRE(g.trouverProduit("R1") == nullptr);
    REQUIRE(g.trouverProduit("R2") != nullptr);
    REQUIRE(banc.depotProduits->disque.size() == 1);

    REQUIRE_THROWS_AS(g.supprimerProduit("R1"), ProduitIntrouvableException);
}

// ---------------------------------------------------------------------------------------
// Mouvements
// ---------------------------------------------------------------------------------------

TEST_CASE("GestionnaireStock : une entrée augmente le stock et laisse une trace", "[stock][mouvements]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1", 10));

    g.ajouterStock("R1", 5, "Réception fournisseur");

    REQUIRE(g.trouverProduit("R1")->getQuantiteStock() == 15);
    REQUIRE(g.getHistorique().size() == 1);
    const auto& mvt = g.getHistorique()[0];
    REQUIRE(mvt.getType() == TypeMouvement::ENTREE);
    REQUIRE(mvt.getReferenceProduit() == "R1");
    REQUIRE(mvt.getQuantite() == 5);
    REQUIRE(mvt.getAuteur() == "Réception fournisseur");
    REQUIRE(banc.depotMouvements->disque.size() == 1);   // le journal est sauvegardé aussi
}

TEST_CASE("GestionnaireStock : une sortie diminue le stock", "[stock][mouvements]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1", 10));

    g.retirerStock("R1", 4, "Vente");

    REQUIRE(g.trouverProduit("R1")->getQuantiteStock() == 6);
    REQUIRE(g.getHistorique()[0].getType() == TypeMouvement::SORTIE);
    REQUIRE(g.getHistorique()[0].getQuantite() == 4);
}

TEST_CASE("GestionnaireStock refuse une sortie supérieure au stock", "[stock][mouvements]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1", 10));
    int avant = banc.sauvegardes();

    REQUIRE_THROWS_AS(g.retirerStock("R1", 11), StockInsuffisantException);

    REQUIRE(g.trouverProduit("R1")->getQuantiteStock() == 10);   // inchangé
    REQUIRE(g.getHistorique().empty());                          // aucune trace d'un mouvement qui n'a pas eu lieu
    REQUIRE(banc.sauvegardes() == avant);

    REQUIRE_NOTHROW(g.retirerStock("R1", 10));                   // sortir exactement tout le stock est permis
    REQUIRE(g.trouverProduit("R1")->getQuantiteStock() == 0);
}

TEST_CASE("GestionnaireStock refuse les quantités nulles ou négatives", "[stock][mouvements]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1", 10));

    int quantite = GENERATE(0, -1, -100);
    REQUIRE_THROWS_AS(g.ajouterStock("R1", quantite), std::invalid_argument);
    REQUIRE_THROWS_AS(g.retirerStock("R1", quantite), std::invalid_argument);

    REQUIRE(g.trouverProduit("R1")->getQuantiteStock() == 10);
    REQUIRE(g.getHistorique().empty());
}

TEST_CASE("GestionnaireStock : mouvement sur un produit inconnu", "[stock][mouvements]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;

    REQUIRE_THROWS_AS(g.ajouterStock("ABSENT", 1), ProduitIntrouvableException);
    REQUIRE_THROWS_AS(g.retirerStock("ABSENT", 1), ProduitIntrouvableException);
    REQUIRE_THROWS_AS(g.ajusterStock("ABSENT", 1), ProduitIntrouvableException);
    REQUIRE(g.getHistorique().empty());
}

TEST_CASE("GestionnaireStock : un ajustement fixe une valeur absolue et journalise l'écart", "[stock][mouvements]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1", 10));

    g.ajusterStock("R1", 25, "Inventaire");    // +15
    g.ajusterStock("R1", 20, "Inventaire");    // -5

    REQUIRE(g.trouverProduit("R1")->getQuantiteStock() == 20);
    REQUIRE(g.getHistorique().size() == 2);
    REQUIRE(g.getHistorique()[0].getType() == TypeMouvement::AJUSTEMENT);
    REQUIRE(g.getHistorique()[0].getQuantite() == 15);
    REQUIRE(g.getHistorique()[1].getQuantite() == -5);   // l'écart peut être négatif

    REQUIRE_THROWS_AS(g.ajusterStock("R1", -1), std::invalid_argument);
    REQUIRE_NOTHROW(g.ajusterStock("R1", 0));            // un inventaire peut trouver zéro
}

TEST_CASE("GestionnaireStock : les mouvements sont numérotés dans l'ordre", "[stock][mouvements]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1", 10));

    g.ajouterStock("R1", 1);
    g.retirerStock("R1", 1);
    g.ajusterStock("R1", 7);

    REQUIRE(g.getHistorique()[0].getId() == 1);
    REQUIRE(g.getHistorique()[1].getId() == 2);
    REQUIRE(g.getHistorique()[2].getId() == 3);
}

// ---------------------------------------------------------------------------------------
// Alertes de seuil (pattern Observer)
// ---------------------------------------------------------------------------------------

TEST_CASE("GestionnaireStock notifie les observateurs quand le stock atteint le seuil", "[stock][observer]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    ObservateurEnregistreur observateur;
    g.ajouterObservateur(&observateur);
    g.ajouterProduit(produit("R1", 10, 3));

    g.retirerStock("R1", 5);                      // reste 5 : au-dessus du seuil
    REQUIRE(observateur.references.empty());

    g.retirerStock("R1", 2);                      // reste 3 : seuil atteint
    REQUIRE(observateur.references == std::vector<std::string>{"R1"});

    g.retirerStock("R1", 1);                      // reste 2 : toujours sous le seuil
    REQUIRE(observateur.references.size() == 2);
}

TEST_CASE("GestionnaireStock ne notifie pas pour une entrée de stock", "[stock][observer]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    ObservateurEnregistreur observateur;
    g.ajouterObservateur(&observateur);
    g.ajouterProduit(produit("R1", 1, 3));        // déjà sous le seuil dès l'ajout

    g.ajouterStock("R1", 1);

    REQUIRE(observateur.references.empty());
}

TEST_CASE("GestionnaireStock notifie plusieurs observateurs, et aussi après un ajustement", "[stock][observer]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    ObservateurEnregistreur a, b;
    g.ajouterObservateur(&a);
    g.ajouterObservateur(&b);
    g.ajouterProduit(produit("R1", 10, 3));

    g.ajusterStock("R1", 1);

    REQUIRE(a.references.size() == 1);
    REQUIRE(b.references.size() == 1);
}

// ---------------------------------------------------------------------------------------
// Statistiques
// ---------------------------------------------------------------------------------------

TEST_CASE("GestionnaireStock : valeur du stock", "[stock][stats]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;

    REQUIRE(g.valeurTotaleStock() == Catch::Approx(0.0));

    g.ajouterProduit(produit("R1", 10, 3, "Info", 5.0));    // 50
    g.ajouterProduit(produit("R2", 4, 1, "Info", 2.5));     // 10
    g.ajouterProduit(produit("R3", 3, 1, "Alim", 20.0));    // 60

    REQUIRE(g.valeurTotaleStock() == Catch::Approx(120.0));

    auto parCategorie = g.valeurStockParCategorie();
    REQUIRE(parCategorie.size() == 2);
    REQUIRE(parCategorie["Info"] == Catch::Approx(60.0));
    REQUIRE(parCategorie["Alim"] == Catch::Approx(60.0));
}

TEST_CASE("GestionnaireStock : produits sous le seuil", "[stock][stats]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("OK", 10, 3));
    g.ajouterProduit(produit("LIMITE", 3, 3));
    g.ajouterProduit(produit("BAS", 1, 3));

    auto bas = g.produitsSousLeSeuil();
    REQUIRE(bas.size() == 2);
    REQUIRE(bas[0]->getReference() == "LIMITE");
    REQUIRE(bas[1]->getReference() == "BAS");
}

TEST_CASE("GestionnaireStock : produits dormants et nombre de mouvements", "[stock][stats]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("ACTIF", 10));
    g.ajouterProduit(produit("DORMANT", 10));

    g.ajouterStock("ACTIF", 1);
    g.retirerStock("ACTIF", 1);

    auto compteurs = g.nombreMouvementsParProduit();
    REQUIRE(compteurs.size() == 1);
    REQUIRE(compteurs["ACTIF"] == 2);

    auto dormants = g.produitsDormants();
    REQUIRE(dormants.size() == 1);
    REQUIRE(dormants[0]->getReference() == "DORMANT");
}

TEST_CASE("GestionnaireStock::definirSeuilPourTous", "[stock]")
{
    BancDeTest banc;
    auto& g = banc.gestionnaire;
    g.ajouterProduit(produit("R1", 10, 3));
    g.ajouterProduit(produit("R2", 10, 4));
    int avant = banc.sauvegardes();

    g.definirSeuilPourTous(8);

    REQUIRE(g.trouverProduit("R1")->getSeuilAlerte() == 8);
    REQUIRE(g.trouverProduit("R2")->getSeuilAlerte() == 8);
    REQUIRE(banc.sauvegardes() == avant + 1);
    REQUIRE_THROWS_AS(g.definirSeuilPourTous(-1), std::invalid_argument);
    REQUIRE(g.trouverProduit("R1")->getSeuilAlerte() == 8);   // inchangé après le refus
}

// ---------------------------------------------------------------------------------------
// Chargement
// ---------------------------------------------------------------------------------------

TEST_CASE("GestionnaireStock::charger reprend l'état sauvegardé", "[stock][chargement]")
{
    BancDeTest banc;
    banc.depotProduits->disque.push_back(produit("R1", 7));
    banc.depotProduits->disque.push_back(produit("R2", 9));

    banc.gestionnaire.charger();

    REQUIRE(banc.gestionnaire.nombreDeProduits() == 2);
    REQUIRE(banc.gestionnaire.trouverProduit("R2")->getQuantiteStock() == 9);
}

TEST_CASE("GestionnaireStock::charger poursuit la numérotation du journal", "[stock][chargement]")
{
    BancDeTest banc;
    banc.depotProduits->disque.push_back(produit("R1", 10));
    auto t = std::chrono::system_clock::now();
    banc.depotMouvements->disque.emplace_back(4, "R1", TypeMouvement::ENTREE, 1, t, "a");
    banc.depotMouvements->disque.emplace_back(7, "R1", TypeMouvement::SORTIE, 1, t, "b");

    banc.gestionnaire.charger();
    banc.gestionnaire.ajouterStock("R1", 1);

    const auto& h = banc.gestionnaire.getHistorique();
    REQUIRE(h.size() == 3);
    REQUIRE(h.back().getId() == 8);   // jamais un id déjà présent sur disque
}

TEST_CASE("GestionnaireStock::charger refuse des références en double", "[stock][chargement]")
{
    BancDeTest banc;
    banc.depotProduits->disque.push_back(produit("R1"));
    banc.depotProduits->disque.push_back(produit("R2"));
    banc.depotProduits->disque.push_back(produit("R1"));

    REQUIRE_THROWS_AS(banc.gestionnaire.charger(), FormatFichierInvalideException);
    REQUIRE(banc.gestionnaire.nombreDeProduits() == 0);   // aucun état à moitié chargé
}