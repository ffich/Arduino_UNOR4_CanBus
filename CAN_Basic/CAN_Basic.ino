/*
  Can Bus Read/Write example
*/

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/
#include <Arduino_CAN.h>

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/
//#define EMULATE_IO_CONTROL

/**************************************************************************************
 * CONSTANTS
 **************************************************************************************/
static uint32_t const CAN_TX_ID = 0x700;
static uint32_t const CAN_RX_ID = 0x600;
static uint32_t const CAN_FILTER_MASK_STANDARD = 0x1FFC0000;

/**************************************************************************************
 * VARIABLES
 **************************************************************************************/
static uint16_t msg_up_cnt = 0;
static uint16_t msg_down_cnt = 0xFFFF;
static uint8_t led_sts = 0;

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/

void setup()
{
  Serial.begin(115200);
  while (!Serial) { }

  /* Set Mask */
  CAN.setFilterMask_Standard(CAN_FILTER_MASK_STANDARD);

  /* Set RX filter to CAN_RX_ID */
  for (int mailbox = 0; mailbox < R7FA4M1_CAN::CAN_MAX_NO_STANDARD_MAILBOXES; mailbox++)
  {
    CAN.setFilterId_Standard(mailbox, CAN_RX_ID);
  }


  /* Init Can Module */
  if (!CAN.begin(CanBitRate::BR_500k))
  {
    Serial.println("CAN.begin(...) failed.");
    for (;;) {}
  }
}

void loop()
{
  /* -------------- MESSAGE TRANSMISSION -------------- */

  /* Assemble a CAN message with the format of
   * 0xCA 0xFE 0x12 0x34 [2 bytes counter up] [2 bytes counter down]
   */
  uint8_t const msg_data[] = {0xCA,0xFE,0x12,0x34,0,0,0,0};

  /* Increase/decrease the message counters */
  msg_up_cnt++;
  msg_down_cnt--;  

  /* Prepare TX Message */
  CanMsg msg;
  msg.id = CanStandardId(CAN_TX_ID);
  msg.data_length = 8;
  memcpy((void *)(msg.data), msg_data, 8);
  memcpy((void *)(msg.data + 4), &msg_up_cnt, sizeof(msg_up_cnt));
  memcpy((void *)(msg.data + 6), &msg_down_cnt, sizeof(msg_down_cnt));  

  /* Transmit the CAN message, capture and display an
   * error core in case of failure.
   */
  if (int const rc = CAN.write(msg); rc < 0)
  {
    Serial.print  ("CAN.write(...) failed with error code ");
    Serial.println(rc);
    for (;;) { }
  }

  /* -------------- MESSAGE RECEPTION -------------- */
  if (CAN.available())
  {
    /* Read message from bus */
    CanMsg msg = CAN.read();
    /* Pruint on debug serial */
    Serial.println(msg);

  #ifdef EMULATE_IO_CONTROL
    /* Control a 'Virtual' LED using bit 0 of byte 0 */
    led_sts = msg.data[0] & 0x01;
    /* Print status on debug serial */
    if (led_sts)
    {
      Serial.println("LED ON");
    }
    else
    {
      Serial.println("LED OFF");
    }
  #endif  
  }

  /* Cycle delay */
  delay(10);
}

