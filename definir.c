#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pss.h"   // declara hash_string

#define TAMFIL 100
#define MAXTAM 99  // maximo de bytes para llave + valor

typedef struct {
  unsigned char tamLlave;  // 0 si la fila esta vacia
  char llaveYDef[MAXTAM];  // llave (tamLlave bytes) + valor rellenado con ' '
} Fila;

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Uso: ./definir <diccionario> <llave> <definicion>\n");
    exit(1);
  }

  char *nom = argv[1];
  char *llave = argv[2];
  char *def = argv[3];

  int tamLlave = strlen(llave);
  int tamDef = strlen(def);
  if (tamLlave + tamDef > MAXTAM) { //posible error
    fprintf(stderr, "Tamanno de la llave mas tamanno de la definicion (%d) "
            "exceden maximo permitido (%d)\n", tamLlave + tamDef, MAXTAM);
    exit(1);
  }

  // "r+b": lectura y escritura sin borrar el contenido
  FILE *dicc = fopen(nom, "r+b");
  if (dicc == NULL) { //error al abrir 
    perror(nom);
    exit(1);
  }

  // Cantidad de filas = tamaño del archivo / tamaño de fila
  if (fseek(dicc, 0, SEEK_END) != 0) {
    perror(nom);
    exit(1);
  }
  long tamArch = ftell(dicc);
  if (tamArch < 0) {
    perror(nom);
    exit(1);
  }
  long nfilas = tamArch / TAMFIL;

  if (nfilas == 0) {  // no hay donde insertar
    fprintf(stderr, "%s: el diccionario esta lleno\n", nom);
    exit(1);
  }

  // Fila inicial: hash modulo cantidad de filas
  long pos = hash_string(llave) % nfilas;
  if (fseek(dicc, pos * TAMFIL, SEEK_SET) != 0) {
    perror(nom);
    exit(1);
  }

  Fila fil;
  // A lo mas nfilas intentos: si damos la vuelta completa, esta lleno
  for (long i = 0; i < nfilas; i++) {
    if (fread(&fil, TAMFIL, 1, dicc) != 1) {
      perror(nom);
      exit(1);
    }

    if (fil.tamLlave == 0) {
      // Fila vacia: armamos la fila nueva
      Fila nueva;
      nueva.tamLlave = tamLlave;
      memcpy(nueva.llaveYDef, llave, tamLlave);
      memcpy(nueva.llaveYDef + tamLlave, def, tamDef);
      memset(nueva.llaveYDef + tamLlave + tamDef, ' ',
             MAXTAM - tamLlave - tamDef);

      // Entre una lectura y una escritura se debe llamar a fseek
      if (fseek(dicc, pos * TAMFIL, SEEK_SET) != 0) {
        perror(nom);
        exit(1);
      }
      if (fwrite(&nueva, TAMFIL, 1, dicc) != 1) {
        perror(nom);
        exit(1);
      }
      // fclose vacia el buffer: aqui pueden aparecer errores de escritura
      if (fclose(dicc) != 0) {
        perror(nom);
        exit(1);
      }
      return 0;
    }

    if (fil.tamLlave == tamLlave &&
        memcmp(fil.llaveYDef, llave, tamLlave) == 0) {
      fprintf(stderr, "La llave %s ya se encuentra en el diccionario\n", llave);
      exit(1);
    }

    // Siguiente fila (tabla circular)
    pos = (pos + 1) % nfilas;
    if (pos == 0) {  // llegamos al final: volver al comienzo
      if (fseek(dicc, 0, SEEK_SET) != 0) {
        perror(nom);
        exit(1);
      }
    }
    // Si no, fread ya dejo el cursor al comienzo de la fila siguiente
  }

  fprintf(stderr, "%s: el diccionario esta lleno\n", nom);
  exit(1);
}
