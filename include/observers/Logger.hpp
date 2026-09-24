#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

#include "observers/IObservateurStock.hpp"
#include "models/Produit.hpp"

// Premier observateur concret du pattern Observer : à chaque notification
// (produit passé sous son seuil critique), écrit une ligne horodatée dans
// un fichier de log. N'affecte jamais l'exécution de l'application même
// en cas d'échec d'écriture (un log qui échoue ne doit pas faire planter
// une opération de stock).
class Logger : public IObservateurStock
{
    private:
        std::string cheminFichier;

        static std::string horodatage()
        {
            auto maintenant = std::chrono::system_clock::now();
            auto tempsC = std::chrono::system_clock::to_time_t(maintenant);
            std::tm tmLocal{};
#if defined(_WIN32)
            localtime_s(&tmLocal, &tempsC);
#else
            localtime_r(&tempsC, &tmLocal);
#endif
            std::ostringstream oss;
            oss << std::put_time(&tmLocal, "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

    public:
        explicit Logger(std::string chemin) : cheminFichier(std::move(chemin)) {}

        void notifierSeuilCritique(const Produit& produit) override
        {
            std::ofstream fichier(cheminFichier, std::ios::app);
            if (!fichier) return; // Un échec de log ne doit jamais interrompre une opération métier.

            fichier << "[" << horodatage() << "] ALERTE SEUIL CRITIQUE - "
                    << "Ref: " << produit.getReference()
                    << " | Nom: " << produit.getNom()
                    << " | Stock: " << produit.getQuantiteStock()
                    << " | Seuil: " << produit.getSeuilAlerte() << "\n";
        }
};