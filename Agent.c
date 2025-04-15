// COMP2521 24T3 - Assignment 2
// Implementation of the Agent ADT
// Written by Melina Salardini (z5393518)
// on 14/11/2024

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Agent.h"
#include "Map.h"


#define MAX_ROAD_LENGTH 1000

// This struct stores information about an individual agent and can be
// used to store information that the agent needs to remember.
struct agent {
    char *name;
    int startLocation;
    int location;
    int maxStamina;
    int stamina;
    int strategy;
    Map map;

    int *visitCounts;

    int *dfsPath;
    int dfsPathLen;
    int dfsPathIndex;
};

// function prototypes
static struct move chooseRandomMove(Agent agent, Map m);
static int filterRoads(Agent agent, struct road roads[], int numRoads,
                       struct road legalRoads[]);
static struct move chooseCheapestLeastVisited(Agent agent, Map m);
static void dfsHelper(Agent agent, Map m, int currCity, bool *visited,
                        int *dfsPath, int *pathIndex);
static struct move chooseDFS(Agent agent, Map m);
static int compareRoadByID(const void *a, const void *b);
static int MapGetRoadLength(Map m, int from, int to);


/**
 * Creates a new agent
 */
Agent AgentNew(int start, int stamina, int strategy, Map m, char *name) {
    if (start >= MapNumCities(m)) {
        fprintf(stderr, "error: starting city (%d) is invalid\n", start);
        exit(EXIT_FAILURE);
    }
    
    Agent agent = malloc(sizeof(struct agent));
    if (agent == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }
    
    // initilize the agent fields
    agent->startLocation = start;
    agent->location = start;
    agent->maxStamina = stamina;
    agent->stamina = stamina;
    agent->strategy = strategy;
    agent->map = m;
    agent->name = strdup(name);
    
    // initlize the visitCounts
    agent->visitCounts = calloc(MapNumCities(m), sizeof(int));
    if (agent->visitCounts == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }
    agent->visitCounts[start] = 1;
    
    agent->dfsPath = NULL;
    agent->dfsPathLen = 0;
    agent->dfsPathIndex = 0;

    return agent;
}

/**
 * Frees all memory allocated to the agent
 * NOTE: You should not free the map because the map is owned by the
 *       main program, and the main program will free it
 */
void AgentFree(Agent agent) {
    free(agent->name);
    free(agent->visitCounts);
    free(agent->dfsPath);
    free(agent);
}

////////////////////////////////////////////////////////////////////////
// Gets information about the agent
// NOTE: It is expected that these functions do not need to be modified

/**
 * Gets the name of the agent
 */
char *AgentName(Agent agent) {
    return agent->name;
}

/**
 * Gets the current location of the agent
 */
int AgentLocation(Agent agent) {
    return agent->location;
}

/**
 * Gets the amount of stamina the agent currently has
 */
int AgentStamina(Agent agent) {
    return agent->stamina;
}

////////////////////////////////////////////////////////////////////////
// Making moves

/**
 * Calculates the agent's next move
 * NOTE: Does NOT actually carry out the move
 */
struct move AgentGetNextMove(Agent agent, Map m) {
    if (agent->strategy == STATIONARY) {
        return (struct move){agent->location, 0};
    } else if (agent->strategy == RANDOM) {
        return chooseRandomMove(agent, m);
    } else if (agent->strategy == CHEAPEST_LEAST_VISITED) {
        return chooseCheapestLeastVisited(agent, m);
    } else if (agent->strategy == DFS) {
        return chooseDFS(agent, m);
    } else {
        printf("error: strategy not implemented yet\n");
        exit(EXIT_FAILURE);
    }
}

// a function to choose the next move using the DFS strategy
static struct move chooseDFS(Agent agent, Map m) {
    // initialize a new DFS path if all cities are visited or route is empty
    if (agent->dfsPath == NULL || agent->dfsPathIndex >= agent->dfsPathLen) {
        free(agent->dfsPath);
        agent->dfsPath = malloc(MapNumCities(m) * MapNumCities(m) * sizeof(int));

        bool *visited = calloc(MapNumCities(m), sizeof(bool));
        agent->dfsPathLen = 0;
        dfsHelper(agent, m, agent->location, visited, agent->dfsPath, &agent->dfsPathLen);
        agent->dfsPathIndex = 1;
        free(visited);
    }

    int nextCity = agent->dfsPath[agent->dfsPathIndex];
    int staminaNeeded = MapGetRoadLength(m, agent->location, nextCity);

    // if the stamina wasnt enough recharge
    if (staminaNeeded > agent->stamina) {
        return (struct move){agent->location, 0};
    }

    // move to the next city in the dfs path
    agent->dfsPathIndex++;
    return (struct move){nextCity, staminaNeeded};
}

// a helper function perform dfs and return the route as a sequence of cities
static void dfsHelper(Agent agent, Map m, int currCity, bool *visited,
                        int *dfsPath, int *pathIndex) {
    // mark the current city as visited
    visited[currCity] = true;

    // add current city to the dfs path
    dfsPath[*pathIndex] = currCity;
    (*pathIndex)++;

    // get all adjacent cities to the current city
    struct road *roads = malloc(MapNumCities(m) * sizeof(struct road));
    int numRoads = MapGetRoadsFrom(m, currCity, roads);

    // sort cities by IDs to prioritize lower IDs
    qsort(roads, numRoads, sizeof(struct road), compareRoadByID);

    // explore all the neighboring cities with the lowest IDs first
    for (int i = 0; i < numRoads; i++) {
        int toCity = roads[i].to;
        if (!visited[toCity]) {
            dfsHelper(agent, m, toCity, visited, dfsPath, pathIndex);
        }
    }
    
    if (dfsPath[*pathIndex - 1] != currCity) {
        dfsPath[*pathIndex] = currCity;
        (*pathIndex)++;
    }

    free(roads);
}

// a function to compare the cities base on their IDs
static int compareRoadByID(const void *a, const void *b) {
    struct road *roadA = (struct road *)a;
    struct road *roadB = (struct road *)b;
    return roadA->to - roadB->to;
}

// a function that finds the length of the road between two cities (from and to)
// returns the road length if a road exists, or -1 if no road exists.
static int MapGetRoadLength(Map m, int from, int to) {
    struct road *roads = malloc(MapNumCities(m) * sizeof(struct road));
    int numRoads = MapGetRoadsFrom(m, from, roads);
    
    // find the 'to' city and return the length between the cities
    for (int i = 0; i < numRoads; i++) {
        if (roads[i].to == to) {
            int length = roads[i].length;
            free(roads);
            return length;
        }
    }
    
    free(roads);
    return -1;
}

// a function to choose the cheapest least visited move for the agent
// if number of visits and required stamina was the same it should
// prioritize ID
static struct move chooseCheapestLeastVisited(Agent agent, Map m) {
    struct road *roads = malloc(MapNumCities(m) * sizeof(struct road));
    struct road *legalRoads = malloc(MapNumCities(m) * sizeof(struct road));

    // Get all roads to adjacent cities
    int numRoads = MapGetRoadsFrom(m, agent->location, roads);

    // Filter out roads that the agent does not have enough stamina for
    int numLegalRoads = filterRoads(agent, roads, numRoads, legalRoads);

    int chosenCity = agent->location;
    int minVisit = agent->visitCounts[agent->location];
    int minStamina = MAX_ROAD_LENGTH;

    // update the choosen city and its properties if we found a city
    // that meets the requirments
    for (int i = 0; i < numLegalRoads; i++) {
        int to = legalRoads[i].to;
        int len = legalRoads[i].length;

        if (agent->visitCounts[to] < minVisit ||
        (agent->visitCounts[to] == minVisit && len < minStamina) ||
        (agent->visitCounts[to] == minVisit && len == minStamina
        && to < chosenCity)) {
            minVisit = agent->visitCounts[to];
            minStamina = len;
            chosenCity = to;
        }
    }

    free(legalRoads);
    free(roads);
    
    return (struct move){chosenCity, minStamina};
}

// a function to to choose a random move for the agent
static struct move chooseRandomMove(Agent agent, Map m) {
    struct road *roads = malloc(MapNumCities(m) * sizeof(struct road));
    struct road *legalRoads = malloc(MapNumCities(m) * sizeof(struct road));

    // Get all roads to adjacent cities
    int numRoads = MapGetRoadsFrom(m, agent->location, roads);

    // Filter out roads that the agent does not have enough stamina for
    int numLegalRoads = filterRoads(agent, roads, numRoads, legalRoads);

    struct move move;
    if (numLegalRoads > 0) {
        // nextMove is randomly chosen from the legal roads
        int k = rand() % numLegalRoads;
        move = (struct move){legalRoads[k].to, legalRoads[k].length};
    } else {
        // The agent must stay in the same location
        move = (struct move){agent->location, 0};
    }

    free(legalRoads);
    free(roads);
    return move;
}


// a function that takes an array with all the possible roads and put the one
// the agent has enough stamina for into the legalRoads array
// it will return the number of legal roads
static int filterRoads(Agent agent, struct road roads[], int numRoads,
                        struct road legalRoads[]) {
    int numLegalRoads = 0;
    for (int i = 0; i < numRoads; i++) {
        if (roads[i].length <= agent->stamina) {
            legalRoads[numLegalRoads++] = roads[i];
        }
    }
    return numLegalRoads;
}

/**
 * Executes a given move by updating the agent's internal state
 */
void AgentMakeNextMove(Agent agent, struct move move) {
    if (move.to == agent->location) {
        agent->stamina = agent->maxStamina;
    } else {
        agent->stamina -= move.staminaCost;
    }
    agent->location = move.to;
    agent->visitCounts[move.to]++;
}

////////////////////////////////////////////////////////////////////////
// Learning information

/**
 * Tells the agent where the thief is
 */
void AgentTipOff(Agent agent, int thiefLocation) {
    // TODO: Stage 3
}

////////////////////////////////////////////////////////////////////////
// Displaying state

/**
 * Prints information about the agent (for debugging purposes)
 */
void AgentShow(Agent agent) {
    // TODO: You can implement this function however you want
    //       You can leave this function blank if you want
}

////////////////////////////////////////////////////////////////////////
