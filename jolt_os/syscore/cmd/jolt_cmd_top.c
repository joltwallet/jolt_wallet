#include "esp32/clk.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdio.h"

int jolt_cmd_top( int argc, char** argv )
{
    /* Gets Task CPU usage statistics */
    char* pcWriteBuffer = malloc( 2048 );
    printf( "Current Freq: %d Hz.\n", esp_clk_cpu_freq() );
    printf( "Task            Abs Time (uS)           %%Time\n"
            "*********************************************\n" );
    vTaskGetRunTimeStats( pcWriteBuffer );
    printf( pcWriteBuffer );
    free( pcWriteBuffer );
    return 0;
}
