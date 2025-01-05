/*!*********************************************************************************************************************
@file user_app1.c                                                                
@brief User's tasks / applications are written here.  This description
should be replaced by something specific to the task.

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void UserApp1Initialize(void)
- void UserApp1RunActiveState(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                          /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;                /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_pfStateMachine;               /*!< @brief The state machine function pointer */
//static u32 UserApp1_u32Timeout;                           /*!< @brief Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void UserApp1Initialize(void)

@brief
Initializes the State Machine and its variables.

Should only be called once in main init section.

Requires:
- NONE

Promises:
- NONE

*/
void UserApp1Initialize(void)
{
  for (u8 i = 0; i < U8_TOTAL_LEDS; i++)
    LedOff((LedNameType)i);

  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_pfStateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void UserApp1RunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void UserApp1RunActiveState(void)
{
  UserApp1_pfStateMachine();

} /* end UserApp1RunActiveState */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
/* What does this state do? */
static void UserApp1SM_Idle(void)
{
  /*
  static u8 blinking = 0;
  static LedRateType blink_rates[] = {LED_1HZ, LED_2HZ, LED_4HZ, LED_8HZ};
  static u8 blink_index = 0;
  static LedRateType blink_rate = LED_1HZ;

  if (IsButtonPressed(BUTTON0))
    LedOn(BLUE0);
  else
    LedOff(BLUE0);

  if (WasButtonPressed(BUTTON1)) {
    ButtonAcknowledge(BUTTON1);
    if (!blinking) {
      LedBlink(RED1, blink_rate);
      blinking = 1;
    }
    else {
      LedOff(RED1);
      blinking = 0;
      blink_rate = blink_rates[0];
    }
  }

  if (WasButtonPressed(BUTTON0)) {
    ButtonAcknowledge(BUTTON0);
    if (blinking) {
      blink_index++;
      if (blink_index == (sizeof(blink_rates) / sizeof(LedRateType)))
        blink_index = 0;
      blink_rate = blink_rates[blink_index];
      LedBlink(RED1, blink_rate);
    }
  }

  if (IsButtonHeld(BUTTON0, 2000))
    LedOn(LCD_BL);
  else
    LedOff(LCD_BL);
  */
  /*
  static u8 correct = 0; 
  static u8 password[10] = {1, 2, 1, 2};
  static u8 entered[10];
  static u8 index = 0;

  LedOn(RED3);
  LedOn(GREEN3);

  if (IsButtonPressed(BUTTON0)) {
    LedOn(BLUE0);
    entered[index] = 1;
    index++;
  }
  else
    LedOff(BLUE0);

  if (IsButtonPressed(BUTTON1)) {
    LedOn(BLUE1);
    entered[index] = 2;
    index++;
  }
  else
    LedOff(BLUE1);

  if (IsButtonHeld(BUTTON0, 2000) && IsButtonHeld(BUTTON1, 2000)) {
    int i;
    for (i = 0; i < 10; i++) {
      if (entered[i] != password[i]) {
        correct = 0;
        break;
      }
    }
    if (i == 10)
      correct = 1;

    LedOff(RED3);
    LedOff(GREEN3);
    if (correct)
      LedBlink(GREEN3, LED_1HZ);
    else
      LedBlink(RED3, LED_1HZ);
     
    static u16 counter = 0;
    counter++;
    if(counter == 3000) {
      LedOff(RED3);
      LedOff(GREEN3);
      LedOn(GREEN3);
      if (!correct)
        LedOn(RED3);
    }

    index = 0;
    correct = 0;
    for (int j = 0; j < 10; j++)
      entered[j] = 0;
  }
  */

 
} /* end UserApp1SM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
