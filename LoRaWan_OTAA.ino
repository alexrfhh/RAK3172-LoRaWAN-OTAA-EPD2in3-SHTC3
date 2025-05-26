#include "SparkFun_SHTC3.h"
#include "src/rak14000.h"


#define OTAA_PERIOD (10000)

#define OTAA_DEVEUI                                \
   {                                                \
     0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x05, 0x2F, 0x45 \
   }
#define OTAA_APPEUI                                \
  {                                                \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11 \
  }
#define OTAA_APPKEY                                                                                \
  {                                                                                                \
    0x9A, 0xBC, 0x94, 0xF6, 0xE5, 0xE1, 0x47, 0xF8, 0x0A, 0x85, 0x55, 0x08, 0xB5, 0x0E, 0x96, 0x49 \
  }

/** Power control for RAK14000 */
#define POWER_RAK WB_IO2

uint8_t OTAA_BAND = 6;
uint16_t OTAA_MASK = 0x0001;

bool errDown;
uint32_t counter = 0;
float Temp, Hum;

/** Display buffer */
unsigned char image[4000];
/** Instance for paint and write */
Paint paint(image, 122, 250);
/** Color definitions */
uint16_t bg_color = 1;
uint16_t txt_color = 0;
uint16_t uplink_counter;
uint16_t upcount[40] = {};

SHTC3 mySHTC3;
EPD_213_BW display;

void recvCallback(SERVICE_LORA_RECEIVE_T *data)
{
 if (data->BufferSize > 0)
  {
    Serial.println("Something received!");
    for (int i = 0; i < data->BufferSize; i++)
    {
      Serial.printf("%x", data->Buffer[i]);
    }
    Serial.print("\r\n");
  }
}

void joinCallback(int32_t status)
{
  Serial.printf("Join status: %d\r\n", status);
}

void sendCallback(int32_t status)
{
  if (status == 0)
  {
    Serial.println("Successfully sent");
 
  }
  else
  {
    Serial.println("Sending failed");
  }
}

void joinAttempt()
{
  if (api.lorawan.njm.get())
  {
    Serial.println("Wait for LoRaWAN join...");
  
    while (api.lorawan.njs.get() == 0)
    {
      api.lorawan.join();
      delay(5000);
    }
    
    Serial.println("Joined");
  }
}

void setup_lorawan()
{

  Serial.begin(115200, RAK_AT_MODE);

  Serial.println("RAKwireless LoRaWan OTAA Example");
  Serial.println("------------------------------------------------------");

  uint8_t node_device_eui[8] = OTAA_DEVEUI;
  uint8_t node_app_eui[8] = OTAA_APPEUI;
  uint8_t node_app_key[16] = OTAA_APPKEY;

  api.lorawan.njm.set(1);
  api.lorawan.appeui.set(node_app_eui, 8);
  api.lorawan.appkey.set(node_app_key, 16);
  api.lorawan.deui.set(node_device_eui, 8);
  api.lorawan.band.set(OTAA_BAND);
  api.lorawan.mask.set(&OTAA_MASK);
  api.lorawan.deviceClass.set(RAK_LORA_CLASS_A);
  api.lorawan.adr.set(true);
  api.lorawan.dr.set(5);
  api.lorawan.rety.set(0);      //1
  api.lorawan.cfm.set(0);      //1

  /** Check LoRaWan Status*/
  Serial.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");            // Check Duty Cycle status
  Serial.printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED"); // Check Confirm status
  uint8_t assigned_dev_addr[4] = {0};
  api.lorawan.daddr.get(assigned_dev_addr, 4);
  Serial.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]); // Check Device Address
  Serial.printf("Uplink period is %ums\r\n", OTAA_PERIOD);
  Serial.printf("NJM: %d\r\n", api.lorawan.njm.get());
  Serial.println("");
  api.lorawan.registerRecvCallback(recvCallback);
  api.lorawan.registerJoinCallback(joinCallback);
  api.lorawan.registerSendCallback(sendCallback);

}

void errorDecoder(SHTC3_Status_TypeDef message)                             // The errorDecoder function prints "SHTC3_Status_TypeDef" resultsin a human-friendly way
{
  switch(message)
  {
    case SHTC3_Status_Nominal : Serial.println("OK - EPD"); break;
    case SHTC3_Status_Error : Serial.println("Error - EPD"); break;
    case SHTC3_Status_CRC_Fail : Serial.println("CRC Fail - EPD"); break;
    default : Serial.println("Unknown return code - EPD"); break;
  }
}

void clear_rak14000(void)
{
	paint.SetRotate(ROTATE_270);
	display.Init(FULL);
	paint.Clear(1);
}

void rak14000_text(int16_t x, int16_t y, char *text)
{
	sFONT *use_font;
	use_font = &Font16;
	paint.DrawStringAt(x, y, text, use_font, 0);
}

void print_epd(char* text)
{
  clear_rak14000();
  uplink_counter++;
  sprintf((char*)upcount,"Uplink counter:%d", uplink_counter);
  Serial.println((char*)upcount);
  rak14000_text(0, 5,(char*)upcount);
  rak14000_text(0, 30, (char*) text);
  display.Display(image);
}

void setup()
{
  /* The 3V3_S has to be enable via WB_IO2 GPIO. Otherwise, the module will not work. */
  /** Power control for RAK14000 */
  pinMode(POWER_RAK, OUTPUT); 
  pinMode(POWER_RAK, INPUT_PULLUP);
	digitalWrite(POWER_RAK, HIGH); 

  Wire.begin();
  errorDecoder(mySHTC3.begin());

  setup_lorawan();

  //display.Clear();

  clear_rak14000();

	paint.drawBitmap(130, 94, (uint8_t *)alb_img, 120, 28);

  display.Display(image);

  delay(1000);

  
  if (api.system.timer.create(RAK_TIMER_0, (RAK_TIMER_HANDLER)uplink_routine, RAK_TIMER_PERIODIC))
  {
    api.system.timer.start(RAK_TIMER_0, OTAA_PERIOD, NULL);
  }

}

void uplink_routine()
{
  if (api.lorawan.njs.get() == true)
  {
   mySHTC3.update();

  /** Payload of Uplink *//**/
  uint8_t data_len = 0;
  uint8_t tx_buff[30]; 
  

  Temp = mySHTC3.toDegC();
  Hum = mySHTC3.toPercent();
  
  data_len = sprintf ((char*)tx_buff, "T= %.2f H= %.2f",Temp, Hum);

  Serial.println("Data Packet:");
 
  for (int i = 0; i < data_len; i++)
  {
    Serial.printf("0x%02X ", tx_buff[i]);
  }
  Serial.println("");
  
  Serial.println("Data sent:");
  Serial.println((char*)tx_buff);
  //Serial.println(Temp);
  //Serial.println(Hum);

  /** Send the data package */

  if (api.lorawan.send(data_len, tx_buff, 2, true, 1))
  {
    Serial.println("Sending is requested");
    print_epd((char*)tx_buff);
  }
  else
  {
    Serial.println("Sending failed");
    joinAttempt();
  }
  }
  else 
  {
    Serial.println("Can't send uplink, device not joined");
  }
}


void loop()
{
  
  api.system.sleep.all();

}
