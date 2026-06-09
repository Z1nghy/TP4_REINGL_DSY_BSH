/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************
#include "app.h"
#include "peripheral/oc/plib_oc.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
                DRV_OC0_Enable();
                DRV_OC1_Enable();
                
                DRV_TMR0_Start();
                DRV_TMR1_Start();
                
                /* ADC */
                //DRV_ADC_Initialize();
                DRV_ADC_Open();
                DRV_ADC_Start();
                PLIB_ADC_SampleAutoStartEnable(ADC_ID_1);
                
                PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_5,true);
            if (appInitialized)
            {
            
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
        
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

/* -----------------------------------------------------------------------
 * Regulateur PI - boucle de vitesse
 *
 * Appele par l'ISR du Timer 1 (periode = 749 ticks, prescaler /64,
 * Fclk = 40MHz  =>  Ts = 749*64/40e6 = 1.198 ms ~ 1.2 ms)
 *
 * ref     : consigne ADC  (0..1023, 10 bits)
 * measure : mesure vitesse ADC (0..1023, 10 bits)
 * retour  : duty cycle en % (0..100), sature
 * ----------------------------------------------------------------------- */

 
#define PWM_MAX    100.0f
#define PWM_MIN   -100.0f
#define DEADBAND    10.0f    /* LSB ADC ? ajuster selon le bruit mesure */
 static float somme_erreurs = 0.0;
// le P controleur fonctionelle
int P_Controller(uint16_t ref, uint16_t measure)
{
    const float Kp = 0.5f;

    /* Calcul de l'erreur */
    float erreur = ((float)ref - (float)measure);

    /* Régulateur P */
    float output = Kp * erreur;

    /* Saturation */
    if (output > PWM_MAX)
        output = PWM_MAX;

    if (output < PWM_MIN)
        output = PWM_MIN;

    return (int)output;
}

int PI_Controller(uint16_t ref, uint16_t measure) 
{ 
    const float Kp = 0.7f; 
    /* a ajuster : regle la reactivite */ 
    const float Ki = 0.03f; 
    /* a ajuster : regle la precision statique */ 
    /* --- Calcul de l'erreur ------------------------------------------- */ 
    float erreur = ((float)ref - (float)measure); 
    /* --- Zone morte : ignore le bruit ADC ------------------------------ */ 
    if (erreur > -DEADBAND && erreur < DEADBAND) erreur = 0.0f; 
    /* --- Integrale (formulation TP4) ----------------------------------- */ 
    somme_erreurs += erreur; /* --- Anti-windup (formulation TP4 : limite = PWM_MAX / Ki) --------- */ 
    float limite_integrale = PWM_MAX / Ki; 
    if (somme_erreurs > limite_integrale) somme_erreurs = limite_integrale; 
    if (somme_erreurs < 0.0f) somme_erreurs = 0.0f; 
    /* --- Sortie PI ----------------------------------------------------- */ 
    float output = (Kp * erreur) + (Ki * somme_erreurs); 

    /* --- Saturation finale dans [-1000, +1000] ------------------------- */ 
    if (output > PWM_MAX) output = PWM_MAX; 
    if (output < PWM_MIN) output = PWM_MIN; 
    return (int)output; 
}

int PID_Controller(uint16_t ref, uint16_t measure)
{
    const float Kp = 1.5f;
    const float Ki = 0.05f;
    const float Ts = 0.0012f;

    static float integral = 0.0f;

    float error;
    float output;

    error = (float)ref - (float)measure;

    /* Intégrale */
    integral += error * Ki * Ts;

    /* Limitation intégrale */
    if (integral > 500.0f)
        integral = 500.0f;

    if (integral < -500.0f)
        integral = -500.0f;

    output = (Kp * error) + integral;
    /* --- Saturation finale dans [-1000, +1000] ------------------------- */ 
    if (output > PWM_MAX) output = PWM_MAX; 
    if (output < PWM_MIN) output = PWM_MIN; 

    return (int)output;
}

void duty_calculator(int cmd)
{
    // Saturation
    if (cmd < -100) cmd = -100;
    if (cmd > 100)  cmd = 100;

    // Conversion :
    // -100 -> 0
    //   0  -> 50
    // +100 -> 100
    int duty = (cmd + 100) / 2;

    int val = 10 * duty;

    OC2R  = val - 1;
    OC2RS = 1000 - val;
    OC3R  = 1000 - val;
    OC3RS = 0;
}

/*******************************************************************************
 End of File
 */
