#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <math.h>
#include <string.h>

// Fonction de calcul intensif réaliste
double intensive_computation(int value) {
    double result = (double)value;
    // Calculs mathématiques qui préservent la diversité des données
    for (int i = 0; i < 5000; i++) {
        result = result * 1.0001 + sin(result / 100.0);
        result = fmod(result, 1000.0) + 1.0;  // Évite overflow, garde diversité
        result += cos((double)i) * 0.01;
    }
    return result;
}

// Fonction de réduction monoprocesseur
void monoprocessor_reduction(const int* data, int size) {
    printf("[MONO] Début du calcul intensif sur %d éléments...\n", size);
    double sum = 0.0;
    
    double start_compute = MPI_Wtime();
    for (int i = 0; i < size; i++) {
        sum += intensive_computation(data[i]);
        // Affichage du progrès
        if (i > 0 && i % (size/10) == 0) {
            printf("[MONO] Progrès: %.1f%% (élément %d/%d)\n", 
                   (double)i/size * 100, i, size);
        }
    }
    double end_compute = MPI_Wtime();
    
    printf("[MONO] Temps de calcul pur: %.6f secondes\n", end_compute - start_compute);
    printf("[MONO] Somme finale: %.2f\n", sum);
}

// Fonction de réduction collective avec MPI_Scatter et MPI_Reduce
void collective_reduction(int* data, int size, int rank, int world_size) {
    double start_time, end_time;
    
    // Calcul de la portion standard (sans reste)
    int base_portion = size / world_size;
    int remainder = size % world_size;
    
    // Taille de portion pour ce processus
    int my_portion_size = base_portion;
    if (rank < remainder) {
        my_portion_size++;  // Les premiers processus prennent le reste
    }
    
    // Allocation pour les données locales
    int* local_data = (int*)malloc(my_portion_size * sizeof(int));
    
    // Préparation des tailles et déplacements pour Scatterv
    int* sendcounts = NULL;
    int* displs = NULL;
    
    if (rank == 0) {
        sendcounts = (int*)malloc(world_size * sizeof(int));
        displs = (int*)malloc(world_size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < world_size; i++) {
            sendcounts[i] = base_portion;
            if (i < remainder) sendcounts[i]++;
            displs[i] = offset;
            offset += sendcounts[i];
        }
        
        printf("[COLLECTIVE] Distribution avec MPI_Scatterv...\n");
        for (int i = 0; i < world_size; i++) {
            printf("[COLLECTIVE] Processus %d: %d éléments (indices %d à %d)\n", 
                   i, sendcounts[i], displs[i], displs[i] + sendcounts[i] - 1);
        }
    }
    
    // Distribution des données avec MPI_Scatterv (gère les tailles variables)
    start_time = MPI_Wtime();
    MPI_Scatterv(data, sendcounts, displs, MPI_INT,
                 local_data, my_portion_size, MPI_INT,
                 0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("[COLLECTIVE] Temps de distribution: %.6f secondes\n", end_time - start_time);
    }
    
    // Vérification des données reçues (debug)
    printf("[PROCESSUS %d] Reçu %d éléments. Premiers: [%d, %d, %d] Derniers: [%d, %d, %d]\n", 
           rank, my_portion_size, 
           local_data[0], local_data[1], local_data[2],
           local_data[my_portion_size-3], local_data[my_portion_size-2], local_data[my_portion_size-1]);
    
    // Synchronisation avant calcul
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Calcul local intensif
    start_time = MPI_Wtime();
    printf("[PROCESSUS %d] Calcul intensif sur %d éléments...\n", rank, my_portion_size);
    
    double local_sum = 0.0;
    for (int i = 0; i < my_portion_size; i++) {
        local_sum += intensive_computation(local_data[i]);
        
        // Affichage du progrès
        if (i > 0 && i % (my_portion_size/5) == 0) {
            printf("[PROCESSUS %d] Progrès: %.1f%% (%.2f)\n", 
                   rank, (double)i/my_portion_size * 100, local_sum);
        }
    }
    end_time = MPI_Wtime();
    
    printf("[PROCESSUS %d] Calcul terminé en %.6f secondes, somme locale: %.2f\n", 
           rank, end_time - start_time, local_sum);
    
    // Collecte des résultats locaux (optionnel, pour debug)
    double* all_local_sums = NULL;
    if (rank == 0) {
        all_local_sums = (double*)malloc(world_size * sizeof(double));
    }
    
    start_time = MPI_Wtime();
    MPI_Gather(&local_sum, 1, MPI_DOUBLE, 
               all_local_sums, 1, MPI_DOUBLE, 
               0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("[COLLECTIVE] Temps de collecte (MPI_Gather): %.6f secondes\n", end_time - start_time);
        printf("[COLLECTIVE] Sommes locales collectées:\n");
        for (int i = 0; i < world_size; i++) {
            printf("  Processus %d: %.2f\n", i, all_local_sums[i]);
        }
    }
    
    // Réduction globale avec MPI_Reduce
    start_time = MPI_Wtime();
    double global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("[COLLECTIVE] Temps de réduction (MPI_Reduce): %.6f secondes\n", end_time - start_time);
        printf("[COLLECTIVE] Somme globale finale: %.2f\n", global_sum);
        
        // Vérification manuelle
        double manual_sum = 0.0;
        for (int i = 0; i < world_size; i++) {
            manual_sum += all_local_sums[i];
        }
        printf("[COLLECTIVE] Vérification manuelle: %.2f (différence: %.6f)\n", 
               manual_sum, fabs(global_sum - manual_sum));
        
        free(all_local_sums);
        free(sendcounts);
        free(displs);
    }
    
    free(local_data);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Paramètres du problème
    int data_size = 1000;
    int* global_data = NULL;

    if (rank == 0) {
        printf("=====================================\n");
        printf("    RÉDUCTION PARALLÈLE MPI\n");
        printf("=====================================\n");
        printf("Taille des données: %d éléments\n", data_size);
        printf("Nombre de processus: %d\n", world_size);
        printf("Calcul intensif: 5,000 opérations/élément\n");
        printf("Utilisation: MPI_Scatterv + MPI_Gather + MPI_Reduce\n");
        printf("=====================================\n\n");
        
        // Génération des données
        global_data = (int*)malloc(data_size * sizeof(int));
        srand(42);  // Graine fixe pour reproductibilité
        
        printf("Génération de %d nombres aléatoires...\n", data_size);
        for (int i = 0; i < data_size; i++) {
            global_data[i] = rand() % 100 + 1;  // Valeurs 1-100
        }
        
        // Affichage d'un échantillon
        printf("Échantillon des données (10 premiers): ");
        for (int i = 0; i < 10; i++) {
            printf("%d ", global_data[i]);
        }
        printf("\n");
        printf("Échantillon des données (10 derniers): ");
        for (int i = data_size-10; i < data_size; i++) {
            printf("%d ", global_data[i]);
        }
        printf("\n\n");
    }

    // Synchronisation avant début
    MPI_Barrier(MPI_COMM_WORLD);
    double total_start = MPI_Wtime();

    // Exécution selon le mode
    if (world_size == 1) {
        if (rank == 0) {
            printf("=== MODE MONOPROCESSEUR ===\n");
            monoprocessor_reduction(global_data, data_size);
        }
    } else {
        printf("=== MODE PARALLÈLE ===\n");
        collective_reduction(global_data, data_size, rank, world_size);
    }

    // Temps total et statistiques finales
    double total_end = MPI_Wtime();
    
    if (rank == 0) {
        printf("\n=====================================\n");
        printf("         RÉSULTATS FINAUX\n");
        printf("=====================================\n");
        
        if (world_size == 1) {
            printf("Mode d'exécution: MONOPROCESSEUR\n");
        } else {
            printf("Mode d'exécution: PARALLÈLE (%d processus)\n", world_size);
            printf("Distribution: MPI_Scatterv (gestion tailles variables)\n");
            printf("Collecte: MPI_Gather (debug)\n");
            printf("Réduction: MPI_Reduce (somme globale)\n");
            printf("Accélération théorique maximale: %.2fx\n", (double)world_size);
        }
        
        printf("Temps d'exécution total: %.6f secondes\n", total_end - total_start);
        printf("Opérations totales: %d × 5,000 = %d opérations\n", 
               data_size, data_size * 5000);
        printf("Débit: %.0f opérations/seconde\n", 
               (data_size * 5000.0) / (total_end - total_start));
        
        if (world_size > 1) {
            printf("\nPour comparer avec la version séquentielle:\n");
            printf("  mpirun -np 1 ./collective_reduction\n");
        }
        
        printf("=====================================\n");
        free(global_data);
    }

    MPI_Finalize();
    return 0;
}