#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <math.h>
#include <string.h>

// Fonction de calcul intensif pour simuler un travail réel
double intensive_computation(int value) {
    double result = (double)value;
    // Calculs mathématiques intensifs
    for (int i = 0; i < 10000; i++) {
        result = sqrt(result * result + 1.0);
        result = sin(result) * cos(result) + 1.0;
        result = log(result + 1.0) * exp(result / 1000.0);
    }
    return result;
}

// Fonction de réduction monoprocesseur avec calcul intensif
void monoprocessor_reduction(const int* data, int size) {
    printf("[MONO] Début du calcul intensif...\n");
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += intensive_computation(data[i]);
        // Affichage du progrès
        if (i % (size/10) == 0) {
            printf("[MONO] Progrès: %.1f%%\n", (double)i/size * 100);
        }
    }
    printf("[MONO] Somme finale: %.2f\n", sum);
}

// Fonction de réduction collective optimisée
void collective_reduction(int* data, int size, int rank, int world_size) {
    double start_time, end_time;
    
    // Calcul de la portion pour chaque processus
    int portion_size = size / world_size;
    int remainder = size % world_size;
    
    // Ajustement pour le dernier processus (gestion du reste)
    if (rank == world_size - 1) {
        portion_size += remainder;
    }
    
    int* local_data = (int*)malloc(portion_size * sizeof(int));
    
    // Mesure du temps de distribution
    start_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("[COLLECTIVE] Distribution des données...\n");
        // Le processus 0 distribue ses données et celles des autres
        for (int i = 0; i < world_size; i++) {
            int send_size = size / world_size;
            if (i == world_size - 1) send_size += remainder;
            
            if (i == 0) {
                // Copie locale pour le processus 0
                memcpy(local_data, data, send_size * sizeof(int));
            } else {
                // Envoi aux autres processus
                MPI_Send(data + i * (size / world_size), send_size, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        // Les autres processus reçoivent leurs données
        MPI_Recv(local_data, portion_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    end_time = MPI_Wtime();
    if (rank == 0) {
        printf("[COLLECTIVE] Temps de distribution: %.6f secondes\n", end_time - start_time);
    }
    
    // Synchronisation avant le calcul
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Calcul local intensif avec mesure de temps
    start_time = MPI_Wtime();
    printf("[PROCESSUS %d] Début du calcul sur %d éléments...\n", rank, portion_size);
    
    double local_sum = 0.0;
    for (int i = 0; i < portion_size; i++) {
        local_sum += intensive_computation(local_data[i]);
        
        // Affichage du progrès pour chaque processus
        if (i % (portion_size/5) == 0) {
            printf("[PROCESSUS %d] Progrès: %.1f%%\n", rank, (double)i/portion_size * 100);
        }
    }
    
    end_time = MPI_Wtime();
    printf("[PROCESSUS %d] Calcul terminé en %.6f secondes, somme locale: %.2f\n", 
           rank, end_time - start_time, local_sum);
    
    // Synchronisation avant la réduction
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Réduction globale avec mesure de temps
    start_time = MPI_Wtime();
    double global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    // Affichage des résultats
    if (rank == 0) {
        printf("[COLLECTIVE] Temps de réduction: %.6f secondes\n", end_time - start_time);
        printf("[COLLECTIVE] Somme globale finale: %.2f\n", global_sum);
    }
    
    free(local_data);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Paramètres optimisés
    int data_size = 1000;  // Taille réduite mais calcul intensif
    int* global_data = NULL;

    // Génération des données (root seulement)
    if (rank == 0) {
        printf("=== COMPARAISON MONOPROCESSEUR vs PARALLÈLE ===\n");
        printf("Taille des données: %d éléments\n", data_size);
        printf("Nombre de processus: %d\n", world_size);
        printf("Calcul intensif: 10,000 opérations par élément\n\n");
        
        global_data = (int*)malloc(data_size * sizeof(int));
        srand(42);  // Graine fixe pour reproductibilité
        printf("Génération des données...\n");
        for (int i = 0; i < data_size; i++) {
            global_data[i] = rand() % 100 + 1;  // Valeurs 1-100 (éviter 0)
        }
        printf("Données générées avec succès!\n\n");
    }

    // Synchronisation avant démarrage
    MPI_Barrier(MPI_COMM_WORLD);
    double total_start_time = MPI_Wtime();

    // Exécution selon le nombre de processus
    if (world_size == 1) {
        if (rank == 0) {
            printf("=== EXÉCUTION MONOPROCESSEUR ===\n");
            monoprocessor_reduction(global_data, data_size);
        }
    } else {
        printf("=== EXÉCUTION PARALLÈLE ===\n");
        collective_reduction(global_data, data_size, rank, world_size);
    }

    // Mesure du temps total
    double total_end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("\n=== RÉSULTATS FINAUX ===\n");
        if (world_size == 1) {
            printf("Mode: MONOPROCESSEUR\n");
        } else {
            printf("Mode: PARALLÈLE (%d processus)\n", world_size);
            printf("Accélération théorique max: %.2fx\n", (double)world_size);
        }
        printf("Temps d'exécution total: %.6f secondes\n", total_end_time - total_start_time);
        
        if (world_size > 1) {
            printf("\nPour comparer, exécutez:\n");
            printf("mpirun -np 1 ./collective_reduction\n");
        }
        
        free(global_data);
    }

    MPI_Finalize();
    return 0;
}