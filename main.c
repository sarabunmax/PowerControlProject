/*
 * File:   main.c
 * Author: sarab
 *
 * Created on 14 ??????? 2024, 13:54
 */


#include <xc.h>
#include "config.h"

// структура для зберігання параметрів режиму потужності двигуна //
struct regim
{
    // Значення стартового числа таймеру для затримки імпульсу на OUT1/OUT2 //
    uint8_t TMR1L_OUT;
    uint8_t TMR1H_OUT;
    
    // стан світлодіодів в кожному з режимів  //
    uint8_t led1_val : 1;
    uint8_t led2_val : 1;
    uint8_t led3_val : 1 ;
    uint8_t led4_val : 1;
    uint8_t led5_val : 1;
};

// Масив параметрів для кожного з 5 режимів роботи вигуна //
struct regim regims[] = 

{
    {51974, 51974 >> 8, 0, 1, 1, 1, 1},
    {51774, 51774 >> 8, 0, 0, 1, 1, 1},
    {53374, 53374 >> 8, 0, 0, 0, 1, 1},
    {54974, 54974 >> 8, 0, 0, 0, 0, 1},
    {56574, 56574 >> 8, 0, 0, 0, 0, 0}
};



// Структура для збереження прапорів
struct
{
    uint8_t POWER_FLAG : 1;   // Прапор стану пристрою (0 - увімкнено / 1- вимкнено)
    uint8_t BUTTON_FLAG : 2;  // Прапор для детекції відпускання кнопки PLUS / MINUS
    uint8_t FILTER_FLAG : 1;  // Прапор стану аварійної зупинки
    volatile uint8_t SMOOTH_START_FLAG : 1;  // Прапор для стану плавного пуску двигуна
    volatile uint8_t TMR_FLAG : 2;           // Прапор для таймеру (реалізація часових відрізків згідно режиму)
}flags;



void sysclk_init(uint8_t init_flag); // Налаштування частоти роботи контролера
void pins_init(void);          // Налаштування входів/виходів
void tim_init(void);           // Налаштування режиму роботи таймера
void adc_init(void);           // Налаштування режиму роботи АЦП
void __interrupt() isr(void);  // Обробка переривань

void zero_detect(void);            // Функція детекції перетину нуля
void blink_leds(void);             // Функція моргання світлодіодами LED1-LED5
void set_regim(struct regim rgm);  // Функція встановлення обраного режиму потужності двигуна

 // Стартове число таймера для затримки 20 мкрс //
uint8_t TMR1L_20us = 65494;
uint8_t TMR1H_20us = 65494 >> 8;

 // Стартове число таймера для затримки 900 мс (блимання LED у вимкненому стані) //
uint8_t TMR1L_900ms = 31068;
uint8_t TMR1H_900ms = 31068 >> 8;

// Стартове число таймера для затримки 100 мс (блимання LED у вимкненому стані) //
uint8_t TMR1L_100ms = 53034;
uint8_t TMR1H_100ms = 53034 >> 8;

// Стартове число таймера для плавного пуску двигуна //
uint8_t TMR1L_smooth = 0;
uint8_t TMR1H_smooth = 0;

uint8_t current_regim = 0;  // Значення поточного режиму потужності двигуна

void main(void) 
{
    sysclk_init(0); 
    pins_init();    
    tim_init();
    adc_init();
    blink_leds();
    
    
    flags.BUTTON_FLAG = 0;
    flags.FILTER_FLAG = 0;
    flags.POWER_FLAG = 0;
    flags.SMOOTH_START_FLAG = 0;
    flags.TMR_FLAG = 0;
    
    while(1) 
    {
        if(flags.POWER_FLAG)
        {LED_P = 1;}
        if(!BUTTON_POWER && !flags.POWER_FLAG)  // Якщо кнопка POWER натиснута і пристрій вимкнено
        {
            LED_P = 1;
            flags.SMOOTH_START_FLAG = 1;
            if(!BUTTON_PLUS)  // Якщо кнопка PLUS натиснута
            {flags.BUTTON_FLAG = 1;}// Фіксуємо натискання кнопки PLUS та POWER разом
                
            else if(!BUTTON_MINUS) // Якщо кнопка PLUS натиснута
            {flags.BUTTON_FLAG = 2;}     // Фіксуємо натискання кнопки PLUS та POWER разом
                
            else
            {
                sysclk_init(0);
                flags.POWER_FLAG = 1;        // Пристрій увімкнено
                switch(flags.BUTTON_FLAG)
                {
                    case 0:
                        current_regim = 2;     // Встановлення значення поточного режиму (№3)
                        break;
                            
                    case 1:
                        flags.BUTTON_FLAG = 0;    // Скидаємо прапор
                        current_regim = 4;  // Встановлення значення поточного режиму (№5)
                        break;
                            
                    case 2:
                        flags.BUTTON_FLAG = 0;    // Скидаємо прапор
                        current_regim = 0;  // Встановлення значення поточного режиму (№1)
                        break;
                }
                zero_detect();
            }
        }
        
       
        else if(!BUTTON_POWER && flags.POWER_FLAG)  // Якщо кнопка POWER натиснута і пристрій увімкнено
        {
            if(!BUTTON_FILTER)  // Якщо аварійна зупинка
            {flags.SMOOTH_START_FLAG = 1;}
                
            else
            {
                if(!BUTTON_PLUS)  // Якщо кнопка PLUS натиснута
                {flags.BUTTON_FLAG = 1;}// Фіксуємо натискання кнопки PLUS та POWER разом
                
                else if(!BUTTON_MINUS) // Якщо кнопка PLUS натиснута
                {flags.BUTTON_FLAG = 2;}     // Фіксуємо натискання кнопки PLUS та POWER разом
                
                 else
                {
                    switch(flags.BUTTON_FLAG)
                    {              
                        case 1:
                            flags.BUTTON_FLAG = 0;    // Скидаємо прапор
                            if(current_regim < 4)  // Перевірка на вихід з діапазону режимів(1 - 5)
                            {
                                current_regim ++;  // Збільшення значення поточного режиму
                            }
                            break;
                            
                        case 2:
                            flags.BUTTON_FLAG = 0;    // Скидаємо прапор
                            if(current_regim > 0)  // Перевірка на вихід з діапазону режимів(1 - 5)
                            {
                                current_regim --;  // Зменшення значення поточного режиму
                            }
                            break;
                    }
                }
                set_regim(regims[current_regim]);
            }
            
            zero_detect();  // Перевіряємо на перетин "0"-точки
        }
        
        else if(BUTTON_POWER && flags.POWER_FLAG)  // Якщо кнопка POWER вимкнена і пристрій увімкнено
        {
            // Гасимо світлодіоди LED1 - LED5 //
            LED1 = 1;
            LED2 = 1;
            LED3 = 1;
            LED4 = 1;
            LED5 = 1;
            
            flags.POWER_FLAG = 0;  // Пристрій вимкнено
            sysclk_init(1);  // Зниження частоти контролера до 500 кГц
            flags.TMR_FLAG = 0;    // Початковий відрізок періоду таймера
            TMR1H = 0;       // Встановлюємо стартове число
            TMR1L = 0;
            T1CONbits.TMR1ON = 1;  // Запускаємо таймер
        }
        
    }
}


// Обробник переривань //
void __interrupt() isr(void)
{
    if(TMR1IE && TMR1IF)  // Є переривання від TIMER1
    {
        if(flags.POWER_FLAG && BUTTON_FILTER)  // Пристрій увімкнено 
        {
            switch(flags.TMR_FLAG)  // Дивимось який період пройшов
            {
                case 0:
                    PIT = 1;  // Зупиняємо імпульс регулювання напруги
                    if(!flags.SMOOTH_START_FLAG)
                    {
                        TMR1L = regims[current_regim].TMR1L_OUT;  // Затримка імпульсу для двигунів згідно режиму роботи
                        TMR1H = regims[current_regim].TMR1H_OUT;
                    }
                    
                    else
                    {
                        TMR1L = TMR1L_smooth;
                        TMR1H = TMR1H_smooth;
                    }
                    
                    flags.TMR_FLAG = 1; // Наступний відрізок періоду
                    break;

                case 1:
                    OUT1 = 0;  // Даємо імпульс на керування двигуном
                    OUT2 = 0;  // Даємо імпульс на керування двигуном
                    TMR1L = TMR1L_20us;  // Затримка 20 мкрс
                    TMR1H = TMR1H_20us;
                    flags.TMR_FLAG = 2;  // Наступний відрізок періоду
                    break;
                    
                case 2:
                    OUT1 = 1;   // Зупиняємо імпульс на керування двигуном
                    OUT2 = 1;   // Зупиняємо імпульс на керування двигуном
                    flags.TMR_FLAG = 0;
                    T1CONbits.TMR1ON = 0;  // Зупиняємо таймер
                    break;
            }     
        }
        
        else if(flags.POWER_FLAG && !BUTTON_FILTER)  // Пристрій увімкнено але аварійна зупинка двигуна
        {
            if(flags.TMR_FLAG == 0)
            {
                PIT = 0;  // Даємо імпульс на регулювання напруги
                TMR1L = TMR1L_20us;  // Затримка 20 мкрс
                TMR1H = TMR1H_20us;
                flags.TMR_FLAG = 1;
            }
            
            else
            {
                PIT = 1;  // Вимикаємо імпульс на регулювання напруги
                T1CONbits.TMR1ON = 0;  // Зупиняємо таймер
            }
        }
        
        else  // Пристрій вимкнено
        {
            switch (flags.TMR_FLAG)
            {
                case 0:
                    // Встановлюємо таймер на невистачаючий період до 0.9 с //
                    TMR1L = TMR1L_900ms;  
                    TMR1H = TMR1H_900ms;
                    
                    flags.TMR_FLAG = 1;
                    break;
                    
                case 1:
                    LED_P = 1;  // Вмикаємо світлодіод
                    
                    // Відраховуємо 0.1 с //
                    TMR1L = TMR1L_100ms;
                    TMR1H = TMR1H_100ms;
                    
                    flags.TMR_FLAG = 2;
                    break;
                    
                case 2:
                    LED_P = 0;  // Вимикаємо світлодіод
                    
                    // Скидаємо таймер на новий цикл //
                    TMR1L = 0;
                    TMR1H = 0;
                    flags.TMR_FLAG = 0;
                    
                    break;
            }
        }
        TMR1IF = 0;   // скидаємо прапор переривання;
    }
}

// Функція налаштування системної частоти //
void sysclk_init(uint8_t init_flag)
{
    OSCCONbits.SCS = 1;  // Встановлюємо внутрішнє джерело тактування
    
    if(init_flag == 0)
    {
         // Налаштування системної частоти на 8 МГц //
        OSCCONbits.IRCF0 = 1;
        OSCCONbits.IRCF1 = 1;   
        OSCCONbits.IRCF2 = 1;
    }
    
    else
    {
        // Налаштування системної частоти на 500 МГц //
        OSCCONbits.IRCF0 = 1;
        OSCCONbits.IRCF1 = 1;   
        OSCCONbits.IRCF2 = 0;
    }
   
    
}

// Фугкція налаштування роботи АЦП //
void adc_init(void)
{
    ADCON0 = 0b10101101;    // Праве вирівнювання, опорна напруга = VDD, канал = AN11
    ADCON1bits.ADCS2 = 0;
    ADCON1bits.ADCS1 = 0;   // внтутрішнє джерело тактування
    ADCON1bits.ADCS0 = 0.;
}

// Функція налаштування таймера  //
void tim_init(void)
{
    //-----------------------------TIMER1-------------------------------------//
    
    T1CONbits.TMR1CS = 0;  // Внуторішнє джерело тактування (Fosc / 4)
    
    // Встановлюємо переддільник 1:1 //
    T1CONbits.T1CKPS0 = 0;    
    T1CONbits.T1CKPS1 = 0;
    
    INTCONbits.GIE = 1;   // Дозволяємо глобальні переривання
    INTCONbits.PEIE = 1;  // Дозволяємо переривання від периферії
    TMR1H = 0;
    TMR1L = 0;
    TMR1IF = 0;
    PIE1bits.TMR1IE = 1;  // Дозволяємо переривання від TIMER1 
}

// Функція налаштування входів/виходів //
void pins_init(void)
{
    PORTA = 1;
    ANSEL = 0;
    ANSELH = 0b00001000;
    INTCONbits.INTE = 0;
    CM1CON0bits.C1ON = 0;
    PCONbits.ULPWUE = 0;
    __delay_ms(5);
    TRISA = 0b00001001;
    TRISB = 0b01100000;
    TRISC = 0b01001111;
    __delay_ms(5);
    
    LED1 = 1;
    LED2 = 1;
    LED3 = 1;
    LED4 = 1;
    LED5 = 1; 
    OUT1 = 1;
    OUT2 = 1;
    PIT = 1;
   __delay_ms(5);
}

//  Функція для моргання світлодіодами (Індикатор появи напруги на контролері) //
void blink_leds(void)
{
    LED1 = 0;
    __delay_ms(200);
    LED1 = 1;
    LED2 = 0;
    __delay_ms(200);
    LED2 = 1;
    LED3 = 0;
    __delay_ms(200);
    LED3 = 1;
    LED4 = 0;
    __delay_ms(200);
    LED4 = 1;
    LED5 = 0;
    __delay_ms(200);
    LED5 = 1;
}


// Функція встановлення режиму потужності двигуна
void set_regim(struct regim rgm)
{
    // встановлення стану світлодіодів //
    LED1 = rgm.led1_val;
    LED2 = rgm.led2_val;
    LED3 = rgm.led3_val;
    LED4 = rgm.led4_val;
    LED5 = rgm.led5_val;
}

// smooth start function



// Функція детекції переходу синусоїди через "0"-точку
void zero_detect(void)
{
    static uint16_t smooth_start_val = 48574;
    static uint8_t led_p_period = 90;  // Зміннна для відліку періоду блимання LED1 - LED5
    static uint8_t led_p_period_flag = 0;
    static uint8_t flag_zero = 0; // Прапор для детекції перетину через 0
    static uint8_t run_flag = 0;  // Прапор дозволу реакції на перетин через 0
    uint16_t adc_val = 0;         // Значення прочитане від АЦП
    
    ADCON0bits.GO = 1;  // Старт роботи АЦП
    while(GO);          // Очікування результату
    
    // Запис результату роботи АЦП //
    adc_val = (uint16_t)ADRESH << 8;
    adc_val |= ADRESL;
    
    // Перевірка на перетин нуля //
    if(adc_val <= 1 && flag_zero)  // Перетин нуля при спаді напівхвилі
    {
        flag_zero = 0;  // Закінчення позитивної напівхвилі
        run_flag = 1;
    }
    else if(adc_val >= 1 && !flag_zero)  // Перетин нуля при рості напівхвилі
    {
        flag_zero = 1;  // Закінчення негативної напівхвилі
        run_flag = 1;
    }
    
    else run_flag = 0;  // Перетину нуля немає
    
    if(run_flag)
    {
        if(BUTTON_FILTER) 
        {
            // Встановлення стартового числа таймеру//
            flags.TMR_FLAG = 0;
            TMR1L = TMR1L_20us;  
            TMR1H = TMR1H_20us;
            PIT = 0;  // Подаємо імпульс для регулювання напруги
            
            if(flags.SMOOTH_START_FLAG)   // Якщо плавний пуск
            {
                
                if((uint8_t)smooth_start_val != regims[current_regim].TMR1L_OUT
                    && smooth_start_val >> 8 != regims[current_regim].TMR1H_OUT)  // Перевіряємо чи не вийшли на необхідний режим
                {
                    // Записуємо стартове число таймера для імпульсу //
                    TMR1L_smooth = smooth_start_val;
                    TMR1H_smooth = smooth_start_val >> 8;
                    
                    smooth_start_val += 200; // Зменшуємо затримку імпульсу на 1%
                }
                
                else
                {
                    smooth_start_val = 48574;  // Встановлюємо початкове значення для плавного пуску
                    flags.SMOOTH_START_FLAG = 0;     // Скидаємо прапор плавного пуску
                }
            }
        }
        
        else
        {
            PIT = 1;
            TMR1L = 51534;  // Затримка в 70% часу напівперіоду синусоїди  
            TMR1H = 51534 >> 8;
            flags.TMR_FLAG = 0;
 
            // Реалізуємо блимання світлодіодами LED1-LED5 згідно режиму (частота 1 Гц)
            if (led_p_period != 0){ led_p_period -= 1; }
            else
            {
                // пройшло 900 мс //
                if(!led_p_period_flag)
                {
                    LED1 = regims[current_regim].led1_val;
                    LED2 = regims[current_regim].led2_val;
                    LED3 = regims[current_regim].led3_val;
                    LED4 = regims[current_regim].led4_val;
                    LED5 = regims[current_regim].led5_val;
                    led_p_period_flag = 1;
                    led_p_period = 10;
                }
                
                // пройшло 100 мс //
                else
                {
                    LED1 = 1;
                    LED2 = 1;
                    LED3 = 1;
                    LED4 = 1;
                    LED5 = 1;
                    led_p_period_flag = 0;
                    led_p_period = 90;
                }
            }
        }
        T1CONbits.TMR1ON = 1;  // Запуск таймеру для імпульсу
    }
}

