#include <Keypad.h>

/******************************************************************************************************************************************
 * Programa réplica máquina del tiempo del Delorian. Cuenta con 6 display para la fecha, 3 barras de Neopixel conectados en paralelo; 4 botones
 * 3 ldes naranjas en combinación con la barra Neopixel; 3 leds rojo-verde-amarillo para seleccionar la fecha a cambiar y un teclado matricial.
 * El programa inicialmente ilumina las barras de neopixel, el botón negro cambia la forma en la que las barras se ilumina. Cuando se pulsa uno de
 * los otros botones de colores interrumpirá el neopixel y dará paso al cambio de fecha en el display
 * 
 * Autor:                   Kot
 * Version:                 1.0
 * IDE:                     Arduino 1.8.11
 * Fecha de creación:       02/10/2019
 * Última actualización:    --/--/----
 * 
 ******************************************************************************************************************************************/



#include "Adafruit_NeoPixel.h"
#include "TM1637Display.h"
#include "JC_Button.h"
#include <Keypad.h>


/**********************************************    VARIABLES    ***************************************************************************/

//pines
const int NEOPIXELPIN = 10;     //pin de datos de las barras NeoPixel en el pin digital 10
const int BLACKBUTTON = 21;     //boton negro en el pin digital 21
const int REDBUTTON = 52;       //boton negro en el pin digital 24
const int GREENBUTTON = 50;     //boton negro en el pin digital 23
const int YELLOWBUTTON = 48;    //boton negro en el pin digital 22




const int RedLed = 49;
const int GreenLed = 53;
const int YellowLed = 51;




        //DESTINATION TIME
const int CLK_MR = 22;    //clk mes rojo
const int DIO_MR = 24;    //dio mes rojo
const int CLK_YR = 31;    //clk año rojo
const int DIO_YR = 33;    //dio año rojo
        //PRESENT TIME
const int CLK_MG = 26;    //clk mes verde
const int DIO_MG = 28;    //dio mes verde
const int CLK_YG = 35;    //clk año verde
const int DIO_YG = 37;    //dio año verde
        //LAST TIME DEPARTED
const int CLK_MY = 30;    //clk mes amarillo
const int DIO_MY = 32;    //dio mes amarillo
const int CLK_YY = 39;    //clk año amarillo
const int DIO_YY = 41;    //dio año amarillo



//variables
const int NUM_LEDS = 8;

int modo = 1;                   //inicio de los modos disponibles para la barra de leds

volatile bool flag = false;              //bandera pasa salir de los bucles for del neoPixel. Volatile es para que el valor pueda ser cambiado dentro de una interrupción o al haber cambios en los puertos de entrada como sería un interruptor
boolean blackButtonFlag = false;
boolean RedButtonflag = false;
boolean GreenButtonflag = false;
boolean YellowButtonflag = false;


const byte ROWS = 4; //cuatro filas
const byte COLS = 3; //tres columnas
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //pines de las filas
byte colPins[COLS] = {8, 7, 6};    //pines de las columnas


char keyPressed;
int i; //for
int numR[4] = {1, 9, 8, 5};
int numG[4] = {2, 0, 1, 5};
int numY[4] = {1, 9, 5, 5};

int colorLeds[7][3] =
{
  {255,255,255},
  {255, 0,  0 },
  {0,  255, 0 },
  {0,  0,  255},
  {255,255, 0 },
  {255, 0, 255},
  {0, 255, 255},
};
int seleccionColor = 0;   //para recorrer el array
int R = colorLeds[seleccionColor][3];
int G = colorLeds[seleccionColor][3];
int B = colorLeds[seleccionColor][3];
bool colorChange = false; //para verificar que se ha pulsado 1s

//CREACIÓN DE NUEVOS OBJETOS
//barras de leds
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);  //El último parametro es para indicar que el led solo es de r,g,b y el flujo de bits es de 800khz para los leds WS2812
Button deb_Black(BLACKBUTTON);  //debounce time= 50; inner pull up enable; ir a la libreria para tocar estos valores


//displays
TM1637Display d_mesRojo(CLK_MR, DIO_MR);   //DESTINATION TIME
TM1637Display d_anioRojo(CLK_YR, DIO_YR);
TM1637Display d_mesVerde(CLK_MG, DIO_MG);   //PRESENT TIME
TM1637Display d_anioVerde(CLK_YG, DIO_YG);
TM1637Display d_mesAmarillo(CLK_MY, DIO_MY);   //LAST TIME DEPARTED
TM1637Display d_anioAmarillo(CLK_YY, DIO_YY);


//Teclado
Keypad keypad3x4 = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
/******************************************    DECLARACION DE FUNCIONES    ****************************************************************/

void interrupt_blackButton(void);
void interrupt_redButton(void);
void interrupt_greenButton(void);
void interrupt_yellowButton(void);


void displayMatrixRed(int[]);
void displayMatrixGreen(int[]);
void displayMatrixYellow(int[]);


/******************************************    CONFIGURACIÓN    ***************************************************************************/
void setup()
{
  Serial.begin(9600);

  deb_Black.begin();  
  pinMode(REDBUTTON,INPUT_PULLUP);
  pinMode(GREENBUTTON,INPUT_PULLUP);
  pinMode(YELLOWBUTTON,INPUT_PULLUP);

    pinMode (RedLed, OUTPUT);
    pinMode (GreenLed, OUTPUT);
    pinMode ( YellowLed , OUTPUT);
  
  
  pixels.begin();
  pixels.setBrightness(100);

        //DESTINATION TIME
  d_mesRojo.setBrightness(0x0a);
  d_mesRojo.showNumberDec(9999);
  d_anioRojo.setBrightness(0x0a);
  d_anioRojo.showNumberDec(1985);
        //PRESENT TIME
  d_mesVerde.setBrightness(0x0a);
  d_mesVerde.showNumberDec(9999);
  d_anioVerde.setBrightness(0x0a);
  d_anioVerde.showNumberDec(2015);
        //LAST TIME DEPARTED
  d_mesAmarillo.setBrightness(0x0a);
  d_mesAmarillo.showNumberDec(9999);
  d_anioAmarillo.setBrightness(0x0a);
  d_anioAmarillo.showNumberDec(1955);

        
  attachInterrupt(digitalPinToInterrupt(BLACKBUTTON), interrupt_blackButton, FALLING);       //corresponde a la interrupcion 0 del arduino; funcion ISR; el modo de que se interrumpa
}




/******************************************    PROGRAMA PRINCIPAL    *************************************************************************/
void loop()
{

  flag = false;
  pixels.neoPixelModo(modo);
  if(blackButtonFlag)
  {
    blackButtonFlag = false;
    modo++;
    if(modo > 5)
    {
      modo = 1;
      seleccionColor++;
      if(seleccionColor > 6)
        seleccionColor = 0;
      R = colorLeds[seleccionColor][0];
      G = colorLeds[seleccionColor][1];
      B = colorLeds[seleccionColor][2];
    }
  }


  /*DISPLAY ROJO*/
  while(RedButtonflag)
  {
    for(i=0;i<4;i++)
    {
      char keyPressed = keypad3x4.waitForKey();

      if(keyPressed != NO_KEY && keyPressed != '*' && keyPressed != '#')
      {
        numR[i] = keyPressed - '0';
        displayMatrixRed(numR);
      }
    }
    keyPressed == NO_KEY;
    keyPressed = keypad3x4.getKey();
    digitalWrite(RedLed, LOW);  // apaga el LED:
    RedButtonflag = false ; // termina la fecha establecida en rojo 
  }

  /*DISPLAY VERDE*/
  while(GreenButtonflag)
  {
    for(i=0;i<4;i++)
    {
      char keyPressed = keypad3x4.waitForKey();

      if(keyPressed != NO_KEY && keyPressed != '*' && keyPressed != '#')
      {
        numG[i] = keyPressed - '0';
        displayMatrixGreen(numG);
      }
    }
    keyPressed == NO_KEY;
    keyPressed = keypad3x4.getKey();
    digitalWrite(GreenLed, LOW);  // apaga el LED:
    GreenButtonflag = false ; // termina la fecha establecida en verde 
  }


  /*DISPLAY AMARILLO*/
  while(YellowButtonflag)
  {
    for(i=0;i<4;i++)
    {
      char keyPressed = keypad3x4.waitForKey();

      if(keyPressed != NO_KEY && keyPressed != '*' && keyPressed != '#')
      {
        numY[i] = keyPressed - '0';
        displayMatrixYellow(numY);
      }
    }
    keyPressed == NO_KEY;
    keyPressed = keypad3x4.getKey();
    digitalWrite(YellowLed, LOW);  // apaga el LED:
    YellowButtonflag = false ; // termina la fecha establecida en amarillo 
  }
}

/******************************************    FUNCIONES    ***************************************************************************/

void interrupt_blackButton()
{
    flag = true;
    pixels.fill(pixels.Color(0,0,0),0,NUM_LEDS); 
    pixels.show();
    blackButtonFlag = true;
}

void interrupt_redButton()
{
  flag = true;
  pixels.fill(pixels.Color(0,0,0),0,NUM_LEDS); 
  pixels.show();
  digitalWrite(RedLed, HIGH);
  RedButtonflag = true;
}

void interrupt_greenButton()
{
  flag = true;
  pixels.fill(pixels.Color(0,0,0),0,NUM_LEDS); 
  pixels.show();
  digitalWrite(GreenLed, HIGH);
  GreenButtonflag = true;
}

void interrupt_yellowButton()
{
  flag = true;
  pixels.fill(pixels.Color(0,0,0),0,NUM_LEDS); 
  pixels.show();
  digitalWrite(YellowLed, HIGH);
  YellowButtonflag = true;
}

void displayMatrixRed(int num[])
{
  d_anioRojo.showNumberDec(numR[3], true, 1, 3);
  d_anioRojo.showNumberDec(numR[2], true, 1, 2);
  d_anioRojo.showNumberDec(numR[1], true, 1, 1);
  d_anioRojo.showNumberDec(numR[0], true, 1, 0);
}

void displayMatrixGreen(int num[])
{
  d_anioVerde.showNumberDec(numG[3], true, 1, 3);
  d_anioVerde.showNumberDec(numG[2], true, 1, 2);
  d_anioVerde.showNumberDec(numG[1], true, 1, 1);
  d_anioVerde.showNumberDec(numG[0], true, 1, 0);
}

void displayMatrixYellow(int num[])
{
  d_anioAmarillo.showNumberDec(numY[3], true, 1, 3);
  d_anioAmarillo.showNumberDec(numY[2], true, 1, 2);
  d_anioAmarillo.showNumberDec(numY[1], true, 1, 1);
  d_anioAmarillo.showNumberDec(numY[0], true, 1, 0);
}
