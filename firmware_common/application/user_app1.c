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

// Globals for passing data from the ANT application to the API
extern u32 G_u32AntApiCurrentMessageTimeStamp;                            // From ant_api.c
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;            // From ant_api.c
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];  // From ant_api.c
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;                // From ant_api.c


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_<type>" and be declared as static.
***********************************************************************************************************************/
static u32 UserApp1_u32DataMsgCount = 0;  /* ANT_DATA packet counter */
static u32 UserApp1_u32TickMsgCount = 0;  /* ANT_TICK packet counter */

static fnCode_type UserApp1_pfStateMachine;               /*!< @brief The state machine function pointer */
static u32 UserApp1_u32Timeout;                           /*!< @brief Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/* Function prototypes */
static void UserApp1SM_WaitAntReady(void);
static void UserApp1SM_WaitChannelOpen(void);
static void UserApp1SM_WaitChannelClose(void);
static void UserApp1SM_ChannelOpen(void);

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
  PixelAddressType sStringLocation;
  PixelBlockType G_sLcdClearLine7;
  u8 au8WelcomeMessage[] = "ANT Slave Demo";
  AntAssignChannelInfoType sChannelInfo;

  if(AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) == ANT_UNCONFIGURED)
  {
    sChannelInfo.AntChannel = U8_ANT_CHANNEL_PERIOD_HI_USERAPP; // thought it was U8_ANT_CHANNEL_USERAPP?
    sChannelInfo.AntChannelType = CHANNEL_TYPE_SLAVE;
    sChannelInfo.AntChannelPeriodHi = U8_ANT_CHANNEL_PERIOD_HI_USERAPP;
    sChannelInfo.AntChannelPeriodLo = U8_ANT_CHANNEL_PERIOD_LO_USERAPP;
    
    sChannelInfo.AntDeviceIdHi = U8_ANT_DEVICE_HI_USERAPP;
    sChannelInfo.AntDeviceIdLo = U8_ANT_DEVICE_LO_USERAPP;
    sChannelInfo.AntDeviceType = U8_ANT_DEVICE_TYPE_USERAPP;
    sChannelInfo.AntTransmissionType = U8_ANT_TRANSMISSION_TYPE_USERAPP;
    
    sChannelInfo.AntFrequency = U8_ANT_FREQUENCY_USERAPP;
    sChannelInfo.AntTxPower = U8_ANT_TX_POWER_USERAPP;
    
    sChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
    for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
    {
      sChannelInfo.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
    }
    //AntAssignChannel(&sChannelInfo);
  }

  /* Update LEDs and LCD message for ANT Slave Demo */
  LedOn(RED0);
  /* Write the board string in the middle of last row */
  sStringLocation.u16PixelColumnAddress = U16_LCD_CENTER_COLUMN - (strlen((char const*)au8WelcomeMessage) * (U8_LCD_SMALL_FONT_COLUMNS + U8_LCD_SMALL_FONT_SPACE) / 2);
  sStringLocation.u16PixelRowAddress = U8_LCD_SMALL_FONT_LINE7;
  G_sLcdClearLine7.u16RowSize = 10;
  G_sLcdClearLine7.u16ColumnSize = U16_LCD_RIGHT_MOST_COLUMN;
  G_sLcdClearLine7.u16RowStart = U16_LCD_BOTTOM_MOST_ROW - 10;
  G_sLcdClearLine7.u16ColumnStart = 0;

  LcdClearPixels(&G_sLcdClearLine7);
  LcdLoadString(au8WelcomeMessage, LCD_FONT_SMALL, &sStringLocation);

  /* If good initialization, set state to UserApp1SM_WaitAntReady */
  if( AntAssignChannel(&sChannelInfo) )
  {
    UserApp1_pfStateMachine = UserApp1SM_WaitAntReady;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    LedBlink(RED0, LED_4HZ);
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
/* Wait for ANT channel to be configured */
static void UserApp1SM_WaitAntReady(void) {
  if (AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) == ANT_CONFIGURED) {
    if (AntOpenChannelNumber(U8_ANT_CHANNEL_USERAPP)) {
      LedOn(GREEN0);
      UserApp1_pfStateMachine = UserApp1SM_Idle;
    }
    else {
      UserApp1_pfStateMachine = UserApp1SM_Error;
    }
  }
} /* end UserApp1SM_WaitAntReady() */

/* Hold here until ANT confirms the channel is open. LED status: green blink 2Hz */
static void UserApp1SM_WaitChannelOpen(void) {
  if (AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) == ANT_OPEN) {
    /* Channel opened: go to ChannelOpen state with solid green */
    LedOff(GREEN0);
    LedOn(GREEN0);
    UserApp1_pfStateMachine = UserApp1SM_ChannelOpen;
  }

  /* Check for timeout */
  if (IsTimeUp(&UserApp1_u32Timeout, U8_ANT_SEARCH_TIMEOUT)) {
    AntCloseChannelNumber(U8_ANT_CHANNEL_USERAPP);
    LedOn(RED0);
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }

} /* end UserApp1SM_WaitChannelOpen() */

/* Process messages while channel is open */
static void UserApp1SM_ChannelOpen(void) {
  static u8 u8LastState = 0xff;
  static u8 au8TickMessage[] = "EVENT x\n\r"; /* "x" at index [6] will be replaced by current code */
  static u8 au8LastAntData[ANT_APPLICATION_MESSAGE_BYTES] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  /* Stopped at ChannelOpen state*/
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0xA5, 0, 0, 0};
  static PixelAddressType sStringLocation;
  u8 au8DataContent[] = "xxxxxxxxxxxxxxxx";
  bool bGotNewData;

  /* Check for BUTTON0 to close channel */
  if(WasButtonPressed(BUTTON0)) {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0);

    /* Queue close channel, initialize the u8LastState variable and change LED to blinking green */
    AntCloseChannelNumber(U8_ANT_CHANNEL_USERAPP);
    u8LastState = 0xff;
    LedOff(RED0);
    LedOff(GREEN0);
    LedBlink(GREEN0, LED_2HZ);

    /* Set time and advance states */
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_pfStateMachine = UserApp1SM_WaitChannelClose;
  }

  /* A slave channel can close on its own, so explicitly check channel status */
  if(AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) != ANT_OPEN) {
    u8LastState = 0xff;
    LedOff(RED0);
    LedOff(GREEN0);
    LedBlink(GREEN0, LED_2HZ);
    
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_pfStateMachine = UserApp1SM_WaitChannelClose;
  }

  /* Check for new messages and process */
  if(AntReadAppMessageBuffer()) {
    /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA) {
      /* Just increment a counter for now */
      UserApp1_u32DataMsgCount++;
    }
    else if(G_eAntApiCurrentMessageClass == ANT_TICK) {
      /* A channel period has gone by, Just incremenet a counter for now */
      UserApp1_u32TickMsgCount++;
      /* Look at the TICK contents to check the event code and respond only if it's different */
      if (u8LastState != G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX]) {
        /* The state changed so update u8LastState and queue a debug message to show EVENT CODE */
        u8LastState = G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX];
        au8TickMessage[6] = HexToASCIICharUpper(u8LastState);
        DebugPrintf(au8TickMessage);

        /* Parse u8LastState to update LED status */
        switch(u8LastState) {
          /* Handle "good response" code that can appear when other ANT commands are sent */
          case RESPONSE_NO_ERROR: {
            /* Don't do anything here for now */
            break;
          }
          /* If we are paired but missing messages, blue blinks */
          case EVENT_RX_FAIL: {
            LedOff(GREEN0);
            LedBlink(BLUE0, LED_2HZ);
            break;
          }
          /* If we drop to search, LED is green */
          case EVENT_RX_FAIL_GO_TO_SEARCH: {
            LedOff(BLUE0);
            LedOn(GREEN0);
            break;
          }
          /* If the search times out, the channel should automatically close */
          case EVENT_RX_SEARCH_TIMEOUT: {
            DebugPrintf("Search timeout\r\n");
            break;
          }
          default: {
            DebugPrintf("Unexpected Event\r\n");
            break;
          }
        }
      }
    }
  }

  extern PixelBlockType G_sLcdClearLine7; /* From lcd-NHD-C12864LZ.c */

  if (AntReadAppMessageBuffer()) {
    if (G_eAntApiCurrentMessageClass == ANT_DATA) {
      /* We got some data! Convert it to displayable ASCII characters */
      for (u8 i = 0; i < ANT_DATA_BYTES; i++) {
        au8DataContent[2 * i] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] / 16);
        au8DataContent[2 * i + 1] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] % 16);
      }

      /* Write the board string in the middle of last row */
      sStringLocation.u16PixelColumnAddress = 
        U16_LCD_CENTER_COLUMN - (strlen((char const*)au8DataContent) * 
        (U8_LCD_SMALL_FONT_COLUMNS + U8_LCD_SMALL_FONT_SPACE) / 2);
      sStringLocation.u16PixelRowAddress = U8_LCD_SMALL_FONT_LINE7;
      LcdClearPixels(&G_sLcdClearLine7);
      LcdLoadString(au8DataContent, LCD_FONT_SMALL, &sStringLocation);
    }
    else if (G_eAntApiCurrentMessageClass == ANT_TICK) {
      /* Check the buttons and update corresponding slot in au8TestMessage */
      au8TestMessage[0] = 0x00;
      au8TestMessage[1] = 0x00;
      au8TestMessage[2] = 0x00;
      au8TestMessage[3] = 0x00;

      if (IsButtonPressed(BUTTON0)) {
        au8TestMessage[0] = 0xff;
      }
      if (IsButtonPressed(BUTTON1)) {
        au8TestMessage[1] = 0xff;
      }

      /* A channel period has gone by: typically this is when new
      data should be queued to send */
      au8TestMessage[7]++; //ask about this
      if (au8TestMessage[7] == 0) {
        au8TestMessage[6]++;
        if (au8TestMessage[6] == 0) {
          au8TestMessage[5]++;
        }
      }
      AntQueueBroadcastMessage(U8_ANT_CHANNEL_USERAPP, au8TestMessage);
    }
  }
} /* end UserApp1SM_ChannelOpen() */

/* Wait for channel to close. LED status: green blink 2Hz */
static void UserApp1SM_WaitChannelClose(void) {
  /* Wait for the channel status to update */
  if (AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) == ANT_CLOSED) {
    LedOff(GREEN0);
    LedOff(RED0);
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }

  /* Check for timeout */
  if (IsTimeUp(&UserApp1_u32Timeout, U8_ANT_SEARCH_TIMEOUT)) {
    LedOff(GREEN0);
    LedOff(RED0);
    LedBlink(RED0, LED_4HZ);
    UserApp1_pfStateMachine = UserApp1SM_Error;
  }
}

/* What does this state do? */
static void UserApp1SM_Idle(void)
{
  /* Look for BUTTON0 to open channel */
  if (WasButtonPressed(BUTTON0)) {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0);

    /* Queue open channel and change LED0 from yellow to blinking green to indicate channel is opening */
    AntOpenChannelNumber(U8_ANT_CHANNEL_USERAPP);

    LedOff(RED0);
    LedOff(GREEN0);
    LedBlink(GREEN0, LED_2HZ);

    /* Set timer and advance states */
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_pfStateMachine = UserApp1SM_WaitChannelOpen;
  }
     
} /* end UserApp1SM_Idle() */
     
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
