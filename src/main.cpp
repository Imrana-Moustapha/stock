#include "menus/Menus.hpp"
#include "services/Gestionnairestock.hpp"
#include "repositories/FichierTexteRepository.hpp"
#include "observers/Logger.hpp"

int main()
{
    GestionnaireStock gestionnaire(std::make_unique<FichierTexteRepository>("data/stock.txt"));
    gestionnaire.charger();

    Logger logger("data/alertes.log");
    gestionnaire.ajouterObservateur(&logger);

    menu_principal(gestionnaire);

    gestionnaire.sauvegarder();
    return 0;
}