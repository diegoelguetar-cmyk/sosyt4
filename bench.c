#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#define TOLERANCIA 80

// Los diccionarios de prueba se construyen en este directorio para no
// llenar de archivos el directorio de la tarea
#define DIR "tmp-test"
#define DICC_MIO DIR "/big-mio.ht"
#define DICC_REF DIR "/big-ref.ht"

// ----------------------------------------------------
// Funcion que entrega el tiempo transcurrido desde el lanzamiento del
// programa en milisegundos

static long time0= 0;

static long getTime0() {
    struct timeval Timeval;
    gettimeofday(&Timeval, NULL);
    return Timeval.tv_sec*1000+Timeval.tv_usec/1000;
}

static void resetTime() {
  time0= getTime0();
}

static long getTime() {
  return getTime0()-time0;
}

// ----------------------------------------------------
// Benchmark

void terminar(int rc) {
  system("rm -f " DICC_MIO " " DICC_REF);
  exit(rc);
}

// Construye un diccionario vacio de tam filas en el archivo nom.
// Una fila vacia son 100 bytes en 0, de modo que un diccionario vacio
// es simplemente un archivo de 100*tam bytes en 0.
void crear(char *nom, int tam) {
    char cmd[200];
    sprintf(cmd, "mkdir -p " DIR " && head -c %ld /dev/zero > %s\n",
            100l*tam, nom);
    int rc= WEXITSTATUS(system(cmd));
    if (rc!=0) {
        fprintf(stderr, "No se pudo construir el diccionario %s\n", nom);
        exit(1);
    }
}

// Inserta ins llaves distintas en el diccionario nom, invocando el comando
// binary una vez por insercion, y entrega el tiempo que demoro en
// milisegundos.  Las llaves son largas a proposito: hash_string entrega
// valores chicos para llaves cortas y en ese caso todas las llaves
// quedarian al comienzo de la tabla.
long bench(char *binary, char *nom, int ins) {
    long ini= getTime();
    for (int i= 1; i<=ins; i++) {
        char cmd[200];
        sprintf(cmd, "%s %s llave%d \"definicion numero %d\""
                     "     1> /dev/null 2> /dev/null\n",
                binary, nom, i, i);
        if (i%100==0)
          printf("Ejecutando comando: %s", cmd);
        int rc= WEXITSTATUS(system(cmd));
        if (rc!=0) {
            fprintf(stderr, "Codigo de retorno %d es incorrecto\n", rc);
            terminar(1);
        }
    }
    return getTime()-ini;
}

int main(int argc, char **argv) {
    if (argc!=4) {
        fprintf(stderr, "uso: ./bench <bin-prof> <filas> <inserciones>\n");
        exit(1);
    }
    char *bin= "./definir.bin";
    char *bin_prof= argv[1];
    int tam= atoi(argv[2]);
    int ins= atoi(argv[3]);

    resetTime();

    int intento= 1;
    while (intento<=5) {
        printf("Intento= %d\n", intento);
        printf("Construyendo 2 diccionarios vacios de %d lineas\n", tam);
        crear(DICC_REF, tam);
        crear(DICC_MIO, tam);

        printf("Midiendo tiempo del binario del profesor con %d inserciones\n",
               ins);
        long tiempo_prof= bench(bin_prof, DICC_REF, ins);
        printf("Tiempo= %ld milisegundos\n", tiempo_prof);
        printf("Midiendo tiempo de su solucion con %d inserciones\n", ins);
        long tiempo= bench(bin, DICC_MIO, ins);
        printf("Tiempo= %ld milisegundos\n", tiempo);

        // Los dos diccionarios deben quedar identicos: de nada sirve ser
        // rapido si el diccionario resultante es incorrecto
        int rc= WEXITSTATUS(system("cmp " DICC_MIO " " DICC_REF
                                   " 1> /dev/null 2> /dev/null"));
        if (rc!=0) {
            fprintf(stderr, "Lo siento: el diccionario que construyo su "
                            "solucion no es igual al del profesor.\n");
            fprintf(stderr, "Ejecute make run-g para encontrar el error.\n");
            terminar(1);
        }

        if (tiempo_prof<1)
            tiempo_prof= 1;
        double q= (double)tiempo/(double)tiempo_prof;
        int porciento= (q-1.)*100;
        printf("Porcentaje de sobrecosto: %d %%\n", porciento);
        if (porciento<=TOLERANCIA)
            break;
        printf("Excede en mas del %d %% la version del profesor\n", TOLERANCIA);
        if (intento<5)
            printf("Se hara un nuevo intento\n");
        intento++;
    }
    if (intento>5) {
      fprintf(stderr, "Lo siento: Despues de 5 intentos no satisface "
                      "la eficiencia requerida.\n");
      fprintf(stderr,
              "Si para encontrar la fila de la llave esta recorriendo el\n"
              "archivo fila por fila, en vez de posicionarse directamente\n"
              "con fseek, no aprobara este test.\n"
              "Coloque su computador en modo alto rendimiento, porque el\n"
              "economizador de bateria puede alterar los resultados.\n"
              "No ejecute este programa junto a otros programas que hagan\n"
              "un uso intensivo de la CPU.  En windows puede lanzar el\n"
              "administrador de tareas para verificar que el uso de CPU\n"
              "sea bajo.\n");
      terminar(1);
    }

    printf("Felicitaciones: Aprobo el test de eficiencia\n");

    terminar(0);
    return 0;
}
