#include "vendor/catch_amalgamated.hpp"

#include "services/GestionnaireCommandes.hpp"
#include "support/Support.hpp"

#include <stdexcept>

using Support::date;
using Support::BancDeTestCommandes;

TEST_CASE("GestionnaireCommandes : ajouter et retrouver un fournisseur", "[commandes]")
{
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
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
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
    int a = gc.ajouterFournisseur("A", "x", "x");
    int b = gc.ajouterFournisseur("B", "x", "x");
    REQUIRE(b == a + 1);
}

TEST_CASE("GestionnaireCommandes refuse une commande vers un fournisseur inconnu", "[commandes]")
{
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
    REQUIRE_THROWS_AS(gc.creerCommande(1, date(2026, 1, 1)), std::invalid_argument);
}

TEST_CASE("GestionnaireCommandes refuse une date de commande invalide", "[commandes][dates]")
{
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
    int f = gc.ajouterFournisseur("A", "x", "x");
    REQUIRE_THROWS_AS(gc.creerCommande(f, Support::date(2026, 2, 30)), std::invalid_argument);
}

TEST_CASE("GestionnaireCommandes : créer une commande et lui ajouter des lignes", "[commandes]")
{
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
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
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
    REQUIRE_THROWS_AS(gc.ajouterLigneCommande(1, "R1", 1, 1.0), std::invalid_argument);
}

TEST_CASE("GestionnaireCommandes : changer le statut d'une commande", "[commandes]")
{
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
    int f = gc.ajouterFournisseur("A", "x", "x");
    int idCmd = gc.creerCommande(f, date(2026, 1, 1));

    gc.changerStatut(idCmd, StatutCommande::LIVREE);
    REQUIRE(gc.trouverCommande(idCmd)->getStatut() == StatutCommande::LIVREE);

    REQUIRE_THROWS_AS(gc.changerStatut(999, StatutCommande::ANNULEE), std::invalid_argument);
}

TEST_CASE("GestionnaireCommandes : une commande sans ligne a un total de zéro", "[commandes]")
{
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
    int f = gc.ajouterFournisseur("A", "x", "x");
    int idCmd = gc.creerCommande(f, date(2026, 1, 1));

    REQUIRE(gc.trouverCommande(idCmd)->getMontantTotal() == Catch::Approx(0.0));
}

// ---------------------------------------------------------------------------------------
// Sauvegarde automatique
// ---------------------------------------------------------------------------------------

TEST_CASE("GestionnaireCommandes sauvegarde après chaque opération", "[commandes][persistance]")
{
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;

    // sauvegarder() persiste fournisseurs ET commandes ensemble à chaque appel :
    // les deux compteurs avancent donc de concert, quelle que soit l'opération.
    int f = gc.ajouterFournisseur("A", "x", "x");
    REQUIRE(banc.sauvegardesFournisseurs() == 1);
    REQUIRE(banc.sauvegardesCommandes() == 1);

    int idCmd = gc.creerCommande(f, date(2026, 1, 1));
    REQUIRE(banc.sauvegardesCommandes() == 2);

    gc.ajouterLigneCommande(idCmd, "R1", 1, 1.0);
    REQUIRE(banc.sauvegardesCommandes() == 3);

    gc.changerStatut(idCmd, StatutCommande::LIVREE);
    REQUIRE(banc.sauvegardesCommandes() == 4);
}

TEST_CASE("GestionnaireCommandes ne sauvegarde pas quand une opération échoue", "[commandes][persistance]")
{
    BancDeTestCommandes banc;
    auto& gc = banc.gestionnaire;
    int f = gc.ajouterFournisseur("A", "x", "x");
    int avant = banc.sauvegardesCommandes();

    REQUIRE_THROWS_AS(gc.creerCommande(999, date(2026, 1, 1)), std::invalid_argument);
    REQUIRE(banc.sauvegardesCommandes() == avant);

    int idCmd = gc.creerCommande(f, date(2026, 1, 1));
    avant = banc.sauvegardesCommandes();
    REQUIRE_THROWS_AS(gc.ajouterLigneCommande(idCmd, "R1", -1, 1.0), std::invalid_argument);
    REQUIRE_THROWS_AS(gc.ajouterLigneCommande(idCmd, "R1", 1, -1.0), std::invalid_argument);
    REQUIRE(banc.sauvegardesCommandes() == avant);
}

TEST_CASE("GestionnaireCommandes::charger reprend l'état et poursuit la numérotation", "[commandes][persistance]")
{
    BancDeTestCommandes banc;
    banc.depotFournisseurs->disque.emplace_back(5, "FournTech", "x", "adr");
    banc.depotCommandes->disque.emplace_back(9, 5, date(2026, 1, 1), StatutCommande::EN_COURS);

    banc.gestionnaire.charger();

    REQUIRE(banc.gestionnaire.getFournisseurs().size() == 1);
    REQUIRE(banc.gestionnaire.getCommandes().size() == 1);

    int nouveauFournisseur = banc.gestionnaire.ajouterFournisseur("B", "y", "y");
    int nouvelleCommande = banc.gestionnaire.creerCommande(5, date(2026, 2, 1));

    REQUIRE(nouveauFournisseur == 6);   // jamais un id déjà présent sur disque
    REQUIRE(nouvelleCommande == 10);
}

// ---------------------------------------------------------------------------------------
// Repositories fichier
// ---------------------------------------------------------------------------------------

#include "repositories/FichierTexteFournisseurRepository.hpp"
#include "repositories/FichierTexteCommandeRepository.hpp"

TEST_CASE("FichierTexteFournisseurRepository : aller-retour avec ';' dans les champs", "[commandes][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    FichierTexteFournisseurRepository repo(dossier.fichier("fournisseurs.txt"));

    std::vector<Fournisseur> fournisseurs;
    fournisseurs.emplace_back(1, "Fourn; Tech", "01;02", "12 rue X; bât. B");
    repo.sauvegarder(fournisseurs);

    auto relu = repo.charger();
    REQUIRE(relu.size() == 1);
    REQUIRE(relu[0].getNom() == "Fourn; Tech");
    REQUIRE(relu[0].getAdresse() == "12 rue X; bât. B");
}

TEST_CASE("FichierTexteFournisseurRepository sur un fichier absent renvoie une liste vide", "[commandes][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    FichierTexteFournisseurRepository repo(dossier.fichier("absent.txt"));
    REQUIRE(repo.charger().empty());
}

TEST_CASE("FichierTexteCommandeRepository : aller-retour complet avec lignes", "[commandes][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    FichierTexteCommandeRepository repo(dossier.fichier("commandes.txt"), dossier.fichier("lignes.txt"));

    std::vector<Commande> commandes;
    commandes.emplace_back(1, 5, date(2026, 3, 10), StatutCommande::LIVREE);
    commandes.back().ajouterLigne(LigneCommande("R1; special", 10, 5.5));
    commandes.back().ajouterLigne(LigneCommande("R2", 3, 2.0));
    commandes.emplace_back(2, 5, date(2026, 4, 1), StatutCommande::EN_COURS);   // sans lignes
    repo.sauvegarder(commandes);

    auto relu = repo.charger();
    REQUIRE(relu.size() == 2);
    REQUIRE(relu[0].getIdCommande() == 1);
    REQUIRE(relu[0].getStatut() == StatutCommande::LIVREE);
    REQUIRE(relu[0].getDateCommande() == date(2026, 3, 10));
    REQUIRE(relu[0].getLignes().size() == 2);
    REQUIRE(relu[0].getLignes()[0].getReferenceProduit() == "R1; special");
    REQUIRE(relu[0].getMontantTotal() == Catch::Approx(10 * 5.5 + 3 * 2.0));
    REQUIRE(relu[1].getLignes().empty());   // une commande sans ligne reste valide
}

TEST_CASE("FichierTexteCommandeRepository refuse une ligne orpheline", "[commandes][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    std::string cheminCmd = dossier.fichier("commandes.txt");
    std::string cheminLgn = dossier.fichier("lignes.txt");
    Support::ecrire(cheminCmd, "1;5;2026-01-01;EN_COURS\n");
    Support::ecrire(cheminLgn, "999;R1;1;1.0\n");   // référence une commande #999 qui n'existe pas

    FichierTexteCommandeRepository repo(cheminCmd, cheminLgn);
    REQUIRE_THROWS_AS(repo.charger(), FormatFichierInvalideException);
}

TEST_CASE("FichierTexteCommandeRepository refuse un statut inconnu", "[commandes][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    std::string cheminCmd = dossier.fichier("commandes.txt");
    Support::ecrire(cheminCmd, "1;5;2026-01-01;EXPEDIEE\n");

    FichierTexteCommandeRepository repo(cheminCmd, dossier.fichier("lignes.txt"));
    REQUIRE_THROWS_AS(repo.charger(), FormatFichierInvalideException);
}

TEST_CASE("GestionnaireCommandes : persistance de bout en bout avec de vrais fichiers", "[commandes][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    std::string cheminF = dossier.fichier("fournisseurs.txt");
    std::string cheminC = dossier.fichier("commandes.txt");
    std::string cheminL = dossier.fichier("lignes.txt");

    {
        GestionnaireCommandes gc(std::make_unique<FichierTexteFournisseurRepository>(cheminF),
                                  std::make_unique<FichierTexteCommandeRepository>(cheminC, cheminL));
        int f = gc.ajouterFournisseur("FournTech", "x", "x");
        int idCmd = gc.creerCommande(f, date(2026, 1, 1));
        gc.ajouterLigneCommande(idCmd, "R1", 5, 10.0);
        gc.changerStatut(idCmd, StatutCommande::LIVREE);
    }   // le premier GestionnaireCommandes est détruit ici : tout doit déjà être sur disque

    GestionnaireCommandes gc2(std::make_unique<FichierTexteFournisseurRepository>(cheminF),
                               std::make_unique<FichierTexteCommandeRepository>(cheminC, cheminL));
    gc2.charger();

    REQUIRE(gc2.getFournisseurs().size() == 1);
    REQUIRE(gc2.getCommandes().size() == 1);
    REQUIRE(gc2.getCommandes()[0].getStatut() == StatutCommande::LIVREE);
    REQUIRE(gc2.getCommandes()[0].getLignes().size() == 1);
    REQUIRE(gc2.getCommandes()[0].getMontantTotal() == Catch::Approx(50.0));
}