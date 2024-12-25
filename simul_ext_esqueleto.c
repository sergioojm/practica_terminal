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

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich)
{
      // PRIM_BLOQUE_DATOS = 4
    	fseek(fich, PRIM_BLOQUE_DATOS*SIZE_BLOQUE, SEEK_SET);
    	fwrite(memdatos, MAX_BLOQUES_DATOS, SIZE_BLOQUE, fich);
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich)
{
	fseek(fich, 3 * SIZE_BLOQUE, SEEK_SET);
	fwrite(directorio, 1, SIZE_BLOQUE, fich);
	
	fseek(fich, 2 * SIZE_BLOQUE, SEEK_SET);
	fwrite(inodos, 1, SIZE_BLOQUE, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich)
{
	fseek(fich, SIZE_BLOQUE, SEEK_SET);
	fwrite(ext_bytemaps, 1, SIZE_BLOQUE, fich);	
}

void GrabarSUperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich)
{
	fseek(fich, 0, SEEK_SET);
	fwrite(ext_superblock, 1, SIZE_BLOQUE, fich);
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){

	int res = -1;

      for (int i = 0; i < MAX_FICHEROS; i++)
	{
		if (directorio[i].dir_inodo != 2 && directorio[i].dir_inodo != NULL_INODO) // directorio root y que exista inodo
		      if (strcmp(nombre, directorio[i].dir_nfich) == 0) res = directorio[i].dir_inodo;
	}

      return res;
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

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
      printf("Bloque %i Bytes\n", psup->s_block_size);
      printf("inodos particion = %i\n", psup->s_inodes_count);
      printf("inodods libres = %i\n", psup->s_free_inodes_count);
      printf("Bloques particion = %i\n", psup->s_blocks_count);
	printf("Bloques libres = %i\n", psup->s_free_blocks_count);
      printf("Primer bloque de datos = %i\n", psup->s_first_data_block);
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
{
      int res = 0;
      int dir_inodo = BuscaFich(directorio, inodos, nombre);
	if (dir_inodo == -1)
	{
		printf("Fichero no existente\n");
            res = -1;
	}
      else{
            int allNull = 1;
            for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++)
            {
                  if (inodos->blq_inodos[dir_inodo].i_nbloque[i] != NULL_BLOQUE)
                  {
                        allNull = 0;
                        // Imprime unicamente el bloque, tenga o no \0
                        char caracter = memdatos[inodos->blq_inodos[dir_inodo].i_nbloque[i] - PRIM_BLOQUE_DATOS].dato[0];
                        for (int j = 0; j < SIZE_BLOQUE && caracter != '\0'; j++){
                              printf("%c", caracter);
                              caracter = memdatos[inodos->blq_inodos[dir_inodo].i_nbloque[i] - PRIM_BLOQUE_DATOS].dato[j+1];
                        }
                  }
            }

            if (!allNull){
                  printf("\n");
            }
      }

      return res;
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo)
{
	int existeFichero = BuscaFich(directorio, inodos, nombreantiguo);
	int ficheroNombreNuevo = BuscaFich(directorio, inodos, nombrenuevo);


	// si no existe el fichero, entonces terminar, como ella conmigo
	if (existeFichero == -1)
	{
		printf("Fichero a renombrar no existente, como tu pelo\n”");
		return -1;
	}	

	// si existe algun fichero con el nombre al que queremos cambiar, terminar <repetir chiste>
	if (ficheroNombreNuevo != -1)
	{
		printf("Ese nombre ya existe!\n");
		return -1;
	}

	for (int i = 0; i < MAX_FICHEROS; i++)
	{
		if (directorio[i].dir_inodo != 2 && directorio[i].dir_inodo != NULL_INODO && (strcmp(directorio[i].dir_nfich, nombreantiguo) == 0))
		{
			strcpy(directorio[i].dir_nfich, nombrenuevo);
			printf("Fichero renombrado con existo a %s\n", nombrenuevo);
		}

	}

	return 1;
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
                  EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
                  EXT_DATOS *memdatos)
{
      
      if (strcmp(orden, "dir") == 0)
      {
            Directorio(directorio, ext_blq_inodos);
            *comandoEncontrado = 1;
      }
      else if (strcmp(orden, "info") == 0)
      {
            LeeSuperBloque(ext_superblock);
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
		Renombrar(directorio, ext_blq_inodos, argumento1, argumento2);
	      *comandoEncontrado = 1;
      }
      else if (strcmp(orden, "imprimir") == 0)
      {
            Imprimir(directorio, ext_blq_inodos, memdatos, argumento1);
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
      &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos);

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
            // Orden extra, clear (para Linux)
            if (strcmp(orden, "clear") == 0){
                  system("clear");
            }
            else{
	            printf("Error: Comando desconocido [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n\n");
            }
      }
      else{
            printf("\n"); // Estetico, creo que queda mejor
      }
   }
}


/*

Echar un vistazo a pistas

comprobar q un fichero existe
dentro del directorio -> nombre del fichero y inodo, revisar dentro del directorio si un fichero 

*/
