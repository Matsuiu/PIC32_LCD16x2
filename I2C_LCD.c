/**********************************************************************************
 * I2C_LCD.c                                                                      *
 * MPLAB XC8 compiler LCD driver for I2C LCDs with HD44780 compliant controllers. *
 * https://simple-circuit.com/                                                    *
 *                                                                                *
 *********************************************************************************/


#include <stdint.h>
#include "i2c_lib.h"
#include "Delay.h"

#define I2C_BaudRate           100000
#define SCL_D                  TRISC3
#define SDA_D                  TRISC4
#define LCD_Address            0x4E // A2, A1, A0 = 111 (0 or 1)

#define LCD_BACKLIGHT          0x08
#define LCD_NOBACKLIGHT        0x00
#define LCD_FIRST_ROW          0x80
#define LCD_SECOND_ROW         0xC0
#define LCD_THIRD_ROW          0x94
#define LCD_FOURTH_ROW         0xD4
#define LCD_CLEAR              0x01
#define LCD_RETURN_HOME        0x02
#define LCD_ENTRY_MODE_SET     0x04
#define LCD_CURSOR_OFF         0x0C
#define LCD_UNDERLINE_ON       0x0E
#define LCD_BLINK_CURSOR_ON    0x0F
#define LCD_MOVE_CURSOR_LEFT   0x10
#define LCD_MOVE_CURSOR_RIGHT  0x14
#define LCD_TURN_ON            0x0C
#define LCD_TURN_OFF           0x08
#define LCD_SHIFT_LEFT         0x18
#define LCD_SHIFT_RIGHT        0x1E
#define LCD_TYPE               2       // 0 -> 5x7 | 1 -> 5x10 | 2 -> 2 lines


unsigned char RS, i2c_add, BackLight_State = LCD_BACKLIGHT;

//void I2C_Init(uint32_t i2c_clk_freq);
//void I2C_Start();
//void I2C_Stop();
//void I2C_Write(uint8_t i2c_data);
//void LCD_Write_Nibble(uint8_t n);
//void LCD_Cmd(uint8_t Command);
//void LCD_Goto(uint8_t col, uint8_t row);
//void LCD_PutC(char LCD_Char);
//void LCD_Print(char* LCD_Str);
//void LCD_Begin(uint8_t _i2c_addr);


/********************** I2C functions **************************/
//
//void I2C_Init(uint32_t i2c_clk_freq)
//{
//  SSPCON  = 0x28;  // configure MSSP module to work in I2C mode
//  SSPADD  = (_XTAL_FREQ/(4 * i2c_clk_freq)) - 1;  // set I2C clock frequency
//  SSPSTAT = 0;
//}
//
//void I2C_Start()
//{
//  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));  // wait for MSSP module to be free (not busy)
//  SEN = 1;  // initiate start condition
//}
//
//void I2C_Stop()
//{
//  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));  // wait for MSSP module to be free (not busy)
//  PEN = 1;  // initiate stop condition
//}
//
//void I2C_Write(uint8_t i2c_data)
//{
//  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));  // wait for MSSP module to be free (not busy)
//  SSPBUF = i2c_data;  // update buffer
//}
//
//void I2C_Repeated_Start()
//{
//  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));  // wait for MSSP module to be free (not busy)
//  RSEN = 1;  // initiate repeated start condition
//}
//
//uint8_t I2C_Read(uint8_t ack)
//{
//  uint8_t _data;
//  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));  // wait for MSSP module to be free (not busy)
//  RCEN = 1;
//  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));  // wait for MSSP module to be free (not busy)
//  _data = SSPBUF;  // read data from buffer
//  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));  // wait for MSSP module to be free (not busy)
//  // send acknowledge pulse ? (depends on ack, if 1 send, otherwise don't send)
//  ACKDT = !ack;
//  ACKEN = 1;
//  return _data;  // return data read
//}

/********************** end I2C functions **********************/


void IO_Expander_Write(unsigned char Data) 
{
  i2c_master_start();
  i2c_master_send(LCD_Address); //to write
  i2c_master_send(Data | BackLight_State ); //
  i2c_master_stop(); 
}

void LCD_Write_4Bit(unsigned char Nibble) 
{
  // Get The RS Value To LSB OF Data  
  Nibble |= RS;
  IO_Expander_Write(Nibble | 0x04);
  IO_Expander_Write(Nibble & 0xFB);
  delay_us(50);
}

void LCD_CMD(unsigned char CMD) 
{
  RS = 0; // Command Register Select
  LCD_Write_4Bit(CMD & 0xF0);
  LCD_Write_4Bit((CMD << 4) & 0xF0);
}

void LCD_Init(unsigned char I2C_Add) 
{
  i2c_add = I2C_Add;
  IO_Expander_Write(0x00);
  delay_ms(30);
  LCD_CMD(0x03);
  delay_ms(5);
  LCD_CMD(0x03);
  delay_ms(5);
  LCD_CMD(0x03);
  delay_ms(5);
  LCD_CMD(LCD_RETURN_HOME);
  delay_ms(5);
  LCD_CMD(0x20 | (LCD_TYPE << 2));
  delay_ms(50);
  LCD_CMD(LCD_TURN_ON);
  delay_ms(50);
  LCD_CMD(LCD_CLEAR);
  delay_ms(50);
  LCD_CMD(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  delay_ms(50);
}

void LCD_Write_Char(char Data)
{
  RS = 1;  // Data Register Select
  LCD_Write_4Bit(Data & 0xF0);
  LCD_Write_4Bit((Data << 4) & 0xF0);
}

void LCD_Write_String(char* Str)
{
    int i=0;
    for(i=0; Str[i]!='\0'; i++)
       LCD_Write_Char(Str[i]); 
}

void LCD_Set_Cursor(unsigned char ROW, unsigned char COL) 
{    
  switch(ROW) 
  {
    case 2:
      LCD_CMD(0xC0 + COL-1);
      break;
    case 3:
      LCD_CMD(0x94 + COL-1);
      break;
    case 4:
      LCD_CMD(0xD4 + COL-1);
      break;
    // Case 1  
    default:
      LCD_CMD(0x80 + COL-1);
  }
}

void Backlight() 
{
  BackLight_State = LCD_BACKLIGHT;
  IO_Expander_Write(0);
}

void noBacklight() 
{
  BackLight_State = LCD_NOBACKLIGHT;
  IO_Expander_Write(0);
}

void LCD_SL()
{
  LCD_CMD(0x18);
  delay_us(40);
}

void LCD_SR()
{
  LCD_CMD(0x1C);
  delay_us(40);
}

void LCD_Clear()
{
  LCD_CMD(0x01); 
  delay_us(40);
}