#include "vendor/catch_amalgamated.hpp"

#include "observers/Logger.hpp"
#include "support/Support.hpp"

using namespace Support;

TEST_CASE("Logger écrit une ligne à chaque notification", "[logger]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("alertes.log");
    Logger logger(chemin);

    Produit p("R1", "Souris", "Info", 1, 2, 3, 5);
    logger.notifierSeuilCritique(p);

    std::string contenu = lire(chemin);
    REQUIRE(contenu.find("R1") != std::string::npos);
    REQUIRE(contenu.find("Souris") != std::string::npos);
    REQUIRE(contenu.find("ALERTE SEUIL CRITIQUE") != std::string::npos);
}

TEST_CASE("Logger ajoute au fichier plutôt que de l'écraser", "[logger]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("alertes.log");
    Logger logger(chemin);

    Produit p1("R1", "A", "C", 1, 2, 3, 5);
    Produit p2("R2", "B", "C", 1, 2, 3, 5);
    logger.notifierSeuilCritique(p1);
    logger.notifierSeuilCritique(p2);

    std::string contenu = lire(chemin);
    REQUIRE(contenu.find("R1") != std::string::npos);
    REQUIRE(contenu.find("R2") != std::string::npos);
    REQUIRE(compter(contenu, "ALERTE SEUIL CRITIQUE") == 2);
}

TEST_CASE("Logger : GestionnaireStock déclenche bien l'écriture au bon moment", "[logger][stock]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("alertes.log");
    Logger logger(chemin);

    BancDeTest banc;
    banc.gestionnaire.ajouterObservateur(&logger);
    banc.gestionnaire.ajouterProduit(produit("R1", 10, 3));

    banc.gestionnaire.retirerStock("R1", 5);   // reste 5 : pas d'alerte
    REQUIRE(lire(chemin).empty());

    banc.gestionnaire.retirerStock("R1", 2);   // reste 3 : alerte
    REQUIRE(lire(chemin).find("R1") != std::string::npos);
}