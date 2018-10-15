/*!
    The information contained herein is the exclusive property of SONiX and
    shall not be distributed, or disclosed in whole or in part without prior
    permission of SONiX.
    SONiX reserves the right to make changes without further notice to the
    product to improve reliability, function or design. SONiX does not assume
    any liability arising out of the application or use of any product or
    circuits described herein. All application information is advisor and does
    not from part of the specification.

    \file       LCD_SSD2828.c
    \brief      LCD SSD2828 Funcation
    \author     Pierce
    \version    0.3
    \date       2017/10/26
    \copyright  Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "TIMER.h"
#include "SPI.h"

//------------------------------------------------------------------------------
#if (LCD_PANEL == LCD_SSD2828_Y50019N00N)

static uint32_t *plLCD_SpiData;
//------------------------------------------------------------------------------
static uint16_t LCD_SSD2828_RegRd(uint8_t ubReg)
{
    *plLCD_SpiData = (LCD_SSD2828_CMD_WR | ubReg) << 8;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);

    *plLCD_SpiData = LCD_SSD2828_DAT_RD << 8;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);

    return (uint16_t)(*plLCD_SpiData >> 8);
}
//------------------------------------------------------------------------------
static void LCD_SSD2828_CmdWr(uint8_t ubReg)
{
    *plLCD_SpiData = (LCD_SSD2828_CMD_WR | ubReg) << 8;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);
}
//------------------------------------------------------------------------------
static void LCD_SSD2828_DatWr(uint16_t uwData)
{
    *plLCD_SpiData = (LCD_SSD2828_DAT_WR | uwData) << 8;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);
    TIMER_Delay_us(1);
}
//------------------------------------------------------------------------------
static void LCD_SSD2828_RegWr(uint8_t ubReg, uint16_t uwData)
{
    LCD_SSD2828_CmdWr(ubReg);
    LCD_SSD2828_DatWr(uwData);
}

static void LCD_SSD2828_DriverICRd(uint8_t ubReg, uint16_t uwNum)
{
    uint16_t uwData;
    uint16_t uwBuf[uwNum];
    uint8_t i = 0;
    LCD_SSD2828_RegWr(0xB7, 0x0382);
    uwData = LCD_SSD2828_RegRd(0xC6);
    if ((uwData & 0x0001) == 0) {
        LCD_SSD2828_RegWr(0xC1, uwNum); // Num
        LCD_SSD2828_RegWr(0xC0, 0x0001);
        LCD_SSD2828_RegWr(0xC4, 0x0001);
        LCD_SSD2828_RegWr(0xBC, 0x0001);
        LCD_SSD2828_RegWr(0xBF, (uint16_t)ubReg); // Reg
        TIMER_Delay_ms(20);
        while ((uwData = LCD_SSD2828_RegRd(0xC6))& 0x0001) {
            uwBuf[i] = LCD_SSD2828_RegRd(0xFF);
            printf("buf %x =%x,%x\n", i, uwBuf[i], uwData);
            i++;
        }
        printf("Rd Finish\n");
    } else {
        printf("Rd Err\n");
    }
}

// 0xFF - delay
// 0xFE - end
#if 1
static uint8_t panel0_init_regs[] = {
    3, 0xFF, 0x98, 0x81, 0x03,
//  0xFF, 5,
    1, 0x01, 0x00,
    1, 0x02, 0x00,
    1, 0x03, 0x56,
    1, 0x04, 0x13,
    1, 0x05, 0x00,
    1, 0x06, 0x06,
    1, 0x07, 0x01,
    1, 0x08, 0x00,
    1, 0x09, 0x30,
    1, 0x0A, 0x01,
    1, 0x0B, 0x00,
    1, 0x0C, 0x30,
    1, 0x0D, 0x01,
    1, 0x0E, 0x00,
    1, 0x0F, 0x1B,
    1, 0x10, 0x1B,
    1, 0x11, 0x00,
    1, 0x12, 0x00,
    1, 0x13, 0x00,
    1, 0x14, 0x00,
    1, 0x15, 0x08,
    1, 0x16, 0x08,
    1, 0x17, 0x00,
    1, 0x18, 0x08,
    1, 0x19, 0x00,
    1, 0x1A, 0x00,
    1, 0x1B, 0x00,
    1, 0x1C, 0x00,
    1, 0x1D, 0x00,
    1, 0x1E, 0x40,
    1, 0x1F, 0xC0,
    1, 0x20, 0x02,
    1, 0x21, 0x05,
    1, 0x22, 0x02,
    1, 0x23, 0x00,
    1, 0x24, 0x87,
    1, 0x25, 0x86,
    1, 0x26, 0x00,
    1, 0x27, 0x00,
    1, 0x28, 0x3B,
    1, 0x29, 0x03,
    1, 0x2A, 0x00,
    1, 0x2B, 0x00,
    1, 0x2C, 0x00,
    1, 0x2D, 0x00,
    1, 0x2E, 0x00,
    1, 0x2F, 0x00,
    1, 0x30, 0x00,
    1, 0x31, 0x00,
    1, 0x32, 0x00,
    1, 0x33, 0x00,
    1, 0x34, 0x00,
    1, 0x35, 0x00,
    1, 0x36, 0x00,
    1, 0x37, 0x00,
    1, 0x38, 0x00,
    1, 0x39, 0x35,
    1, 0x3A, 0x01,
    1, 0x3B, 0x40,
    1, 0x3C, 0x00,
    1, 0x3D, 0x01,
    1, 0x3E, 0x00,
    1, 0x3F, 0x00,
    1, 0x40, 0x35,
    1, 0x41, 0x88,
    1, 0x42, 0x00,
    1, 0x43, 0x00,
    1, 0x44, 0x1F,
    1, 0x50, 0x01,
    1, 0x51, 0x23,
    1, 0x52, 0x45,
    1, 0x53, 0x67,
    1, 0x54, 0x89,
    1, 0x55, 0xAB,
    1, 0x56, 0x01,
    1, 0x57, 0x23,
    1, 0x58, 0x45,
    1, 0x59, 0x67,
    1, 0x5A, 0x89,
    1, 0x5B, 0xAB,
    1, 0x5C, 0xCD,
    1, 0x5D, 0xEF,
    1, 0x5E, 0x13,
    1, 0x5F, 0x06,
    1, 0x60, 0x0C,
    1, 0x61, 0x0D,
    1, 0x62, 0x0E,
    1, 0x63, 0x0F,
    1, 0x64, 0x02,
    1, 0x65, 0x02,
    1, 0x66, 0x02,
    1, 0x67, 0x02,
    1, 0x68, 0x02,
    1, 0x69, 0x02,
    1, 0x6A, 0x02,
    1, 0x6B, 0x02,
    1, 0x6C, 0x02,
    1, 0x6D, 0x02,
    1, 0x6E, 0x05,
    1, 0x6F, 0x05,
    1, 0x70, 0x05,
    1, 0x71, 0x02,
    1, 0x72, 0x01,
    1, 0x73, 0x00,
    1, 0x74, 0x08,
    1, 0x75, 0x08,
    1, 0x76, 0x0C,
    1, 0x77, 0x0D,
    1, 0x78, 0x0E,
    1, 0x79, 0x0F,
    1, 0x7A, 0x02,
    1, 0x7B, 0x02,
    1, 0x7C, 0x02,
    1, 0x7D, 0x02,
    1, 0x7E, 0x02,
    1, 0x7F, 0x02,
    1, 0x80, 0x02,
    1, 0x81, 0x02,
    1, 0x82, 0x02,
    1, 0x83, 0x02,
    1, 0x84, 0x05,
    1, 0x85, 0x05,
    1, 0x86, 0x05,
    1, 0x87, 0x02,
    1, 0x88, 0x01,
    1, 0x89, 0x00,
    1, 0x8A, 0x06,
    3, 0xFF, 0x98, 0x81, 0x04,
    1, 0x68, 0xDB,
    1, 0x6D, 0x08,
    1, 0x70, 0x00,
    1, 0x71, 0x00,
    1, 0x66, 0xFE,
   
    1, 0x82, 0x16,
    1, 0x84, 0x16,
    1, 0x85, 0x0F,
    1, 0x32, 0xAC,
    1, 0x8C, 0x80,
    1, 0x3C, 0xF5,
    
    1, 0xB5, 0x02,
    1, 0x31, 0x25,
    1, 0x88, 0x33,
    3, 0xFF, 0x98, 0x81, 0x01,
    1, 0x22, 0x0A,
    1, 0x31, 0x00,
    1, 0x50, 0x73,
    1, 0x51, 0x73,
    1, 0x53, 0x8A,
    1, 0x55, 0x8A,
    1, 0x60, 0x1C,
    1, 0x61, 0x00,
    1, 0x62, 0x0D,
    1, 0x63, 0x00,
    1, 0xA0, 0x00,
    1, 0xA1, 0x14,
    1, 0xA2, 0x27,
    1, 0xA3, 0x12,
    1, 0xA4, 0x16,
    1, 0xA5, 0x2A,
    1, 0xA6, 0x1E,
    1, 0xA7, 0x1E,
    1, 0xA8, 0x99,
    1, 0xA9, 0x1B,
    1, 0xAA, 0x22,
    1, 0xAB, 0x87,
    1, 0xAC, 0x19,
    1, 0xAD, 0x17,
    1, 0xAE, 0x4B,
    1, 0xAF, 0x1E,
    1, 0xB0, 0x26,
    1, 0xB1, 0x51,
    1, 0xB2, 0x63,
    1, 0xB3, 0x39,
    1, 0xC0, 0x00,
    1, 0xC1, 0x35,
    1, 0xC2, 0x44,
    1, 0xC3, 0x15,
    1, 0xC4, 0x1A,
    1, 0xC5, 0x2B,
    1, 0xC6, 0x1D,
    1, 0xC7, 0x20,
    1, 0xC8, 0xAE,
    1, 0xC9, 0x23,
    1, 0xCA, 0x30,
    1, 0xCB, 0xA4,
    1, 0xCC, 0x1F,
    1, 0xCD, 0x1E,
    1, 0xCE, 0x54,
    1, 0xCF, 0x28,
    1, 0xD0, 0x2B,
    1, 0xD1, 0x66,
    1, 0xD2, 0x70,
    1, 0xD3, 0x39,
    3, 0xFF, 0x98, 0x81, 0x00,
    0, 0x35,
    0, 0x11,
    0xff, 120,
    1, 0x36, 0x03, // vflip & hflip
    1, 0xCC, 0x05,
    0xff, 1,
    0, 0x29,
//  0xff, 1,
    0xfe,
};
#endif

#if 1 // ÓîË³ SDA001-IN050HD
static uint8_t panel1_init_regs[] = {
    // Set EXTC
    3,  0xB9, 0xFF, 0x83, 0x94,

    // Set MIPI
    6,  0xBA, 0x63, 0x03, 0x68, 0x6B, 0xB2, 0xC0,

    // Set Power
    10, 0xB1, 0x48, 0x12, 0x72, 0x09, 0x31, 0x54, 0x71, 0x31, 0x50, 0x34,

    // Set Display
    6,  0xB2, 0x00, 0x80, 0x64, 0x06, 0x08, 0x2F,

    // Set CYC
    20, 0xB4, 0x74, 0x76, 0x24, 0x7C, 0x5A, 0x5B, 0x01, 0x0C, 0x86, 0x75,
              0x00, 0x3F, 0x74, 0x76, 0x24, 0x7C, 0x5A, 0x5B, 0x01, 0x0C,

    // Set Gamma
    58, 0xE0, 0x00, 0x05, 0x10, 0x17, 0x19, 0x1D, 0x21, 0x20, 0x43, 0x55,
              0x69, 0x6B, 0x78, 0x8C, 0x96, 0x9C, 0xAB, 0xAF, 0xAB, 0xBA,
              0xC8, 0x63, 0x61, 0x66, 0x6C, 0x6F, 0x74, 0x7F, 0x7F, 0x00,
              0x05, 0x0F, 0x16, 0x19, 0x1D, 0x21, 0x20, 0x43, 0x55, 0x69,
              0x6B, 0x78, 0x8C, 0x96, 0x9C, 0xAB, 0xAF, 0xAB, 0xBA, 0xC8,
              0x63, 0x61, 0x66, 0x6C, 0x6F, 0x74, 0x7F, 0x7F,

    // Set D3
    33, 0xD3, 0x00, 0x00, 0x07, 0x0F, 0x40, 0x07, 0x0A, 0x0A, 0x32, 0x10,
              0x01, 0x00, 0x01, 0x52, 0x15, 0x07, 0x05, 0x07, 0x32, 0x10,
              0x00, 0x00, 0x00, 0x67, 0x44, 0x05, 0x05, 0x37, 0x0E, 0x0E,
              0x27, 0x06, 0x40,

    // Set GIP
    44, 0xD5, 0x20, 0x21, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
              0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
              0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
              0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x19, 0x19,
              0x18, 0x18, 0x24, 0x25,

    // Set D6
    44, 0xD6, 0x25, 0x24, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
              0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
              0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
              0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
              0x19, 0x19, 0x21, 0x20,

    // Set Panel
    1,  0xCC, 0x0b,

    // Set C0
    2,  0xC0, 0x1F, 0x31,

    // Set D4
    1,  0xD4, 0x02,
    1,  0xBD, 0x01,

    // Set GAS
    1,  0xB1, 0x00,

    // Set BD
    1,  0xBD, 0x00,

    // Set Power Option HX5186 Mode
    7,  0xBF, 0x40, 0x81, 0x50, 0x00, 0x1A, 0xFC, 0x01,

    // Set ECO
    1,  0xC6, 0xEF,

    0,  0x11,
    0xFF, 120,
    1,  0x36, 0x03, // vflip & hflip
    0,  0x29,
    0xFF, 20,

    0xFE,
};
#endif

static void lcd_panel_init(uint8_t *regs)
{
    while (*regs != 0xfe) {
        if (regs[0] == 0xff) {
            TIMER_Delay_ms(regs[1]);
            regs += 2;
        } else {
            uint16_t len  = *regs++;
            uint8_t  cmd  = *regs++;
            uint16_t temp = cmd;
            LCD_SSD2828_RegWr(0xBC, len + 1);
            if (len > 0) {
                temp |= ((uint16_t)*regs++ << 8);
                len--;
            }
            LCD_SSD2828_RegWr(0xBF,temp);
            while (len > 0) {
                temp = *regs++; len--;
                if (len > 0) {
                    temp |= ((uint16_t)*regs++ << 8);
                    len--;
                }
                LCD_SSD2828_DatWr(temp);
            }
        }
    }
}

bool bLCD_MIPI_SSD2828_Init(void)
{
    uint16_t    uwId;
    SPI_Setup_t spi_setup;

    printd(DBG_Debug3Lvl,"MIPI SSD2828 Init\n");

    spi_setup.ubSPI_CPOL = 0;
    spi_setup.ubSPI_CPHA = 0;
    spi_setup.tSPI_Mode  = SPI_MASTER;
    spi_setup.uwClkDiv   = ((float)160 / GLB->APBC_RATE) + 0.99 - 1;    // SPI Clock <= 1.5MHz

    plLCD_SpiData = osUncachedMalloc(4);
    if (plLCD_SpiData == NULL) {
        printd(DBG_CriticalLvl, "LCD: osUncachedMalloc fail!!\n");
        while(1);
    }
    SPI_Init(&spi_setup);
	TIMER_Delay_ms(20);
    if (0x2828 != (uwId = LCD_SSD2828_RegRd(0xB0))) {
        printd(DBG_ErrorLvl, "LCD: SSD2828 Fail %X\n", uwId);
        return false;
    }
    LCD_SSD2828_RegWr(0xB1, 0x0418);   // VSA = 2,HSA = 2
    LCD_SSD2828_RegWr(0xB2, 0x1030);   // VBP = 14,VBP = 42
    LCD_SSD2828_RegWr(0xB3, 0x0fc8);   // VFP = 16,HFP = 44
    LCD_SSD2828_RegWr(0xB4, 0x02D0);   // HACT = 720
    LCD_SSD2828_RegWr(0xB5, 0x0500);   // VACT = 1280
    LCD_SSD2828_RegWr(0xB6, 0x003b);   // Vsync Pulse is active low,
                                       // Hsync Pulse is active low,Data is launch at falling edge,SSD2828 latch data at rising edge
                                       // Video with blanking packet.
                                       // Non video data will be transmitted during any BLLP period.
                                       // Non video data will be transmitted using HS mode.
                                       // LP mode will be used during BLLP period.
                                       // The clock lane enters LP mode when there is no data to transmit.
                                       // Burst mode
                                       // 24bpp
    LCD_SSD2828_RegWr(0xB8, 0x0000);
    LCD_SSD2828_RegWr(0xB9, 0x0000);   // Divide by 1,Enable Sys_clk output,PLL power down
    LCD_SSD2828_RegWr(0xBA, 0xc02c);   // 10-251<Fout<500,MS=1,NS=22
    LCD_SSD2828_RegWr(0xBB, 0x0006);   // LP mode clock, Divide by 6
    LCD_SSD2828_RegWr(0xD5, 0x1860);
    LCD_SSD2828_RegWr(0xC9, 0x1604);   // HS Zero Delay = HZD * nibble_clk ; HS Prepare Delay =  4 nibble_clk + HPD * nibble_clk
    LCD_SSD2828_RegWr(0xCA, 0x2303);   // CLK Zero Delay = CZD * nibble_clk ; CLK Prepare Delay =  3 nibble_clk + CPD * nibble_clk
    LCD_SSD2828_RegWr(0xCB, 0x0626);   // CLK Pre Delay = CPED * nibble_clk + 0-1 * lp_clk(min,max) ; CLK Post Delay = CPTD * nibble_clk
    LCD_SSD2828_RegWr(0xCC, 0x0a0c);   // CLK Trail Delay = CTD * nibble_clk ; HS Trail Delay = HTD * nibble_clk
    LCD_SSD2828_RegWr(0xDE, 0x0003);   // 4 lane mode
    LCD_SSD2828_RegWr(0xB9, 0x0001);   // Divide by 1,Enable Sys_clk output,PLL enable
    LCD_SSD2828_RegWr(0xD6, 0x0005);   // RGB,R is in the higher portion of the pixel.
    LCD_SSD2828_RegWr(0xB8, 0x0000);

    //! Driver IC Initial Code
    TIMER_Delay_ms(5);

    LCD_SSD2828_RegWr(0xB7,0x0342); //!< TXD[11]: Transmit on
                                    //!< LPE[10]: Short Packet
                                    //!< EOT[9]: Send EOT Packet at the end of HS transmission
                                    //!< ECD[8]: Disable ECC CRC Check
                                    //!< REN[7]: Write operation
                                    //!< DCS[6]: Generic packet
                                    //!< CSS[5]: The clock source is tx_clk
                                    //!< HCLK[4]: HS clock is enabled
                                    //!< VEN[3]: Video mode is disabled
                                    //!< SLP[2]: Sleep mode is disabled
                                    //!< CKE[1]: Clock Lane Enable
                                    //!< HS[0]: LP mode

    // lcd pannel init
#if SCREEN_RZW 
    lcd_panel_init(panel0_init_regs);  	//current
#else
    lcd_panel_init(panel1_init_regs);	//ÓîË³
#endif
    return true;
}

//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828_Sleep(void)
{
    //! mipi_0data(0x10);
    LCD_SSD2828_RegWr(0xBC,0x0001);
    LCD_SSD2828_RegWr(0xBF,0x0010);
    LCD_SSD2828_RegWr(0xB7,0x0342); //!< TXD[11]: Transmit on
                                    //!< LPE[10]: Short Packet
                                    //!< EOT[9]: Send EOT Packet at the end of HS transmission
                                    //!< ECD[8]: Disable ECC CRC Check
                                    //!< REN[7]: Write operation
                                    //!< DCS[6]: Generic packet
                                    //!< CSS[5]: The clock source is tx_clk
                                    //!< HCLK[4]: HS clock is enabled
                                    //!< VEN[3]: Video mode is disabled
                                    //!< SLP[2]: Sleep mode is disabled
                                    //!< CKE[1]: Clock Lane Enable
                                    //!< HS[0]: LP mode
}

//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828_Wakeup(void)
{
    //! mipi_0data(0x11);
    LCD_SSD2828_RegWr(0xBC,0x0001);
    LCD_SSD2828_RegWr(0xBF,0x0011);
    LCD_MIPI_SSD2828_Start();
}

//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828_Start(void)
{
    LCD_SSD2828_RegWr(0xB7,0x0349); //!< TXD[11]: Transmit on
                                    //!< LPE[10]: Short Packet
                                    //!< EOT[9]: Send EOT Packet at the end of HS transmission
                                    //!< ECD[8]: Disable ECC CRC Check
                                    //!< REN[7]: Write operation
                                    //!< DCS[6]: DCS packet
                                    //!< CSS[5]: The clock source is tx_clk
                                    //!< HCLK[4]: HS clock is enabled
                                    //!< VEN[3]: Video mode is enabled
                                    //!< SLP[2]: Sleep mode is disabled
                                    //!< CKE[1]: Clock Lane Enable
                                    //!< HS[0]: HS mode
}

//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828(void)
{
    //! MIPI SSD2828
    printd(DBG_Debug3Lvl, "LCD MIPI SSD2828\n");
    printd(DBG_Debug3Lvl, "DE Mode\n");
    printd(DBG_Debug3Lvl, "Screen Size 720 x 1280\n");

    //! LCD Setting
    LCD->LCD_MODE       = LCD_DE;
    LCD->SEL_TV         = 0;

    LCD->LCD_HO_SIZE    = 720;
    LCD->LCD_VO_SIZE    = 1280;
    //! Timing
    LCD->LCD_HT_DM_SIZE = 200;
    LCD->LCD_VT_DM_SIZE = 15;
    LCD->LCD_HT_START   = 48;
    LCD->LCD_VT_START   = 16;

    LCD->LCD_HS_WIDTH   = 24;
    LCD->LCD_VS_WIDTH   = 4;

    LCD->LCD_RGB_REVERSE= 0;
    LCD->LCD_EVEN_RGB   = 1;
    LCD->LCD_ODD_RGB    = 1;
    LCD->LCD_HSYNC_HIGH = 0;
    LCD->LCD_VSYNC_HIGH = 0;
    LCD->LCD_DE_HIGH    = 1;
    LCD->LCD_PCK_RIS    = 1;
}

void LCD_PixelPllSetting(void)
{
    float   fPixelClock;
    uint8_t ubFps = 60;
    float   fFVCO;

    //! Calculation Pixel Clock
    switch (LCD->LCD_MODE)
    {
    case LCD_AU_UPS051_8:
    case LCD_AU_UPS051_6:
    case LCD_DE:
        fPixelClock = ((float)ubFps) * (LCD->LCD_HO_SIZE + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
                            (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
        break;
    case LCD_RGB_DUMMY:
        fPixelClock = ((float)ubFps) * ((LCD->LCD_HO_SIZE << 2) + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
                            (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
        break;
    case LCD_AU:
        fPixelClock = ((float)ubFps) * (LCD->LCD_HO_SIZE * 3 + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
                            (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
        break;
    case LCD_YUV422:
        fPixelClock = ((float)ubFps) * ((LCD->LCD_HO_SIZE << 1) + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
                            (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
        break;
    case LCD_BT656_BT601:
        fPixelClock = ((float)ubFps) * LCD->LCD_VO_SIZE * ((LCD->LCD_HO_SIZE << 1) +
                            LCD->LCD_HT_START + (LCD->LCD_HS_WIDTH << 1) + 8) / 1000000;
        break;
    default:
        return;
    }

    printd(DBG_InfoLvl, "LCD Pixel Clock = %f MHz\n", fPixelClock);

    LCD->LCD_PCK_SPEED = 2; // 2;
    GLB->LCDPLL_CK_SEL = 2;
    //! LCD PLL
    if (!GLB->LCDPLL_PD_N) {
        GLB->LCDPLL_LDO_EN = 1;
        GLB->LCDPLL_PD_N   = 1;
        TIMER_Delay_us(400);
        GLB->LCDPLL_SDM_EN = 1;
        TIMER_Delay_us(1);
    }

    GLB->LCDPLL_INT = 12;
    GLB->LCDPLL_FRA = 0xb0c34;
    TIMER_Delay_us(1);
    GLB->LCDPLL_INT_FRA_VLD = 1;
    TIMER_Delay_us(1);

    //! LCD Controller rate
    GLB->LCD_RATE = 1;

    if (GLB->LCDPLL_CK_SEL == 0) {
        fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 6;
    } else if (GLB->LCDPLL_CK_SEL == 1) {
        fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 2;
    } else {
        fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 1;
    }

    if ((fFVCO < 148.3) || (fFVCO > 165)) { //range:148.3M ~ 165M
        printd(DBG_ErrorLvl, "fFVCO=%f MHz,Change PLL pls!\n", fFVCO);
        while(1);
    }
}
#endif
