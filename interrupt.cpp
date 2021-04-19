#include "interrupt.h"
#include "pinout.h"

static bool interrupt_inited = false;
volatile bool charged_int;
volatile bool charge_int;
volatile bool button_int;
volatile bool touch_int;
volatile bool accl_int;

static interrupt interrupts;

#ifdef __cplusplus
extern "C" {
#endif
void GPIOTE_IRQHandler()
{    
    if ((NRF_GPIOTE->EVENTS_PORT != 0))
    {
        NRF_GPIOTE->EVENTS_PORT = 0;

        bool button_pin = digitalRead(PUSH_BUTTON_IN);
        if (button_pin != interrupts.button) {
            interrupts.button = button_pin;
            NRF_GPIO->PIN_CNF[PUSH_BUTTON_IN] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[PUSH_BUTTON_IN] |= ((interrupts.button ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);
        }
        bool charge_pin = digitalRead(POWER_INDICATION);
        if (charge_pin != interrupts.charging) {
            interrupts.charging = charge_pin;
            NRF_GPIO->PIN_CNF[POWER_INDICATION] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[POWER_INDICATION] |= ((interrupts.charging ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);
        }
        bool charged_pin = digitalRead(CHARGE_INDICATION);
        if (charged_pin != interrupts.charged) {
            interrupts.charged = charged_pin;
            NRF_GPIO->PIN_CNF[CHARGE_INDICATION] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[CHARGE_INDICATION] |= ((interrupts.charged ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);
        }
        bool touch_pin = digitalRead(TP_INT);
        // if (touch_pin != interrupts.touch) {
            interrupts.touch = touch_pin;
            NRF_GPIO->PIN_CNF[TP_INT] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[TP_INT] |= ((interrupts.touch ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);
        // }
            
        /*
        bool accl_pin = digitalRead(BMA421_INT);
        if (accl_pin != last_accl_state) {
          last_accl_state = accl_pin;
          NRF_GPIO->PIN_CNF[BMA421_INT] &= ~GPIO_PIN_CNF_SENSE_Msk;
          NRF_GPIO->PIN_CNF[BMA421_INT] |= ((last_accl_state ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);
          if (last_accl_state == false)set_accl_interrupt();
        }
        */
    }

    (void)NRF_GPIOTE->EVENTS_PORT;
}
#ifdef __cplusplus
}
#endif

void initInterrupt()
{
    NRF_GPIOTE->INTENCLR = GPIOTE_INTENSET_PORT_Msk;
    NVIC_DisableIRQ(GPIOTE_IRQn);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_SetPriority(GPIOTE_IRQn, 1);
    NVIC_EnableIRQ(GPIOTE_IRQn);


    NRF_GPIOTE->EVENTS_PORT = 1;
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;

    if (PUSH_BUTTON_OUT != -1) {
        pinMode(PUSH_BUTTON_OUT, OUTPUT);
        digitalWrite(PUSH_BUTTON_OUT, HIGH);
    }
    pinMode(PUSH_BUTTON_IN, INPUT );
    interrupts.button = digitalRead(PUSH_BUTTON_IN);
    NRF_GPIO->PIN_CNF[PUSH_BUTTON_IN] |= ((uint32_t)  (interrupts.button ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High)  << GPIO_PIN_CNF_SENSE_Pos);

    pinMode(POWER_INDICATION, INPUT);
    interrupts.charging = digitalRead(POWER_INDICATION);
    NRF_GPIO->PIN_CNF[POWER_INDICATION] |= ((uint32_t) (interrupts.charging ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);

    pinMode(CHARGE_INDICATION, INPUT);
    interrupts.charged = digitalRead(CHARGE_INDICATION);
    NRF_GPIO->PIN_CNF[CHARGE_INDICATION] |= ((uint32_t)  (interrupts.charged ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);

    pinMode(TP_INT, INPUT);
    interrupts.touch = digitalRead(TP_INT);
    NRF_GPIO->PIN_CNF[TP_INT] |= ((uint32_t)  (interrupts.touch ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);

//  pinMode(BMA421_INT, INPUT);
//  last_accl_state = digitalRead(BMA421_INT);
//  NRF_GPIO->PIN_CNF[BMA421_INT] |= ((uint32_t)  (last_accl_state ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);

    interrupt_inited = true;
}

interrupt* getInterrupts()
{
    return &interrupts;
}

void readInterrupts()
{
}

void clearInterrupts()
{
    interrupts.touch = 0;
    interrupts.button = 0;
}
