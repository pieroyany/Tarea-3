# tarea-3

gcc tdas/*.c tarea3.c -Wno-unused-result -o graphquest
./graphquest



// Imprimir para verificar
        printf("ID: %d\n", h->id);
        printf("Nombre: %s\n", h->nombre);
        printf("Descripcion: %s\n", h->descripcion);
        printf("Items:\n");
        for (int i = 0; i < h->num_items; i++) {
            printf("  - %s (Valor: %d, Peso: %d)\n", h->items[i].nombre, h->items[i].valor, h->items[i].peso);
        }
        printf("Arriba: %d, Abajo: %d, Izquierda: %d, Derecha: %d\n", h->arriba, h->abajo, h->izquierda, h->derecha);
        printf("Es final: %c\n", h->final);
        printf("-----------------------------\n");

        // Liberar memoria
        for (int i = 0; i < h->num_items; i++) {
            free(h->items[i].nombre);
        }
        free(h->items);
        free(h->nombre);
        free(h->descripcion);