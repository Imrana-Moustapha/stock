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
- **Configuration dynamique** : chargement des variables d'environnement depuis un fichier `.env` au démarrage

## Comment configurer le fichier .env

Pour que l'application puisse charger ses configurations au démarrage, vous devez créer un fichier nommé `.env` directement à la racine de votre projet.

### 1. Créer le fichier
À la racine du projet (`~/Bureau/cpp/`), créez un fichier `.env` :
```bash
touch .env

```

### 2. Ajouter les variables requises

Ouvrez le fichier `.env` avec votre éditeur préféré et ajoutez les lignes suivantes en adaptant les valeurs selon vos besoins :

```env
DATABASE_URL=postgres://user:password@localhost:5432/mydb
PORT=8080
API_KEY=mon_secret_12345

```

### 3. Particularité si vous utilisez CMake

Si vous compilez et lancez l'application via CMake, assurez-vous que le fichier `.env` est accessible à l'exécutable (par exemple, en le copiant dans le dossier de travail ou de build) :

```bash
cp ../.env .

```

*(Le fichier `.env` ne doit jamais être versionné dans Git, il est déjà exclu par le `.gitignore`).*

## Structure du projet

```
.
├── bin/                       # Exécutable généré (non versionné)
├── build/                     # Dossier de build CMake (non versionné)
├── data/                      # Fichiers de persistance (stock.txt, mouvements.txt, alertes.log)
├── include/                   # Fichiers d'en-tête (.hpp)
├── src/                       # Fichiers sources (.cpp)
├── .env                       # Fichier de configuration des variables d'environnement
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

L'exécutable compilé est automatiquement redirigé vers le dossier `bin/` à la racine. Pour le lancer :

```bash
# Depuis le dossier build (en s'assurant d'avoir le .env à portée)
cp ../.env .
../bin/cpp

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
git clone [https://github.com/Imrana-Moustapha/stock.git](https://github.com/Imrana-Moustapha/stock.git)
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

## Statut du projet

Les six modules de l'application (produits, mouvements de stock, recherche, commandes/fournisseurs, statistiques, administration) sont fonctionnels.

**Persistance** : les produits (`data/stock.txt`) et le journal des mouvements (`data/mouvements.txt`) sont sauvegardés automatiquement après chaque opération qui les modifie, et rechargés au démarrage. Les alertes de seuil critique sont écrites dans `data/alertes.log`.

## Auteur

Imrana