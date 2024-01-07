/***********************************************************************
 *  MIDI Channel Splitter
 *
 *  DESCRIPTION
 *      <<TODO>> Tells you what the code in the file does or refers to
 *      accompanying header file.
 *
 *  REFERENCES
 *      <<TODO>> Requirements Specification
 *      <<TODO>> Software Specification
 ***********************************************************************/

/*=====================================================================*
    Local Header Files
 *=====================================================================*/


/*=====================================================================*
    Interface Header Files
 *=====================================================================*/
/* None */


/*=====================================================================*
    System-wide Header Files
 *=====================================================================*/
/* None */

/*=====================================================================*
    Pin Defines
 *=====================================================================*/
#define PIN_PORT_A              (5)     // Active low
#define PIN_PORT_B              (6)     // Active low
#define PIN_PORT_C              (7)     // Active low
#define PIN_PORT_D              (8)     // Active low

#define PIN_CFG1                (A0)
#define PIN_CFG2                (A1)

#define PIN_LED                 (A2)    // Active low

/*=====================================================================*
    Private Defines
 *=====================================================================*/
#define NUM_PORTS               (4)     // Number of output ports

/*=====================================================================*
    Private Data Types
 *=====================================================================*/

/*=====================================================================*
    Private Function Prototypes
 *=====================================================================*/
/* None */


/*=====================================================================*
    Private Constants
 *=====================================================================*/


/*=====================================================================*
    Private Data
 *=====================================================================*/
static uint8_t port[NUM_PORTS] = {PIN_PORT_A, PIN_PORT_B, PIN_PORT_C, PIN_PORT_D};
static uint8_t channels[NUM_PORTS] = {0, 1, 2, 3};
static uint8_t config = 0;


/*=====================================================================*
    Arduino Hooks
 *=====================================================================*/

void setup()
{
    // Initialize Serial
    Serial.begin(31250);
    
    // Initialize GPIO
    pinMode(PIN_PORT_A, OUTPUT);
    pinMode(PIN_PORT_B, OUTPUT);
    pinMode(PIN_PORT_C, OUTPUT);
    pinMode(PIN_PORT_D, OUTPUT);

    pinMode(PIN_CFG1, INPUT_PULLUP);
    pinMode(PIN_CFG2, INPUT_PULLUP);

    pinMode(PIN_LED, OUTPUT);
}


/*=====================================================================*/

void loop()
{
    update_config();
 
    while (Serial.available())
    {
        digitalWrite(PIN_LED, LOW);

        // Process the incoming MIDI byte
        uint8_t byte = Serial.read();
        uint8_t msg_channel = parse_midi(byte);

        if (msg_channel == 0xFF)
        {
            // This byte is not associated with a specific channel
            enable_all_ports();
        }
        else
        {
            // This byte is for a specific channel
            disable_all_ports();
            for (uint8_t i = 0; i < NUM_PORTS; i++)
            {
                if (channels[i] == msg_channel)
                {
                    digitalWrite(port[i], LOW);
                    break;
                }
            }
        }

        // Write the byte back out
        Serial.write(byte);

        digitalWrite(PIN_LED, HIGH);
    }   
}


/*=====================================================================*
    Private Function Implementations
 *=====================================================================*/

/*---------------------------------------------------------------------*
 *  NAME
 *      parse_midi
 * 
 *  DESCRIPTION
 *      Parses the given byte of MIDI data and updates
 *      internal state machine.
 * 
 *  RETURNS
 *      The MIDI channel that the byte belongs to,
 *      or 0xFF if the byte is not associated with a channel
 *---------------------------------------------------------------------*/
uint8_t parse_midi(uint8_t byte)
{
    static uint8_t channel = 0xFF;
    
    // SYSTEM REALTIME
    if (byte >= 0xF8)
    {
        return 0xFF;
    }

    // SYSTEM COMMON
    else if (byte >= 0xF0)  // && (byte < 0xF8) implied
    {
        channel = 0xFF;
    }

    // CHANNEL MESSAGE
    else if (byte >= 0x80)  // && (byte < 0xF0) implied
    {
        channel = byte & 0x0F;
    }

    return channel;
}

/*---------------------------------------------------------------------*
 *  NAME
 *      update_config
 * 
 *  DESCRIPTION
 *      Reads the current configuration off of the DIP switches
 *      and updates the active channel list
 * 
 *  RETURNS
 *      None
 *---------------------------------------------------------------------*/
void update_config(void)
{
    uint8_t new_config = (digitalRead(PIN_CFG1) << 1) | digitalRead(PIN_CFG2);
    if (new_config != config)
    {
        config = new_config;
        channels[0] = (config << 2) + 0;
        channels[1] = (config << 2) + 1;
        channels[2] = (config << 2) + 2;
        channels[3] = (config << 2) + 3;
    }
}

/*---------------------------------------------------------------------*
 *  NAME
 *      enable_all_ports
 * 
 *  DESCRIPTION
 *      Enables all four output ports
 * 
 *  RETURNS
 *      None
 *---------------------------------------------------------------------*/
void enable_all_ports(void)
{
    digitalWrite(PIN_PORT_A, LOW);
    digitalWrite(PIN_PORT_B, LOW);
    digitalWrite(PIN_PORT_C, LOW);
    digitalWrite(PIN_PORT_D, LOW);
}

/*---------------------------------------------------------------------*
 *  NAME
 *      disable_all_ports
 * 
 *  DESCRIPTION
 *      Disables all four output ports
 * 
 *  RETURNS
 *      None
 *---------------------------------------------------------------------*/
void disable_all_ports(void)
{
    digitalWrite(PIN_PORT_A, HIGH);
    digitalWrite(PIN_PORT_B, HIGH);
    digitalWrite(PIN_PORT_C, HIGH);
    digitalWrite(PIN_PORT_D, HIGH);
}
