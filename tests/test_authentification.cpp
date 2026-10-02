#include "vendor/catch_amalgamated.hpp"

#include "exceptions/Exceptions.hpp"
#include "models/Utilisateur.hpp"
#include "repositories/FichierTexteUtilisateurRepository.hpp"
#include "services/GestionnaireUtilisateurs.hpp"
#include "support/Support.hpp"
#include "utils/Hachage.hpp"
#include "utils/Sha256.hpp"

#include <set>
#include <stdexcept>

// ---------------------------------------------------------------------------------------
// Sha256 (vecteurs de test officiels, voir NIST/FIPS 180-4)
// ---------------------------------------------------------------------------------------

TEST_CASE("Sha256::hacher correspond aux vecteurs de test officiels", "[crypto][sha256]")
{
    REQUIRE(Sha256::hacher("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    REQUIRE(Sha256::hacher("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    REQUIRE(Sha256::hacher("The quick brown fox jumps over the lazy dog")
            == "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
}

TEST_CASE("Sha256::hacher produit toujours 64 caractères hexadécimaux", "[crypto][sha256]")
{
    for (const std::string& texte : {std::string(""), std::string("x"), std::string(1000, 'A')}) {
        auto h = Sha256::hacher(texte);
        REQUIRE(h.size() == 64);
        REQUIRE(h.find_first_not_of("0123456789abcdef") == std::string::npos);
    }
}

TEST_CASE("Sha256::hacher est déterministe et sensible au moindre changement", "[crypto][sha256]")
{
    REQUIRE(Sha256::hacher("abc") == Sha256::hacher("abc"));
    REQUIRE(Sha256::hacher("abc") != Sha256::hacher("abd"));
}

// ---------------------------------------------------------------------------------------
// Hachage (sel + SHA-256)
// ---------------------------------------------------------------------------------------

TEST_CASE("Hachage : même mot de passe, sels différents, hashs différents", "[crypto][hachage]")
{
    auto h1 = Hachage::hacherNouveauMotDePasse("secret123");
    auto h2 = Hachage::hacherNouveauMotDePasse("secret123");
    REQUIRE(h1 != h2);
}

TEST_CASE("Hachage : vérification correcte du bon et du mauvais mot de passe", "[crypto][hachage]")
{
    auto h = Hachage::hacherNouveauMotDePasse("secret123");
    REQUIRE(Hachage::verifierMotDePasse("secret123", h));
    REQUIRE_FALSE(Hachage::verifierMotDePasse("autre", h));
    REQUIRE_FALSE(Hachage::verifierMotDePasse("", h));
}

TEST_CASE("Hachage::verifierMotDePasse refuse un hash mal formé", "[crypto][hachage]")
{
    REQUIRE_FALSE(Hachage::verifierMotDePasse("x", "pas-de-dollar-ici"));
    REQUIRE_FALSE(Hachage::verifierMotDePasse("x", ""));
}

TEST_CASE("Hachage::genererSel ne produit (quasiment) jamais de collision", "[crypto][hachage]")
{
    std::set<std::string> sels;
    for (int i = 0; i < 200; ++i) sels.insert(Hachage::genererSel());
    REQUIRE(sels.size() == 200);
}

// ---------------------------------------------------------------------------------------
// Utilisateur
// ---------------------------------------------------------------------------------------

TEST_CASE("Utilisateur ne stocke jamais le mot de passe en clair", "[auth][utilisateur]")
{
    Utilisateur u(1, "alice", Hachage::hacherNouveauMotDePasse("motdepasse"), Role::ADMIN);
    REQUIRE(u.getPasswordHash().find("motdepasse") == std::string::npos);
    REQUIRE(u.verifierMotDePasse("motdepasse"));
    REQUIRE_FALSE(u.verifierMotDePasse("mauvais"));
}

TEST_CASE("Utilisateur::changerMotDePasse change le hash et invalide l'ancien mot de passe", "[auth][utilisateur]")
{
    Utilisateur u(1, "alice", Hachage::hacherNouveauMotDePasse("ancien"), Role::ADMIN);
    u.changerMotDePasse("nouveau");
    REQUIRE_FALSE(u.verifierMotDePasse("ancien"));
    REQUIRE(u.verifierMotDePasse("nouveau"));
}

TEST_CASE("libelle(Role) et roleDepuisTexte font l'aller-retour", "[auth][utilisateur]")
{
    for (Role r : {Role::ADMIN, Role::GESTIONNAIRE, Role::MAGASINIER})
        REQUIRE(roleDepuisTexte(libelle(r)) == r);

    REQUIRE_THROWS_AS(roleDepuisTexte("INCONNU"), std::invalid_argument);
}

// ---------------------------------------------------------------------------------------
// GestionnaireUtilisateurs (dépôt en mémoire)
// ---------------------------------------------------------------------------------------

namespace {

class DepotUtilisateursMemoire : public IUtilisateurRepository
{
    public:
        std::vector<Utilisateur> disque;
        int nbSauvegardes = 0;

        void sauvegarder(const std::vector<Utilisateur>& utilisateurs) override
        {
            disque = utilisateurs;
            ++nbSauvegardes;
        }

        std::vector<Utilisateur> charger() override { return disque; }
};

struct BancUtilisateurs
{
    DepotUtilisateursMemoire* depot = nullptr;
    GestionnaireUtilisateurs gestionnaire;

    BancUtilisateurs() : gestionnaire(creer(depot)) {}

    private:
        static GestionnaireUtilisateurs creer(DepotUtilisateursMemoire*& d)
        {
            auto repo = std::make_unique<DepotUtilisateursMemoire>();
            d = repo.get();
            return GestionnaireUtilisateurs(std::move(repo));
        }
};

} // namespace anonyme

TEST_CASE("GestionnaireUtilisateurs::ajouterUtilisateur hache le mot de passe avant de le stocker", "[auth][service]")
{
    BancUtilisateurs banc;
    int id = banc.gestionnaire.ajouterUtilisateur("alice", "motdepasse123", Role::ADMIN);

    REQUIRE(id == 1);
    REQUIRE(banc.gestionnaire.getUtilisateurs().size() == 1);
    REQUIRE(banc.gestionnaire.getUtilisateurs()[0].getPasswordHash().find("motdepasse123") == std::string::npos);
    REQUIRE(banc.depot->nbSauvegardes == 1);
}

TEST_CASE("GestionnaireUtilisateurs refuse un nom d'utilisateur déjà pris", "[auth][service]")
{
    BancUtilisateurs banc;
    banc.gestionnaire.ajouterUtilisateur("alice", "motdepasse123", Role::ADMIN);
    int avant = banc.depot->nbSauvegardes;

    REQUIRE_THROWS_AS(banc.gestionnaire.ajouterUtilisateur("alice", "autremotdepasse", Role::MAGASINIER),
                      std::invalid_argument);
    REQUIRE(banc.gestionnaire.getUtilisateurs().size() == 1);
    REQUIRE(banc.depot->nbSauvegardes == avant);
}

TEST_CASE("GestionnaireUtilisateurs refuse un mot de passe trop court ou un nom vide", "[auth][service]")
{
    BancUtilisateurs banc;
    REQUIRE_THROWS_AS(banc.gestionnaire.ajouterUtilisateur("alice", "123", Role::ADMIN), std::invalid_argument);
    REQUIRE_THROWS_AS(banc.gestionnaire.ajouterUtilisateur("", "motdepasse123", Role::ADMIN), std::invalid_argument);
}

TEST_CASE("GestionnaireUtilisateurs::authentifier valide le bon couple identifiant/mot de passe", "[auth][service]")
{
    BancUtilisateurs banc;
    banc.gestionnaire.ajouterUtilisateur("alice", "motdepasse123", Role::GESTIONNAIRE);

    const Utilisateur* u = banc.gestionnaire.authentifier("alice", "motdepasse123");
    REQUIRE(u != nullptr);
    REQUIRE(u->getUsername() == "alice");
    REQUIRE(u->getRole() == Role::GESTIONNAIRE);
}

TEST_CASE("GestionnaireUtilisateurs::authentifier refuse un mauvais mot de passe ou un inconnu", "[auth][service]")
{
    BancUtilisateurs banc;
    banc.gestionnaire.ajouterUtilisateur("alice", "motdepasse123", Role::ADMIN);

    REQUIRE(banc.gestionnaire.authentifier("alice", "mauvais") == nullptr);
    REQUIRE(banc.gestionnaire.authentifier("inconnu", "motdepasse123") == nullptr);
}

TEST_CASE("GestionnaireUtilisateurs::charger reprend l'état et poursuit la numérotation", "[auth][service]")
{
    BancUtilisateurs banc;
    banc.depot->disque.emplace_back(7, "bob", Hachage::hacherNouveauMotDePasse("x"), Role::ADMIN);

    banc.gestionnaire.charger();
    REQUIRE_FALSE(banc.gestionnaire.estVide());

    int nouvelId = banc.gestionnaire.ajouterUtilisateur("alice", "motdepasse123", Role::MAGASINIER);
    REQUIRE(nouvelId == 8);   // jamais un id déjà présent sur disque
}

TEST_CASE("GestionnaireUtilisateurs::estVide reflète l'état réel", "[auth][service]")
{
    BancUtilisateurs banc;
    REQUIRE(banc.gestionnaire.estVide());
    banc.gestionnaire.ajouterUtilisateur("alice", "motdepasse123", Role::ADMIN);
    REQUIRE_FALSE(banc.gestionnaire.estVide());
}

// ---------------------------------------------------------------------------------------
// FichierTexteUtilisateurRepository
// ---------------------------------------------------------------------------------------

TEST_CASE("FichierTexteUtilisateurRepository : aller-retour, jamais le mot de passe en clair", "[auth][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    FichierTexteUtilisateurRepository repo(dossier.fichier("utilisateurs.txt"));

    std::vector<Utilisateur> utilisateurs;
    utilisateurs.emplace_back(1, "alice", Hachage::hacherNouveauMotDePasse("secret123"), Role::ADMIN);
    repo.sauvegarder(utilisateurs);

    std::string contenuBrut = Support::lire(dossier.fichier("utilisateurs.txt"));
    REQUIRE(contenuBrut.find("secret123") == std::string::npos);

    auto relu = repo.charger();
    REQUIRE(relu.size() == 1);
    REQUIRE(relu[0].getUsername() == "alice");
    REQUIRE(relu[0].getRole() == Role::ADMIN);
    REQUIRE(relu[0].verifierMotDePasse("secret123"));
}

TEST_CASE("FichierTexteUtilisateurRepository sur un fichier absent renvoie une liste vide", "[auth][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    FichierTexteUtilisateurRepository repo(dossier.fichier("absent.txt"));
    REQUIRE(repo.charger().empty());
}

TEST_CASE("FichierTexteUtilisateurRepository::charger refuse un rôle inconnu", "[auth][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    std::string chemin = dossier.fichier("utilisateurs.txt");
    Support::ecrire(chemin, "1;alice;sel$hash;SUPERADMIN\n");

    FichierTexteUtilisateurRepository repo(chemin);
    REQUIRE_THROWS_AS(repo.charger(), FormatFichierInvalideException);
}

TEST_CASE("GestionnaireUtilisateurs : persistance de bout en bout avec de vrais fichiers", "[auth][repo][fichier]")
{
    Support::DossierTemporaire dossier;
    std::string chemin = dossier.fichier("utilisateurs.txt");

    {
        GestionnaireUtilisateurs gu(std::make_unique<FichierTexteUtilisateurRepository>(chemin));
        gu.ajouterUtilisateur("alice", "motdepasse123", Role::ADMIN);
    }   // détruit : tout doit déjà être sur disque

    GestionnaireUtilisateurs gu2(std::make_unique<FichierTexteUtilisateurRepository>(chemin));
    gu2.charger();

    REQUIRE_FALSE(gu2.estVide());
    REQUIRE(gu2.authentifier("alice", "motdepasse123") != nullptr);
    REQUIRE(gu2.authentifier("alice", "mauvais") == nullptr);
}