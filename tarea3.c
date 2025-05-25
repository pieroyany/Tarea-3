#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/heap.h"
#include "tdas/map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Map *habitaciones;

typedef struct {
    char *nombre;
    int valor;
    int peso;
} item_list;

typedef struct {
    int id;
    char *nombre;                                               
    char *descripcion;
    item_list *items; // Array de items
    int num_items; // Número de items en la habitación
    int arriba;
    int abajo;
    int izquierda;
    int derecha;
    char final;
} habitacion;

typedef struct {
    List *inventario; // Lista de items_list
    int peso_total;
    int puntaje;
    int tiempo;
} Jugador;

int is_equal_int(void *a, void *b) {
    return *((int *)a) == *((int *)b);
}

int is_equal_habitacion(void *a, void *b) {
    return a == b; // compara punteros directamente
}

item_list* parse_items(char *cadena, int *num_items) {
    item_list *items = malloc(sizeof(item_list) * 2); // máximo 2 ítems por sala
    *num_items = 0;

    char *token = strtok(cadena, ";");
    while (token != NULL && *num_items < 2) {
        char *nombre = strtok(token, ",");
        char *valor_str = strtok(NULL, ",");
        char *peso_str = strtok(NULL, ",");

        if (nombre && valor_str && peso_str) {
            items[*num_items].nombre = strdup(nombre);
            items[*num_items].valor = atoi(valor_str);
            items[*num_items].peso = atoi(peso_str);
            (*num_items)++;
        }

        token = strtok(NULL, ";");
    }

    return items;
}

List *leer_archivo() {
    FILE *archivo = fopen("data/graphquest.csv", "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    habitaciones = map_create(is_equal_int);

    char **campos = leer_linea_csv(archivo, ',');

    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        habitacion *h = malloc(sizeof(habitacion));


        h->id = atoi(campos[0]);
        h->nombre = strdup(campos[1]);
        h->descripcion = strdup(campos[2]);

        char *items_str = strdup(campos[3]);
        h->items = parse_items(items_str, &h->num_items);
        free(items_str);

        h->arriba = atoi(campos[4]);
        h->abajo = atoi(campos[5]);
        h->izquierda = atoi(campos[6]);
        h->derecha = atoi(campos[7]);
        h->final = (campos[8][0] == 'S' || campos[8][0] == 's') ? 'S' : 'N';

        habitacion *h_ptr = malloc(sizeof(habitacion));
        *h_ptr = *h;

        int *id_ptr = malloc(sizeof(int));
        *id_ptr = h->id;

        map_insert(habitaciones, id_ptr, h_ptr);
    }
    fclose(archivo);
}

Map *construir_grafo(Map *habitaciones) {
    Map *grafo = map_create(is_equal_habitacion);

    MapPair *pair = map_first(habitaciones);
    while (pair != NULL) {
        habitacion *hab = pair->value;
        List *adyacentes = list_create();

        if (hab->arriba != -1) {
            MapPair *p = map_search(habitaciones, &hab->arriba);
            if (p) list_pushBack(adyacentes, p->value);
        }
        if (hab->abajo != -1) {
            MapPair *p = map_search(habitaciones, &hab->abajo);
            if (p) list_pushBack(adyacentes, p->value);
        }
        if (hab->izquierda != -1) {
            MapPair *p = map_search(habitaciones, &hab->izquierda);
            if (p) list_pushBack(adyacentes, p->value);
        }
        if (hab->derecha != -1) {
            MapPair *p = map_search(habitaciones, &hab->derecha);
            if (p) list_pushBack(adyacentes, p->value);
        }

        map_insert(grafo, hab, adyacentes);
        pair = map_next(habitaciones);
    }

    //imprimir
    /*
    printf("----- Grafo de habitaciones -----\n");
    MapPair *par = map_first(grafo);
    while (par != NULL) {
        habitacion *h = par->key;
        List *ady = par->value;
        printf("Habitacion %s (ID: %d) conecta con:\n", h->nombre, h->id);

        habitacion *vecino = list_first(ady);
        while (vecino != NULL) {
            printf("  -> %s (ID: %d)\n", vecino->nombre, vecino->id);
            vecino = list_next(ady);
        }

        printf("----------------------\n");
        par = map_next(grafo);
    }
    */
    return grafo;
}

void mostrar_estado(habitacion *hab_actual, Jugador *jugador) {
    printf("\n=== ESTADO ACTUAL ===\n");
    printf("Ubicación: %s\n", hab_actual->nombre);
    printf("Descripción: %s\n", hab_actual->descripcion);
    
    printf("\nItems en la habitación:\n");
    if (hab_actual->num_items == 0) {
        printf("No hay items disponibles\n");
    } else {
        for (int i = 0; i < hab_actual->num_items; i++) {
            printf("%d. %s (Valor: %d, Peso: %d)\n", 
                   i+1, 
                   hab_actual->items[i].nombre,
                   hab_actual->items[i].valor,
                   hab_actual->items[i].peso);
        }
    }
    
    printf("\nInventario:\n");
    if (list_size(jugador->inventario) == 0) {
        printf("Inventario vacío\n");
    } else {
        item_list *item;
        int i = 1;
        for (item = list_first(jugador->inventario); item != NULL; item = list_next(jugador->inventario)) {
            printf("%d. %s (Valor: %d, Peso: %d)\n", 
                   i++, 
                   item->nombre,
                   item->valor,
                   item->peso);
        }
    }
    
    printf("\nTiempo restante: %d\n", jugador->tiempo);
    printf("Puntaje actual: %d\n", jugador->puntaje);
    printf("Peso total: %d\n", jugador->peso_total);
}

void recoger_item(habitacion *hab_actual, Jugador *jugador, int indice) {
    if (indice < 1 || indice > hab_actual->num_items) {
        printf("Ítem no válido\n");
        return;
    }
    
    item_list *item = &hab_actual->items[indice-1];
    
    item_list *nuevo_item = malloc(sizeof(item_list));
    nuevo_item->nombre = strdup(item->nombre);
    nuevo_item->valor = item->valor;
    nuevo_item->peso = item->peso;
    
    list_pushBack(jugador->inventario, nuevo_item);
    jugador->puntaje += item->valor;
    jugador->peso_total += item->peso;
    
    for (int i = indice-1; i < hab_actual->num_items-1; i++) {
        hab_actual->items[i] = hab_actual->items[i+1];
    }
    hab_actual->num_items--;
    
    jugador->tiempo--;
    printf("Has recogido: %s\n", item->nombre);
} 

int main() {
    leer_archivo();

    Map *grafo = construir_grafo(habitaciones);

    //jugar(habitaciones, grafo);

    return 0;
}
