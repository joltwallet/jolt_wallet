#include "graphics.h"
#include "u8g2.h"

void boot_splash(u8g2_t *u8g2){
    int16_t gap = 18;
    int16_t n_steps = 25;
    int16_t logo_start_x = (u8g2_GetDisplayWidth(u8g2) - 
            GRAPHIC_NANO_LOGO_SMALL_W) / 2;
    int16_t logo_y = (u8g2_GetDisplayHeight(u8g2)-GRAPHIC_NANO_LOGO_SMALL_H)/2;
    int16_t ray_y = (u8g2_GetDisplayHeight(u8g2)-GRAPHIC_NANO_RAY_H)/2;

	u8g2_FirstPage(u8g2);
	do{
		u8g2_DrawXBM( u8g2, logo_start_x, logo_y,
                GRAPHIC_NANO_LOGO_SMALL_W,
                GRAPHIC_NANO_LOGO_SMALL_H,
                graphic_nano_logo_small);
	} while(u8g2_NextPage(u8g2));
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    // animation portion
    int16_t logo_final_x = (u8g2_GetDisplayWidth(u8g2) - 
            GRAPHIC_NANO_LOGO_SMALL_W - GRAPHIC_NANO_RAY_W - gap) / 2;
    int16_t ray_final_x = logo_final_x + GRAPHIC_NANO_LOGO_SMALL_W + gap ;
    int16_t ray_start_x = (u8g2_GetDisplayWidth(u8g2)/2) - GRAPHIC_NANO_RAY_W;
    for(int i=0; i < n_steps; i++){
	    u8g2_FirstPage(u8g2);
        do{
            // Draw RAY
			u8g2_SetDrawColor(u8g2, 1);
			u8g2_DrawXBM( u8g2,
					ray_start_x + i*(ray_final_x - ray_start_x)/n_steps,
					ray_y,
					GRAPHIC_NANO_RAY_W,
					GRAPHIC_NANO_RAY_H,
					graphic_nano_ray);
			// Blank Left Half of Screen
			u8g2_SetDrawColor(u8g2, 0);
			u8g2_DrawBox(u8g2, 0, 0,
					u8g2_GetDisplayWidth(u8g2)/2,
					u8g2_GetDisplayHeight(u8g2));
			// Draw Nano Logo
			u8g2_SetDrawColor(u8g2, 1);
			u8g2_DrawXBM( u8g2,
					logo_start_x + i*(logo_final_x - logo_start_x)/n_steps,
					ray_y,
					GRAPHIC_NANO_LOGO_SMALL_W,
					GRAPHIC_NANO_LOGO_SMALL_H,
					graphic_nano_logo_small);
        } while(u8g2_NextPage(u8g2));
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}
