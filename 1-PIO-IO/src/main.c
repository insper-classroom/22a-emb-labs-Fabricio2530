/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define LED_PIO           PIOC                 // periferico que controla o LED
// #
#define LED_PIO_ID        ID_PIOC        // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX       8                    // ID do LED no PIO
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Configuracoes do botao
#define BUT_PIO			  PIOA
#define BUT_PIO_ID        ID_PIOA
#define BUT_PIO_IDX	      11
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX) // esse já está pronto.

//LED 1
#define LED_PI1			  PIOA 
#define LED_PI1_ID		  ID_PIOA
#define LED_PI1_IDX		  0
#define LED_PI1_IDX_MASK  (1 << LED_PI1_IDX)   // Mascara para CONTROLARMOS o LED

//LED 2
#define LED_PI2			  PIOC
#define LED_PI2_ID		  ID_PIOC
#define LED_PI2_IDX		  30
#define LED_PI2_IDX_MASK  (1 << LED_PI2_IDX)   // Mascara para CONTROLARMOS o LED

//LED 3
#define LED_PI3			  PIOB
#define LED_PI3_ID		  ID_PIOB
#define LED_PI3_IDX		  2
#define LED_PI3_IDX_MASK  (1 << LED_PI3_IDX)   // Mascara para CONTROLARMOS o LED

// Configuracoes do BOTAO 1
#define BUT_PI1			  PIOD
#define BUT_PI1_ID        ID_PIOD
#define BUT_PI1_IDX	      28
#define BUT_PI1_IDX_MASK (1u << BUT_PI1_IDX) // esse já está pronto.

// Configuracoes do BOTAO 2
#define BUT_PI2			  PIOC
#define BUT_PI2_ID        ID_PIOC
#define BUT_PI2_IDX	      31
#define BUT_PI2_IDX_MASK (1u << BUT_PI2_IDX) // esse já está pronto.

// Configuracoes do BOTAO 3
#define BUT_PI3			  PIOA
#define BUT_PI3_ID        ID_PIOA
#define BUT_PI3_IDX	      19
#define BUT_PI3_IDX_MASK (1u << BUT_PI3_IDX) // esse já está pronto.


/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{
	//Initialize the board clock
	sysclk_init();
	
	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID);
	//Inicializa PC8 como saída
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT_PIO,BUT_PIO_IDX_MASK,PIO_DEFAULT);
	pio_pull_up(BUT_PIO,BUT_PIO_IDX_MASK,1);
	
	//INICIANDO O PROCESSO COM A PLACA OLED
	pmc_enable_periph_clk(LED_PI1_ID);
	//Inicializa LED1 como saída
	pio_set_output(LED_PI1, LED_PI1_IDX_MASK, 0, 0, 0);
	
	pmc_enable_periph_clk(LED_PI2_ID);
	//Inicializa LED2 como saída
	pio_set_output(LED_PI2, LED_PI2_IDX_MASK, 0, 0, 0);
	
	pmc_enable_periph_clk(LED_PI3_ID);
	//Inicializa LED3 como saída
	pio_set_output(LED_PI3, LED_PI3_IDX_MASK, 0, 0, 0);
	
	// Inicializa PIO do BOTAO 1
	pmc_enable_periph_clk(BUT_PI1_ID);
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT_PI1,BUT_PI1_IDX_MASK,PIO_DEFAULT);
	pio_pull_up(BUT_PI1,BUT_PI1_IDX_MASK,1);
	
	// Inicializa PIO do BOTAO 2
	pmc_enable_periph_clk(BUT_PI2_ID);
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT_PI2,BUT_PI2_IDX_MASK,PIO_DEFAULT);
	pio_pull_up(BUT_PI2,BUT_PI2_IDX_MASK,1);

	// Inicializa PIO do BOTAO 3
	pmc_enable_periph_clk(BUT_PI3_ID);
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT_PI3,BUT_PI3_IDX_MASK,PIO_DEFAULT);
	pio_pull_up(BUT_PI3,BUT_PI3_IDX_MASK,1);
	



	
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();  
  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  //int pio_get_result = pio_get(BUT_PIO,PIO_INPUT, BUT_PIO_IDX_MASK);
  while (1){ 
	if (!pio_get(BUT_PIO,PIO_INPUT, BUT_PIO_IDX_MASK))
	{
		for (int i=0; i < 6; i++) {
			pio_set(LED_PIO, LED_PIO_IDX_MASK);      // Coloca 1 no pino LED
			delay_ms(500);                       // Delay por software de 200 ms
			pio_clear(LED_PIO, LED_PIO_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(500);                       // Delay por software de 200 ms
		} 
	
	} else if (!pio_get(BUT_PI1,PIO_INPUT, BUT_PI1_IDX_MASK))
		{
			for (int i=0; i < 6; i++) {
				pio_set(LED_PI1, LED_PI1_IDX_MASK);      // Coloca 1 no pino LED
				delay_ms(500);                       // Delay por software de 200 ms
				pio_clear(LED_PI1, LED_PI1_IDX_MASK);    // Coloca 0 no pino do LED
				delay_ms(500);                       // Delay por software de 200 ms
			}
	} else if (!pio_get(BUT_PI2,PIO_INPUT, BUT_PI2_IDX_MASK)) {
		
		for (int i=0; i < 6; i++) {
			pio_set(LED_PI2, LED_PI2_IDX_MASK);      // Coloca 1 no pino LED
			delay_ms(500);                       // Delay por software de 200 ms
			pio_clear(LED_PI2, LED_PI2_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(500);
		}
	} else if (!pio_get(BUT_PI3,PIO_INPUT, BUT_PI3_IDX_MASK)) {
		
		for (int i=0; i < 6; i++) {
			pio_set(LED_PI3, LED_PI3_IDX_MASK);      // Coloca 1 no pino LED
			delay_ms(500);                       // Delay por software de 200 ms
			pio_clear(LED_PI3, LED_PI3_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(500);
		}
	}
	
  }
  return 0;
}
