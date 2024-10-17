/*
    ВНИМАНИЕ! 
Для выполнения этого задания у вас должен быть способ передавать с платы на компьютер 
сообщения через интерфейс UART. Для этого к плате stm32f0discovery должен быть подключен UART-USB адаптер.
Пин PB6 - это UART TX
Пин PB7 - это UART RX
 
Подключение потенциометра к плате:
 _________
/         \
|    _    |
|   (_)   |
|         |
\_________/
 |   |   |
 |   |   |
 |   |   |
GND PC0  3V

PC0 - нулевой канал АЦП. Есть ещё другие: PC1, PC2, PC3, PC4, PC5. 
Номер пина в порту совпадает с номером канала.

*/ 

// подключение библиотек
#include <stdio.h>
#include "xtimer.h"
#include "timex.h"
#include "periph/adc.h"
#include "thread.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "msg.h"

char thread_one_stack[THREAD_STACKSIZE_DEFAULT];
char thread_two_stack[THREAD_STACKSIZE_DEFAULT];

static kernel_pid_t thread_one_pid, thread_two_pid;
static float max_voltage = 10.0f; //TODO: check

void *thread_one(void *arg)
{
  (void) arg;
  msg_t msg; int sample = 0; float voltage = 0, old_voltage = 0;
  adc_init(ADC_LINE(0)) ;
  while(1){
        sample = adc_sample(ADC_LINE(0),  ADC_RES_12BIT);
        old_voltage = voltage;
        voltage = 5.0f*sample/1024/4;
        printf("ADC: %d VOLTAGE: %f \r\n", sample, voltage);
        
        if(old_voltage != voltage){
          msg.content.ptr = &voltage;
          msg_send(&msg, thread_two_pid);
        }
        
        xtimer_usleep(100'000);
  }
  
  return NULL;
}

void *thread_two(void *arg) 
{
  (void) arg;
  msg_t msg;
  gpio_init(GPIO_PIN(PORT_C,8),GPIO_OUT);
  
  while(1){
    msg_receive(&msg);
    if(*((float*)msg.content.ptr) > max_voltage/2){
      gpio_set(GPIO_PIN(PORT_C,8));
    } else {
      gpio_clear(GPIO_PIN(PORT_C,8));
    }
  }
  
  return NULL;
}

int main(void)
{
    thread_one_pid = thread_create(thread_one_stack, sizeof(thread_one_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  thread_one, NULL, "thread_one");

    thread_two_pid = thread_create(thread_two_stack, sizeof(thread_two_stack),
                  THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                  thread_two, NULL, "thread_two");
    while(1){
    
    }
    
    return 0;
}

/*
Задание 1: переключите разрешение АЦП на 12 бит.
Задание 2: сделайте так, чтобы в консоль выводилась строка "ADC: xxx VOLTAGE: yyy ",
            где xxx - число, получаемое с АЦП (тип int), yyy - измеренное напряжение в вольтах (тип float).
            Чтобы выводить переменные типа float через printf, в Makefile нужно добавить модуль 
            USEMODULE += printf_float
Задание 3: вынесите чтение АЦП в отдельный поток. После этого добавьте другой поток, который будет 
            включать светодиод PC8, если АЦП измерил напряжение больше половины от максимального, 
            и выключать, если меньше.
Задание 4: модифицирейте код предудщего решения так, чтобы ручка потенциометра задавала период моргания светодиода. 
            одно крайнее положение ручки должно соответствовать периоду 100 мс, другое - 2 с.
*/
