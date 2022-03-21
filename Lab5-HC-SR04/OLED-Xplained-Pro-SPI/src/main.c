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
volatile char subida = 0;
volatile char descida = 0;
volatile double tempo = 0;
double freq = (float) 1/(2*0.000058);

volatile double echo_flag = 0;

volatile double dist = 0;
char str[300];


void echo_callback(void){
	//if (!pio_get(ECHO_PI, PIO_INPUT, ECHO_PI_IDX_MASK)) {
		
	if (!echo_flag) {
		
		RTT_init(freq, 0, 0);	
		//pin_toggle(LED_PI2, LED_PI2_IDX_MASK);
		//delay_ms(300);
		//pin_toggle(LED_PI2, LED_PI2_IDX_MASK);
		echo_flag = 1;
		
	} 
	
	else {
		echo_flag = 0;
		
		tempo = rtt_read_timer_value(RTT);
		
		//	'apin_toggle(LED_PI3, LED_PI3_IDX_MASK);
		//delay_ms(300);
		//pin_toggle(LED_PI3, LED_PI3_IDX_MASK);
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
	gfx_mono_draw_string("Aguardando", 0,16, &sysfont);
  /* Insert application code here, after the board has been initialized. */
	while(1) {
			
			if (!pio_get(BUT_PI1,PIO_INPUT, BUT_PI1_IDX_MASK)) {
					
					pio_set(TRIG_PI,TRIG_PI_IDX_MASK);
					delay_us(10);
					pio_clear(TRIG_PI,TRIG_PI_IDX_MASK);
					
					pin_toggle(LED_PI1, LED_PI1_IDX_MASK);
					delay_ms(300);
					pin_toggle(LED_PI1, LED_PI1_IDX_MASK);
				}
			
			
			if (tempo != 0) {
				
				double  tempo_real= (float) tempo/freq;
				double distancia_cm = (340*tempo_real*100.0)/2.0;
				
				sprintf(str, "%lf", distancia_cm);
				gfx_mono_draw_string(str, 0,16, &sysfont);
				
				
				
			}
			
	}
}
