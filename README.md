# MPI Collective Reduction - Calcul Parallèle avec Docker

![MPI](https://img.shields.io/badge/MPI-Parallel%20Computing-blue)
![Docker](https://img.shields.io/badge/Docker-Containerized-green)
![Performance](https://img.shields.io/badge/Speedup-3.62x-brightgreen)

## 📌 Présentation

Ce projet démontre une implémentation de **réduction collective MPI** pour effectuer des calculs intensifs sur un cluster de machines. Il compare les performances entre :

- Une version **monoprocesseur** (séquentielle)
- Une version **parallèle** utilisant 4 nœuds MPI

L'approche utilise :

- Monoprocesseur : Traite tous les éléments séquentiellement

- Collective MPI :
  
  Distribution des données (scatter)

  Calcul parallèle local

  Agrégation des résultats (reduce)

## 🧠 Explication Technique

### Algorithme de Réduction
```python
Données initiales (N éléments)
       ↓
Scatter → Distribution vers P processus
       ↓
Calcul intensif en parallèle (10k opérations/élément)
       ↓
Reduce → Somme globale
Un exemple de réduction collective MPI avec distribution intelligente des données et calcul intensif, exécuté dans un cluster Docker.

```
### 🛠 Installation & Exécution
```python
    1. Lancer le cluster MPI:
        docker compose up --build -d
        
    2. Se connecter au nœud maître: 
        docker exec -it --user vagrant mpi-node1 bash
        cd ./mpi_code
        
    3. Compiler le programme:
        mpicc collective_reduction.c -o collective_reduction -lm
        
    4. Exécuter en parallèle (4 processus):
        mpirun -np 4 -host mpi-node1,mpi-node2,mpi-node3,mpi-node4 ./collective_reduction

    5. Comparer avec la version séquentielle:
        mpirun -np 1 ./collective_reduction
    NB :
       Utilisez les commandes si les noeuds ne sont pas connectés sur le noeud principal
       vagrant@mpi-node1: ssh vagrant@mpi-node2
       vagrant@mpi-node1: ssh vagrant@mpi-node3
       vagrant@mpi-node1: ssh vagrant@mpi-node4

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
