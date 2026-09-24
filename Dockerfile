# ---------- Étape 1 : compilation ----------
# gcc:13 fournit g++ 13, avec un support suffisant de -std=c++23 pour ce projet
# (notamment std::chrono::year_month_day utilisé dans le modèle de domaine).
FROM gcc:13 AS builder

WORKDIR /app

# 'make' n'est pas garanti présent dans l'image gcc de base.
RUN apt-get update \
    && apt-get install -y --no-install-recommends make \
    && rm -rf /var/lib/apt/lists/*

# On ne copie que ce qui est nécessaire à la compilation (voir .dockerignore) :
# les sources et le Makefile, pas data/ ni bin/ qui n'ont rien à faire dans l'image.
COPY Makefile ./
COPY include ./include
COPY src ./src

# Compile sans lancer le programme (make run exigerait un terminal interactif
# pendant le build, ce qui n'est pas possible).
RUN make build

# ---------- Étape 2 : exécution ----------
FROM debian:bookworm-slim

# libstdc++6 est nécessaire pour exécuter un binaire compilé avec g++,
# même si les outils de compilation eux-mêmes ne sont plus nécessaires ici.
RUN apt-get update \
    && apt-get install -y --no-install-recommends libstdc++6 \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /app/data

WORKDIR /app

COPY --from=builder /app/bin/mon_programme ./bin/mon_programme

# Le contenu de data/ doit survivre à la suppression du conteneur :
# à monter explicitement au lancement avec -v $(pwd)/data:/app/data
VOLUME ["/app/data"]

# Application interactive (menus lus sur stdin) : à lancer avec 'docker run -it'.
ENTRYPOINT ["./bin/mon_programme"]
