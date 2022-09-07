#include <libmaple/libmaple.h>
#include <libmaple/usart.h>
#include <libmaple/dma.h>

//specs
//64mhz HSI
//TimerX PWM dac @ 2048pts 31.25kHz srate
//ADC PSC_6 = 10.666Mhz
//ADC smpr_71_5 = 126.98kHz = 3.9spl / srate
//ADC smpr_55_5 = 156.86kHz = 5.01spl / srate
//ADC smpr_41_5 = 197.53kHz = 6.32spl / srate
//ADC smpr_28_5 = 260.16kHz = 8.32spl / srate

//ADC 4096 theoretical dynamic range = 72dB
//pDAC 2048 theoretical dynamic range = 66dB
//r2rDAC potentially problematic updating two GPIO registers
//possibly DMA periph increment %2

//srate 64 block expiry in 2ms

int has_block;
int16_t adc_error;
int16_t abuf[64*4];
int16_t dbuf[64];

void adc_buf_irq(){
  has_block = dma_get_irq_cause(DMA1,DMA_CH1) == DMA_TRANSFER_COMPLETE ? 2 : 1;
}

void linkTimers(timer_dev* master, int master_ch, timer_dev* slave, int slave_ch){
  timer_resume(master);
  timer_resume(slave);
}

void setCodec(){
  adc_init(ADC1);
  adc_set_prescaler(ADC_PRE_PCLK2_DIV_6); //48MHz / 4 = 12MHz
  adc_set_sample_rate(ADC1, ADC_SMPR_239_5);
  adc_set_extsel(ADC1, ADC_ADC12_TIM2_CC2);
  adc_set_exttrig(ADC1, 2);
  ADC1->regs->CR2 |= ADC_CR2_DMA;
  adc_enable(ADC1);
//  delay(1);
//  adc_calibrate(ADC1);
//  delay(1);
//  adc_error = ADC1->regs->DR;

  //adc timer
  timer_pause(TIMER2);
  timer_set_prescaler(TIMER2, 0);
  timer_set_reload(TIMER2, (2048)-1); //srate*4 oversample adc 
  timer_set_compare(TIMER2, TIMER_CH2, 1);
//  timer_dma_enable_req(TIMER2, 2);

  //dac timer
  timer_pause(TIMER1);
  timer_set_prescaler(TIMER1, 0);
  timer_set_reload(TIMER1, 2048-1); //srate
  timer_set_compare(TIMER1, TIMER_CH1, 1024-1);
  //timer_dma_enable_req(TIMER1,TIMER_CH1);

  auto *ddata = &(TIMER1->regs).gen->CCR1;
  auto *adata = &(ADC1->regs->DR);
  
  dma_init(DMA1);

  //adc copy
  dma_disable(DMA1, DMA_CH1);
  dma_set_priority(DMA1, DMA_CH1, DMA_PRIORITY_VERY_HIGH);
  int m = DMA_CIRC_MODE | DMA_TRNS_CMPLT | DMA_HALF_TRNS | DMA_MINC_MODE;
  dma_setup_transfer(DMA1, DMA_CH1 , adata , DMA_SIZE_16BITS, abuf, DMA_SIZE_16BITS, m);
  dma_set_num_transfers(DMA1, DMA_CH1, 64);  
  dma_attach_interrupt(DMA1, DMA_CH1, adc_buf_irq);

  //dac copy
  dma_disable(DMA1, DMA_CH2);
  dma_set_priority(DMA1, DMA_CH2, DMA_PRIORITY_HIGH);
  m = DMA_CIRC_MODE | DMA_MINC_MODE | DMA_FROM_MEM;
  dma_setup_transfer(DMA1, DMA_CH2 , dbuf , DMA_SIZE_16BITS, ddata, DMA_SIZE_16BITS, m);
  dma_set_num_transfers(DMA1, DMA_CH2, 64);  

  dma_enable(DMA1, DMA_CH1);
  dma_enable(DMA1, DMA_CH2);
  timer_resume(TIMER1);
  timer_resume(TIMER2);

  pinMode(PA8, PWM);
  gpio_set_mode(GPIOA,0,GPIO_INPUT_ANALOG);
}


void setup(){
  pinMode(PC13,OUTPUT);

  for(int i=0; i<4; i++){
    gpio_toggle_bit(GPIOC,13);
    delay(100);
  }

  Serial.begin(9600);
  delay(1000);
  Serial.println("Hello");
  
  setCodec();

  for(int i=0; i<3; i++){
    gpio_toggle_bit(GPIOC,13);
    delay(100);
  }
}

void loop(){
  return;
  if(has_block){
//      auto s = has_block == 1 ? 0 : 32*4;
//      auto e = s+(32*4);
//      for(int i=s; i<e; i+4){
//        auto avr = abuf[i] + abuf[i+1] + abuf[i+2] + abuf[i+3];
//        avr >>= 2;
//        avr >>= 1;
//        if(avr > 2048) avr = 2048;
//        if(avr < 0) avr = 0;
//        dbuf[i>>2] = avr;
//      }

      auto s = has_block == 1 ? 0 : 32;
      auto e = s+(32);
      for(int i=s; i<e; i++){
        auto avr = abuf[i];
        avr >>= 1;
        if(avr > 2048) avr = 2048;
        if(avr < 0) avr = 0;
        dbuf[i] = avr;
      }
      has_block = 0;
      gpio_toggle_bit(GPIOC,13);
  }
}
