#include <asf.h>
#include <stdio.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botão 1
#define BUT_PI1			  PIOD
#define BUT_PI1_ID        ID_PIOD
#define BUT_PI1_IDX	      28 
#define BUT_PI1_IDX_MASK (1u << BUT_PI1_IDX) // esse já está pronto.

// Botão 2
#define BUT_PI2			  PIOC
#define BUT_PI2_ID        ID_PIOC
#define BUT_PI2_IDX	      31
#define BUT_PI2_IDX_MASK (1u << BUT_PI2_IDX) // esse já está pronto.

// Botão 3
#define BUT_PI3			  PIOA
#define BUT_PI3_ID        ID_PIOA
#define BUT_PI3_IDX	      19
#define BUT_PI3_IDX_MASK (1u << BUT_PI3_IDX) // esse já está pronto.

void gfx_mono_draw_string(const char *str, const gfx_coord_t x, const gfx_coord_t y, const struct font *font);

volatile char but_subida_flag = 0;
volatile char but_descida_flag = 0;
volatile char but_2_flag = 1;
volatile char but3_flag = 0;

char str[128];
int frequencia = 300;

void but1_callback(void){
	if (!pio_get(BUT_PI1, PIO_INPUT, BUT_PI1_IDX_MASK)) {
			but_descida_flag = 1;
			but_subida_flag = 0;
	} else if (pio_get(BUT_PI1, PIO_INPUT, BUT_PI1_IDX_MASK)) {
		// PINO == 1 --> Borda de subida
		but_subida_flag = 1;
		but_descida_flag = 0;
	}	
}

void but2_callback(void){
	if (but_2_flag == 0) {
		but_2_flag = 1;	
	} else {
		but_2_flag = 1;
	}
}

void but3_callback(void) {
	but3_flag = 1;
}

void pisca_led(int n, int t){
	gfx_mono_generic_draw_filled_rect(60, 14, 50, 10, GFX_PIXEL_CLR);
	gfx_mono_generic_draw_rect(60, 14, 50, 10, GFX_PIXEL_SET);
	int width = 50;
	int inc = width/n;
	for (int i=0;i<=n;i++){
		if (but_2_flag!=1) {
			gfx_mono_generic_draw_filled_rect(60, 14, inc*i, 9, GFX_PIXEL_SET);
			pio_clear(LED_PIO, LED_IDX_MASK);
			delay_ms(t);
			pio_set(LED_PIO, LED_IDX_MASK);
			delay_ms(t);	
				
		}
	}
}

void limpa_visor(int frequencia) {
	sprintf(str, "%d", frequencia);
	gfx_mono_generic_draw_filled_rect(60, 14, 50, 10, GFX_PIXEL_CLR);
	gfx_mono_generic_draw_rect(60, 14, 50, 10, GFX_PIXEL_SET);
}

void atualiza_display(int frequencia) {
	sprintf(str, "%d", frequencia);
	gfx_mono_draw_string(str, 0, 14, &sysfont);
}

void io_init(void)
{

  // Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

  // Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PI1_ID);
	pmc_enable_periph_clk(BUT_PI2_ID);

  // Configura PIO para lidar com o pino do botão como entrada
  // com pull-up
	pio_configure(BUT_PI1, PIO_INPUT, BUT_PI1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PI1, BUT_PI1_IDX_MASK, 60);
	
	pio_configure(BUT_PI2, PIO_INPUT, BUT_PI2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PI2, BUT_PI2_IDX_MASK, 60);

  // Configura interrupção no pino referente ao botao e associa
  // função de callback caso uma interrupção for gerada
  // a função de callback é a: but_callback()
  pio_handler_set(BUT_PI1,
                  BUT_PI1_ID,
                  BUT_PI1_IDX_MASK,
                  PIO_IT_EDGE,
                  but1_callback);
				  
  pio_handler_set(BUT_PI2,
					BUT_PI2_ID,
					BUT_PI2_IDX_MASK,
					PIO_IT_RISE_EDGE,
					but2_callback);
		
  pio_handler_set(BUT_PI3,
					BUT_PI3_ID,
					BUT_PI3_IDX_MASK,
					PIO_IT_RISE_EDGE,
					but3_callback);
  
  // Ativa interrupção e limpa primeira IRQ gerada na ativacao
  pio_enable_interrupt(BUT_PI1, BUT_PI1_IDX_MASK);
  pio_get_interrupt_status(BUT_PI1);
  
  pio_enable_interrupt(BUT_PI2, BUT_PI2_IDX_MASK);
  pio_get_interrupt_status(BUT_PI2);
  
  pio_enable_interrupt(BUT_PI3, BUT_PI3_IDX_MASK);
  pio_get_interrupt_status(BUT_PI3);
  
  // Configura NVIC para receber interrupcoes do PIO do botao
  // com prioridade 4 (quanto mais próximo de 0 maior)
  NVIC_EnableIRQ(BUT_PI1_ID);
  NVIC_SetPriority(BUT_PI1_ID, 4); // Prioridade 4
  
  NVIC_EnableIRQ(BUT_PI2_ID);
  NVIC_SetPriority(BUT_PI2_ID, 10); // Prioridade 4
  
  NVIC_EnableIRQ(BUT_PI3_ID);
  NVIC_SetPriority(BUT_PI3_ID, 4); // Prioridade 4
}


int main (void)
{
	board_init();
	sysclk_init();
	delay_init();

	// Init OLED
	gfx_mono_ssd1306_init();
  
	// Escreve na tela um circulo e um texto
	io_init();
	sprintf(str, "%d", frequencia);

	/* Insert application code here, after the board has been initialized. */
	while(1) {
		
		if (!but_2_flag) {
			while (but_descida_flag) {
				
				int cont = 0;
				while(but_descida_flag) {
					cont++; 
					if (cont > 150000000) {
						frequencia -= 100;
						atualiza_display(frequencia);
						delay_ms(100);
					}
				}
				
				if (cont <=  150000000) {
					frequencia += 100;
					atualiza_display(frequencia);
					delay_ms(100);
				}
				
			}
			if (but_subida_flag) {
				pisca_led(5, frequencia);
				but_subida_flag = 0;
				atualiza_display(frequencia);
			} else {
				
			}
			
			if (but3_flag) {
				frequencia -= 100;
				atualiza_display(frequencia);
				delay_ms(100);
				//limpa_visor(frequencia);
				but3_flag = 0;
			}
			
		} 
		gfx_mono_draw_string(str, 0, 14, &sysfont);
		but_2_flag = 0;
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		gfx_mono_generic_draw_rect(60, 14, 50, 10, GFX_PIXEL_SET);
	}
}
