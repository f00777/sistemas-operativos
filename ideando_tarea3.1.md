# Guía de Desarrollo: Simulador de Asignación de Memoria (memsim)

## 1. Descripción General
[cite_start]El objetivo de la tarea es construir `memsim`, un simulador por consola que modela el gestor de memoria de un sistema operativo utilizando estrategias de partición dinámica[cite: 8]. [cite_start]El simulador no asignará memoria real del sistema, sino que utilizará estructuras de datos clásicas para llevar un "registro contable" de los fragmentos libres y ocupados en una memoria simulada de tamaño inicial $N$[cite: 9].

## 2. Parámetros de Ejecución
[cite_start]El programa debe ejecutarse desde la línea de comandos recibiendo exactamente tres argumentos[cite: 16]:
1. [cite_start]**Tamaño total de la memoria:** En bytes[cite: 16].
2. [cite_start]**Estrategia de asignación:** `FIRST_FIT`, `BEST_FIT` o `WORST_FIT`[cite: 11].
3. [cite_start]**Archivo de traza (Trace File):** Ruta al archivo `.txt` con las operaciones a simular[cite: 16].

**Ejemplo de ejecución:**
[cite_start]`./memsim 1024 FIRST_FIT traza1.txt` [cite: 18]

---

## 3. Políticas de Asignación a Implementar
Dependiendo del parámetro ingresado, el simulador debe buscar un bloque libre al momento de asignar memoria:
* [cite_start]**First-Fit:** Asigna el primer bloque libre encontrado (recorriendo desde el inicio) que sea mayor o igual al tamaño solicitado[cite: 12].
* [cite_start]**Best-Fit:** Busca en toda la memoria el bloque libre más pequeño que sea mayor o igual al solicitado, minimizando el espacio libre restante[cite: 13].
* [cite_start]**Worst-Fit:** Busca en toda la memoria el bloque libre más grande disponible y le asigna el proceso[cite: 14].

---

## 4. Instrucciones del Archivo de Traza
El simulador procesará línea por línea los siguientes comandos:

* [cite_start]**ALLOC `<ID_Proceso>` `<SIZE>`:** Solicita asignar memoria para un proceso[cite: 20]. 
  * *Comportamiento:* Si el bloque libre encontrado es más grande que `<SIZE>`, el nodo libre **se divide**. El tamaño se ajusta al requerido por el proceso (pasa a estar ocupado) y se crea un nuevo nodo dinámico justo después con el espacio libre sobrante.
* [cite_start]**FREE `<ID_Proceso>`:** Libera toda la memoria ocupada por el proceso[cite: 21].
  * [cite_start]*Comportamiento (Coalescencia Obligatoria):* Al liberar el nodo, se debe verificar si los vecinos contiguos están libres[cite: 30]. [cite_start]Si es así, se fusionan inmediatamente en un único nodo de mayor tamaño[cite: 30].
* [cite_start]**COMPACT:** Compacta la memoria[cite: 25].
  * [cite_start]*Comportamiento:* Todos los bloques ocupados se "desplazan" hacia la dirección 0 (eliminando los huecos intermedios)[cite: 32]. [cite_start]Todo el espacio libre se consolida en un único y gran bloque al final de la memoria[cite: 33].

---

## 5. Diseño Interno: La Estructura de Datos
[cite_start]Para cumplir con la prohibición de usar librerías de alto nivel (como `vector`), se deben utilizar primitivas elementales de C[cite: 52, 53]. 

**Estructura recomendada:** Una **Lista Doblemente Enlazada**.
* **Justificación:** Permite realizar la coalescencia en tiempo constante ($O(1)$) ya que cada nodo tiene acceso directo a su nodo vecino anterior (`prev`) y siguiente (`next`).
* **Dinámica de Nodos:** Los nodos **no tienen tamaño fijo**. Se inician como un único nodo gigante del tamaño total de la memoria. A lo largo del programa, estos nodos se dividen (en `ALLOC`) o se fusionan y eliminan de la lista (en `FREE` mediante coalescencia).

---

## 6. Condiciones de Término
El bucle principal de lectura del archivo termina bajo una de dos condiciones:
1. [cite_start]**Éxito:** Se procesa la última línea del archivo de texto[cite: 35].
2. [cite_start]**Falla Prematura:** Una petición `ALLOC` falla porque no existe un bloque libre lo suficientemente grande (fragmentación externa)[cite: 35]. El simulador se detiene inmediatamente.

---

## 7. Reporte de Estadísticas Finales
[cite_start]Al finalizar (por éxito o por falla), se debe imprimir en la consola (`stdout`) el siguiente reporte[cite: 36]:

* [cite_start]**Procesos asignados (Dato Histórico Acumulativo):** Cantidad total de operaciones `ALLOC` exitosas a lo largo de toda la ejecución[cite: 37].
* [cite_start]**Memoria utilizada (Estado Final):** Porcentaje de la memoria ocupada en el instante preciso de término[cite: 38].
* [cite_start]**Índice de Fragmentación Externa (Estado Final):** Calculado con los datos de la lista al terminar, usando la fórmula[cite: 39]:
  [cite_start]$$F_{ext}=1-\frac{B_{max}}{M_{libre}}$$ [cite: 40]
  [cite_start]*(Donde $B_{max}$ es el tamaño del mayor bloque libre y $M_{libre}$ es la suma total de bytes libres [cite: 41]).*
* **Estado final de la memoria:** Impresión visual y secuencial de la lista. [cite_start]Ejemplo: `[Libre 100] -> [Ocupado P1 200] -> [Libre 724]`[cite: 42, 43].

---

## 8. Reglas Críticas y Entregables
[cite_start]Faltar a estas restricciones implica calificación mínima[cite: 55]:
* [cite_start]**Lenguaje:** Estándar máximo C17[cite: 47].
* [cite_start]**Compilación:** Obligatorio usar los flags `gcc -Wall -Wextra -std=c17`[cite: 47].
* [cite_start]**Entrega (21 de Junio, 23:59, grupos máx. 3 personas)[cite: 6]:**
  * [cite_start]Código fuente[cite: 58].
  * [cite_start]Archivos de prueba para demostrar casos borde[cite: 59].
  * [cite_start]Informe en PDF (máximo 3 páginas) sobre estructuras de datos y fragmentación observada[cite: 60, 61].
  * [cite_start]Archivo `README.md` sobre cómo compilar, interactuar y un ejemplo de ejecución exitoso[cite: 54].