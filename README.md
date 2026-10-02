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
- **Authentification** : connexion par identifiant/mot de passe, mots de passe hachés (sel + SHA-256), accès à l'administration réservé aux comptes `ADMIN`

## Authentification

Au démarrage, l'application demande un identifiant et un mot de passe. Les comptes sont persistés dans `data/utilisateurs.txt`, avec le mot de passe **haché** (sel aléatoire + SHA-256) — jamais stocké en clair, nulle part.

Trois rôles existent : `ADMIN`, `GESTIONNAIRE`, `MAGASINIER`. Seul un compte `ADMIN` accède au menu Administration.

### Premier démarrage : créer le compte admin

Tant qu'aucun utilisateur n'existe, l'application propose de créer le premier compte `ADMIN`, de deux façons :

- **Interactive** (par défaut) : l'application demande un identifiant et un mot de passe au premier lancement.
- **Automatique via `.env`** (pratique pour Docker ou un déploiement scripté) : définir `ADMIN_BOOTSTRAP_USERNAME` et `ADMIN_BOOTSTRAP_PASSWORD` avant le premier lancement.

```bash
cp .env.example .env
# puis éditer .env et changer ADMIN_BOOTSTRAP_PASSWORD
```

Ces deux variables ne servent qu'à l'amorçage : une fois le premier compte créé, elles n'ont plus aucun effet, et le mot de passe fourni n'est jamais relu en clair par la suite. Les comptes suivants se créent depuis le menu Administration (accessible uniquement à un `ADMIN` déjà connecté).

**`.env` n'est jamais versionné** (couvert par `.gitignore`) — seul `.env.example`, sans valeurs réelles, est committé pour documenter les variables attendues.

### Autres variables

`DATA_PATH` (optionnelle) change le dossier où sont stockés les fichiers de données ; `data/` par défaut.

## Structure du projet

```
.
├── bin/                       # Exécutable généré (non versionné)
├── build/                     # Dossier de build CMake (non versionné)
├── data/                      # Fichiers de persistance : stock.txt, mouvements.txt,
│                               #   fournisseurs.txt, commandes.txt, commande_lignes.txt,
│                               #   utilisateurs.txt, alertes.log (aucun non versionné)
├── include/                   # Fichiers d'en-tête (.hpp)
├── src/                       # Fichiers sources (.cpp)
├── tests/                     # Tests unitaires (Catch2)
├── .env                       # Configuration locale (non versionné)
├── .env.example                # Modèle documenté, à copier en .env
├── .gitignore
├── .dockerignore
├── Dockerfile
├── docker-compose.yml
├── CMakeLists.txt
├── Makefile
└── README.md

```

## Prérequis

* Un compilateur supportant C++23 (GCC 13 ou plus récent recommandé)
* `make` et/ou **CMake** (version 3.14 ou supérieure)

## Compilation et exécution

### Option 1 : Avec Make (Méthode rapide)

```bash
make

```

Compile le projet et lance l'exécutable automatiquement (`bin/mon_programme`).

Cibles disponibles :

| Commande | Effet |
| --- | --- |
| `make` ou `make all` | Compile si besoin, puis lance le programme |
| `make build` | Compile seulement, sans lancer le programme |
| `make clean` | Supprime les fichiers objets, dépendances et exécutables |

### Option 2 : Avec CMake (Méthode recommandée pour les IDE et configurations modernes)

```bash
mkdir -p build
cd build
cmake ..
cmake --build .

```

L'exécutable compilé est automatiquement redirigé vers le dossier `bin/` à la racine, sous le même nom qu'avec Make (`bin/mon_programme`). Comme il cherche `.env` et `data/` par rapport à son répertoire de travail, lance-le depuis la racine du projet plutôt que depuis `build/` :

```bash
cd ..            # retour à la racine du projet, si tu étais dans build/
./bin/mon_programme

```

## Architecture logicielle

Le projet est organisé en couches aux responsabilités distinctes :

| Couche | Rôle |
| --- | --- |
| Modèle (`include/models/`) | Classes de données métier (`Produit`, `Commande`, `Fournisseur`, `MouvementStock`, `Utilisateur`) |
| Logique / Service | Règles métier : seuils, calculs de valeur, validations |
| Persistance | Interface abstraite pour la sauvegarde/chargement des données |
| Interface (CLI) | Menu console (`menu.cpp`, `src/menus/`) |

### Design patterns

* **Factory** — création polymorphe des produits selon leur type
* **Observer** — notification automatique lors du franchissement d'un seuil de stock
* **Strategy** — politiques de tarification interchangeables

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
| --- | --- |
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
git commit -m "Ajoute la lecture du fichier .env"

```


4. Pousser la branche et ouvrir une Pull Request vers `develop` :
```bash
git push -u origin feature/nom-de-la-fonctionnalite

```



### Conventions de nommage des branches

* `feature/xxx` — nouvelle fonctionnalité
* `fix/xxx` — correction de bug
* `refactor/xxx` — refactorisation sans changement de comportement
* `docs/xxx` — documentation uniquement

## Utilisation avec Docker

L'application peut être compilée et exécutée dans un conteneur, sans installer GCC ni `make` sur la machine hôte.

### Construire et lancer l'image

```bash
docker build -t gestion-stock .
docker run -it --rm -v $(pwd)/data:/app/data gestion-stock

```

### Avec Docker Compose

```bash
docker compose run --rm stock

```

### Premier démarrage : créer le compte admin

Le conteneur est lancé avec `-it`, donc la création interactive du premier compte admin (décrite dans [Authentification](#authentification)) fonctionne normalement. Pour un démarrage sans interaction (script, CI), passe les identifiants d'amorçage via `-e` plutôt que par un fichier `.env` dans l'image — `.env` n'est volontairement pas copié dans l'image, pour éviter d'y figer un secret :

```bash
docker run -it --rm -v $(pwd)/data:/app/data \
  -e ADMIN_BOOTSTRAP_USERNAME=admin \
  -e ADMIN_BOOTSTRAP_PASSWORD=change-moi \
  gestion-stock

```

## Statut du projet

Les six modules de l'application (produits, mouvements de stock, recherche, commandes/fournisseurs, statistiques, administration) sont fonctionnels, avec authentification et contrôle d'accès par rôle.

**Persistance** : produits, mouvements, fournisseurs, commandes et comptes utilisateurs sont tous sauvegardés automatiquement après chaque opération qui les modifie, et rechargés au démarrage. Les alertes de seuil critique sont écrites dans `data/alertes.log`.

**Tests** : suite de tests unitaires (Catch2) couvrant le modèle, les services, les repositories et le hachage de mots de passe. Voir `tests/`.

## Auteur

Imrana