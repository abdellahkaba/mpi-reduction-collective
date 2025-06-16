# MPI Collective Reduction - Calcul Parallèle avec Docker

![MPI](https://img.shields.io/badge/MPI-Parallel%20Computing-blue)
![Docker](https://img.shields.io/badge/Docker-Containerized-green)
![Performance](https://img.shields.io/badge/Speedup-3.62x-brightgreen)

## 📌 Présentation

Ce projet démontre une implémentation de **réduction collective MPI** pour effectuer des calculs intensifs sur un cluster de machines. Il compare les performances entre :

- Une version **monoprocesseur** (séquentielle)
- Une version **parallèle** utilisant 4 nœuds MPI

L'approche utilise :
- `MPI_Scatterv` pour une distribution équilibrée des données
- `MPI_Reduce` pour la réduction finale
- Un cluster Docker pour l'exécution distribuée

## 🧠 Explication Technique

### Algorithme de Réduction
```python
Données initiales (N éléments)
       ↓
MPI_Scatterv → Distribution vers P processus
       ↓
Calcul intensif en parallèle (10k opérations/élément)
       ↓
MPI_Reduce → Somme globale
Un exemple de réduction collective MPI avec distribution intelligente des données et calcul intensif, exécuté dans un cluster Docker.

```
### 🛠 Installation & Exécution
```python
    1. Lancer le cluster MPI:
        docker compose up --build -d
        
    2. Se connecter au nœud maître: 
        docker exec -it --user vagrant mpi-node1 bash
        
    3. Compiler le programme:
        mpicc collective_reduction.c -o collective_reduction -lm
        
    4. Exécuter en parallèle (4 processus):
        mpirun -np 4 -host mpi-node1,mpi-node2,mpi-node3,mpi-node4 ./collective_reduction

    5. Comparer avec la version séquentielle:
        mpirun -np 1 ./collective_reduction
```  
### 📊 Résultats de Performance

```python
## Comparaison Monoprocesseur vs Parallèle (4 nœuds)

| Métrique               | Parallèle (4x) | Séquentiel | Accélération |
|------------------------|---------------|------------|--------------|
| Temps total            | 0.201s        | 0.730s     | 3.62x        |
| Temps calcul moyen     | 0.191s        | 0.730s     | -            |
| Temps distribution     | 0.002s        | N/A        | -            |
| Somme finale           | 818.36        | 818.36     | -            |

**Efficacité parallèle**: 90.5% (3.62x/4.00x théorique)

```
### Sortie Type

```plaintext
=== EXÉCUTION PARALLÈLE ===
[COLLECTIVE] Distribution des données...
[PROCESSUS 0] Début du calcul sur 250 éléments...
[PROCESSUS 0] Calcul terminé en 0.194s, somme locale: 204.59
[COLLECTIVE] Somme globale finale: 818.36
Temps total: 0.201s
