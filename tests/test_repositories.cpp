#include "vendor/catch_amalgamated.hpp"

#include "exceptions/Exceptions.hpp"
#include "factories/ProduitFactory.hpp"
#include "repositories/FichierTexteMouvementRepository.hpp"
#include "repositories/FichierTexteRepository.hpp"
#include "support/Support.hpp"

using namespace Support;

// ---------------------------------------------------------------------------------------
// FichierTexteRepository (produits)
// ---------------------------------------------------------------------------------------

TEST_CASE("FichierTexteRepository::charger sur un fichier absent renvoie une liste vide", "[repo][fichier]")
{
    DossierTemporaire dossier;
    FichierTexteRepository repo(dossier.fichier("inexistant.txt"));
    REQUIRE(repo.charger().empty());
}

TEST_CASE("FichierTexteRepository : aller-retour standard et périssable", "[repo][fichier]")
{
    DossierTemporaire dossier;
    FichierTexteRepository repo(dossier.fichier("stock.txt"));

    std::vector<std::unique_ptr<Produit>> produits;
    produits.push_back(produit("R1", 10, 3));
    produits.push_back(ProduitFactory::creerProduit(TypeProduit::PERISSABLE, "R2", "Yaourt", "Alim",
                                                     0.5, 1.2, 40, 10, date(2026, 10, 15)));
    repo.sauvegarder(produits);

    auto relu = repo.charger();
    REQUIRE(relu.size() == 2);
    REQUIRE(relu[0]->getReference() == "R1");

    auto* p2 = dynamic_cast<ProduitPerissable*>(relu[1].get());
    REQUIRE(p2 != nullptr);
    REQUIRE(p2->getDatePeremption() == date(2026, 10, 15));
}

TEST_CASE("FichierTexteRepository : ';' et '\\' dans les champs texte survivent au disque", "[repo][fichier]")
{
    DossierTemporaire dossier;
    FichierTexteRepository repo(dossier.fichier("stock.txt"));

    std::vector<std::unique_ptr<Produit>> produits;
    produits.push_back(ProduitFactory::creerProduit(TypeProduit::STANDARD, "R1",
                                                     "Câble; USB\\C", "Info;Cat", 1, 2, 3, 1));
    repo.sauvegarder(produits);

    auto relu = repo.charger();
    REQUIRE(relu[0]->getNom() == "Câble; USB\\C");
    REQUIRE(relu[0]->getCategorie() == "Info;Cat");
}

TEST_CASE("FichierTexteRepository : sauvegarder puis charger redonne exactement le même contenu", "[repo][fichier]")
{
    DossierTemporaire dossier;
    FichierTexteRepository repo(dossier.fichier("stock.txt"));

    std::vector<std::unique_ptr<Produit>> vide;
    repo.sauvegarder(vide);
    REQUIRE(repo.charger().empty());   // écrase un ancien contenu par une liste vide
}

TEST_CASE("FichierTexteRepository::charger refuse une date invalide dans le fichier", "[repo][fichier][dates]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("stock.txt");
    ecrire(chemin, "PERISSABLE;R1;N;C;1;2;3;1;2026-13-45\n");

    FichierTexteRepository repo(chemin);
    REQUIRE_THROWS_AS(repo.charger(), FormatFichierInvalideException);
}

TEST_CASE("FichierTexteRepository::charger refuse un type de produit inconnu", "[repo][fichier]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("stock.txt");
    ecrire(chemin, "ELECTRONIQUE;R1;N;C;1;2;3;1\n");

    FichierTexteRepository repo(chemin);
    REQUIRE_THROWS_AS(repo.charger(), FormatFichierInvalideException);
}

TEST_CASE("FichierTexteRepository::charger refuse une ligne numériquement corrompue", "[repo][fichier]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("stock.txt");
    ecrire(chemin, "STANDARD;R1;N;C;abc;2;3;1\n");

    FichierTexteRepository repo(chemin);
    REQUIRE_THROWS_AS(repo.charger(), FormatFichierInvalideException);
}

TEST_CASE("FichierTexteRepository::charger ignore les lignes vides", "[repo][fichier]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("stock.txt");
    ecrire(chemin, "STANDARD;R1;N;C;1;2;3;1\n\n\nSTANDARD;R2;N;C;1;2;3;1\n");

    FichierTexteRepository repo(chemin);
    REQUIRE(repo.charger().size() == 2);
}

// ---------------------------------------------------------------------------------------
// FichierTexteMouvementRepository
// ---------------------------------------------------------------------------------------

TEST_CASE("FichierTexteMouvementRepository : aller-retour, y compris auteur avec ';'", "[repo][fichier][mouvements]")
{
    DossierTemporaire dossier;
    FichierTexteMouvementRepository repo(dossier.fichier("mvt.txt"));

    auto t = std::chrono::system_clock::now();
    std::vector<MouvementStock> historique;
    historique.emplace_back(1, "R1", TypeMouvement::ENTREE, 5, t, "Jean; fournisseur");
    historique.emplace_back(2, "R1", TypeMouvement::SORTIE, 2, t, "Vente");
    historique.emplace_back(3, "R1", TypeMouvement::AJUSTEMENT, -1, t, "Inventaire");
    repo.sauvegarder(historique);

    auto relu = repo.charger();
    REQUIRE(relu.size() == 3);
    REQUIRE(relu[0].getAuteur() == "Jean; fournisseur");
    REQUIRE(relu[0].getType() == TypeMouvement::ENTREE);
    REQUIRE(relu[1].getType() == TypeMouvement::SORTIE);
    REQUIRE(relu[2].getType() == TypeMouvement::AJUSTEMENT);
    REQUIRE(relu[2].getQuantite() == -1);   // un écart d'ajustement négatif doit survivre

    // L'horodatage est stocké en secondes : comparer à la seconde près.
    auto attendu = std::chrono::floor<std::chrono::seconds>(t);
    auto obtenu = std::chrono::floor<std::chrono::seconds>(relu[0].getDateHeure());
    REQUIRE(attendu == obtenu);
}

TEST_CASE("FichierTexteMouvementRepository sur un fichier absent renvoie une liste vide", "[repo][fichier][mouvements]")
{
    DossierTemporaire dossier;
    FichierTexteMouvementRepository repo(dossier.fichier("absent.txt"));
    REQUIRE(repo.charger().empty());
}

TEST_CASE("FichierTexteMouvementRepository::charger refuse un type inconnu", "[repo][fichier][mouvements]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("mvt.txt");
    ecrire(chemin, "1;TRANSFERT;R1;5;1700000000;a\n");

    FichierTexteMouvementRepository repo(chemin);
    REQUIRE_THROWS_AS(repo.charger(), FormatFichierInvalideException);
}

TEST_CASE("FichierTexteMouvementRepository::charger refuse une ligne mal formée", "[repo][fichier][mouvements]")
{
    DossierTemporaire dossier;
    std::string chemin = dossier.fichier("mvt.txt");
    ecrire(chemin, "1;ENTREE;R1;5\n");   // il manque des colonnes

    FichierTexteMouvementRepository repo(chemin);
    REQUIRE_THROWS_AS(repo.charger(), FormatFichierInvalideException);
}

TEST_CASE("FichierTexteRepository : aller-retour avec un produit électronique", "[repo][fichier][electronique]")
{
    DossierTemporaire dossier;
    FichierTexteRepository repo(dossier.fichier("stock.txt"));

    std::vector<std::unique_ptr<Produit>> produits;
    produits.push_back(ProduitFactory::creerProduit(TypeProduit::ELECTRONIQUE, "R1", "Casque; Bluetooth", "Audio",
                                                      20, 40, 5, 2, std::nullopt, 24, std::string("SN\\001")));
    repo.sauvegarder(produits);

    auto relu = repo.charger();
    REQUIRE(relu.size() == 1);
    auto* p = dynamic_cast<ProduitElectronique*>(relu[0].get());
    REQUIRE(p != nullptr);
    REQUIRE(p->getNom() == "Casque; Bluetooth");   // le ';' survit à l'échappement
    REQUIRE(p->getDureeGarantieMois() == 24);
    REQUIRE(p->getNumeroSerie() == "SN\\001");     // le '\' aussi
}

TEST_CASE("FichierTexteRepository : les trois types cohabitent dans le même fichier", "[repo][fichier]")
{
    DossierTemporaire dossier;
    FichierTexteRepository repo(dossier.fichier("stock.txt"));

    std::vector<std::unique_ptr<Produit>> produits;
    produits.push_back(produit("STD", 5));
    produits.push_back(ProduitFactory::creerProduit(TypeProduit::PERISSABLE, "PER", "N", "C", 1, 2, 3, 1,
                                                      date(2026, 10, 15)));
    produits.push_back(ProduitFactory::creerProduit(TypeProduit::ELECTRONIQUE, "ELEC", "N", "C", 1, 2, 3, 1,
                                                      std::nullopt, 12, std::string("SN")));
    repo.sauvegarder(produits);

    auto relu = repo.charger();
    REQUIRE(relu.size() == 3);
    REQUIRE(dynamic_cast<ProduitPerissable*>(relu[1].get()) != nullptr);
    REQUIRE(dynamic_cast<ProduitElectronique*>(relu[2].get()) != nullptr);
}