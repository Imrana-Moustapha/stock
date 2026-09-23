# Système de Gestion de Stock

Application console en C++ moderne (C++23) pour la gestion d'un stock de produits : suivi des quantités, mouvements de stock, fournisseurs et commandes, alertes de seuil critique, et journal d'audit des opérations.

Ce projet sert de support pour la mise en pratique d'une architecture orientée objet rigoureuse : principes SOLID, design patterns (Factory, Observer, Strategy) et pratiques modernes du C++ (RAII, smart pointers, `std::chrono`, STL avancée).

## Fonctionnalités

- **Gestion des produits** : ajout, modification, suppression, hiérarchie polymorphe (produits standards, périssables, etc.)
- **Mouvements de stock** : entrées, sorties, ajustements, avec journal d'audit horodaté
- **Recherche et filtres avancés** : par nom, référence, catégorie, proximité de péremption, seuil critique
- **Commandes et fournisseurs** : suivi des fournisseurs, propositions de réapprovisionnement, statut des commandes
- **Statistiques** : valeur totale du stock, produits les plus mouvementés, détection des produits dormants
- **Administration** : import/export de données, configuration des seuils d'alerte, gestion des utilisateurs et des rôles

## Structure du projet

```
.
├── include/
│   ├── header.hpp          # Déclarations communes (menus, utilitaires, couleurs)
│   └── models/              # Classes du modèle de domaine
│       ├── Produit.hpp
│       ├── Fournisseur.hpp
│       ├── Commande.hpp
│       ├── MouvementStock.hpp
│       └── Utilisateur.hpp
├── src/
│   ├── main.cpp
│   ├── menu.cpp
│   ├── sub-menu/            # Sous-menus de l'application
│   │   ├── produits.cpp
│   │   ├── mouvements.cpp
│   │   ├── recherche.cpp
│   │   ├── commandes.cpp
│   │   ├── statistiques.cpp
│   │   └── administration.cpp
│   └── utils/
│       └── utils.cpp
├── Makefile
└── README.md
```

## Prérequis

- Un compilateur supportant C++23 (GCC 13 ou plus récent recommandé)
- `make`

## Compilation

```bash
make
```

Génère l'exécutable `mon_programme` à la racine du projet.

Pour nettoyer les fichiers objets et l'exécutable :

```bash
make clean
```

## Exécution

```bash
./mon_programme
```

## Architecture logicielle

Le projet est organisé en couches aux responsabilités distinctes :

| Couche | Rôle |
|---|---|
| Modèle (`include/models/`) | Classes de données métier (`Produit`, `Commande`, `Fournisseur`, `MouvementStock`, `Utilisateur`) |
| Logique / Service | Règles métier : seuils, calculs de valeur, validations (à venir) |
| Persistance | Interface abstraite pour la sauvegarde/chargement des données (à venir) |
| Interface (CLI) | Menu console (`menu.cpp`, `src/sub-menu/`) |

### Design patterns

- **Factory** — création polymorphe des produits selon leur type
- **Observer** — notification automatique lors du franchissement d'un seuil de stock
- **Strategy** — politiques de tarification interchangeables

## Statut du projet

En développement actif. Le squelette de l'architecture (modèle, menus, Makefile) est en place ; la logique métier et la persistance sont en cours d'implémentation.

## Auteur

Imrana