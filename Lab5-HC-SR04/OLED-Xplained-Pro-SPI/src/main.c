#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

//LED 2
#define LED_PI2			  PIOC
#define LED_PI2_ID		  ID_PIOC
#define LED_PI2_IDX		  30
#define LED_PI2_IDX_MASK  (1 << LED_PI2_IDX)   // Mascara para CONTROLARMOS o LED

// Configuracoes do BOTAO 1
#define BUT_PI1			  PIOD
#define BUT_PI1_ID        ID_PIOD
#define BUT_PI1_IDX	      28
#define BUT_PI1_IDX_MASK (1u << BUT_PI1_IDX) 

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
volatile char tempo = 0;


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
		tempo+=1;
	}
}

void init(void) {
	//Initialize the board clock
	sysclk_init();
	
	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	pmc_enable_periph_clk(LED_PI2_ID);
	//Inicializa LED2 como saída
	pio_set_output(LED_PI2, LED_PI2_IDX_MASK, 0, 0, 0);
	
	// Inicializa PIO do BOTAO 1
	pmc_enable_periph_clk(BUT_PI1_ID);
	
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT_PI1,BUT_PI1_IDX_MASK,PIO_DEFAULT);
	pio_pull_up(BUT_PI1,BUT_PI1_IDX_MASK,1);
	pio_configure(BUT_PI1, PIO_INPUT, BUT_PI1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PI1, BUT_PI1_IDX_MASK, 60);
	
	//Definindo o ECHO PIO como input
	pmc_enable_periph_clk(ECHO_PI_ID);
	pio_configure(ECHO_PI, PIO_INPUT,ECHO_PI_IDX_MASK, PIO_DEFAULT);
	
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
  /* Insert application code here, after the board has been initialized. */
	while(1) {
			
			if (!pio_get(BUT_PI1,PIO_INPUT, BUT_PI1_IDX_MASK)) {
					pio_set(TRIG_PI,TRIG_PI_IDX_MASK);
					delay_us(10);
					pio_clear(TRIG_PI,TRIG_PI_IDX_MASK);
				}
			
			if (subida) {
				RTT_init(1, 0, RTT_MR_RTTINCIEN);
			}
			
			if (descida) {	
				rtt_read_timer_value(RTT);
			}
		
			pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
