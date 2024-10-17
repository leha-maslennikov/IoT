#include "thread.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "msg.h"

char thread_one_stack[THREAD_STACKSIZE_DEFAULT];
char thread_two_stack[THREAD_STACKSIZE_DEFAULT];

static kernel_pid_t thread_one_pid, thread_two_pid;

void btn_handler(void *arg)
{
  (void)arg;
  msg_t msg;
  int cnt = 1;
  // TODO: ubrat drebezg
  if (gpio_read(GPIO_PIN(PORT_A,0)) > 0){
    msg.content.value = cnt;
    cnt = 1 + cnt % 5;
    msg_send(&msg, thread_one_pid);    
  } else {
    msg_send(&msg, thread_two_pid);
  }

}

void *thread_one(void *arg)
{
  (void) arg;
  msg_t msg;
  gpio_init(GPIO_PIN(PORT_C,8),GPIO_OUT);
  while(1){
    msg_receive(&msg);
    for(int i = 0; i < msg.content.value; i++) {
      gpio_toggle(GPIO_PIN(PORT_C,8));
      xtimer_usleep(300'000);
      gpio_toggle(GPIO_PIN(PORT_C,8));
      xtimer_usleep(300'000);
    }
  }
  
  return NULL;
}

static msg_t rcv_queue[8];

void *thread_two(void *arg) 
{
  (void) arg;
  gpio_init(GPIO_PIN(PORT_C,9),GPIO_OUT);
  msg_init_queue(rcv_queue, 8);
  int cnt = 1'000'000;
  while(1){
    while(msg_avail()){
      msg_receive(&msg);
      cnt -= 100'000;
      if(!cnt) cnt = 1'000'000;
    }
    gpio_set(GPIO_PIN(PORT_C,9));
    xtimer_usleep(cnt);
    gpio_clear(GPIO_PIN(PORT_C,9));
    xtimer_usleep(cnt);
  }    
  return NULL;
}


int main(void)
{
  gpio_init_int(GPIO_PIN(PORT_A,0),GPIO_IN,GPIO_BOTH,btn_handler,NULL);
  
  thread_one_pid = thread_create(thread_one_stack, sizeof(thread_one_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  thread_one, NULL, "thread_one");

  thread_two_pid = thread_create(thread_two_stack, sizeof(thread_two_stack),
                  THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                  thread_two, NULL, "thread_two");

  while (1){

    }

    return 0;
}

/* 
  Задание 1: Добавьте в код подавление дребезга кнопки
  Задание 2: Сделайте так, чтобы из прерывания по нажатию кнопки в поток thread_one передавалось целое число,
              которое означает, сколько раз должен моргнуть светодиод на пине PC8 после нажатия кнопки.
              После каждого нажатия циклически инкрементируйте значение от 1 до 5.
              Передать значение в сообщении можно через поле msg.content.value
  Задание 3: Сделайте так, чтобы из прерывания по отпусканию кнопки в поток thread_two передавалось целое число,
              которое задает значение интервала между морганиями светодиода на пине PC9. 
              Моргание светодиода не должно останавливаться.
              После каждого нажатия циклически декрементируйте значение от 1000000 до 100000 с шагом 100000.
              Чтобы послать сообщение асинхронно и без блокирования принимающего потока, нужно воспользоваться очередью. 
              Под очередь нужно выделить память в начале программы так: static msg_t rcv_queue[8];
              Затем в принимающем потоке нужно ее инициализировать так: msg_init_queue(rcv_queue, 8);
              Поток может проверять, лежит ли что-то в очереди, это делается функцией msg_avail(), 
              которая возвращает количество элементов в очереди. 

  Что поизучать: https://riot-os.org/api/group__core__msg.html
*/
