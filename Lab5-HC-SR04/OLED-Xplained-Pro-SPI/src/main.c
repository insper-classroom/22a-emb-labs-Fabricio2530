#include <asf.h>

#include "PIO_OLED.h"
#include "PIO_FUNCTIONS.h"
#include "TC-RTT-RTC.h"

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

//PINO ECHO
#define ECHO_PI				PIOD
#define	ECHO_PI_ID			ID_PIOD
#define ECHO_PI_IDX			30
#define ECHO_PI_IDX_MASK	(1 << ECHO_PI_IDX)	

//PINO TRIGGER
#define TRIG_PI				PIOA
#define	TRIG_PI_ID			ID_PIOA
#define TRIG_PI_IDX			6
#define TRIG_PI_IDX_MASK	(1 << TRIG_PI_IDX)

//Variaveis globais
volatile double tempo = 0;
double freq = (float) 1/(2*0.000058);
volatile char aguardo = 0;
volatile double echo_flag = 0;
int contador_vect = 0;

double distancias[] = {0.0, 0.0, 0.0, 0.0};
	
void grafico_distancias(int dist) {
	distancias[contador_vect] = dist;
	
	//BLOCO DE LIMPEZA
	gfx_mono_generic_draw_filled_rect(76, 2, 48, 24, GFX_PIXEL_CLR);
	
	//BLOCO DE DESENHO
	
	if (contador_vect < 3) {
		contador_vect += 1;
	} else {
		contador_vect = 0;
	}
	
	for (int i = 0; i < 4; i++) {
		
		int altura = (distancias[i]*19)/400;
		
		if (i == 0) {
			gfx_mono_draw_pixel(87, 19 - altura, GFX_PIXEL_SET); //v[0]
		} else if ( i == 1) {
			gfx_mono_draw_pixel(95, 19 - altura, GFX_PIXEL_SET); // v[1]
		} else if (i == 2) {
			gfx_mono_draw_pixel(100, 19 - altura,GFX_PIXEL_SET); //v[2]
		} else {
			gfx_mono_draw_pixel(110, 19 - altura, GFX_PIXEL_SET); // v[3]
		}
	}
	
}

void echo_callback(void){
	//if (!pio_get(ECHO_PI, PIO_INPUT, ECHO_PI_IDX_MASK)) {
		
	if (!echo_flag) {
		
		RTT_init(freq, 0, 0);	
		echo_flag = 1;
		aguardo = 1;
		
		// 42.5 é 1/2*TMAX
		int TC_t = 42.5 + 0.2*(tempo);
		TC_init(TC2, ID_TC6, 0, TC_t);
		tc_start(TC2, 0);
		
	} else if (echo_flag && aguardo) {
		
		//reinicia a flag do echo
		echo_flag = 0;
		
		tempo = rtt_read_timer_value(RTT);
		aguardo = 0;
		tc_stop(TC2, 0);
	}
}

void TC6_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC2, 0);

	/** Muda o estado do LED (pisca) **/
	
	if (aguardo) {
		
		tempo = 0;
		//Se ultrapassa o tempo, reinicia o tempo, a flag de aguardo e do echo
		aguardo = 0;
		echo_flag = 0;
		
		tc_stop(TC2, 0);
		gfx_mono_draw_string("Erro!", 0,10, &sysfont);
	}
	
	
}

void init(void) {
	//Initialize the board clock
	sysclk_init();
	oled_init();
	
	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	//Definindo o ECHO PIO como input
	pmc_enable_periph_clk(ECHO_PI_ID);
	pio_set_input(ECHO_PI,ECHO_PI_IDX_MASK,PIO_DEFAULT);
	
	//Definindo o ECHO PIO como output
	pmc_enable_periph_clk(TRIG_PI_ID);
	pio_configure(TRIG_PI, PIO_OUTPUT_0,TRIG_PI_IDX_MASK, PIO_DEFAULT);
	
	pio_handler_set(ECHO_PI,
		ECHO_PI_ID,
		ECHO_PI_IDX_MASK,
		PIO_IT_EDGE,
		echo_callback);
		
	pio_enable_interrupt(ECHO_PI, ECHO_PI_IDX_MASK);
	pio_get_interrupt_status(ECHO_PI);
	
	NVIC_EnableIRQ(ECHO_PI_ID);
	NVIC_SetPriority(ECHO_PI_ID, 4); // Prioridade 4

}

int main (void)
{
	board_init();
	sysclk_init();
	init();
	delay_init();

  // Init OLED
	gfx_mono_ssd1306_init();
	gfx_mono_draw_string("Wait!", 0,10, &sysfont);
	gfx_mono_generic_draw_rect(75, 1, 50, 26, GFX_PIXEL_SET);
	
	while(1) {
			
			if (flag_but1) {
					
					pio_set(TRIG_PI,TRIG_PI_IDX_MASK);
					delay_us(10);
					pio_clear(TRIG_PI,TRIG_PI_IDX_MASK);
					
					pin_toggle(LED_PI1, LED_PI1_IDX_MASK);
					delay_ms(300);
					pin_toggle(LED_PI1, LED_PI1_IDX_MASK);
					flag_but1  = 0;
					
				}
			
			
			if (tempo != 0) {
				
				char str[300];
				double  tempo_real= (float) tempo/freq;
				double distancia_cm = (340*tempo_real*100.0)/2.0;
				sprintf(str, "%.1f  ", distancia_cm);
				gfx_mono_draw_string(str, 0,10, &sysfont);	
				grafico_distancias(distancia_cm);
			}
			
			pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
