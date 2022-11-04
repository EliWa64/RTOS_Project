/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
#include "queue.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )
	
/* Wake Time Options: [for test]
 * 1- xTaskGetTickCount()
 * 2- 0
 */
#define TASK_WAKE_TIME					0


TaskHandle_t Button_1_Monitor_Handler = NULL;
TaskHandle_t Button_2_Monitor_Handler = NULL;
TaskHandle_t Periodic_Transmitter_Handler = NULL;
TaskHandle_t Uart_Receiver_Handler = NULL;
TaskHandle_t Load_1_Simulation_Handler = NULL;
TaskHandle_t Load_2_Simulation_Handler = NULL;

SemaphoreHandle_t xMutexSemaphore;

int inTime_B1, outTime_B1, totalTime_B1;
int inTime_B2, outTime_B2, totalTime_B2;
int inTime_PT, outTime_PT, totalTime_PT;
int inTime_UART_Rx, outTime_UART_Rx, totalTime_UART_Rx;
int inTime_Load1, outTime_Load1, totalTime_Load1;
int inTime_Load2, outTime_Load2, totalTime_Load2;
int systemTime;
int cpu_load;



unsigned char* g_uartReceiverParameters[2];
static const signed char RISING_EDGE[] = "\nRISING_EDGE ";
static const signed char FALLING_EDGE[] = "\nFALLING_EDGE";

static const signed char PT_MESSAGE[] = "\nTask message";

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );

char runTimeStatsBuff[240];
/*-----------------------------------------------------------*/


		
/* Task to be created. */
void Button_1_Monitor( void * pvParameters )
{
		TickType_t xLastWakeTime;
	
		/* Assign a tag value of 1 to myself. */
		vTaskSetApplicationTaskTag( NULL, ( void * ) 1 );
			
		// Initialise the xLastWakeTime variable with the current time.
		xLastWakeTime = TASK_WAKE_TIME;
	
    for( ;; )
    {
				static unsigned char loc_buffer = PIN_IS_HIGH;
			
				if(loc_buffer == PIN_IS_LOW)
				{
						if(GPIO_read(PORT_1, PIN0) == PIN_IS_HIGH)
						{
								xTaskNotifyIndexed( Uart_Receiver_Handler, 0, (uint32_t) &RISING_EDGE, eSetValueWithOverwrite );
								loc_buffer = PIN_IS_HIGH;
						}
				}
				else if (loc_buffer == PIN_IS_HIGH)
				{
					if(GPIO_read(PORT_1, PIN0) == PIN_IS_LOW)
						{
								xTaskNotifyIndexed( Uart_Receiver_Handler, 0, (uint32_t) &FALLING_EDGE, eSetValueWithOverwrite );
								loc_buffer = PIN_IS_LOW;
						}
				}
				
				// Wait for the next cycle.
				vTaskDelayUntil( &xLastWakeTime, 5 );
    } //for
} /*Task_1*/


/* Task to be created. */
void Button_2_Monitor( void * pvParameters )
{
		TickType_t xLastWakeTime;
	
		/* Assign a tag value of 2 to myself. */
		vTaskSetApplicationTaskTag( NULL, ( void * ) 2 );
	
		// Initialise the xLastWakeTime variable with the current time.
		xLastWakeTime = TASK_WAKE_TIME;
	
    for( ;; )
    {
				static unsigned char loc_buffer = PIN_IS_HIGH;
			
				if(loc_buffer == PIN_IS_LOW)
				{
						if(GPIO_read(PORT_1, PIN1) == PIN_IS_HIGH)
						{
								xTaskNotifyIndexed( Uart_Receiver_Handler, 1, (uint32_t) &RISING_EDGE, eSetValueWithOverwrite );
								loc_buffer = PIN_IS_HIGH;
						}
				}
				else if (loc_buffer == PIN_IS_HIGH)
				{
					if(GPIO_read(PORT_1, PIN1) == PIN_IS_LOW)
						{
								xTaskNotifyIndexed( Uart_Receiver_Handler, 1, (uint32_t) &FALLING_EDGE, eSetValueWithOverwrite );
								loc_buffer = PIN_IS_LOW;
						}
				}
				
				// Wait for the next cycle.
				vTaskDelayUntil( &xLastWakeTime, 5 );
    }//for
} /*Task_2*/


/* Task to be created. */
void Periodic_Transmitter( void * pvParameters )
{
		TickType_t xLastWakeTime;
	
		/* Assign a tag value of 2 to myself. */
		vTaskSetApplicationTaskTag( NULL, ( void * ) 3 );
	
		// Initialise the xLastWakeTime variable with the current time.
		xLastWakeTime = TASK_WAKE_TIME;
	
    for( ;; )
    {
				xTaskNotifyIndexed( Uart_Receiver_Handler, 2, (uint32_t) &PT_MESSAGE, eSetValueWithOverwrite );
			
				// Wait for the next cycle.
				vTaskDelayUntil( &xLastWakeTime, 10 );
    }//for
} /*Periodic_Transmitter*/



/* Task to be created. */
void Uart_Receiver( void * pvParameters )
{
		TickType_t xLastWakeTime;
		uint32_t  loc_mailboxIndexBuffer[3] = {0};

		/* Assign a tag value of 2 to myself. */
		vTaskSetApplicationTaskTag( NULL, ( void * ) 4 );
	
		// Initialise the xLastWakeTime variable with the current time.
		xLastWakeTime = TASK_WAKE_TIME;
	
    for( ;; )
    {
				if(loc_mailboxIndexBuffer[0] == NULL)
				{
						loc_mailboxIndexBuffer[0] = ulTaskNotifyTakeIndexed( 0,      /* Use the 0th notification */
																																 pdTRUE,  /* Clear the notification value 
																																						 before exiting. */
																																 0 ); 		/* Block indefinitely. */;
				}
				else
				{
						if(vSerialPutString((const signed char*) (loc_mailboxIndexBuffer[0]), 13) == pdTRUE)
						{
								loc_mailboxIndexBuffer[0] = NULL;
						}
				}
				
				
				if(loc_mailboxIndexBuffer[1] == NULL)
				{
						loc_mailboxIndexBuffer[1] = ulTaskNotifyTakeIndexed( 1,      /* Use the 0th notification */
																																 pdTRUE,  /* Clear the notification value 
																																						 before exiting. */
																																 0 ); 		/* Block indefinitely. */;
				}
				else
				{
						if(vSerialPutString((const signed char*) (loc_mailboxIndexBuffer[1]), 13) == pdTRUE)
						{
								loc_mailboxIndexBuffer[1] = NULL;
						}
				}
				
				
				if(loc_mailboxIndexBuffer[2] == NULL)
				{
						loc_mailboxIndexBuffer[2] = ulTaskNotifyTakeIndexed( 2,      /* Use the 0th notification */
																																 pdTRUE,  /* Clear the notification value 
																																						 before exiting. */
																																 0 ); 		/* Block indefinitely. */;
				}
				else
				{
						if(vSerialPutString((const signed char*) (loc_mailboxIndexBuffer[2]), 13) == pdTRUE)
						{
								loc_mailboxIndexBuffer[2] = NULL;
						}
				}
				
				// Wait for the next cycle.
				vTaskDelayUntil( &xLastWakeTime, 2 );
    }//for
} /*Uart_Receiver*/


/* Task to be created. */
void Load_1_Simulation( void * pvParameters )
{
		TickType_t xLastWakeTime;
	
		/* Assign a tag value of 2 to myself. */
		vTaskSetApplicationTaskTag( NULL, ( void * ) 5 );
	
		// Initialise the xLastWakeTime variable with the current time.
		xLastWakeTime = TASK_WAKE_TIME;
	
    for( ;; )
    {
				int i;
				for(i=0; i<33333; i++)
				{
					 i=i;
				}
				// Wait for the next cycle.
				vTaskDelayUntil( &xLastWakeTime, 1 );
    }//for
} /*Load_1_Simulation*/



/* Task to be created. */
void Load_2_Simulation( void * pvParameters )
{
		TickType_t xLastWakeTime;
	
		/* Assign a tag value of 2 to myself. */
		vTaskSetApplicationTaskTag( NULL, ( void * ) 6 );
	
		// Initialise the xLastWakeTime variable with the current time.
		xLastWakeTime = TASK_WAKE_TIME;
	
    for( ;; )
    {
				int i;
				for(i=0; i<80000; i++)
				{
					 i=i;
				}
				// Wait for the next cycle.
				vTaskDelayUntil( &xLastWakeTime, 10 );
    }//for
} /*Load_1_Simulation*/


void Runtime_Statistics( void * pvParameters )
{
		TickType_t xLastWakeTime;
	
		/* Assign a tag value of 2 to myself. */
		vTaskSetApplicationTaskTag( NULL, ( void * ) 7 );
	
		// Initialise the xLastWakeTime variable with the current time.
		xLastWakeTime = TASK_WAKE_TIME;
	
    for( ;; )
    {
			
				vTaskGetRunTimeStats(runTimeStatsBuff);
				xSerialPutChar('\n');
				xSerialPutChar('\n');
				vSerialPutString((const signed char*) (runTimeStatsBuff), 240);
			
				// Wait for the next cycle.
				vTaskDelayUntil( &xLastWakeTime, 100 );
    }//for
} /*Periodic_Transmitter*/

/* Task to be created. */



void vApplicationTickHook(void)
{
	GPIO_write(PORT_0, PIN1, PIN_IS_HIGH);
	GPIO_write(PORT_0, PIN1, PIN_IS_LOW);
}

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
	xMutexSemaphore = xSemaphoreCreateMutex();
	
	
    /* Create the task, storing the handle. */
    xTaskPeriodicCreate(
											Button_1_Monitor,       /* Function that implements the task. */
											"B1",          /* Text name for the task. */
											100,      /* Stack size in words, not bytes. */
												( void * ) 0,    /* Parameter passed into the task. */
											1,/* Priority at which the task is created. */
											&Button_1_Monitor_Handler
											,5);      /* Used to pass out the created task's handle. */
										
		/* Create the task, storing the handle. */
    xTaskPeriodicCreate(
											Button_2_Monitor,       /* Function that implements the task. */
											"B2",          /* Text name for the task. */
											100,      /* Stack size in words, not bytes. */
											( void * ) 0,    /* Parameter passed into the task. */
											1,/* Priority at which the task is created. */
											&Button_2_Monitor_Handler
											,5);      /* Used to pass out the created task's handle. */
											
											/* Create the task, storing the handle. */
    xTaskPeriodicCreate(
											Periodic_Transmitter,       /* Function that implements the task. */
											"P_Tx",          /* Text name for the task. */
											100,      /* Stack size in words, not bytes. */
											( void * ) 0,    /* Parameter passed into the task. */
											1,/* Priority at which the task is created. */
											&Periodic_Transmitter_Handler
											,10);      /* Used to pass out the created task's handle. */
											/* Create the task, storing the handle. */
    xTaskPeriodicCreate(
											Uart_Receiver,       /* Function that implements the task. */
											"Uart_Rx",          /* Text name for the task. */
											100,      /* Stack size in words, not bytes. */
											( void * ) 0,    /* Parameter passed into the task. */
											1,/* Priority at which the task is created. */
											&Uart_Receiver_Handler
											,2);      /* Used to pass out the created task's handle. */
		xTaskPeriodicCreate(
											Load_1_Simulation,       /* Function that implements the task. */
											"Load1",          /* Text name for the task. */
											100,      /* Stack size in words, not bytes. */
											( void * ) 0,    /* Parameter passed into the task. */
											1,/* Priority at which the task is created. */
											&Load_1_Simulation_Handler
											,1);      /* Used to pass out the created task's handle. */
		xTaskPeriodicCreate(
											Load_2_Simulation,       /* Function that implements the task. */
											"Load2",          /* Text name for the task. */
											100,      /* Stack size in words, not bytes. */
											( void * ) 0,    /* Parameter passed into the task. */
											1,/* Priority at which the task is created. */
											&Load_2_Simulation_Handler
											,10);      /* Used to pass out the created task's handle. */
		xTaskPeriodicCreate(
											Runtime_Statistics,       /* Function that implements the task. */
											"RTime Stats",          /* Text name for the task. */
											100,      /* Stack size in words, not bytes. */
											( void * ) 0,    /* Parameter passed into the task. */
											1,/* Priority at which the task is created. */
											&Load_2_Simulation_Handler
											,100);      /* Used to pass out the created task's handle. */
										




	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/



void vSwitchedInHandler(int taskTag)
{
		if(taskTag == 0)
		{
				GPIO_write(PORT_0, PIN9, PIN_IS_HIGH);
		} //if
		else if(taskTag == 1)
		{
				GPIO_write(PORT_0, PIN2, PIN_IS_HIGH);
				inTime_B1 = T1TC;
		} //if
		else if(taskTag == 2)
		{
				GPIO_write(PORT_0, PIN3, PIN_IS_HIGH);
				inTime_B2 = T1TC;
		} //if
		else if(taskTag == 3)
		{
				GPIO_write(PORT_0, PIN4, PIN_IS_HIGH);
				inTime_PT = T1TC;
		} //if
		else if(taskTag == 4)
		{
				GPIO_write(PORT_0, PIN5, PIN_IS_HIGH);
				inTime_UART_Rx = T1TC;
		} //if
		else if(taskTag == 5)
		{
				GPIO_write(PORT_0, PIN6, PIN_IS_HIGH);
				inTime_Load1 = T1TC;
		} //if
		else if(taskTag == 6)
		{
				GPIO_write(PORT_0, PIN7, PIN_IS_HIGH);
				inTime_Load2 = T1TC;
		} //if
		else if(taskTag == 7)
		{
				GPIO_write(PORT_0, PIN8, PIN_IS_HIGH);
		} //if
}


void vSwitchedOutHandler(int taskTag)
{
		if(taskTag == 0)
		{
				GPIO_write(PORT_0, PIN9, PIN_IS_LOW);
		} //if
		else if(taskTag == 1)
		{
				GPIO_write(PORT_0, PIN2, PIN_IS_LOW);
				outTime_B1 = T1TC;
				totalTime_B1 += outTime_B1 - inTime_B1;
		} //if
		else if(taskTag == 2)
		{
				GPIO_write(PORT_0, PIN3, PIN_IS_LOW);
				outTime_B2 = T1TC;
				totalTime_B2 += outTime_B2 - inTime_B2;
		} //if
		else if(taskTag == 3)
		{
				GPIO_write(PORT_0, PIN4, PIN_IS_LOW);
				outTime_PT = T1TC;
				totalTime_PT += outTime_PT - inTime_PT;
		} //if
		else if(taskTag == 4)
		{
				GPIO_write(PORT_0, PIN5, PIN_IS_LOW);
				outTime_UART_Rx = T1TC;
				totalTime_UART_Rx += outTime_UART_Rx - inTime_UART_Rx;
		} //if
		else if(taskTag == 5)
		{
				GPIO_write(PORT_0, PIN6, PIN_IS_LOW);
				outTime_Load1 = T1TC;
				totalTime_Load1 += outTime_Load1 - inTime_Load1;
		} //if
		else if(taskTag == 6)
		{
				GPIO_write(PORT_0, PIN7, PIN_IS_LOW);
				outTime_Load2 = T1TC;
				totalTime_Load2 += outTime_Load2 - inTime_Load2;
		} //if
		else if(taskTag == 7)
		{
				GPIO_write(PORT_0, PIN8, PIN_IS_LOW);
		} //if
		
		systemTime = T1TC;
		cpu_load = ((totalTime_B1 + totalTime_B2 + totalTime_PT + totalTime_UART_Rx +
				totalTime_Load1 + totalTime_Load2) / (float)systemTime) * 100;
}
