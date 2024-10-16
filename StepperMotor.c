/*  example.c: 
*   Name : Nahid Abbas
*   Date: Oct 13 2023
*   Description : lab 6: code for controlling stepper motorstep using the LV8712T driver board.F411 
                        [As reference GPIO connections : (PS PIN5 GPIOC)(OE PIN9 GPIOC)(RES PIN8 GPIOC)
                        (FR PIN6 GPIOC)(STEP PIN1 GPIOA)(ATT1 PIN5 GPIOB)(ATT2 PIN6 GPIOB)(MD1 PIN10 GPIOB)
                        (MD2 PIN PIN11 GPIOA)(VERF PIN4 GPIOA)].
 */                       

#include <stdio.h>
#include <stdint.h>
#include "common.h"


TIM_HandleTypeDef htim11; 

void timDelay(uint32_t msToDelay); 
void HAL_TIM_TimeElapsed(TIM_HandleTypeDef *htim); 
volatile int cnt = 0;   

ParserReturnVal_t stepperInit(int mode) 
{ 
  if(mode != CMD_INTERACTIVE) return CmdReturnOk; 

 GPIO_InitTypeDef  GPIO_InitStruct = {0}; 
 __HAL_RCC_GPIOA_CLK_ENABLE();
 __HAL_RCC_GPIOB_CLK_ENABLE();
 __HAL_RCC_GPIOC_CLK_ENABLE(); 
    
 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_11, 0);  
 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5 | GPIO_PIN_6 |  GPIO_PIN_10 , 0);                         
 HAL_GPIO_WritePin(GPIOC,  GPIO_PIN_5| GPIO_PIN_8 | GPIO_PIN_9  |  GPIO_PIN_6 ,0);                                              
      
                       
 GPIO_InitStruct.Pin = GPIO_PIN_1  | GPIO_PIN_4 | GPIO_PIN_11 ;
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
 GPIO_InitStruct.Pull = GPIO_NOPULL; 
 HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
    
    
 GPIO_InitStruct.Pin = GPIO_PIN_5  |GPIO_PIN_6|  GPIO_PIN_10 ; 
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
 GPIO_InitStruct.Pull = GPIO_NOPULL; 
 HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  
    
    
 GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_6 ; 
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
 GPIO_InitStruct.Pull = GPIO_NOPULL; 
 HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); 
    
    
  /* TIMERCH11 Initialization  */    
 __HAL_RCC_TIM11_CLK_ENABLE(); 

    uint16_t freq = 10000;//0.1ms time base 

    htim11.Instance = TIM11; 
    htim11.Init.Prescaler = HAL_RCC_GetPCLK2Freq() / freq - 1;   
    htim11.Init.CounterMode = TIM_COUNTERMODE_UP; 
    htim11.Init.Period = 9; 
    htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; 
    htim11.Init.RepetitionCounter = 0; 
    htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; 
    HAL_TIM_Base_Init(&htim11); 


    /* TIM11 interrupt priority */ 
 HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 0, 0); 

    HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn); 


    /* Steppermotor Initilization */ 
 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5,1);// PS  high Operating mode 
 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, 1);// RES high  
 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,1);//VREF high (no DAC)
 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,1); //MD1 high
 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11,1); //MD2 high
 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, 0); //ATT1  low 100% current
 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6,0); //ATT2 low

 return CmdReturnOk; 
} 

 ParserReturnVal_t stepperEnable(int mode)    //stepper motor eneable command function
{ 
   if(mode != CMD_INTERACTIVE) return CmdReturnOk; 

    int fetchResult = 0; 
    uint32_t stMotor = 0; 

   fetchResult = fetch_uint32_arg(&stMotor); 
      
       if(fetchResult) 
       { 
  	    printf("Inter 1 for Enable and 0 for Disable \n"); 
  	    return CmdReturnBadParameter1; 
       } 

       if(stMotor == 1) 
       { 
           HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9 , 0); // OE LOW (ENABLE)
       } 
       else 
    { 
           HAL_GPIO_WritePin(GPIOC ,GPIO_PIN_9, 1); // OE HIGH(DISABLE)
    } 
      return CmdReturnOk; 
} 



   ParserReturnVal_t stepperSteps(int mode) 
{ 
    if(mode != CMD_INTERACTIVE) return CmdReturnOk; 

  
     int fResult = 0; 
     int32_t steps = 0; 
     uint32_t delay = 0; 


 fResult = fetch_int32_arg(&steps); 
  if(fResult) 
  { 
      printf("provide  steps and delay.\n"); 
      return CmdReturnBadParameter1; 
  } 

  fResult = fetch_uint32_arg(&delay); 
    if (fResult) 
  { 
      printf("define the delay between steps.\n"); 
      return CmdReturnBadParameter1; 
  } 

    if(steps > 0) 
  { 
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 , 0); //FR LOW
         // while 
           for ( int i = 0; i < steps ; i++) 
       { 
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 1);  //STEP high
          
          timDelay(delay); 

          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1, 0); //STEP LOW
          
          timDelay(delay); 

       } 
  } 
  else 
  { 
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 , 1); //FR high unticlockwise
        
         for( int i = 0 ; i < -steps ; i++) 
      { 

          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 , 1); //STEP high
          
          timDelay(delay); 

          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1 , 0); 
          
          timDelay(delay); 
      } 
  } 

   return CmdReturnOk; 

}  

   

ADD_CMD("stepperinit", stepperInit, "                STEPPER initialization") 
ADD_CMD("stepperenable", stepperEnable, "                STEPPER output enabling") 
ADD_CMD("step", stepperSteps, "                STEPPER control") 

  
   void delay(uint32_t Delay) 
  { 

  HAL_TIM_Base_Start_IT(&htim11); 

  while(cnt < Delay) 
  { 
        asm volatile ("nop\n");
     WDTFeed(); 
  } 
  HAL_TIM_Base_Stop_IT(&htim11); 
  
  cnt = 0;
} 
void TIM1_TRG_COM_TIM11_IRQHandler(void) 
{ 

 HAL_TIM_IRQHandler(&htim11); 
} 

void HAL_TIM_TimeElapsed(TIM_HandleTypeDef *htim)
{ 
      if (htim == &htim11 ) {
          cnt++;            
       } 


}
