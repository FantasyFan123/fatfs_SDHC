/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * [File Name]     main.c
 * [Platform]      FRDM-K64F
 * [Project]       fatfs_SDHC
 * [Version]       1.00
 * [Author]        B55335
 * [Date]          12/24/2015
 * [Language]      'C'
 * [History]       1.00 - Original Release
 *
 */

//-----------------------------------------------------------------------
// Standard C/C++ Includes
//-----------------------------------------------------------------------

#include <stdio.h>
//-----------------------------------------------------------------------
// KSDK Includes
//-----------------------------------------------------------------------
#include "main.h"
//-----------------------------------------------------------------------
// Application Includes
//-----------------------------------------------------------------------
#include "fsl_os_abstraction.h"
#include "diskio.h"
#include "ff.h"
//-----------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------
bool sdhc_detect(void);
//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const char *str = "fatfs on SDcard demo using K64 SDHC.\n";
//-----------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------
uint8_t buff[100];
//memset(buff,0,100);
//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------

int main(void)
{
  
    FRESULT fr;		                        // FatFs return code 
    DRESULT ds;		                        // Disk functions return code 
    FATFS FatFs;	                        // FatFs system object 
    FIL fil;                                    // fill*/
    
    uint32_t size,sizetmp;

    // Initialize Operating System Abstraction layer 
    OSA_Init();
    // Configure board specific pin muxing
    hardware_init();
    // Initialize UART terminal
    dbg_uart_init();
    
/*--------------------------fatfs init-------------------------------*/
    
    /* Configure SDHC module pins */
    configure_sdhc_pins(BOARD_SDHC_INSTANCE);
    GPIO_DRV_InputPinInit(&sdhcCdPin[0]);
    if(!sdhc_detect())
    {
        printf("Please insert SD Card\r\n");
        // Wait for SD Card insertion 
        while (!sdhc_detect());
    }
    printf("SD Card inserted\r\n");
    printf("Initializing SD Card\r\n");
    
    MPU_HAL_Disable(MPU);      
    // Initialize SDHC driver and SD Card 
    ds = disk_initialize(SD);
    if(ds)
    {
        printf("Failed to initialize SD disk\r\n");
        for(;;){}
    }
    
    // Select current logical device driver (0 = USB, 1 = SD) 
    fr = f_chdrive(SD);

    printf("Mounting file system to SD Card volume...\r\n");
    
    // Mount file system to the SDCARD volume 
    fr = f_mount(SD, &FatFs);
    if(fr)
    {
        printf("Error mounting file system\r\n");
        for(;;){}
    }
    
    //open file,if no vaild file exist,creat a file with read and write
    fr = f_open(&fil,"test.bin",FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
    if(fr)
    {
        printf("Open file error!\r\n");
        printf("fr = %d\r\n", fr);
        for(;;){}
    }
    
    f_puts(str,&fil);                            //write string to file 
    f_sync(&fil);                                //f_puts does not write the data to disk, need to call f_sync to synchronize or close file
                                                //then data has been write into disk. 
    size = f_size(&fil);                        //get size
    printf("size = %d\n",size);
    printf("strlen = %d\n",strlen(str));        //compare size
    
    f_lseek(&fil,0);                            //Point the pointer to the starting address of the file
    fr = f_read(&fil,buff,size,&sizetmp);
    if(fr)
    {
        printf("Read file error!\r\n");
        printf("return = %d\r\n", fr);
    }
    printf("\nstring is \n%s", str);
    printf("\nfile content is \n%s", (char const *)buff);
    f_close(&fil);
    for (;;)                                         // Forever loop
    {
        __asm("NOP");
    }


}

/*FUNCTION*----------------------------------------------------------------
* Function Name : sdhc_detect
* Comments : Detect if the SD Card is present or not with a GPIO pin
* Returns: [0] - Card not available
* [1] - Card is present
*END*--------------------------------------------------------------------*/
bool sdhc_detect(void)
{
	uint32_t value = 0;
	if (sdhcCdPin[0].config.pullSelect == kPortPullUp) /* pull up */
	{
            value = GPIO_DRV_ReadPinInput(sdhcCdPin[0].pinName);
            return (!value);
	}
	else /* pull down */
	{
            return (GPIO_DRV_ReadPinInput(sdhcCdPin[0].pinName));
	}
}

//interrupt handler
void SDHC_IRQHandler(void)
{
    SDHC_DRV_DoIrq(0);
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
