#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombre);

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombreantiguo, char *nombrenuevo);

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
             EXT_DATOS *memdatos, char *nombre); 
             
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich);

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich);

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);



void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){
      // Los bytemaps son caracteres, por eso %c,
      // pero al ser 1 y 0, se debería poder poner %i,
      //lo veremos cuando funciones

      // Imprime el bytemap de los primeros 25 bloques
      for (int i = 0; i < 25; i++) printf("%c", ext_bytemaps->bmap_bloques[i]);
      // Imprime el bytemap de inodos
      for (int i = 0; i < MAX_INODOS; i++) printf("%c", ext_bytemaps->bmap_bloques[i]);
      // que hace: ext_bytemaps->bmap_relleno ??
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2)
{
	/*
	 *	Buscar en strcomando, la orden (primer espacio)
	 *	luego buscar el argumento1 (segundo espacio)
	 *	y por ultimo buscar el argumento2 (tercer espacio)
	 * */
	
	char* subString = strtok(strcomando, " ");
	int argument_index = 0;
	int res = -1;


	while(subString != NULL)
	{
		
		switch(argument_index)
		{
			// para la orden
			case 0:
				strcpy(orden, subString);
				res = 0;
			break;

			// para el argumento1
			case 1:
				strcpy(argumento1, subString);
			break;

			// para el argumento2
			case 2:
				strcpy(argumento2, subString);
			break;

			default:
				; // maximo de argumentos compatibles excedido
			break;
		}


		argument_index++;
		subString = strtok(NULL, " ");
	}

	return res;
}

void handleComand(char *orden, char *argumento1, char *argumento2, int *comandoEncontrado){
      
      if (strcmp(orden, "dir\n") == 0)
      {
            //Directorio(&directorio, &ext_blq_inodos);
            printf("dir");
            comandoEncontrado = 1;
      }
      else if (strcmp(orden, "info\n") == 0)
      {
            // Llamar funciones
            printf("info");
            comandoEncontrado = 1;
      }
      else if (strcmp(orden, "bytemaps\n") == 0)
      {
            // Llamar funciones
            printf("bytemaps");
            comandoEncontrado = 1;
      }
      else if (strcmp(orden, "rename\n") == 0)
      {
            // Llamar funciones
            printf("rename");
            comandoEncontrado = 1;
      }
      else if (strcmp(orden, "imprimir\n") == 0)
      {
            // Llamar funciones
            printf("imprimir");
            comandoEncontrado = 1;
      }
      else if (strcmp(orden, "remove\n") == 0)
      {
            // Llamar funciones
            printf("remove");
            comandoEncontrado = 1;
      }
      else if (strcmp(orden, "copy\n") == 0)
      {
            // Llamar funciones
            printf("copy");
            comandoEncontrado = 1;
      }
}


int main()
{
   char comando[LONGITUD_COMANDO];
   char orden[LONGITUD_COMANDO];
   char argumento1[LONGITUD_COMANDO];
   char argumento2[LONGITUD_COMANDO];

   int i, j;
   unsigned long int m;
   EXT_SIMPLE_SUPERBLOCK ext_superblock;
   EXT_BYTE_MAPS ext_bytemaps;
   EXT_BLQ_INODOS ext_blq_inodos;
   EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
   EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
   EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
   int entradadir;
   int grabardatos;
   FILE *fent;

   // Lectura del fichero completo de una sola vez

   fent = fopen("particion.bin", "r+b");
   fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);

/* COMENTADOS PARA TESTING

   memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
   memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
   memcpy(&ext_bytemaps, (EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
   memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
   memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

*/
   // Buce de tratamiento de comandos
   for (;;)
   {
	int comandoEncontrado = 0;

	do
      {
         printf(">> ");
         fflush(stdin);
         fgets(comando, LONGITUD_COMANDO, stdin);
      } while (ComprobarComando(comando, orden, argumento1, argumento2) != 0);

      handleComand(orden, argumento1, argumento2, comandoEncontrado);

/*
      // Escritura de metadatos en comandos rename, remove, copy
      Grabarinodosydirectorio(&directorio, &ext_blq_inodos, fent);
      GrabarByteMaps(&ext_bytemaps, fent);
      GrabarSuperBloque(&ext_superblock, fent);

      if (grabardatos)
      {
            GrabarDatos(&memdatos, fent);
      }
   
      grabardatos = 0;
*/
      // Si el comando es salir se habrán escrito todos los metadatos
      // faltan los datos y cerrar
      if (strcmp(orden, "salir\n") == 0)
      {
            //GrabarDatos(&memdatos, fent);
            fclose(fent);
	      printf("\nSaliendo......");
	      return 0;
      }

      if (comandoEncontrado == 0)
      {
	      printf("Error: Comando desconocido [bytemaps, copy, dir, info, imprimir, rename, remove, salir ]\n");
      }
   }
}


/*

Echar un vistazo a pistas

comprobar q un fichero existe
dentro del directorio -> nombre del fichero y inodo, revisar dentro del directorio si un fichero 

*/
