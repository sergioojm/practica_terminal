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


// FUNCIONES DE DEPENDENCIA

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){

      for (int i = 0; i < MAX_FICHEROS; i++)
	{
		if (directorio[i].dir_inodo != 2 && directorio[i].dir_inodo != NULL_INODO) // directorio root y que exista inodo
		      if (strcmp(nombre, directorio[i].dir_nfich) == 0) return i;
	}

      return -1;
}


// FUNCIONES DE ACCION DE COMANDOS

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos)
{
	for (int i = 0; i < MAX_FICHEROS; i++ )
	{
		if (directorio[i].dir_inodo != 2 && directorio[i].dir_inodo != NULL_INODO) // directorio root y que exista inodo
		{
			printf("%s \t Inodo: %i \t size (Bytes): %i \t Bloques: ", directorio[i].dir_nfich, directorio[i].dir_inodo, inodos->blq_inodos[directorio[i].dir_inodo].size_fichero);	
		
			
			for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++)
			{
				if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE)
				{
					printf("%i ", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
				}
			}

			printf("\n");
		}

	}
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){
      
      // Del 0 al 24 incluidos
	printf("Bloques [0-24]:\t");
      for (int i = 0; i < 25; i++) printf("%u ", ext_bytemaps->bmap_bloques[i]);

      printf("\nInodos:\t");
      for (int i = 0; i < MAX_INODOS; i++) printf("%u ", ext_bytemaps->bmap_bloques[i]);
      printf("\n");
}

void PrintInfo(EXT_SIMPLE_SUPERBLOCK *ext_superblock){
      printf("Bloque %i Bytes\n", ext_superblock->s_block_size);
      printf("inodos particion = %i\n", ext_superblock->s_inodes_count);
      printf("inodods libres = %i\n", ext_superblock->s_free_inodes_count);
      printf("Bloques particion = %i\n", ext_superblock->s_blocks_count);
	printf("Bloques libres = %i\n", ext_superblock->s_free_blocks_count);
      printf("Primer bloque de datos = %i\n", ext_superblock->s_first_data_block);
}


// FUNCIONES PRE-ACCION

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2)
{
      // Cambiamos ultimo '\n' del comando por '\0'
      if (strcomando[strlen(strcomando)-1] == '\n') strcomando[strlen(strcomando)-1] = '\0';

      // Y reiniciamos orden argumento1 y argumento2 para que no tengan basura
      strcpy(orden, "\0");
      strcpy(argumento1, "\0");
      strcpy(argumento2, "\0");

	/*
	 *	Buscar en strcomando, la orden (primer espacio)
	 *	luego buscar el argumento1 (segundo espacio)
	 *	y por ultimo buscar el argumento2 (tercer espacio)
	*/
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

void handleComand(char *orden, char *argumento1, char *argumento2, int *comandoEncontrado,
                  EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *ext_blq_inodos,
                  EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock)
{
      
      if (strcmp(orden, "dir") == 0)
      {
            Directorio(directorio, ext_blq_inodos);
            *comandoEncontrado = 1;
      }
      else if (strcmp(orden, "info") == 0)
      {
            PrintInfo(ext_superblock);
            *comandoEncontrado = 1;
      }
      else if (strcmp(orden, "bytemaps") == 0)
      {
            // Llamar funciones
	    Printbytemaps(ext_bytemaps);
            *comandoEncontrado = 1;
      }
      else if (strcmp(orden, "rename") == 0)
      {
            // Llamar funciones
            printf("rename");
            *comandoEncontrado = 1;
      }
      else if (strcmp(orden, "imprimir") == 0)
      {
            // Llamar funciones
            printf("imprimir");
            *comandoEncontrado = 1;
      }
      else if (strcmp(orden, "remove") == 0)
      {
            // Llamar funciones
            printf("remove");
            *comandoEncontrado = 1;
      }
      else if (strcmp(orden, "copy") == 0)
      {
            // Llamar funciones
            printf("copy");
            *comandoEncontrado = 1;
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


   memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
   memcpy(directorio, (EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
   memcpy(&ext_bytemaps, (EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
   memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
   memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

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

      handleComand(orden, argumento1, argumento2, &comandoEncontrado, directorio,
      &ext_blq_inodos, &ext_bytemaps, &ext_superblock);

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
      if (strcmp(orden, "salir") == 0)
      {
            //GrabarDatos(&memdatos, fent);
            fclose(fent);
	      printf("Saliendo......\n\n");
	      return 0;
      }

      if (comandoEncontrado == 0)
      {
	      printf("Error: Comando desconocido [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
      }
      printf("\n"); // Estetico, creo que queda mejor
   }
}


/*

Echar un vistazo a pistas

comprobar q un fichero existe
dentro del directorio -> nombre del fichero y inodo, revisar dentro del directorio si un fichero 

*/
