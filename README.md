# 🧩 GraphQuest – Aventura en C basada en grafos

## 📌 Descripción

**GraphQuest** es un videojuego de exploración basado en texto, desarrollado en lenguaje C, donde el jugador debe recorrer un laberinto representado como un grafo. Cada habitación contiene ítems, descripciones narrativas, caminos a otras habitaciones y una posible salida.

---

## ⚙️ Cómo compilar y ejecutar

### ✅ Requisitos

- [Visual Studio Code](https://code.visualstudio.com/)
- Extensión **C/C++**
- Compilador **gcc**
- Archivos `.c` y `.h` en la carpeta `tdas/`
- Archivo CSV de habitaciones en `data/graphquest.csv`

### 💻 Compilación

```bash
gcc tdas/*.c main.c -o graphquest
```

### ▶️ Ejecución

```bash
./graphquest
```

---

## 🧪 Estructura del CSV

```csv
id,nombre,descripcion,items,arriba,abajo,izquierda,derecha,final
1,Entrada,"Empieza tu aventura aquí","Sierra,10,5;Venda,5,2",2,-1,-1,-1,N
...
```

---

## 📚 Funciones implementadas

### Lectura y estructura de datos

- `item_list* parse_items(char *cadena, int *num_items)`
- `List* leer_archivo()`
- `Map* construir_grafo(Map *habitaciones)`

### Lógica del juego

- `void jugar(Map *habitaciones, Map *grafo, int id_inicial)`
- `void mostrar_banner()`
- `void mostrar_estado(habitacion *hab_actual, Jugador *jugador, Map *habitaciones)`
- `void recoger_item(habitacion *hab_actual, Jugador *jugador, int indice)`
- `void descartar_item(Jugador *jugador, int indice)`
- `int mostrar_opciones_movimiento(habitacion *hab_actual, Map *habitaciones)`
- `int mover_jugador(habitacion *hab_actual, Jugador *jugador, int direccion)`

### Utilidades

- `int leer_opcion_valida(int min, int max)`
- `int is_equal_int(void *a, void *b)`
- `int is_equal_habitacion(void *a, void *b)`

---

## 🔧 Funciones que podrían mejorarse

| Función | Observación | Posible mejora |
|--------|-------------|-----------------|
| `leer_archivo()` | Copia estructuras y duplica memoria innecesariamente | Cargar datos directamente sin duplicar `habitacion` |
| `mostrar_opciones_movimiento()` | Código repetido por dirección | Refactorizar con bucle y punteros |
| `mover_jugador()` | Parcialmente duplicada en `jugar()` | Centralizar cálculo de movimiento |
| `descartar_item()` | No pide confirmación | Agregar confirmación antes de borrar |
| `parse_items()` | Límite rígido de 2 ítems | Usar lista dinámica (`List*`) en lugar de array |

---
