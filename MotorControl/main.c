#include <driverlib.h>

#define SINE_TABLE_SIZE 90
#define SINE_MAX 32767 // Maximum value for 16-bit signed integer

uint16_t pwm_period = (100/2); // [us] 10kHz Switching Frequency, for Up-Down Counter
uint16_t control_period = 1000; // [us] for Up Counter
const float ramp_rate = 1; //Frequency Ramp Rate
const float rated_freq = 45; // [Hz] Output RPM = 60*rated_freq

// Variables for PWM control
uint16_t phase_acc = 0;
uint16_t phase_step = 0;
float freq = 0;

// Quarter-symmetry sine table (0° to 90°)
const int16_t sine_table[SINE_TABLE_SIZE] = {
    0, 571, 1143, 1715, 2286, 2857, 3428, 3998, 4567, 5136,
    5704, 6271, 6838, 7404, 7969, 8533, 9096, 9659, 10220, 10880,
    11539, 12197, 12853, 13508, 14161, 14813, 15463, 16111, 16757, 17401,
    18043, 18683, 19321, 19957, 20590, 21221, 21849, 22474, 23097, 23716,
    24333, 24946, 25556, 26163, 26766, 27366, 27962, 28554, 29142, 29726,
    30305, 30881, 31452, 32019, 32580, 33138, 33690, 34238, 34781, 35319,
    35851, 36378, 36900, 37416, 37927, 38433, 38932, 39426, 39914, 40396,
    40872, 41342, 41806, 42264, 42716, 43161, 43600, 44033, 44459, 44878,
    45291, 45697, 46097, 46489, 46875, 47254, 47626, 47990, 48348, 48698,
};

// Function to get sine value from angle using quarter-symmetry
inline int16_t get_sine(uint16_t angle) {
    angle %= 360;
    if (angle < 90) return sine_table[angle];
    if (angle < 180) return sine_table[180 - angle];
    if (angle < 270) return -sine_table[angle - 180];
    return -sine_table[360 - angle];
}

// Initializes ADC for current sensor NOT CURRENTLY USED
/*
void initADC(void)
{
    ADC12_B_initParam adcParam = {0};
    adcParam.sampleHoldSignalSourceSelect = ADC12_B_SAMPLEHOLDSOURCE_SC;
    adcParam.clockSourceSelect = ADC12_B_CLOCKSOURCE_ADC12OSC;
    adcParam.clockSourceDivider = ADC12_B_CLOCKDIVIDER_1;
    adcParam.clockSourcePredivider = ADC12_B_CLOCKPREDIVIDER__1;
    adcParam.internalChannelMap = ADC12_B_NOINTCH;
    ADC12_B_init(ADC12_B_BASE, &adcParam);

    ADC12_B_enable(ADC12_B_BASE);

    ADC12_B_setupSamplingTimer(ADC12_B_BASE, ADC12_B_CYCLEHOLD_16_CYCLES, ADC12_B_CYCLEHOLD_4_CYCLES, ADC12_B_MULTIPLESAMPLESDISABLE);

    ADC12_B_configureMemoryParam memParam = {0};
    memParam.memoryBufferControlIndex = ADC12_B_MEMORY_0;
    memParam.inputSourceSelect = ADC12_B_INPUT_A12;
    memParam.refVoltageSourceSelect = ADC12_B_VREFPOS_INTBUF_VREFNEG_VSS;
    memParam.endOfSequence = ADC12_B_NOTENDOFSEQUENCE;
    memParam.differentialModeSelect = ADC12_B_DIFFERENTIAL_MODE_DISABLE;
    memParam.windowComparatorSelect = ADC12_B_WINDOW_COMPARATOR_DISABLE;
    ADC12_B_configureMemory(ADC12_B_BASE, &memParam);
}
*/

// Initializes Timer B for PWM signals
void initPWM(uint16_t timer_period)
{
    Timer_B_initUpDownModeParam pwmTimer = {0};
    pwmTimer.clockSource = TIMER_B_CLOCKSOURCE_SMCLK;
    pwmTimer.clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_1;
    pwmTimer.timerPeriod = timer_period;
    pwmTimer.timerInterruptEnable_TBIE = TIMER_B_TBIE_INTERRUPT_DISABLE;
    pwmTimer.captureCompareInterruptEnable_CCR0_CCIE = TIMER_B_CCIE_CCR0_INTERRUPT_DISABLE;
    pwmTimer.startTimer = false;
    pwmTimer.timerClear = TIMER_B_DO_CLEAR;
    Timer_B_initUpDownMode(TIMER_B0_BASE, &pwmTimer);
}

// Initializes PWM Compare Registers
void initPWMCompare(uint16_t compare_register, uint16_t outputMode, uint16_t duty_cycle)
{
    Timer_B_initCompareModeParam compareParam = {0};
    compareParam.compareRegister = compare_register;
    compareParam.compareInterruptEnable = TIMER_B_CAPTURECOMPARE_INTERRUPT_DISABLE;
    compareParam.compareOutputMode = outputMode;
    compareParam.compareValue = duty_cycle;
    Timer_B_initCompareMode(TIMER_B0_BASE, &compareParam);
}

// Initializes Timer A for control loop
void initControl(uint16_t timer_period)
{
    Timer_A_initUpDownModeParam controlTimer = {0};
    controlTimer.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    controlTimer.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    controlTimer.timerPeriod = timer_period;
    controlTimer.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    controlTimer.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE;
    controlTimer.startTimer = false;
    controlTimer.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_initUpDownMode(TIMER_A0_BASE, &controlTimer);
}

// ISR for frequency
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void)
{
    // Ramp up frequency
    if(freq < rated_freq)
    {
        freq += ramp_rate;
        phase_step = (uint16_t)(freq * 0.001 * 360); // 0.001s control period
    }   

    // Update phase accumulator
    phase_acc = (phase_acc + phase_step) % 360;    

    Timer_A_clearTimerInterrupt(TIMER_A0_BASE);
}

void main()
{
    // Current Measurements Variables
    /*
    uint16_t adc1_input = 0;
    float adc_conv = 0;
    float current_a = 0;
    */

    WDT_A_hold(WDT_A_BASE);

    // Set L/R Enable pins as outputs then high
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN0); 
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN1);
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN3);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN1);
    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN3);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2);

    // Set Pin P3.0 for ADC input (ADD MORE FOR OTHER PHASES)
    //GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0, GPIO_TERNARY_MODULE_FUNCTION);

    // Set Pins for PWM output
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION); //Phase A
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION); //Phase B
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION); //Phase C

    PMM_unlockLPM5();

    // Setup Voltage Reference for ADC to 2.5V *USE A VOLTAGE DIVIDER, SENSOR HAS 5V Output*
    Ref_A_setReferenceVoltage(REF_A_BASE, REF_A_VREF2_5V);
    Ref_A_enableReferenceVoltage(REF_A_BASE);

    __enable_interrupt();

    // initADC();

    // Initialize Control Timer
    initControl(control_period);

    // Initialize PWM Timer
    initPWM(pwm_period);

    // Set Compare Registers
    initPWMCompare(TIMER_B_CAPTURECOMPARE_REGISTER_4, TIMER_B_OUTPUTMODE_TOGGLE_RESET, 0);
    initPWMCompare(TIMER_B_CAPTURECOMPARE_REGISTER_5, TIMER_B_OUTPUTMODE_TOGGLE_RESET, 0);
    initPWMCompare(TIMER_B_CAPTURECOMPARE_REGISTER_6, TIMER_B_OUTPUTMODE_TOGGLE_RESET, 0);

    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_B_startCounter(TIMER_B0_BASE, TIMER_B_UPDOWN_MODE);

    while(1)
    {
        // Calculate phase angles
        uint16_t phase = phase_acc;
        uint16_t phase_b = (phase + 120) % 360;
        uint16_t phase_c = (phase + 240) % 360;

        // Update PWM duty cycles
        Timer_B_setCompareValue(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_4,
            ((int32_t)get_sine(phase) * 25) / SINE_MAX + 25);
        Timer_B_setCompareValue(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_5,
            ((int32_t)get_sine(phase_b) * 25) / SINE_MAX + 25);
        Timer_B_setCompareValue(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_6,
            ((int32_t)get_sine(phase_c) * 25) / SINE_MAX + 25);

        // ADC code
        /*
        ADC12_B_startConversion(ADC12_B_BASE, ADC12_B_MEMORY_0, ADC12_B_SINGLECHANNEL);
        adc1_input = ADC12_B_getResults(ADC12_B_BASE, ADC12_B_MEMORY_0);
        while(ADC12_B_isBusy(ADC12_B_BASE));
        adc_conv = (4095-adc1_input)*0.0002442*5; ADC_Output = (4095-ADC_input)/4095*5V
        //Voltage to Current Conversion
        current_a = (2.5-adc_conv)*15.1515151515; // I = (2.5 - output_voltage)/(66 V/mA)
        */
    }
}