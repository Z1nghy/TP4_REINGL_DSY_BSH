/*******************************************************************************
  system_interrupt.c

  ISR pour :
    - ADC      : moyenne glissante 16 echantillons
    - Timer 1  : boucle de vitesse (~1.2 ms)
    - Timer 2  : boucle de position (0.1 ms)
 *******************************************************************************/

#include "system/common/sys_common.h"
#include "app.h"
#include "system_definitions.h"

// *****************************************************************************
// Section: Globals
// *****************************************************************************

volatile uint16_t adc_position;

/* Consigne de vitesse calculee par la boucle de position et
 * consommee par la boucle de vitesse */
volatile uint16_t speed_setpoint;

volatile uint16_t adc_speed = 0;
volatile uint16_t adc_ref   = 0;

static uint32_t speed_sum = 0;
static uint32_t ref_sum   = 0;

static uint16_t speed_buffer[16] = {0};
static uint16_t ref_buffer[16]   = {0};

static uint8_t index = 0;

// *****************************************************************************
// Section: ADC ISR ? Moyenne glissante sur 16 echantillons
// *****************************************************************************

void __ISR(_ADC_VECTOR, ipl3AUTO) _IntHandlerDrvAdc(void)
{
    uint16_t speed_raw = PLIB_ADC_ResultGetByIndex(ADC_ID_1, 0);
    uint16_t ref_raw   = PLIB_ADC_ResultGetByIndex(ADC_ID_1, 1);
    uint16_t pos_raw   = PLIB_ADC_ResultGetByIndex(ADC_ID_1, 2);

    speed_sum -= speed_buffer[index];
    ref_sum   -= ref_buffer[index];

    speed_buffer[index] = speed_raw;
    ref_buffer[index]   = ref_raw;

    speed_sum += speed_raw;
    ref_sum   += ref_raw;

    adc_speed = (uint16_t)(speed_sum >> 4);   /* /16 */
    adc_ref   = (uint16_t)(ref_sum   >> 4);   /* /16 */

    index = (index + 1) & 0x0F;              /* modulo 16 */

    PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_ADC_1);
}

// *****************************************************************************
// Section: Timer 1 ISR ? Boucle de vitesse (~1.2 ms)
// *****************************************************************************

volatile int erreur_debug;
volatile int sortie_debug;

void __ISR(_TIMER_1_VECTOR, ipl1AUTO) IntHandlerDrvTmrInstance0(void)
{
    erreur_debug = (int)adc_ref - (int)adc_speed;

    /*
     * PI_Controller retourne une valeur signee dans [-1000, +1000].
     * Pas de facteur multiplicatif ? duty_calculator attend [-1000, +1000].
     */
    sortie_debug = PID_Controller(adc_ref, adc_speed);

    duty_calculator(sortie_debug);

    PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_TIMER_1);
}

// *****************************************************************************
// Section: Timer 2 ISR ? Boucle de position (0.1 ms)
// *****************************************************************************

void __ISR(_TIMER_2_VECTOR, ipl1AUTO) IntHandlerDrvTmrInstance1(void)
{
//    speed_setpoint = (uint16_t)PID_Position(adc_ref, adc_position);
    PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_TIMER_2);
}

/*******************************************************************************
 End of File
*/