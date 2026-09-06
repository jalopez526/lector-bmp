#include "bmp.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PRIMERA_CABECERA_BMP 14
#define SEGUNDA_CABECERA_BMP 40

static uint32_t leer_u32(const uint8_t* bytes, size_t i)
{
	return (uint32_t)bytes[i] |
		((uint32_t)bytes[i + 1] << 8) |
		((uint32_t)bytes[i + 2] << 16) |
		((uint32_t)bytes[i + 3] << 24);
}

static uint16_t leer_u16(const uint8_t* bytes, size_t i)
{
	return (uint16_t)bytes[i] |
		((uint16_t)bytes[i + 1] << 8);
}

imagen_bmp* cargar_bmp(const char* nombre_archivo)
{
	uint8_t cabecera_primera[PRIMERA_CABECERA_BMP];
	FILE* archivo = fopen(nombre_archivo, "rb");

	if(archivo == NULL)
	{
		printf("Error al cargar archivo %s\n", nombre_archivo);
		return NULL;
	}

	if(fread(cabecera_primera, sizeof(uint8_t), PRIMERA_CABECERA_BMP, archivo) != PRIMERA_CABECERA_BMP)
	{
		printf("No se pudo leer la cabecera BMP\n");
		fclose(archivo);
		return NULL;
	}

	if(cabecera_primera[0] != 'B' || cabecera_primera[1] != 'M')
	{
		printf("El archivo no es un BMP\n");
		fclose(archivo);
		return NULL;
	}


	uint32_t offset_pixeles = leer_u32(cabecera_primera, 10);
	uint8_t cabecera_segunda[SEGUNDA_CABECERA_BMP];
	if(fread(cabecera_segunda, sizeof(uint8_t), SEGUNDA_CABECERA_BMP, archivo) != SEGUNDA_CABECERA_BMP)
	{
		printf("No se pudo leer la cabecera DIB\n");
		fclose(archivo);
		return NULL;
	}

	uint32_t tamano_dib = leer_u32(cabecera_segunda, 0);
	int32_t ancho = (int32_t)leer_u32(cabecera_segunda, 4);
	int32_t alto = (int32_t)leer_u32(cabecera_segunda, 8);
	uint16_t planos = leer_u16(cabecera_segunda, 12);
	uint16_t bits_por_pixel = leer_u16(cabecera_segunda, 14);
	uint32_t compresion = leer_u32(cabecera_segunda, 16);

	int32_t alto_abs = alto < 0 ? -alto : alto;
	if(tamano_dib < 40 || ancho <= 0 || alto == 0 || planos != 1 ||
		bits_por_pixel != 32 || compresion != 3 ||
		ancho > 65535 || alto_abs > 65535)
	{
		printf("Este archivo no es un BMP o no es soportado\n");
		fclose(archivo);
		return NULL;
	}

	imagen_bmp* imagen = malloc(sizeof(imagen_bmp));
	if(imagen == NULL)
	{
		fclose(archivo);
		return NULL;
	}

	imagen->ancho = (uint16_t)ancho;
	imagen->alto = (uint16_t)alto_abs;
	imagen->pixeles = malloc((size_t)ancho * (size_t)alto_abs * sizeof(color));
	if(imagen->pixeles == NULL)
	{
		free(imagen);
		fclose(archivo);
		return NULL;
	}

	if(fseek(archivo, (long)offset_pixeles, SEEK_SET) != 0)
	{
		printf("No se pudo saltar a los pixeles\n");
		destruir_bmp(imagen);
		fclose(archivo);
		return NULL;
	}

	uint32_t bytes_por_fila = (uint32_t)ancho * 4;
	uint32_t padding = (4 - (bytes_por_fila % 4)) % 4;
	int de_abajo_arriba = alto > 0;

	for(int32_t fila = 0; fila < alto_abs; fila++)
	{
		int32_t y = de_abajo_arriba ? (alto_abs - 1 - fila) : fila;
		for(int32_t x = 0; x < ancho; x++)
		{
			uint8_t bgra[4];
			if(fread(bgra, sizeof(uint8_t), 4, archivo) != 4)
			{
				printf("No se pudieron leer los pixeles\n");
				destruir_bmp(imagen);
				fclose(archivo);
				return NULL;
			}

			color pixel;
			pixel.azul = bgra[0];
			pixel.verde = bgra[1];
			pixel.rojo = bgra[2];
			imagen->pixeles[y * ancho + x] = pixel;
		}

		if(padding > 0 && fseek(archivo, (long)padding, SEEK_CUR) != 0)
		{
			printf("No se pudo saltar el padding\n");
			destruir_bmp(imagen);
			fclose(archivo);
			return NULL;
		}
	}

	fclose(archivo);
	return imagen;
}

void destruir_bmp(imagen_bmp* imagen)
{
	if(imagen == NULL)
	{
		return;
	}

	free(imagen->pixeles);
	free(imagen);
}

