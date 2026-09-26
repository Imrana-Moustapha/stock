#pragma once

// Déclaration anticipée : ce header n'a besoin que d'une référence à
// GestionnaireStock, pas de sa définition complète (évite un couplage
// inutile pour les fichiers qui n'ont besoin que des prototypes de menu).
class GestionnaireStock;

void menu_principal(GestionnaireStock& gestionnaire);
void sous_menu_produits(GestionnaireStock& gestionnaire);
void sous_menu_mouvements(GestionnaireStock& gestionnaire);
void sous_menu_recherche(GestionnaireStock& gestionnaire);
void sous_menu_commandes();
void sous_menu_statistiques(GestionnaireStock& gestionnaire);
void sous_menu_administration();