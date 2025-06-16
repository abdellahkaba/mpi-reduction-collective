# MPI Collective Reduction avec Docker

[![MPI](https://img.shields.io/badge/MPI-Parallel%20Computing-blue)](https://www.open-mpi.org/)
[![Docker](https://img.shields.io/badge/Docker-Containerized-green)](https://www.docker.com/)

Un exemple de réduction collective MPI avec distribution intelligente des données et calcul intensif, exécuté dans un cluster Docker.

## 🛠 Installation & Exécution

    Lancer le cluster MPI:
    docker compose up --build -d
    Se connecter au nœud maître:
    docker exec -it --user vagrant mpi-node1 bash


## 📊 Résultats de Performance

### Comparaison Monoprocesseur vs Parallèle (4 nœuds)

| Métrique               | Parallèle (4x) | Séquentiel | Accélération |
|------------------------|---------------|------------|--------------|
| Temps total            | 0.201s        | 0.730s     | 3.62x        |
| Temps calcul moyen     | 0.191s        | 0.730s     | -            |
| Temps distribution     | 0.002s        | N/A        | -            |
| Somme finale           | 818.36        | 818.36     | -            |

**Efficacité parallèle**: 90.5% (3.62x/4.00x théorique)

### Sortie Type

```plaintext
=== EXÉCUTION PARALLÈLE ===
[COLLECTIVE] Distribution des données...
[PROCESSUS 0] Début du calcul sur 250 éléments...
[PROCESSUS 0] Calcul terminé en 0.194s, somme locale: 204.59
[COLLECTIVE] Somme globale finale: 818.36
Temps total: 0.201s
