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

int is_equal_int(void *a, void *b) {
    return *((int *)a) == *((int *)b);
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

        // Insertar en el mapa
        map_insert(habitaciones, id_ptr, h_ptr);

        /*
        printf("ID: %d\n", h.id);
        printf("Nombre: %s\n", h.nombre);
        printf("Descripcion: %s\n", h.descripcion);
        printf("Items:\n");
        for (int i = 0; i < h.num_items; i++) {
            printf("  - %s (Valor: %d, Peso: %d)\n", h.items[i].nombre, h.items[i].valor, h.items[i].peso);
        }
        printf("Arriba: %d, Abajo: %d, Izquierda: %d, Derecha: %d\n", h.arriba, h.abajo, h.izquierda, h.derecha);
        printf("Es final: %c\n", h.final);
        printf("-----------------------------\n");
        */

    }
    fclose(archivo);
}


int main() {
    leer_archivo();

    printf("Habitaciones cargadas:\n");
    MapPair *par = map_first(habitaciones);
    while (par != NULL) {
        int *id = (int *)par->key;
        habitacion *h = (habitacion *)par->value;

        printf("ID: %d\n", *id);
        printf("Nombre: %s\n", h->nombre);
        printf("Descripcion: %s\n", h->descripcion);
        printf("Items:\n");
        for (int i = 0; i < h->num_items; i++) {
            printf("  - %s (Valor: %d, Peso: %d)\n", h->items[i].nombre, h->items[i].valor, h->items[i].peso);
        }
        printf("Arriba: %d, Abajo: %d, Izquierda: %d, Derecha: %d\n", h->arriba, h->abajo, h->izquierda, h->derecha);
        printf("Es final: %c\n", h->final);
        printf("-----------------------------\n");

        par = map_next(habitaciones);
    }

    return 0;
}
