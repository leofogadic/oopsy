/* Includes **************************************************************** */
#include "clock.h"

/* Private types *********************************************************** */

/* Private constants ******************************************************* */

/* Private macros ********************************************************** */

/* Private variables ******************************************************* */

/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */


/* Private functions ******************************************************* */
void initCLOCK(void)
{
	volatile uint32_t k;
	uint8_t flag = 0;
	volatile uint32_t DIVN, DIVM, DIVP, DIVQ, DIVR;
	
	// modification of MCOx only after MCU reset and before PLL config
	RCC->CFGR &= ~((RCC_CFGR_MCO1PRE)|(RCC_CFGR_MCO1));
	RCC->CFGR |= (RCC_CFGR_MCO1PRE_0);									// x1 -> 24MHz out
	RCC->CFGR |= (RCC_CFGR_MCO1_1);										// HSE24 clock source		
	
	
	PWR->CR3 |= (PWR_CR3_SCUEN);
	if(PWR->CR3 & (PWR_CR3_SCUEN))
		NVIC_SystemReset();
	
	// supply configuration update enable
	PWR->CR3 |= (PWR_CR3_LDOEN);
	__DMB();
	
	for(k=0;k<1000000;k++)
	{
		if(PWR->CSR1 & (PWR_CSR1_ACTVOSRDY))
		{
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		NVIC_SystemReset();
	}	
	
	PWR->D3CR |= (PWR_D3CR_VOS);
	// Enable SYSCFG clock mondatory for I/O Compensation Cell
	RCC->APB4ENR |= (RCC_APB4ENR_SYSCFGEN);
	__DMB();
		
	// voltage scaling for maximum system frequency
	
	SYSCFG->PWRCR |= (SYSCFG_PWRCR_ODEN);
	flag = 0;
	for(k=0;k<1000000;k++)
	{
		if(PWR->D3CR & PWR_D3CR_VOSRDY)
		{
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		NVIC_SystemReset();
	}

	RCC->CR |= RCC_CR_HSEON;
	__DMB();
	flag = 0;
	for(k=0;k<1000000;k++)
	{
		if(RCC->CR & (RCC_CR_HSERDY))
		{
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		NVIC_SystemReset();
	}
	
	// set input clock for PLL's to HSE
	RCC->PLLCKSELR = RCC_PLLCKSELR_PLLSRC_HSE;
	__DMB();
	// PLL1 config & reduce input clock from 16MHz to 2MHz
	DIVM = 8;
	DIVN = 480;
	DIVP = 2;
	DIVQ = 10;
	DIVR = 4;
	RCC->PLL1DIVR = ((DIVR-1)<<24)|((DIVQ-1)<<16)|((DIVP-1)<<9)|(DIVN-1);
	__DMB();
	RCC->PLLCKSELR |= (DIVM<<4);
	__DMB();
	RCC->PLL1FRACR = 0x00000000;
	__DMB();
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL1FRACEN);
	__DMB();
	// PLL2 config & reduce input clock from 16MHz to 2MHz
	DIVM = 16;
	DIVN = 480;
	DIVP = 1;
	DIVQ = 5;		// -> pll2_q_ck 96MHz
	DIVR = 2;;
	RCC->PLL2DIVR = ((DIVR-1)<<24)|((DIVQ-1)<<16)|((DIVP-1)<<9)|(DIVN-1);
	RCC->PLLCKSELR |= (DIVM<<12);
	RCC->PLL2FRACR = 0x00000000;
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL2FRACEN);
	
	// PLL3 config & reduce input clock from 16MHz to 2MHz
	DIVM = 16;
	DIVN = 160;
	DIVP = 2;
	DIVQ = 2;
	DIVR = 80;
	RCC->PLL3DIVR = ((DIVR-1)<<24)|((DIVQ-1)<<16)|((DIVP-1)<<9)|(DIVN-1);
	RCC->PLLCKSELR |= (DIVM<<20);
	RCC->PLL3FRACR = 0x00000000;
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL3FRACEN);
	
	// enable PLL 1,2 & 3 outputs P,Q & R
	//RCC->PLLCFGR = 0x01FF0444;
	RCC->PLLCFGR = 0x01FF0111;
	__DMB();
	// start all three PLL's
	//RCC->CR |= (RCC_CR_PLL1ON)|(RCC_CR_PLL2ON)|(RCC_CR_PLL3ON);
	RCC->CR |= (RCC_CR_PLL1ON);
	__DMB();
	flag = 0;
	for(k=0;k<1000000;k++)
	{
		if(RCC->CR & (RCC_CR_PLL1RDY))
		{
			flag++;
			break;
		}
	}
	RCC->CR |= (RCC_CR_PLL2ON);
	__DMB();
	for(k=0;k<1000000;k++)
	{
		if(RCC->CR & (RCC_CR_PLL2RDY))
		{
			flag++;
			break;
		}
	}
	RCC->CR |= (RCC_CR_PLL3ON);
	__DMB();
	for(k=0;k<1000000;k++)
	{
		if(RCC->CR & (RCC_CR_PLL3RDY))
		{
			flag++;
			break;
		}
	}
	
	if(flag < 3)
	{
		NVIC_SystemReset();
	}
	
	// set flash latency
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	__DMB();
	FLASH->ACR |= FLASH_ACR_LATENCY_3WS;
	__DMB();
	

	
	
	//RCC->D1CFGR = (RCC_D1CFGR_HPRE_DIV2)|(RCC_D1CFGR_D1PPRE_DIV4)|(RCC_D1CFGR_D1CPRE_DIV1);
	//RCC->D2CFGR = (RCC_D2CFGR_D2PPRE1_DIV4)|(RCC_D2CFGR_D2PPRE2_DIV4);
	//RCC->D3CFGR = (RCC_D3CFGR_D3PPRE_DIV4);
	RCC->D1CFGR = (RCC_D1CFGR_HPRE_DIV2)|(RCC_D1CFGR_D1PPRE_DIV2)|(RCC_D1CFGR_D1CPRE_DIV1);
	__DMB();
	RCC->D2CFGR = (RCC_D2CFGR_D2PPRE1_DIV2)|(RCC_D2CFGR_D2PPRE2_DIV2);
	__DMB();
	RCC->D3CFGR = (RCC_D3CFGR_D3PPRE_DIV2);
	__DMB();
	
	// USART clock
	RCC->D2CCIP2R |= 0x00000009;

	// FMC
	RCC->D1CCIPR |= 0x00000001;
	
	// select CPU clock to PLL1 P output		
	RCC->CFGR |= (RCC_CFGR_SW_PLL1);
	__DMB();
	flag = 0;
	for(k=0;k<1000000;k++)
	{
		if(RCC->CFGR & (RCC_CFGR_SWS_PLL1))
		{
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		NVIC_SystemReset();
	}
	

	// activate CSI clock mondatory for I/O Compensation Cell
	RCC->CR |= (RCC_CR_CSION);
	__DMB();
	
	
	// Enables the I/O Compensation Cell
	SYSCFG->CCCSR |= (SYSCFG_CCCSR_EN);
	__DMB();
	
	
	//{
		//RCC->CR |= RCC_CR_HSI48ON;
		//__DMB();
		//// wait for clock to become stable
		//while((RCC->CR & (RCC_CR_HSI48RDY)) == 0x00000000);
		
		////// select USB clock source to HSI48
		//RCC->D2CCIP2R |= RCC_D2CCIP2R_USBSEL;
		//// select USB clock source to PLL1Q clk
		////RCC->D2CCIP2R |= RCC_D2CCIP2R_USBSEL_0;

		//// enable power detector
		//PWR->CR3 |= (PWR_CR3_USB33DEN);
	//}
}


/* ***************************** END OF FILE ******************************* */






