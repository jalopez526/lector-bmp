#include "pintor.h"
#include "bmp.h"

#include <stdio.h>

int main(void)
{
	sdl_config config;
	imagen_bmp* imagen = cargar_bmp("test.bmp");

	if(imagen == NULL)
	{
		fprintf(stderr, "No se pudo cargar test.bmp\n");
		return 1;
	}

	iniciar(&config);
	for(uint16_t y = 0; y < imagen->alto; y++)
		for(uint16_t x = 0; x < imagen->ancho; x++)
			pintar(&config, x, y, &imagen->pixeles[y * imagen->ancho + x]);

	SDL_RenderPresent(config.render);
	SDL_Delay(5000);

	SDL_DestroyRenderer(config.render);
	SDL_DestroyWindow(config.ventana);
	SDL_Quit();
	destruir_bmp(imagen);
}
