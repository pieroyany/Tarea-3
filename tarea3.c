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
    int *key1 = (int *)a;
    int *key2 = (int *)b;
    return *key1 == *key2;
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

void descartar_item(Jugador *jugador, int indice) {
    if (indice < 1 || indice > list_size(jugador->inventario)) {
        printf("Ítem no válido\n");
        return;
    }
    
    item_list *item = list_first(jugador->inventario);
    for (int i = 1; i < indice; i++) {
        item = list_next(jugador->inventario);
    }
    
    printf("Has descartado: %s\n", item->nombre);
    jugador->puntaje -= item->valor;
    jugador->peso_total -= item->peso;
    
    list_popCurrent(jugador->inventario);
    free(item->nombre);
    free(item);
    
    jugador->tiempo--;
}

void mostrar_opciones_movimiento(habitacion *hab_actual) {
    printf("\nOpciones de movimiento:\n");
    if (hab_actual->arriba != -1) printf("1. Arriba (Habitación %d)\n", hab_actual->arriba);
    if (hab_actual->abajo != -1) printf("2. Abajo (Habitación %d)\n", hab_actual->abajo);
    if (hab_actual->izquierda != -1) printf("3. Izquierda (Habitación %d)\n", hab_actual->izquierda);
    if (hab_actual->derecha != -1) printf("4. Derecha (Habitación %d)\n", hab_actual->derecha);
}

int mover_jugador(habitacion *hab_actual, Jugador *jugador, int direccion) {
    int nuevo_id = -1;
    
    switch(direccion) {
        case 1: // Arriba
            nuevo_id = hab_actual->arriba;
            break;
        case 2: // Abajo
            nuevo_id = hab_actual->abajo;
            break;
        case 3: // Izquierda
            nuevo_id = hab_actual->izquierda;
            break;
        case 4: // Derecha
            nuevo_id = hab_actual->derecha;
            break;
        default:
            printf("Dirección no válida\n");
            return hab_actual->id;
    }
    
    if (nuevo_id == -1) {
        printf("No puedes moverte en esa dirección\n");
        return hab_actual->id;
    }
    
    // calcular tiempo
    int tiempo_consumido = (jugador->peso_total + 1) / 10;
    if (tiempo_consumido < 1) tiempo_consumido = 1;
    
    jugador->tiempo -= tiempo_consumido;
    printf("Te has movido. Tiempo consumido: %d\n", tiempo_consumido);
    
    return nuevo_id;
}

void jugar(Map *habitaciones, Map *grafo) {
    int id_actual = 1; // Empezamos en la habitación 1 (Entrada principal)
    Jugador jugador;
    jugador.inventario = list_create();
    jugador.peso_total = 0;
    jugador.puntaje = 0;
    jugador.tiempo = 50; // Tiempo inicial
    
    while (jugador.tiempo > 0) {
        // Crear una clave para la búsqueda (así como se hizo en leer_archivo)
        int *clave = malloc(sizeof(int));
        *clave = id_actual;
        
        MapPair *pair = map_search(habitaciones, clave);
        free(clave); // Liberamos la memoria inmediatamente
        
        if (!pair) {
            printf("Error: habitacion no encontrada.\n");
            break;
        }
        
        habitacion *hab_actual = pair->value;
        
        // Resto de tu código...
        mostrar_estado(hab_actual, &jugador);
        
        if (hab_actual->final == 'S') {
            printf("\nFelicidades! Has llegado a la salida.\n");
            printf("Puntaje final: %d\n", jugador.puntaje);
            break;
        }
        
        printf("\nOpciones:\n");
        printf("1. Recoger item\n");
        printf("2. Descartar item\n");
        printf("3. Moverse\n");
        printf("4. Reiniciar partida\n");
        printf("5. Salir del juego\n");
        printf("Selecciona una opcion: ");
        
        int opcion;
        scanf("%d", &opcion);
        
        switch(opcion) {
            case 1: // Recoger ítem
                if (hab_actual->num_items > 0) {
                    printf("Selecciona el item a recoger (1-%d): ", hab_actual->num_items);
                    int item_idx;
                    scanf("%d", &item_idx);
                    recoger_item(hab_actual, &jugador, item_idx);
                } else {
                    printf("No hay items para recoger en esta habitacion.\n");
                }
                break;
                
            case 2: // Descartar ítem
                if (list_size(jugador.inventario) > 0) {
                    printf("Selecciona el item a descartar (1-%d): ", list_size(jugador.inventario));
                    int item_idx;
                    scanf("%d", &item_idx);
                    descartar_item(&jugador, item_idx);
                } else {
                    printf("No tienes items para descartar.\n");
                }
                break;
                
            case 3: // Moverse
                mostrar_opciones_movimiento(hab_actual);
                printf("Selecciona una direccion: ");
                int direccion;
                scanf("%d", &direccion);
                id_actual = mover_jugador(hab_actual, &jugador, direccion);
                break;
                
            case 4: // Reiniciar partida
                // Limpiar inventario
                while (list_size(jugador.inventario) > 0) {
                    item_list *item = list_popFront(jugador.inventario);
                    free(item->nombre);
                    free(item);
                }
                jugador.peso_total = 0;
                jugador.puntaje = 0;
                jugador.tiempo = 50;
                id_actual = 1;
                printf("Partida reiniciada.\n");
                break;
                
            case 5: // Salir del juego
                printf("Juego terminado. Puntaje final: %d\n", jugador.puntaje);
                // Limpiar inventario antes de salir
                while (list_size(jugador.inventario) > 0) {
                    item_list *item = list_popFront(jugador.inventario);
                    free(item->nombre);
                    free(item);
                }
                list_clean(jugador.inventario);
                free(jugador.inventario);
                return;
                
            default:
                printf("Opción no valida.\n");
        }
        
        if (jugador.tiempo <= 0) {
            printf("\n¡Se te ha acabado el tiempo!\n");
            printf("Puntaje final: %d\n", jugador.puntaje);
        }
    }
    
    // Limpiar inventario al finalizar
    while (list_size(jugador.inventario) > 0) {
        item_list *item = list_popFront(jugador.inventario);
        free(item->nombre);
        free(item);
    }
    list_clean(jugador.inventario);
    free(jugador.inventario);
}

int main() {
    leer_archivo();

    Map *grafo = construir_grafo(habitaciones);

    jugar(habitaciones, grafo);

    return 0;
}
