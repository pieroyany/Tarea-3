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

int leer_opcion_valida(int min, int max) {
    int opcion;
    char buffer[100];
    while (1) {
        printf("Seleccione una opcion (%d-%d): ", min, max);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error de lectura.\n");
            continue;
        }
        
        if (sscanf(buffer, "%d", &opcion) != 1) {
            printf("Entrada invalida. Por favor ingrese un numero.\n");
            continue;
        }
        
        if (opcion >= min && opcion <= max) {
            return opcion;
        }
        
        printf("Opcion fuera de rango. Intente nuevamente.\n");
    }
}

void mostrar_banner() {
    limpiarPantalla();
    printf("------------------------------------------\n");
    printf("-           G R A P H Q U E S T          -\n");
    printf("------------------------------------------\n\n");
}

void mostrar_estado(habitacion *hab_actual, Jugador *jugador, Map *habitaciones) {
    printf("\n------------------------------------------\n");
    printf("- %-38s -\n", hab_actual->nombre);
    printf("------------------------------------------\n");
    printf("- %-38s -\n", hab_actual->descripcion);
    printf("------------------------------------------\n");
    
    // Mostrar items en la habitación
    if (hab_actual->num_items > 0) {
        printf("-  Items disponibles:                   -\n");
        for (int i = 0; i < hab_actual->num_items; i++) {
            printf("-  %d. %-20s (%2d pts, %2d kg) -\n", 
                  i+1, hab_actual->items[i].nombre,
                  hab_actual->items[i].valor,
                  hab_actual->items[i].peso);
        }
    } else {
        printf("-  No hay items en esta habitacion      -\n");
    }
    printf("------------------------------------------\n");
    
    // Mostrar inventario
    if (list_size(jugador->inventario) > 0) {
        printf("-  Tu inventario:                       -\n");
        item_list *item;
        int i = 1;
        for (item = list_first(jugador->inventario); item != NULL; item = list_next(jugador->inventario)) {
            printf("-  %d. %-20s (%2d pts, %2d kg) -\n", 
                  i++, item->nombre, item->valor, item->peso);
        }
    } else {
        printf("-  Inventario vacio                     -\n");
    }
    printf("------------------------------------------\n");
    
    // Mostrar estado del jugador
    printf("- Tiempo: %-4d  Puntaje: %-4d  Peso: %-3d  -\n", 
          jugador->tiempo, jugador->puntaje, jugador->peso_total);
    printf("------------------------------------------\n");
}

void recoger_item(habitacion *hab_actual, Jugador *jugador, int indice) {
    if (indice < 1 || indice > hab_actual->num_items) {
        printf("Item no valido\n");
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
        printf("Item no valido\n");
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

int mostrar_opciones_movimiento(habitacion *hab_actual, Map *habitaciones) {
    printf("\nOpciones de movimiento disponibles:\n");
    int opcion_num = 1;
    int opciones[4] = {-1, -1, -1, -1}; // Almacena los IDs de las habitaciones adyacentes
    
    // Mapear las opciones disponibles
    if (hab_actual->arriba != -1) {
        int *clave = malloc(sizeof(int));
        *clave = hab_actual->arriba;
        MapPair *pair = map_search(habitaciones, clave);
        free(clave);
        
        if (pair) {
            printf("%d. Arriba -> %s\n", opcion_num, ((habitacion*)pair->value)->nombre);
            opciones[opcion_num-1] = hab_actual->arriba;
            opcion_num++;
        }
    }
    
    if (hab_actual->abajo != -1) {
        int *clave = malloc(sizeof(int));
        *clave = hab_actual->abajo;
        MapPair *pair = map_search(habitaciones, clave);
        free(clave);
        
        if (pair) {
            printf("%d. Abajo -> %s\n", opcion_num, ((habitacion*)pair->value)->nombre);
            opciones[opcion_num-1] = hab_actual->abajo;
            opcion_num++;
        }
    }
    
    if (hab_actual->izquierda != -1) {
        int *clave = malloc(sizeof(int));
        *clave = hab_actual->izquierda;
        MapPair *pair = map_search(habitaciones, clave);
        free(clave);
        
        if (pair) {
            printf("%d. Izquierda -> %s\n", opcion_num, ((habitacion*)pair->value)->nombre);
            opciones[opcion_num-1] = hab_actual->izquierda;
            opcion_num++;
        }
    }
    
    if (hab_actual->derecha != -1) {
        int *clave = malloc(sizeof(int));
        *clave = hab_actual->derecha;
        MapPair *pair = map_search(habitaciones, clave);
        free(clave);
        
        if (pair) {
            printf("%d. Derecha -> %s\n", opcion_num, ((habitacion*)pair->value)->nombre);
            opciones[opcion_num-1] = hab_actual->derecha;
            opcion_num++;
        }
    }
    
    // Si no hay opciones disponibles
    if (opcion_num == 1) {
        printf("No hay salidas disponibles desde esta habitacion.\n");
        return -1;
    }
    
    // Pedir selección al usuario
    printf("Seleccione una opcion (1-%d): ", opcion_num-1);
    int seleccion = leer_opcion_valida(1, opcion_num-1);
    
    return opciones[seleccion-1]; // Retorna el ID de la habitación seleccionada
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
            printf("Dirección no valida\n");
            return hab_actual->id;
    }
    
    if (nuevo_id == -1) {
        printf("No puedes moverte en esa direccion\n");
        return hab_actual->id;
    }
    
    // calcular tiempo
    int tiempo_consumido = (jugador->peso_total + 1) / 10;
    if (tiempo_consumido < 1) tiempo_consumido = 1;
    
    jugador->tiempo -= tiempo_consumido;
    printf("Te has movido. Tiempo consumido: %d\n", tiempo_consumido);
    
    return nuevo_id;
}

void jugar(Map *habitaciones, Map *grafo, int id_inicial) {
    int id_actual = id_inicial;
    Jugador jugador;
    jugador.inventario = list_create();
    jugador.peso_total = 0;
    jugador.puntaje = 0;
    jugador.tiempo = 50; // Tiempo inicial
    
    while (jugador.tiempo > 0) {
        mostrar_banner();
        
        int *clave = malloc(sizeof(int));
        *clave = id_actual;
        MapPair *pair = map_search(habitaciones, clave);
        free(clave);
        
        if (!pair) {
            printf("Error: habitacion no encontrada.\n");
            break;
        }
        
        habitacion *hab_actual = pair->value;
        mostrar_estado(hab_actual, &jugador, habitaciones);
        
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
        
        int opcion = leer_opcion_valida(1, 5);
        
        switch(opcion) {
            case 1: // Recoger ítem
                if (hab_actual->num_items > 0) {
                    printf("Seleccione el item a recoger (1-%d): ", hab_actual->num_items);
                    int item_idx = leer_opcion_valida(1, hab_actual->num_items);
                    recoger_item(hab_actual, &jugador, item_idx);
                } else {
                    printf("No hay items para recoger en esta habitacion.\n");
                    presioneTeclaParaContinuar();
                }
                break;
                
            case 2: // Descartar ítem
                if (list_size(jugador.inventario) > 0) {
                    printf("Seleccione el item a descartar (1-%d): ", list_size(jugador.inventario));
                    int item_idx = leer_opcion_valida(1, list_size(jugador.inventario));
                    descartar_item(&jugador, item_idx);
                } else {
                    printf("No tienes items para descartar.\n");
                    presioneTeclaParaContinuar();
                }
                break;
                
            case 3: // Moverse
            {
                int id_destino = mostrar_opciones_movimiento(hab_actual, habitaciones);
                if (id_destino != -1) {
                // Calcular tiempo consumido
                    int tiempo_consumido = (jugador.peso_total + 1) / 10;
                    if (tiempo_consumido < 1) tiempo_consumido = 1;
        
                    jugador.tiempo -= tiempo_consumido;
                    id_actual = id_destino;
                    printf("Te has movido. Tiempo consumido: %d\n", tiempo_consumido);
                }
                presioneTeclaParaContinuar();
                break;
            }
                
            case 4: // Reiniciar partida
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
                presioneTeclaParaContinuar();
                break;
                
            case 5: // Salir del juego
                printf("Juego terminado. Puntaje final: %d\n", jugador.puntaje);
                while (list_size(jugador.inventario) > 0) {
                    item_list *item = list_popFront(jugador.inventario);
                    free(item->nombre);
                    free(item);
                }
                list_clean(jugador.inventario);
                free(jugador.inventario);
                return;
        }
        
        if (jugador.tiempo <= 0) {
            printf("\nSe te ha acabado el tiempo!\n");
            printf("Puntaje final: %d\n", jugador.puntaje);
            presioneTeclaParaContinuar();
        }
    }
    
    // Limpieza final
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
    if (!grafo) {
        printf("Error al construir el grafo.\n");
        return 1;
    }
    
    while (1) {
        mostrar_banner();
        printf("MENU PRINCIPAL\n");
        printf("1. Nueva partida\n");
        printf("2. Salir\n");
        
        int opcion = leer_opcion_valida(1, 2);
        
        if (opcion == 1) {
            jugar(habitaciones, grafo, 1);
        } else {
            break;
        }
    }
    
    
    
    return 0;
}
