#include "ui/Saisie.hpp"
#include "ui/Console.hpp"
#include "utils/DateUtils.hpp"

#include <charconv>
#include <optional>

#if defined(_WIN32)
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace Couleur;

namespace {

std::string rogner(const std::string& texte)
{
    const char* espaces = " \t\r\n";
    auto debut = texte.find_first_not_of(espaces);
    if (debut == std::string::npos) return "";
    auto fin = texte.find_last_not_of(espaces);
    return texte.substr(debut, fin - debut + 1);
}

// Analyse stricte : toute la chaîne doit être un nombre (« 12abc » est refusé).
std::optional<int> analyserEntier(const std::string& brut)
{
    std::string texte = rogner(brut);
    if (texte.empty()) return std::nullopt;

    int valeur = 0;
    auto [fin, erreur] = std::from_chars(texte.data(), texte.data() + texte.size(), valeur);
    if (erreur != std::errc() || fin != texte.data() + texte.size()) return std::nullopt;
    return valeur;
}

std::optional<double> analyserDouble(const std::string& brut)
{
    std::string texte = rogner(brut);
    for (char& c : texte)
        if (c == ',') c = '.';   // clavier français : « 1,5 »
    if (texte.empty()) return std::nullopt;

    double valeur = 0;
    auto [fin, erreur] = std::from_chars(texte.data(), texte.data() + texte.size(), valeur);
    if (erreur != std::errc() || fin != texte.data() + texte.size()) return std::nullopt;
    return valeur;
}

} // namespace anonyme

std::string lireLigne()
{
    std::string ligne;
    if (!std::getline(std::cin, ligne))
        throw EntreeInterrompue{};
    if (!ligne.empty() && ligne.back() == '\r')   // tolère les fins de ligne Windows
        ligne.pop_back();
    return ligne;
}

std::string lireTexte(const std::string& invite)
{
    while (true) {
        std::cout << invite;
        std::string texte = rogner(lireLigne());
        if (!texte.empty()) return texte;
        std::cout << ROUGE << "[!] Ce champ ne peut pas être vide." << RESET << "\n";
    }
}

int lireEntier(const std::string& invite)
{
    while (true) {
        std::cout << invite;
        if (auto valeur = analyserEntier(lireLigne())) return *valeur;
        std::cout << ROUGE << "[!] Veuillez entrer un nombre entier valide." << RESET << "\n";
    }
}

int lireEntierNonNegatif(const std::string& invite)
{
    while (true) {
        int valeur = lireEntier(invite);
        if (valeur >= 0) return valeur;
        std::cout << ROUGE << "[!] La valeur ne peut pas être négative." << RESET << "\n";
    }
}

int lireEntierStrictementPositif(const std::string& invite)
{
    while (true) {
        int valeur = lireEntier(invite);
        if (valeur > 0) return valeur;
        std::cout << ROUGE << "[!] La valeur doit être strictement positive." << RESET << "\n";
    }
}

double lireDouble(const std::string& invite)
{
    while (true) {
        std::cout << invite;
        if (auto valeur = analyserDouble(lireLigne())) return *valeur;
        std::cout << ROUGE << "[!] Veuillez entrer un nombre valide." << RESET << "\n";
    }
}

double lireDoubleNonNegatif(const std::string& invite)
{
    while (true) {
        double valeur = lireDouble(invite);
        if (valeur >= 0) return valeur;
        std::cout << ROUGE << "[!] La valeur ne peut pas être négative." << RESET << "\n";
    }
}

std::chrono::year_month_day lireDate(const std::string& libelle)
{
    while (true) {
        int annee = lireEntier("\t\t" + libelle + " - année (AAAA) : ");
        int mois  = lireEntier("\t\t" + libelle + " - mois (1-12) : ");
        int jour  = lireEntier("\t\t" + libelle + " - jour (1-31) : ");

        if (auto date = DateUtils::creer(annee, mois, jour))
            return *date;
        std::cout << ROUGE << "[!] Cette date n'existe pas, veuillez la ressaisir." << RESET << "\n";
    }
}

bool lireChoix(int& choix)
{
    auto valeur = analyserEntier(lireLigne());
    if (!valeur) {
        std::cout << RESET << "\n\t\t" << ROUGE << "[!] Erreur de saisie." << RESET << "\n";
        attendreEntree();
        return false;
    }
    choix = *valeur;
    std::cout << RESET;
    return true;
}

std::string lireMotDePasse(const std::string& invite)
{
    std::cout << invite;

#if defined(_WIN32)
    if (!_isatty(_fileno(stdin))) {
        return lireLigne();   // flux redirigé (tests) : rien à masquer
    }
    std::string mdp;
    int c;
    while ((c = _getch()) != '\r' && c != '\n') {
        if (c == '\b') {
            if (!mdp.empty()) { mdp.pop_back(); std::cout << "\b \b"; }
        } else {
            mdp += static_cast<char>(c);
            std::cout << '*';
        }
    }
    std::cout << "\n";
    return mdp;
#else
    if (!isatty(fileno(stdin))) {
        return lireLigne();   // flux redirigé (tests, Docker non interactif) : rien à masquer
    }

    termios ancien{};
    tcgetattr(STDIN_FILENO, &ancien);
    termios silencieux = ancien;
    silencieux.c_lflag &= ~ECHO;   // désactive l'écho, garde le canonique (backspace fonctionne)
    tcsetattr(STDIN_FILENO, TCSANOW, &silencieux);

    std::string mdp;
    try {
        mdp = lireLigne();
    } catch (...) {
        tcsetattr(STDIN_FILENO, TCSANOW, &ancien);
        throw;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &ancien);
    std::cout << "\n";
    return mdp;
#endif
}

void attendreEntree()
{
    std::cout << "\n\t\tAppuyez sur Entrée pour continuer...";
    lireLigne();
}