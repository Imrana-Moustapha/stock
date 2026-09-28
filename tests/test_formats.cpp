#include "vendor/catch_amalgamated.hpp"

#include "exceptions/Exceptions.hpp"
#include "repositories/FormatTexte.hpp"
#include "utils/Csv.hpp"

#include <string>
#include <vector>

using Strings = std::vector<std::string>;

// ---------------------------------------------------------------------------------------
// FormatTexte : fichiers de données délimités par ';'
// ---------------------------------------------------------------------------------------

namespace {
    std::string assembler(const Strings& champs)
    {
        std::string ligne;
        for (std::size_t i = 0; i < champs.size(); ++i) {
            if (i) ligne += ';';
            ligne += FormatTexte::echapper(champs[i], ';');
        }
        return ligne;
    }
}

TEST_CASE("FormatTexte : un texte sans caractère spécial n'est pas modifié", "[format]")
{
    REQUIRE(FormatTexte::echapper("Souris sans fil", ';') == "Souris sans fil");
}

TEST_CASE("FormatTexte : le délimiteur et l'antislash sont échappés", "[format]")
{
    REQUIRE(FormatTexte::echapper("a;b", ';') == "a\\;b");
    REQUIRE(FormatTexte::echapper("a\\b", ';') == "a\\\\b");
}

TEST_CASE("FormatTexte : aller-retour sur des champs difficiles", "[format]")
{
    Strings champs = GENERATE(
        Strings{"simple", "deux", "trois"},
        Strings{"Câble; USB", "Info;Cat", "x"},
        Strings{"a\\b", "\\", "\\\\"},
        Strings{"fin;", ";début", ";"},
        Strings{"", "vide avant", ""},
        Strings{"seul"});

    REQUIRE(FormatTexte::decouper(assembler(champs), ';') == champs);
}

TEST_CASE("FormatTexte::decouper conserve un champ vide final", "[format]")
{
    REQUIRE(FormatTexte::decouper("a;b;", ';') == Strings{"a", "b", ""});
}

TEST_CASE("FormatTexte::decouper refuse un échappement incomplet", "[format]")
{
    REQUIRE_THROWS_AS(FormatTexte::decouper("abc\\", ';'), FormatFichierInvalideException);
}

// ---------------------------------------------------------------------------------------
// Csv : import / export
// ---------------------------------------------------------------------------------------

TEST_CASE("Csv::echapper n'ajoute des guillemets que si nécessaire", "[csv]")
{
    REQUIRE(Csv::echapper("Souris") == "Souris");
    REQUIRE(Csv::echapper("Câble USB, 2 m") == "\"Câble USB, 2 m\"");
    REQUIRE(Csv::echapper("dit \"oui\"") == "\"dit \"\"oui\"\"\"");
}

TEST_CASE("Csv::decouper gère les champs entre guillemets", "[csv]")
{
    REQUIRE(Csv::decouper("a,b,c") == Strings{"a", "b", "c"});
    REQUIRE(Csv::decouper("a,\"b,c\",d") == Strings{"a", "b,c", "d"});
    REQUIRE(Csv::decouper("\"x \"\"y\"\" z\"") == Strings{"x \"y\" z"});
    REQUIRE(Csv::decouper("a,,c") == Strings{"a", "", "c"});
    REQUIRE(Csv::decouper("a,b,") == Strings{"a", "b", ""});
}

TEST_CASE("Csv::decouper tolère les fins de ligne Windows", "[csv]")
{
    REQUIRE(Csv::decouper("a,b\r") == Strings{"a", "b"});
}

TEST_CASE("Csv : aller-retour sur des champs difficiles", "[csv]")
{
    Strings champs = GENERATE(
        Strings{"simple", "deux"},
        Strings{"Câble USB, 2 m", "Info"},
        Strings{"dit \"oui\"", "x, y", "z"},
        Strings{"", "", ""},
        Strings{"\"", ",", "\"\""});

    std::string ligne;
    for (std::size_t i = 0; i < champs.size(); ++i) {
        if (i) ligne += ',';
        ligne += Csv::echapper(champs[i]);
    }
    REQUIRE(Csv::decouper(ligne) == champs);
}