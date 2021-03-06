/**
  ******************************************************************************
  * @file    stm32f3xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"

/* USER CODE BEGIN 0 */
#include "LiquidCrystal.h"
#include <stdlib.h>
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int count =1;
char temp;
extern unsigned char data[1];
extern int move;
extern int map[8][20];
extern int length_snake; 
extern int head_i;
extern int head_j;
extern int end_i;
extern int end_j;
extern int apple_i;
extern int apple_j;
int pre = 2;
int cut = 1;
int fast = 5;
int counter = 1;
int score = 1;
float time1 = 0;
int counter_repeat = 1;
int c =1;
uint32_t adcValue;
int num =0;
FILE *fptr;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc2;
void numberToBCD(int i){
	int x1 = i&1;
	int x2 = i&2;
	int x3 = i&4;
	int x4 = i&8;
	if(x1>0) x1 = 1;
	if(x2>0) x2 = 1;
	if(x3>0) x3 = 1;
	if(x4>0) x4 = 1;
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,x1);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,x2);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,x3);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,x4);
}


void draw_app(){
	srand((int)__HAL_TIM_GET_COUNTER(&htim2));
	while(1){
		apple_j = rand() % 20;
		apple_i = rand() % 8 ;
		if(map[apple_i][apple_j] == 0)
			break;
	}
	setCursor(apple_j,apple_i/2);
	if(apple_i %2 == 0)
		write(5);
	else
		write(6);
}

void Game_Over(){
	char num[10];

	setCursor(0,0);
	print("Game over                ");
	setCursor(0,2);
	print("Your score is:           ");
	sprintf(num,"%d",score);
	setCursor(14,2);
	print(num);
	HAL_TIM_Base_Stop_IT(&htim2);
}

void collide(){
	for(int i = 0; i < 8 && c;i++){
		for(int j =0; j<20 && c; j++){
			if(i == head_i && j == head_j && map[i][j] != 0 && map[i][j] != 1){
				HAL_TIM_Base_Stop_IT(&htim2);
				c =0;
				setCursor(0,1);
				print("                    ");
				setCursor(0,3);
				print("                      ");
				Game_Over();
				break;
			}
		}
	}
}

void Direction_apple(){
	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11| GPIO_PIN_12| GPIO_PIN_13| GPIO_PIN_14| GPIO_PIN_15,0);

	if(head_i == apple_i && apple_j >= head_j)
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,1);
	else if(head_i == apple_i && apple_j < head_j)
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_15,1);
	else if(head_j == apple_j && apple_i >= head_i)
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,1);
	else if(head_j == apple_j && apple_i < head_i)
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,1);
	else if(head_j < apple_j && apple_i < head_i)
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,1);
	else if(head_j < apple_j && apple_i > head_i)
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,1);
	else if(head_j > apple_j && apple_i < head_i)
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,1);
	else if(head_j > apple_j && apple_i > head_i)
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,1);
		
}


void Check_eating_apple(){
	if(head_j == apple_j){
		setCursor(head_j,head_i/2);
		if(head_i == apple_i +1 && head_i%2 ==1){
			write(4);
		}
		else if(head_i == apple_i - 1 && head_i%2 ==0)
			write(3);
		else if(head_i == apple_i){
			draw_app();	
			length_snake += 1;
			fast += 5;
			counter_repeat = 1;
			cut=0;
			score = (score + fast*length_snake)/20;
		}
	}
}
void moving(){
	cut = 1;
	for(int i = 0; i<8; i++){
		for(int j =0 ; j<20 ; j++){
			if(map[i][j] != 0 ){
				 if(map[i][j] == 1){
						head_i = i;
						head_j = j ;
				 }
				 else if(map[i][j] == length_snake ){
						end_i = i;
						end_j = j;
				 }
				map[i][j] += 1; 
				}
		}
	}

	switch(move){
		
		case 1:
			if(head_i - 1 < 0){
				head_i = 8;
			}
			setCursor(head_j,head_i/2-1);
			if(pre == 2){		
				write(1);
				pre = 1;
			}
			else if(pre == 0){
				if(head_i %2 == 0){
				write(1);
				pre = 1;
				}
			}
			else if(pre == 1){
				setCursor(head_j,head_i/2);
				write(0);
				pre = 0;
			}
			head_i -= 1; 
			collide();
			map[head_i][head_j] = 1;
			break;
			
		case 2:
			if(head_j - 1 <0){
				head_j = 20;
			}
			setCursor(head_j-1,head_i/2);
			head_j -= 1 ;
			if(pre == 2){
				write(2);
			}
			else if(pre == 0){
				if(map[head_i-1][head_j] != 0 && head_i%2 == 1)
					write(0);
				else if(map[head_i+1][head_j] != 0 && head_i%2 == 0)
					write(0);
				else if(head_i %2 == 1){
					write(1);
					pre = 1;
				}
				else{
					write(2);
					pre = 2;
				}
			}
			else if(pre == 1){		
				write(1);
			}
			collide();
			map[head_i][head_j] = 1;
			break;
				
		case 3:
			if(head_i + 1 > 7){
				head_i = -1 ;
			}
			setCursor(head_j,head_i/2+1);	
			
			if(pre == 2){
				setCursor(head_j,head_i/2);		
				write(0);
				pre = 0;
			}
			else if(pre == 0){
				if(head_i%2 == 1 || head_i == -1){
					if(head_i == -1){
						setCursor(head_j,head_i+1);	
					}
					write(2);	
					pre = 2;
				}
			}
			else if(pre == 1){
				write(2);
				pre = 2;
			}
			head_i += 1;
			collide();
			map[head_i][head_j] = 1;
			break;
		case 4:
			if(head_j +1 > 19){
				head_j = -1 ;
			}
			setCursor(head_j+1,head_i/2);		
			head_j += 1;
			if(pre == 2){
				write(2);
				
			}
			else if(pre == 0){
				if(map[head_i-1][head_j] != 0 && head_i%2 == 1)
					write(0);
				else if(map[head_i+1][head_j] != 0 && head_i%2 == 0)
					write(0);
				else if(head_i %2 == 1){
					write(1);
					pre = 1;
				}
				else{
					write(2);
					pre = 2;
				}
			}
			else if(pre == 1){
				write(1);
			}
			collide();
			map[head_i][head_j] = 1;
			break;
	}
	Check_eating_apple();
	
	if(apple_i%2 == 0 && map[apple_i+1][apple_j] != 0){
		setCursor(apple_j,apple_i/2);
		write(4);
	}
	if(apple_i%2 == 1 && map[apple_i-1][apple_j] != 0){
		setCursor(apple_j,apple_i/2);
		write(3);
	}
	if(cut){
		map[end_i][end_j] = 0;
		setCursor(end_j,end_i/2);
		//going up
		if(map[end_i-1][end_j] == length_snake  && end_i%2 ==1 ){
			write(2);	
		}
		//going down
		else if(map[end_i+1][end_j] == length_snake  && (end_i)%2 == 0){
			write(1);
		}//L move
		else if(map[end_i][end_j+1] == length_snake && map[end_i+1][end_j+1] != 0 && map[end_i+1][end_j] != 0 && end_i %2 == 0){
			write(1);
		}
		else if(map[end_i][end_j-1] == length_snake && map[end_i+1][end_j-1] != 0 && map[end_i+1][end_j] != 0 && end_i%2 ==0){
			write(1);
		}
		else if(map[end_i][end_j-1] == length_snake && map[end_i-1][end_j-1] != 0 && map[end_i-1][end_j] != 0 && end_i%2 ==1){
			write(2);
		}
		else if(map[end_i][end_j+1] == length_snake && map[end_i-1][end_j+1] != 0 && map[end_i-1][end_j] != 0&& end_i%2 ==1	){
			write(2);
		}
		// apple and snake in one square
		else if(apple_j == end_j && apple_i +1 == end_i && end_i%2 ==1){
			write(5);
		}
		else if(apple_j == end_j && apple_i -1 == end_i && end_i%2 ==0){
			write(6);
		}
		else{
			temp = ' ';
			print(&temp);
		}
	}
	Direction_apple();
}





void save_file(){
	
 
	fptr = fopen("C://program//Users//mohadesh//Desktop//file.txt","w");

	if(fptr == NULL)
	{
		setCursor(0,0);
		print("Error!");           
	}

	fprintf(fptr,"%d",num);
	fclose(fptr); 
}
void start(){
	for(int j = 0; j<8; j ++){
		for(int i = 0; i < 20 ; i++){
			map[j][i] = 0;
		}
	}
	setCursor(0,0);
	print("                ");
	setCursor(0,1);
	print("                              ");
	map[0][0] = 3;
	map[0][1] = 2;
	map[0][2] = 1;
	
	setCursor(0,0);
	write(2);
	write(2);
	write(2);
	srand((int)__HAL_TIM_GET_COUNTER(&htim2));
	apple_j = rand() % 20;
	apple_i = rand() % 8 ;
	setCursor(apple_j,apple_i/2);
	if(apple_i %2 == 0)
		write(5);
	else
		write(6);
	HAL_ADC_Start_IT(&hadc2);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);

}

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart2;

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F3xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f3xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles EXTI line0 interrupt.
*/
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */
		if(time1 < HAL_GetTick()){
		time1 = HAL_GetTick() + 100;
		for(int i =0 ; i<4 ; i++){
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_RESET);
		if(i == 0){
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
			if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_0)){
				move = 4;
			}
		}
		else if(i == 1){
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);
			if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_0)){
				move = 3;
			}
		}
		else if(i == 2){
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET);
			if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_0)){
				move = 2;
			}
		}
		else if(i == 3){
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_SET);
			if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_0)){
				move = 1;
			}
		}
	}
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_SET);

	}
  /* USER CODE END EXTI0_IRQn 1 */
}

/**
* @brief This function handles ADC1 and ADC2 interrupts.
*/
void ADC1_2_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_2_IRQn 0 */

  /* USER CODE END ADC1_2_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc2);
  /* USER CODE BEGIN ADC1_2_IRQn 1 */
	adcValue = HAL_ADC_GetValue(&hadc2);
	int t =  (adcValue *30/63);
	if(0<= t && t<=5)
		fast = MIN(fast , 5);
	else if(5<t && t <=10)
		fast = MIN(fast , 10);
	else if(10<t && t<= 15)
		fast = MIN(fast , 15);
	else if(15<t && t<= 20)
		fast = MIN(fast ,20);
	else if(20 <t && t<=25)
		fast = MIN(fast , 25);
	else if(25<t && t<=30)
		fast = MIN(fast , 30);
	
	HAL_ADC_Start_IT(&hadc2);
  /* USER CODE END ADC1_2_IRQn 1 */
}

/**
* @brief This function handles TIM2 global interrupt.
*/
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */
		if(count){
			count = 0;
		}
		else{
			switch(fast){
				case 5:
					if(counter_repeat < 10){
						counter_repeat += 1;
					}
					else{
						moving();
						counter_repeat = 1;
					}
					break;
				case 10:
					if(counter_repeat < 5){
						counter_repeat += 1;
					}
					else{
						moving();
						counter_repeat = 1;
					}
					break;
				case 15:
					if(counter_repeat < 3){
						counter_repeat += 1;
					}
					else{
						moving();
						counter_repeat = 1;
					}
					break;
				case 20:
					if(counter_repeat %2 == 0 && counter_repeat != 10){
						moving();
						counter_repeat += 1;
					}
					else if(counter_repeat == 10){
						counter_repeat = 0;
					}
					else{
						counter_repeat += 1;
					}
					break;
				case 25:
					if(counter_repeat < 2){
						counter_repeat += 1;
					}
					else{
						moving();
						counter_repeat = 1;
					}
					break;
				case 30:
					if(counter_repeat %2 == 1 && counter_repeat != 10){
						moving();
						counter_repeat += 1;
					}
					else if(counter_repeat == 10){
						moving();
						counter_repeat = 0;
					}
					else{
						counter_repeat += 1;
					}
					break;
			}
		}

  /* USER CODE END TIM2_IRQn 1 */
}

/**
* @brief This function handles TIM3 global interrupt.
*/
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */
	
	//seven seg
	if(counter == 1){
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 | GPIO_PIN_4,0);
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2,1);
		numberToBCD(length_snake%10);
		counter +=1;		
	}
	else if(counter == 2){
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2 | GPIO_PIN_4,0);
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3,1);
		numberToBCD(length_snake/10);
		counter +=1;
	}
	else if(counter == 3){
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1 | GPIO_PIN_4,0);
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3 | GPIO_PIN_4,1);
		numberToBCD(fast%10);
		counter +=1;
	}
	else if(counter ==4){	
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0 |GPIO_PIN_4 ,0);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,1);
			numberToBCD(fast/10);
			counter = 1;
	}
  /* USER CODE END TIM3_IRQn 1 */
}

/**
* @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
*/
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
	//start game
	if(data[0] == 0x30){
		save_file();
	}
	else if(data[0] == 0x31){
		start();
	}
	//loading
	else if(data[0] == 0x32){
	
	}
	//w
	if(data[0] == 0x77 || data[0] == 0x57){
		move = 1;
	}
	//a
	else if(data[0] == 0x61 || data[0] == 0x41){
		move = 2;
	}
	//s
	else if(data[0] == 0x73 || data[0] == 0x53){
		move = 3;
	}
	//d
	else if(data[0] == 0x64 || data[0] == 0x44){
		move = 4;
	}
	else 
		move = 4;
	HAL_UART_Receive_IT(&huart2, data,sizeof(data));
  /* USER CODE END USART2_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
