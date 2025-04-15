// COMP2521 24T3 - Assignment 2
// Implementation of the Map ADT
// Written by Melina Salardini (z5393518)
// on 14/11/2024

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Map.h"

#define DEFAULT_NUM_ROADS 0

struct map {
    int numCities;
    int numRoads;
    char **cityNames;
    int **roads;
};

// function prototypes
static int compareRoads(const void *a, const void *b);


Map MapNew(int numCities) {
    // allocate memory for the map
    Map m = malloc(sizeof(*m));
    if (m == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }

    m->numCities = numCities;
    m->numRoads = DEFAULT_NUM_ROADS;

    // allocate memory for cityNames and initilize them to NULL
    m->cityNames = malloc(numCities * sizeof(char *));
    if (m->cityNames == NULL) {
        fprintf(stderr, "out of memory");
        free(m);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numCities; i++) {
        m->cityNames[i] = NULL;
    }

    // allocate memory for roads
    m->roads = malloc(numCities * sizeof(int *));
    if (m->roads == NULL) {
        fprintf(stderr, "out of memory");
        free(m->cityNames);
        free(m);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < numCities; i++) {
        m->roads[i] = malloc(numCities * sizeof(int));
        if (m->roads[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(m->roads[j]);
            }
            free(m->roads);
            free(m->cityNames);
            free(m);
            fprintf(stderr, "error: out of memory\n");
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < numCities; j++) {
            m->roads[i][j] = 0;
        }
    }

    return m;
}

void MapFree(Map m) {
    if (m == NULL) return;

    // free cityNames
    for (int i = 0; i < m->numCities; i++) {
        free(m->cityNames[i]);
    }
    free(m->cityNames);

    // free roads
    for (int i = 0; i < m->numCities; i++) {
        free(m->roads[i]);
    }
    free(m->roads);

    // free map
    free(m);
}

int MapNumCities(Map m) {
    return m->numCities;
}

int MapNumRoads(Map m) {
    return m->numRoads;
}

void MapSetName(Map m, int city, char *name) {
    if (m->cityNames[city] != NULL) {
        free(m->cityNames[city]);
    }
    m->cityNames[city] = strdup(name);
}

char *MapGetName(Map m, int city) {
    if (m->cityNames[city] == NULL) {
        return "unnamed";
    } else {
        return m->cityNames[city];
    }
}

void MapInsertRoad(Map m, int city1, int city2, int length) {
    if (m->roads[city1][city2] == 0) {
        m->roads[city1][city2] = length;
        m->roads[city2][city1] = length;
        m->numRoads++;
    }
}

int MapContainsRoad(Map m, int city1, int city2) {
    return m->roads[city1][city2];
}

int MapGetRoadsFrom(Map m, int city, struct road roads[]) {
    int count = 0;
    for (int i = 0; i < m->numCities; i++) {
        if (m->roads[city][i] > 0) {
            roads[count].from = city;
            roads[count].to = i;
            roads[count].length = m->roads[city][i];
            count++;
        }
    }

    // Sort roads array by 'to' field using qsort
    qsort(roads, count, sizeof(struct road), compareRoads);

    return count;
}

// a function to compare to instanses
// to be used in qsort
static int compareRoads(const void *a, const void *b) {
    struct road *roadA = (struct road *)a;
    struct road *roadB = (struct road *)b;
    return roadA->to - roadB->to;
}

/**
 * !!! DO NOT EDIT THIS FUNCTION !!!
 * This function will work once the other functions are working
 */
void MapShow(Map m) {
    printf("Number of cities: %d\n", MapNumCities(m));
    printf("Number of roads: %d\n", MapNumRoads(m));
    
    struct road *roads = malloc(MapNumRoads(m) * sizeof(struct road));
    if (roads == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);    
    }
    
    for (int i = 0; i < MapNumCities(m); i++) {
        printf("[%d] %s has roads to:", i, MapGetName(m, i));
        int numRoads = MapGetRoadsFrom(m, i, roads);
        for (int j = 0; j < numRoads; j++) {
            if (j > 0) {
                printf(",");
            }
            printf(" [%d] %s (%d)", roads[j].to, MapGetName(m, roads[j].to),
                   roads[j].length);
        }
        printf("\n");
    }
    
    free(roads);
}

