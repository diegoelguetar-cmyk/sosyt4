#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
    
#include "pss.h"
 
#define TAMFIL (sizeof(Fila))

typedef struct {
  unsigned char tamLlave; // Tamano de la llave, 0 si la fila esta vacia
  char llaveYDef[99]; // Los primeros tamLLave bytes corresponden a la llave
                      // El resto corresponden al valor, relleno con espacios
                      // en blanco al final.
} Fila;

int main(int argc, char **argv) {
  if (argc!=2) {
    fprintf(stderr, "Uso: ./revisar <archivo>\n");
    exit(1);
  }
  char *nom= argv[1];
  FILE *dicc = fopen(nom, "r");
  if (dicc==NULL) {
    perror(nom);
    exit(1);
  }

  Fila fil;

  // Leemos todas las filas en el archivo
  while (fread(&fil, TAMFIL, 1, dicc)==1) {
    char llave[TAMFIL], valor[TAMFIL];
    int tamLlave= fil.tamLlave;
    if (tamLlave==0) // La fila esta vacia, la ignoramos
      continue; // Comienza una nueva iteracion de for saltandose lo que viene
    strncpy(llave, fil.llaveYDef, tamLlave);
    llave[tamLlave]= 0; // Para que sea un string
    strncpy(valor, fil.llaveYDef+tamLlave, TAMFIL-tamLlave-1);
    valor[TAMFIL-tamLlave-1]= 0; // Termina el string, pero falta borrar
                                 // espacios en blanco al final del valor
    char *fin= valor+TAMFIL-tamLlave-2;
    while (fin>=valor && *fin==' ')
      fin--;
    fin[1]= 0;
    printf("%s ===> %s\n", llave, valor);
  }
  if (ferror(dicc)) {
    perror(nom);
    exit(1);
  }
  fclose(dicc);
  return 0;
}
