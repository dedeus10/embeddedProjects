/*
     ###########   WeeVee Electronic Solutions ################

      Projetos em Desenvolvimento
---------------------------------------------------------------------------------------------
Overview:
  $Placa dedicada para testes v1.0

      Data:   06/03/19
     Atualização: -


   ---- Autor: Luis Felipe de Deus
   
*/

//Include the microcontroller
#include <16F628A.h>

//Configurations of Fuses
#use delay(clock = 4000000, internal) //Internal osc with 4MHz
#FUSES NOWDT      //No Watch Dog Timer
#FUSES INTRC_IO   //Internal RC Osc, no CLKOUT
#FUSES NOCPD      //No EE protection
#FUSES PROTECT    //Code not protected from reading
#FUSES NOMCLR     //Master Clear pin used for I/O
#FUSES PUT        //Power Up Timer
#FUSES NOBROWNOUT //No brownout reset

//Serial communication protocol rs232
#use rs232(baud = 9600, xmit = pin_b2, rcv = pin_b1, TIMEOUT = 500)

//Definitions of pins MCU
#define r_Sys_F pin_b5    //Acionamento do Sistema Fase (Relé Rede)
#define r_Sys_N pin_b4    //Acionamento do Sistema Neutro (Relé Rede)
#define start_PC pin_a6   //Start no Power do PC (Optoacoplador 4N25)
#define btn_status pin_b0 //Chave ON/OFF

//Código Infravermelho do comando alvo
#define IR_CODE_AOC 0xBD807F       //POWER BUTTON TV AOC
#define IR_CODE_SAMSUNG 0xE0E040BF //POWER BUTTON TV Samsung

#define send_reports       //Descomentar para habilitar envio de dados RS-232


/*---------------------- PROTOCOL NEC DE COMUNICAÇÃO IR ----------------------*/
void ir_send(unsigned int32 code)
{
  unsigned char i = 0;
  // Desativa interrupções para previnir reentrada
  disable_interrupts(GLOBAL);

  // Envia start of frame
  set_pwm1_duty(13);
  delay_us(9000);

  set_pwm1_duty(0);
  delay_us(4500);

  // Envio do codigo do power
  while (i < 32)
  {
#bit first = code .31

    // Transmite bit 1
    if (first)
    {
      set_pwm1_duty(13);
      delay_us(560);
      set_pwm1_duty(0);
      delay_us(1690);
    }

    // Transmite bit 0
    else
    {
      set_pwm1_duty(13);
      delay_us(560);
      set_pwm1_duty(0);
      delay_us(560);
    }

    code <<= 1;
    i++;
  }

  // Bit de parada
  set_pwm1_duty(13);
  delay_us(560);
  set_pwm1_duty(0);

  //Habilita interrupções 
  enable_interrupts(GLOBAL);
}

/*-------------------------------------- TIMER1 50 milisegundos -----------------------------------------*/
#int_timer1
void int_timer_1(void)
{
  set_timer1(15536); // Estouro a cada 50 ms
}
 

/*-------------------------------------- MAIN FUNCTION -----------------------------------------*/
#zero_ram
void main(void)
{

  setup_timer_1(T1_INTERNAL | T1_DIV_BY_1); //Enable timer1 (OSc internal for timer, and prescaler divided by one)
  set_timer1(15536);                        //Set overflow timer1 in 50ms
  enable_interrupts(INT_TIMER1);            //Enable interrupts for timer1

  setup_oscillator(OSC_4MHZ); //Configura oscilador para 4 MHz
  setup_ccp1(CCP_PWM);        //Configura CCP1 como saída PWM
  set_pwm1_duty(0);           //Seta PWM para 0

  setup_timer_2(T2_DIV_BY_1, 26, 1); //Seta timer2 sem prescaler, valor 26, sem poscaler
  enable_interrupts(GLOBAL);         //Enable global interrupts

  output_low(r_Sys_F);  //Set System how init status 0
  output_low(r_Sys_N);  //Set PC how init status 0
  output_low(start_PC); //Set Start PC how init status 0

#ifdef send_reports
  printf("------------Inicialização do Sistema Sync v1.0 ----------- \n\r");
#endif

  while (1)
  {   
      //Just send the data
      ir_send(IR_CODE_AOC);
  }
}
