#include <asf.h>

#include "PIO_OLED.h"

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

//PINO ECHO
#define ECHO_PI				PIOA
#define	ECHO_PI_ID			ID_PIOA
#define ECHO_PI_IDX			21
#define ECHO_PI_IDX_MASK	(1 << ECHO_PI_IDX)	

//PINO TRIGGER
#define TRIG_PI				PIOB
#define	TRIG_PI_ID			ID_PIOB
#define TRIG_PI_IDX			4
#define TRIG_PI_IDX_MASK	(1 << TRIG_PI_IDX)

//Variaveis globais
volatile char subida = 0;
volatile char descida = 0;
volatile double tempo = 0;
volatile double tempo_rtt = 0;
volatile double dist = 0;
char str[300];


void echo_callback(void){
	if (!pio_get(ECHO_PI, PIO_INPUT, ECHO_PI_IDX_MASK)) {
		descida = 1;
		subida = 0;
		
	} else if (pio_get(ECHO_PI, PIO_INPUT, ECHO_PI_IDX_MASK)) {
		// PINO == 1 --> Borda de subida
		subida = 1;
		descida = 0;
	}
}

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		//pin_toggle(LED_PI2, LED_PI2_IDX_MASK);    // BLINK Led
		tempo_rtt+=1;
	}
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
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
			
			if (subida) {
				
				RTT_init(1/(2*0.000058), 0, RTT_MR_RTTINCIEN);
				
				pin_toggle(LED_PI2, LED_PI2_IDX_MASK);
				delay_ms(300);
				pin_toggle(LED_PI2, LED_PI2_IDX_MASK);
			
			}
			
			if (descida) {	
				tempo = rtt_read_timer_value(RTT);
				
				pin_toggle(LED_PI3, LED_PI3_IDX_MASK);
				delay_ms(300);
				pin_toggle(LED_PI3, LED_PI3_IDX_MASK);
			}
			
			
			if (tempo != 0) {
				//regra de 3: se 2*0.000058s = 0.02m, t (segundos) = x (m)
				
				dist = (tempo*0.02)/(2*0.000058);
				
				sprintf(str, "%lf", dist);
				
				
			}
			
	}
}
