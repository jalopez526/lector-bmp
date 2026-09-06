#pragma once

#include "pintor.h"

typedef struct
{
	uint16_t ancho;
	uint16_t alto;
	color* pixeles;
} imagen_bmp;


imagen_bmp* cargar_bmp(const char* nombre_archivo);
void destruir_bmp(imagen_bmp* imagen);
