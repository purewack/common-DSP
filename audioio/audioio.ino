#include 

//specs
//64mhz HSI
//TimerX PWM dac @ 2048pts 31.25kHz srate
//ADC PSC_6 = 10.666Mhz
//ADC smpr_71_5 = 126.98kHz = 3.9spl / srate
//ADC smpr_55_5 = 156.86kHz = 5.01spl / srate
//ADC smpr_41_5 = 197.53kHz = 6.32spl / srate
//ADC smpr_28_5 = 260.16kHz = 8.32spl / srate

void setup(){
    //setup uart

    //setup adc
    adc_init(ADC1);
    adc_init(ADC2);


    //setup pdac
}

void loop(){

}