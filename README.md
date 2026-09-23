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
├── bin/                      # Exécutable généré (non versionné)
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
├── .gitignore
├── Makefile
└── README.md
```

## Prérequis

- Un compilateur supportant C++23 (GCC 13 ou plus récent recommandé)
- `make`

## Compilation et exécution

```bash
make
```

Compile le projet (si nécessaire) dans `bin/mon_programme`, puis lance l'exécutable automatiquement.

Cibles disponibles :

| Commande | Effet |
|---|---|
| `make` ou `make all` | Compile si besoin, puis lance le programme |
| `make build` | Compile seulement, sans lancer le programme |
| `make run` | Identique à `make` : compile si besoin puis lance |
| `make clean` | Supprime les fichiers objets, dépendances et l'exécutable |

Make ne recompile que ce qui a changé : un `.cpp` modifié entraîne la recompilation de son objet, et grâce à `-MMD -MP`, un `.hpp` modifié entraîne la recompilation de tous les fichiers qui l'incluent.

Pour lancer l'exécutable manuellement sans passer par `make run` :

```bash
./bin/mon_programme
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

## Dépôt

```bash
git clone https://github.com/Imrana-Moustapha/stock.git
cd stock
git checkout develop
```

La branche par défaut du dépôt est **`develop`** : c'est elle que vous récupérez automatiquement après un `git clone`, et c'est sur elle que doit partir tout nouveau travail.

## Workflow Git pour les collaborateurs

Le projet suit un modèle à deux branches principales :

| Branche | Rôle |
|---|---|
| `main` | Code stable, prêt à être livré. On n'y pousse jamais directement. |
| `develop` | Branche d'intégration, par défaut. Toutes les fonctionnalités y sont fusionnées avant de partir vers `main`. |

### Ajouter une fonctionnalité ou corriger un bug

1. Partir toujours de `develop` à jour :
   ```bash
   git checkout develop
   git pull origin develop
   ```
2. Créer une branche dédiée, nommée selon ce qu'elle contient :
   ```bash
   git checkout -b feature/nom-de-la-fonctionnalite
   # ou : git checkout -b fix/nom-du-bug
   ```
3. Committer par petites étapes, avec des messages clairs :
   ```bash
   git add .
   git commit -m "Ajoute la validation de saisie du menu produits"
   ```
4. Pousser la branche et ouvrir une Pull Request vers `develop` (jamais directement vers `main`) :
   ```bash
   git push -u origin feature/nom-de-la-fonctionnalite
   ```
5. Une fois la Pull Request relue et validée, elle est fusionnée dans `develop`. `main` n'est mis à jour que lors d'une livraison stable, via une Pull Request `develop` → `main`.

### Conventions de nommage des branches

- `feature/xxx` — nouvelle fonctionnalité
- `fix/xxx` — correction de bug
- `refactor/xxx` — refactorisation sans changement de comportement
- `docs/xxx` — documentation uniquement

### Avant de committer

- Vérifier que le projet compile sans avertissement : `make clean && make build`
- Ne jamais committer les fichiers générés (`*.o`, `*.d`, le dossier `bin/`) — déjà exclus par `.gitignore`



En développement actif. Le squelette de l'architecture (modèle, menus, Makefile) est en place ; la logique métier et la persistance sont en cours d'implémentation.
