/*
 * GSM.c:
 *	GSM access routines
 ***********************************************************************
 * This file is part of wiringOli:
 *	http://morind.free.fr
 *
 *    wiringOli is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringoli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringOli.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */
#include <oliExt.h>
#include <wiringOli.h>
#include <wiringSerial.h>
#include <errno.h>
#include "GSM.h"

char *p_comm_buf;               // Pointer to the communication buffer
int comm_buf_len;               // Number of characters in the buffer
int rx_state;                   // Internal state of rx state machine
uint16_t start_reception_tmout; // Max tmout for starting reception
uint16_t interchar_tmout;       // Previous time in ms
unsigned long prev_time;        // Previous time in ms
int last_speaker_volume;

char comm_buf[COMM_BUF_LEN+1];

int gFD;			// Variables connected with communication buffer

// SIM900 Communication ********************************************************
int GetCommLineStatus(void)
{
  return comm_line_status;
}

void SetCommLineStatus(int new_status)
{
  comm_line_status = new_status;
}

/**********************************************************
Function to enable or disable echo
Echo(1)   enable echo mode
Echo(0)   disable echo mode
**********************************************************/
void Echo(int state)
{
	if (state == 0 || state == 1)
	{
	  SetCommLineStatus(CLS_ATCMD);
	  serialPuts(gFD, "ATE");
	  switch(state)
	  {
		case 0:
		serialPuts(gFD, "0");
		break;
		case 1:
		serialPuts(gFD, "1");
		break;
		default:
		break;
	  }
	  //serialPuts(gFD, (int)state);    
	  serialPuts(gFD, "\r");
	  delay(400);
	  SetCommLineStatus(CLS_FREE);
	}
}

/**********************************************************
Method sends AT command and waits for response

return:
AT_RESP_ERR_NO_RESP = -1,   // no response received
AT_RESP_ERR_DIF_RESP = 0,   // response_string is different from the response
AT_RESP_OK = 1,             // response_string was included in the response
**********************************************************/
char SendATCmdWaitResp(char *AT_cmd_string, uint16_t start_comm_tmout, uint16_t max_interchar_tmout, char const *response_string, int no_of_attempts)
{
	int status;
	char ret_val = AT_RESP_ERR_NO_RESP;
	int i;

	for (i = 0; i < no_of_attempts; i++)
	{
	  // Wait 500ms before sending next repeated AT command so if we have no_of_attempts=1 tmout will not occurred
		if (i > 0) delay(500);
		serialPuts(gFD, AT_cmd_string);
		serialPuts(gFD, "\r");

		status = WaitResp(start_comm_tmout, max_interchar_tmout);

		if (status == RX_FINISHED)
		{
		  // Something was received but what was received?
			if(IsStringReceived(response_string))
			{
				ret_val = AT_RESP_OK;
				break;  // Response is OK => finish
			}
			else ret_val = AT_RESP_ERR_DIF_RESP;
		}
		else
		{
		  // Nothing was received
		  ret_val = AT_RESP_ERR_NO_RESP;
		}
	}
  return (ret_val);
}

/**********************************************************
Method waits for response

start_comm_tmout    - maximum waiting time for receiving the first response
character (in msec.)
max_interchar_tmout - maximum tmout between incoming characters
in msec.
return:
RX_FINISHED         finished, some character was received
RX_TMOUT_ERR        finished, no character received
initial communication tmout occurred
**********************************************************/
int WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
	int status;

	RxInit(start_comm_tmout, max_interchar_tmout);
	// Wait until response is not finished
	do
	{
	  status = IsRxFinished();
	}
	while (status == RX_NOT_FINISHED);
	return (status);
}

/**********************************************************
Method waits for response with specific response string

start_comm_tmout    - maximum waiting time for receiving the first response
character (in msec.)
max_interchar_tmout - maximum tmout between incoming characters
in msec.
expected_resp_string - expected string
return:
RX_FINISHED_STR_RECV,     finished and expected string received
RX_FINISHED_STR_NOT_RECV  finished, but expected string not received
RX_TMOUT_ERR              finished, no character received
initial communication tmout occurred
**********************************************************/
int WaitRespAdd(uint16_t start_comm_tmout, uint16_t max_interchar_tmout, char const *expected_resp_string)
{
  int status;
  int ret_val;

  RxInit(start_comm_tmout, max_interchar_tmout);
  // Wait until response is not finished
  do
  {
    status = IsRxFinished();
  } while (status == RX_NOT_FINISHED);
	if (status == RX_FINISHED)
	{
	  // Something was received but what was received?
		if(IsStringReceived(expected_resp_string))
		{
		  // Expected string was received
		  ret_val = RX_FINISHED_STR_RECV;
		}
		else ret_val = RX_FINISHED_STR_NOT_RECV;
	}
	else
	{
		// nothing was received
		ret_val = RX_TMOUT_ERR;
	}
  return (ret_val);
}

/**********************************************************
Initializes receiving process

start_comm_tmout    - maximum waiting time for receiving the first response character (in ms)
max_interchar_tmout - maximum tmout between incoming characters in ms
if there is no other incoming character longer then specified
tmout(in ms) receiving process is considered as finished
**********************************************************/
void RxInit(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
	rx_state = RX_NOT_STARTED;
	start_reception_tmout = start_comm_tmout;
	interchar_tmout = max_interchar_tmout;
	prev_time = millis();
	comm_buf[0] = 0x00; 		// End of string
	p_comm_buf = &comm_buf[0];
	comm_buf_len = 0;
}

/**********************************************************
Method checks if receiving process is finished or not.
Rx process is finished if defined inter-character tmout is reached

returns:
RX_NOT_FINISHED = 0,// not finished yet
RX_FINISHED,        // finished - inter-character tmout occurred
RX_TMOUT_ERR,       // initial communication tmout occurred
**********************************************************/
int IsRxFinished(void)
{
	int num_of_bytes;
	int ret_val = RX_NOT_FINISHED;  // Default not finished
	// Rx state machine
  if (rx_state == RX_NOT_STARTED)
  {
	  // Reception is not started yet - check tmout
	  if (!serialDataAvail(gFD))
	  {
  	  // Still no character received => check timeout
  		if ((unsigned long)(millis() - prev_time) >= start_reception_tmout)
  		{
  			// Timeout elapsed => GSM module didn't start with response so communication is takes as finished
  			comm_buf[comm_buf_len] = 0x00;
  			ret_val = RX_TMOUT_ERR;
  		}
  	}
  	else
  	{
  		// At least one character received => so init inter-character counting process again and go to the next state
  		prev_time = millis(); // Init tmout for inter-character space
  		rx_state = RX_ALREADY_STARTED;
  	}
  }
	if (rx_state == RX_ALREADY_STARTED)
	{
		// Reception already started
		// check new received bytes
		// only in case we have place in the buffer
		num_of_bytes = serialDataAvail(gFD);
		// If there are some received bytes postpone the timeout
		if (num_of_bytes) prev_time = millis();
		// read all received bytes
		while (num_of_bytes)
		{
			num_of_bytes--;
			*p_comm_buf = serialGetchar(gFD);
			p_comm_buf++;
			comm_buf_len++;
			comm_buf[comm_buf_len] = 0x00;  // And finish currently received characters
		}
		// Finally check the inter-character timeout
		if ((unsigned long)(millis() - prev_time) >= interchar_tmout)
		{
			// Timeout between received character was reached reception is finished
			comm_buf[comm_buf_len] = 0x00;  // For sure finish string again but it is not necessary
			ret_val = RX_FINISHED;
		}
	}
  return (ret_val);
}

/**********************************************************
Method checks received bytes

compare_string - pointer to the string which should be find

return: 0 - string was NOT received
        1 - string was received
**********************************************************/
int IsStringReceived(char const *compare_string)
{
	char *ch;
	int ret_val = 0;

	if(comm_buf_len)
	{
	  ch = strstr((char *)comm_buf, compare_string);
		if (ch != NULL)
		{
			ret_val = 1;
		}
		else
		{
			//printf("%s\n",*ch);
		}
	}
  return (ret_val);
}

// Serial port *****************************************************************
void serialBegin(long baud)
{
  if ((gFD = serialOpen("/dev/ttyS4", baud, 8, 1)) < 0)
  {
    fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
  }
}

void serialEnd()
{
  serialClose(gFD);
}

long checkBaud()
{
  int i;
  long baud = 9600;
  if (AT_RESP_ERR_DIF_RESP == SendATCmdWaitResp("AT", 500, 100, "OK", 5)) 
	{		
		for (i=1; i<7; i++)
		{
			switch (i) 
			{
				case 1:
					baud = 4800;
				break;
				case 2:
					baud = 9600;
				break;
				case 3:
					baud = 19200;
				break;
				case 4:
					baud = 38400;
				break;
				case 5:
					baud = 57600;
				break;
				case 6:
					baud = 115200;
				break;
			}
			delay(100);
			serialPuts(gFD, "AT+IPR=");
			serialPuts(gFD, ChangeIToS(baud));
			serialPuts(gFD, "\r"); // Send <CR>
			delay(400);
      if (AT_RESP_OK == SendATCmdWaitResp("AT", 500, 100, "OK", 5))
			{
				return(baud);
			}
		}
  }
  return(0);
}

// Others **********************************************************************
char *ChangeIToS(int IntNU)
{
  char *String = (char*)malloc(80);
	sprintf(String, "%d", IntNU);
	return(String);
}

// GSM Power *******************************************************************
void powerOn()
{
  SetCommLineStatus(CLS_ATCMD);
  digitalWriteSIM900_ON(LOW);
  digitalWriteSIM900_RST(LOW);
  
  // generate turn on pulse
  printf("Power On\n");
	digitalWriteSIM900_ON(HIGH);
	delay(1200);
	//sleep(2);
	digitalWriteSIM900_ON(LOW);
	delay(5000);
  SetCommLineStatus(CLS_FREE);
}

void reset()
{
  digitalWriteSIM900_RST(LOW);
  delay(10);
  digitalWriteSIM900_RST(HIGH);
  delay(100);
  digitalWriteSIM900_RST(LOW);
}

int isOn()
{
  if(AT_RESP_OK == SendATCmdWaitResp("AT", 500, 100, "OK", 5))
    return(1);
  else
    return(0);
}

// GSM *************************************************************************
void init()
{
  SetCommLineStatus(CLS_FREE);
  InitParam(PARAM_SET_0);
  InitParam(PARAM_SET_1);
}

/**********************************************************
  Sends parameters for initialization of GSM module

  group:  0 - parameters of group 0 - not necessary to be registered in the GSM
          1 - parameters of group 1 - it is necessary to be registered
**********************************************************/
void InitParam(int group)
{
  switch (group) 
  {
    case PARAM_SET_0:
      // Check comm line
      if (CLS_FREE != GetCommLineStatus()) return;
      SetCommLineStatus(CLS_ATCMD);
      // Must wait a little bit
      delay(400);
      // Reset to the factory settings
      SendATCmdWaitResp("AT&F", 1000, 50, "OK", 5);
      // switch off echo
      //SendATCmdWaitResp("ATE0", 500, 50, "OK", 5);
      // setup fixed baud rate
      //SendATCmdWaitResp("AT+IPR=9600", 500, 50, "OK", 5);
      // setup mode
      //SendATCmdWaitResp("AT#SELINT=1", 500, 50, "OK", 5);
      // Switch ON User LED - just as signalization we are here
      //SendATCmdWaitResp("AT#GPIO=8,1,1", 500, 50, "OK", 5);
      // Sets GPIO9 as an input = user button
      //SendATCmdWaitResp("AT#GPIO=9,0,0", 500, 50, "OK", 5);
      // allow audio amplifier control
      //SendATCmdWaitResp("AT#GPIO=5,0,2", 500, 50, "OK", 5);
      // Switch OFF User LED- just as signalization we are finished
      //SendATCmdWaitResp("AT#GPIO=8,0,1", 500, 50, "OK", 5);
      SetCommLineStatus(CLS_FREE);
      break;

    case PARAM_SET_1:
      // Check comm line
      if (CLS_FREE != GetCommLineStatus()) return;
      SetCommLineStatus(CLS_ATCMD);
      // Must wait a little bit
      delay(400);
      // Request calling line identification
      SendATCmdWaitResp("AT+CLIP=1", 500, 50, "OK", 5);
      // Mobile Equipment Error Code
      SendATCmdWaitResp("AT+CMEE=0", 500, 50, "OK", 5);
      // Echo canceller enabled 
      //SendATCmdWaitResp("AT#SHFEC=1", 500, 50, "OK", 5);
      // Ringer tone select (0 to 32)
      //SendATCmdWaitResp("AT#SRS=26,0", 500, 50, "OK", 5);
      // Microphone gain (0 to 7) - response here sometimes takes
      // more than 500msec. so 1000msec. is more safety
      //SendATCmdWaitResp("AT#HFMICG=7", 1000, 50, "OK", 5);
      // set the SMS mode to text
      SendATCmdWaitResp("AT+CMGF=1", 500, 50, "OK", 5);
      // Auto answer after first ring enabled
      // auto answer is not used
      //SendATCmdWaitResp("ATS0=1", 500, 50, "OK", 5);
      // select ringer path to handsfree
      //SendATCmdWaitResp("AT#SRP=1", 500, 50, "OK", 5);
      // select ringer sound level
      //SendATCmdWaitResp("AT+CRSL=2", 500, 50, "OK", 5);
      // we must release comm line because SetSpeakerVolume()
      // checks comm line if it is free
      SetCommLineStatus(CLS_FREE);
      // select speaker volume (0 to 14)
      //SetSpeakerVolume(9);
      // init SMS storage
      InitSMSMemory();
      // select phonebook memory storage
      SendATCmdWaitResp("AT+CPBS=\"SM\"", 1000, 50, "OK", 5);
      break;
  }
}

/**********************************************************
Method initializes memory for the incoming SMS in the Telit
module - SMSs will be stored in the SIM card

!!This function is used internally after first registration
so it is not necessary to used it in the user sketch

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free

        OK ret val:
        -----------
        0 - SMS memory was not initialized
        1 - SMS memory was initialized

**********************************************************/
char InitSMSMemory() 
{
  char ret_val = -1;

  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  ret_val = 0; // not initialized yet

  // Disable messages about new SMS from the GSM module 
  SendATCmdWaitResp("AT+CNMI=2,0", 1000, 50, "OK", 2);

  // send AT command to init memory for SMS in the SIM card
  // response:
  // +CPMS: <usedr>,<totalr>,<usedw>,<totalw>,<useds>,<totals>
  if (AT_RESP_OK == SendATCmdWaitResp("AT+CPMS=\"SM\",\"SM\",\"SM\"", 2000, 2000, "+CPMS:", 10)) 
  {
    ret_val = 1;
  }
  else ret_val = 0;

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method sends SMS

number_str:   pointer to the phone number string
message_str:  pointer to the SMS text string


return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0 - SMS was not sent
        1 - SMS was sent


an example of usage:
        GSM gsm;
        gsm.SendSMS("00XXXYYYYYYYYY", "SMS text");
**********************************************************/
char SendSMS(char *number_str, char *message_str) 
{
  char ret_val = -1;
  char bytesToSend = 0;
  int i;

  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);  
  ret_val = 0; // Still not send
  // Try to send SMS 3 times in case there is some problem
  for (i = 0; i < 3; i++) 
  {
    // send  AT+CMGS="number_str"
    serialPuts(gFD, "AT+CMGS=\"");
    serialPuts(gFD, number_str);
    serialPuts(gFD, "\"\r");
    delay(400);
    // 1000 msec. for initial comm tmout
    // 50 msec. for inter character timeout
    if (RX_FINISHED_STR_RECV == WaitRespAdd(1000, 50, ">")) 
	  {
      // Send SMS text
      serialPuts(gFD, message_str);
      delay(400);
      // Ctrl-Z
      bytesToSend = 26;
      serialPutchar(gFD, bytesToSend);
      if (RX_FINISHED_STR_RECV == WaitRespAdd(7000, 5000, "+CMGS")) 
      {
        // SMS was send correctly 
        ret_val = 1;
        break;
      }
      else continue;
    }
	  else 
	  {
	    // try again
	    continue;
    }
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method sends SMS to the specified SIM phonebook position

sim_phonebook_position:   SIM phonebook position <1..20>
message_str:              pointer to the SMS text string


return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - specified position must be > 0

        OK ret val:
        -----------
        0 - SMS was not sent
        1 - SMS was sent


an example of usage:
        GSM gsm;
        gsm.SendSMS(1, "SMS text");
**********************************************************/
char SendSMSSpecified(int sim_phonebook_position, char *message_str) 
{
  char ret_val = -1;
  char sim_phone_number[20];
  // SMS is not send yet
  ret_val = 0; 
  if (sim_phonebook_position == 0) return (-3);
  if (1 == GetPhoneNumber(sim_phonebook_position, sim_phone_number)) 
  {
    // There is a valid number at the spec. SIM position
    ret_val = SendSMS(sim_phone_number, message_str);
  }
  return (ret_val);
}

/**********************************************************
Method finds out if there is present at least one SMS with
specified status

Note:
if there is new SMS before IsSMSPresent() is executed
this SMS has a status UNREAD and then
after calling IsSMSPresent() method status of SMS
is automatically changed to READ

required_status:  SMS_UNREAD  - new SMS - not read yet
                  SMS_READ    - already read SMS                  
                  SMS_ALL     - all stored SMS

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout

        OK ret val:
        -----------
        0 - there is no SMS with specified status
        1..20 - position where SMS is stored 
                (suitable for the function GetGSM())


an example of use:
        GSM gsm;
        char position;  
        char phone_number[20]; // array for the phone number string
        char sms_text[100];

        position = gsm.IsSMSPresent(SMS_UNREAD);
        if (position) {
          // read new SMS
          gsm.GetGSM(position, phone_num, sms_text, 100);
          // now we have phone number string in phone_num
          // and SMS text in sms_text
        }
**********************************************************/
char IsSMSPresent(int required_status) 
{
  char ret_val = -1;
  char *p_char;
  int status;
  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  ret_val = 0; // Still not present
  switch (required_status) 
  {
    case SMS_UNREAD:
      serialPuts(gFD, "AT+CMGL=\"REC UNREAD\",1\r");
      break;
    case SMS_READ:
      serialPuts(gFD, "AT+CMGL=\"REC READ\",1\r");
      break;
    case SMS_ALL:
      serialPuts(gFD, "AT+CMGL=\"ALL\",1\r");
      break;
  }
  // 5s for initial comm tmout and max. 1500ms for inter character timeout
  RxInit(5000, 1500);
  // Wait response is finished
  do 
  {
    if (IsStringReceived("OK"))
    {
      // perfect - we have some response, but what:

      // there is either NO SMS:
      // <CR><LF>OK<CR><LF>

      // or there is at least 1 SMS
      // +CMGL: <index>,<stat>,<oa/da>,,[,<tooa/toda>,<length>]
      // <CR><LF> <data> <CR><LF>OK<CR><LF>
      status = RX_FINISHED;
      break; // so finish receiving immediately and let's go to check response 
    }
    status = IsRxFinished();
  } while (status == RX_NOT_FINISHED);
  switch (status) 
  {
    case RX_TMOUT_ERR:
      // Response was not received in specific time
      ret_val = -2;
      break;
    case RX_FINISHED:
      // Something was received but what was received?
      if(IsStringReceived("+CMGL:"))
      {
        // There is some SMS with status => get its position
        // response is:
        // +CMGL: <index>,<stat>,<oa/da>,,[,<tooa/toda>,<length>]
        // <CR><LF> <data> <CR><LF>OK<CR><LF>
        p_char = strchr((char *)comm_buf, ':');
        if (p_char != NULL) 
        {
          ret_val = atoi(p_char+1);
        }
      }
      else 
	    {
        // Other response like OK or ERROR
        ret_val = 0;
      }

      // Here we have WaitResp() just for generation tmout 20msec. in case OK was detected not due to receiving
      WaitResp(20, 20); 
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method reads SMS from specified memory(SIM) position

position:     SMS position <1..20>
phone_number: a pointer where the phone number string of received SMS will be placed
              so the space for the phone number string must be reserved - see example
SMS_text  :   a pointer where SMS text will be placed
max_SMS_len:  maximum length of SMS text excluding also string terminating 0x00 character
              
return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - specified position must be > 0

        OK ret val:
        -----------
        GETSMS_NO_SMS       - no SMS was not found at the specified position
        GETSMS_UNREAD_SMS   - new SMS was found at the specified position
        GETSMS_READ_SMS     - already read SMS was found at the specified position
        GETSMS_OTHER_SMS    - other type of SMS was found 


an example of usage:
        GSM gsm;
        char position;
        char phone_num[20]; // array for the phone number string
        char sms_text[100]; // array for the SMS text string

        position = gsm.IsSMSPresent(SMS_UNREAD);
        if (position) {
          // there is new SMS => read it
          gsm.GetGSM(position, phone_num, sms_text, 100);
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG SMS phone number: ", 0);
            gsm.DebugPrint(phone_num, 0);
            gsm.DebugPrint("\r\n          SMS text: ", 0);
            gsm.DebugPrint(sms_text, 1);
          #endif
        }        
**********************************************************/
char GetSMS(int position, char *phone_number, char *SMS_text, int max_SMS_len) 
{
  char ret_val = -1;
  char *p_char; 
  char *p_char1;
  int len;
  int status;
  
  if (position == 0) return(-3);
  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  phone_number[0] = 0;  // end of string for now
  ret_val = GETSMS_NO_SMS; // still no SMS
  //send "AT+CMGR=X" - where X = position
  serialPuts(gFD, "AT+CMGR=");
  serialPuts(gFD, ChangeIToS(position));
  serialPuts(gFD, "\r");
  // Must wait a little bit
  delay(400);
  // 5000 msec. for initial comm tmout
  // 100 msec. for inter character tmout
  status = WaitRespAdd(5000, 100, "+CMGR:");
  switch (status) 
  {
    case RX_TMOUT_ERR:
      // response was not received in specific time
      ret_val = -2;
      break;
    case RX_FINISHED_STR_NOT_RECV:
      // OK was received => there is NO SMS stored in this position
      if(IsStringReceived("OK")) 
      {
        // there is only response <CR><LF>OK<CR><LF> 
        // => there is NO SMS
        ret_val = GETSMS_NO_SMS;
      }
      else if(IsStringReceived("ERROR")) 
      {
        // error should not be here but for sure
        ret_val = GETSMS_NO_SMS;
      }
      break;
    case RX_FINISHED_STR_RECV:
      // find out what was received exactly

      //response for new SMS:
      //<CR><LF>+CMGR: "REC UNREAD","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
		  //There is SMS text<CR><LF>OK<CR><LF>
      if(IsStringReceived("\"REC UNREAD\"")) 
      { 
        // get phone number of received SMS: parse phone number string 
        // +XXXXXXXXXXXX
        // -------------------------------------------------------
        ret_val = GETSMS_UNREAD_SMS;
      }
      //response for already read SMS = old SMS:
      //<CR><LF>+CMGR: "REC READ","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
		  //There is SMS text<CR><LF>
      else if(IsStringReceived("\"REC READ\"")) 
      {
        // get phone number of received SMS
        ret_val = GETSMS_READ_SMS;
      }
      else 
      {
        // other type like stored for sending.. 
        ret_val = GETSMS_OTHER_SMS;
      }

      // extract phone number string
      p_char = strchr((char *)(comm_buf), ',');
      p_char1 = p_char+2; // we are on the first phone number character
      p_char = strchr((char *)(p_char1), '"');
      if (p_char != NULL) 
      {
        *p_char = 0; // end of string
        strcpy(phone_number, (char *)(p_char1));
      }

      // get SMS text and copy this text to the SMS_text buffer
      p_char = strchr(p_char+1, 0x0a);  // find <LF>
      if (p_char != NULL) 
      {
        // next character after <LF> is the first SMS character
        p_char++; // now we are on the first SMS character 

        // find <CR> as the end of SMS string
        p_char1 = strchr((char *)(p_char), 0x0d);  
        if (p_char1 != NULL) 
        {
          // finish the SMS text string 
          // because string must be finished for right behaviour 
          // of next strcpy() function
          *p_char1 = 0; 
        }
        // in case there is not finish sequence <CR><LF> because the SMS is
        // too long (more then 130 characters) sms text is finished by the 0x00
        // directly in the WaitResp() routine

        // find out length of the SMS (excluding 0x00 termination character)
        len = strlen(p_char);

        if (len < max_SMS_len) 
        {
          // buffer SMS_text has enough place for copying all SMS text
          // so copy whole SMS text
          // from the beginning of the text(=p_char position) 
          // to the end of the string(= p_char1 position)
          strcpy(SMS_text, (char *)(p_char));
        }
        else 
        {
          // buffer SMS_text doesn't have enough place for copying all SMS text
          // so cut SMS text to the (max_SMS_len-1)
          // (max_SMS_len-1) because we need 1 position for the 0x00 as finish 
          // string character
          memcpy(SMS_text, (char *)(p_char), (max_SMS_len-1));
          SMS_text[max_SMS_len] = 0; // finish string
        }
      }
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method reads SMS from specified memory(SIM) position and
makes authorization - it means SMS phone number is compared
with specified SIM phonebook position(s) and in case numbers
match GETSMS_AUTH_SMS is returned, otherwise GETSMS_NOT_AUTH_SMS
is returned

position:     SMS position to be read <1..20>
phone_number: a pointer where the tel. number string of received SMS will be placed
              so the space for the phone number string must be reserved - see example
SMS_text  :   a pointer where SMS text will be placed
max_SMS_len:  maximum length of SMS text excluding terminating 0x00 character

first_authorized_pos: initial SIM phonebook position where the authorization process
                      starts
last_authorized_pos:  last SIM phonebook position where the authorization process
                      finishes

                      Note(important):
                      ================
                      In case first_authorized_pos=0 and also last_authorized_pos=0
                      the received SMS phone number is NOT authorized at all, so every
                      SMS is considered as authorized (GETSMS_AUTH_SMS is returned)
              
return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        GETSMS_NO_SMS           - no SMS was found at the specified position
        GETSMS_NOT_AUTH_SMS     - NOT authorized SMS found at the specified position
        GETSMS_AUTH_SMS         - authorized SMS found at the specified position


an example of usage:
        GSM gsm;
        char phone_num[20]; // array for the phone number string
        char sms_text[100]; // array for the SMS text string

        // authorize SMS with SIM phonebook positions 1..3
        if (GETSMS_AUTH_SMS == gsm.GetAuthorizedSMS(1, phone_num, sms_text, 100, 1, 3)) {
          // new authorized SMS was detected at the SMS position 1
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG SMS phone number: ", 0);
            gsm.DebugPrint(phone_num, 0);
            gsm.DebugPrint("\r\n          SMS text: ", 0);
            gsm.DebugPrint(sms_text, 1);
          #endif
        }

        // don't authorize SMS with SIM phonebook at all
        if (GETSMS_AUTH_SMS == gsm.GetAuthorizedSMS(1, phone_num, sms_text, 100, 0, 0)) {
          // new SMS was detected at the SMS position 1
          // because authorization was not required
          // SMS is considered authorized
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG SMS phone number: ", 0);
            gsm.DebugPrint(phone_num, 0);
            gsm.DebugPrint("\r\n          SMS text: ", 0);
            gsm.DebugPrint(sms_text, 1);
          #endif
        }
**********************************************************/
char GetAuthorizedSMS(int position, char *phone_number, char *SMS_text, int max_SMS_len, int first_authorized_pos, int last_authorized_pos)
{
  char ret_val = -1;
  int i; 

  ret_val = GetSMS(position, phone_number, SMS_text, max_SMS_len);
  if (ret_val < 0) 
  {
    // here is ERROR return code => finish
  }
  else if (ret_val == GETSMS_NO_SMS) 
  {
    // no SMS detected => finish
  }
  else if (ret_val == GETSMS_READ_SMS)
  {
    // now SMS can has only READ attribute because we have already read
    // this SMS at least once by the previous function GetSMS()
    //
    // new READ SMS was detected on the specified SMS position =>
    // make authorization now
    if ((first_authorized_pos == 0) && (last_authorized_pos == 0)) 
    {
      // authorization is not required => it means authorization is OK
      ret_val = GETSMS_AUTH_SMS;
    }
    else 
    {
      ret_val = GETSMS_NOT_AUTH_SMS;  // authorization not valid yet
      for (i = first_authorized_pos; i <= last_authorized_pos; i++) 
      {
        if (ComparePhoneNumber(i, phone_number)) 
        {
          // phone numbers are identical
          // authorization is OK
          ret_val = GETSMS_AUTH_SMS;
          break;  // and finish authorization
        }
      }
    }
  }
  return (ret_val);
}

/**********************************************************
Method deletes SMS from the specified SMS position

position:     SMS position <1..20>

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        0 - SMS was not deleted
        1 - SMS was deleted
**********************************************************/
char DeleteSMS(int position) 
{
  char ret_val = -1;

  if (position == 0) return (-3);
  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  ret_val = 0; // not deleted yet
  
  //send "AT+CMGD=XY" - where XY = position
  serialPuts(gFD, "AT+CMGD=");
  serialPuts(gFD, ChangeIToS((int)position));  
  serialPuts(gFD, "\r");
  delay(400);
  // 5000 msec. for initial comm tmout
  // 20 msec. for inter character timeout
  switch (WaitRespAdd(5000, 50, "OK")) 
  {
    case RX_TMOUT_ERR:
      // response was not received in specific time
      ret_val = -2;
      break;
    case RX_FINISHED_STR_RECV:
      // OK was received => SMS deleted
      ret_val = 1;
      break;
    case RX_FINISHED_STR_NOT_RECV:
      // other response: e.g. ERROR => SMS was not deleted
      ret_val = 0; 
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method delete all SMS

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        0 - SMS was not deleted
        1 - SMS was deleted
**********************************************************/
char DeleteAllSMS() 
{
  char ret_val = -1;

  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  ret_val = 0; // not deleted yet
  
  //send "AT+CMGD=1,4" - where XY = position
  serialPuts(gFD, "AT+CMGD=1,4\r");
  delay(400);
  // 5000 msec. for initial comm tmout
  // 20 msec. for inter character timeout
  switch (WaitRespAdd(5000, 50, "OK")) 
  {
    case RX_TMOUT_ERR:
      // response was not received in specific time
      ret_val = -2;
      break;
    case RX_FINISHED_STR_RECV:
      // OK was received => SMS deleted
      ret_val = 1;
      break;
    case RX_FINISHED_STR_NOT_RECV:
      // other response: e.g. ERROR => SMS was not deleted
      ret_val = 0; 
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method reads phone number string from specified SIM position

position:     SMS position <1..20>

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0
        phone_number is empty string

        OK ret val:
        -----------
        0 - there is no phone number on the position
        1 - phone number was found
        phone_number is filled by the phone number string finished by 0x00
                     so it is necessary to define string with at least
                     15 bytes(including also 0x00 termination character)

an example of usage:
        GSM gsm;
        char phone_num[20]; // array for the phone number string

        if (1 == gsm.GetPhoneNumber(1, phone_num)) {
          // valid phone number on SIM pos. #1 
          // phone number string is copied to the phone_num array
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG phone number: ", 0);
            gsm.DebugPrint(phone_num, 1);
          #endif
        }
        else {
          // there is not valid phone number on the SIM pos.#1
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG there is no phone number", 1);
          #endif
        }
**********************************************************/
char GetPhoneNumber(int position, char *phone_number)
{
  char ret_val = -1;

  char *p_char; 
  char *p_char1;

  if (position == 0) return (-3);
  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  ret_val = 0; // not found yet
  phone_number[0] = 0; // phone number not found yet => empty string
  
  //send "AT+CPBR=XY" - where XY = position
  serialPuts(gFD, "AT+CPBR=");
  serialPuts(gFD, ChangeIToS((int)position));  
  serialPuts(gFD, "\r");
  delay(400);
  // 5000 msec. for initial comm tmout
  // 50 msec. for inter character timeout
  switch (WaitRespAdd(5000, 50, "+CPBR")) 
  {
    case RX_TMOUT_ERR:
      // response was not received in specific time
      ret_val = -2;
      break;
    case RX_FINISHED_STR_RECV:
      // response in case valid phone number stored:
      // <CR><LF>+CPBR: <index>,<number>,<type>,<text><CR><LF>
      // <CR><LF>OK<CR><LF>

      // response in case there is not phone number:
      // <CR><LF>OK<CR><LF>
      p_char = strchr((char *)(comm_buf),'"');
      if (p_char != NULL) 
      {
        p_char++;       // we are on the first phone number character
        // find out '"' as finish character of phone number string
        p_char1 = strchr((char *)(p_char),'"');
        if (p_char1 != NULL) 
        {
          *p_char1 = 0; // end of string
        }
        // extract phone number string
        strcpy(phone_number, (char *)(p_char));
        // output value = we have found out phone number string
        ret_val = 1;
      }
      break;
    case RX_FINISHED_STR_NOT_RECV:
      // only OK or ERROR => no phone number
      ret_val = 0; 
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method writes phone number string to the specified SIM position

position:     SMS position <1..20>
phone_number: phone number string for the writing

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        0 - phone number was not written
        1 - phone number was written
**********************************************************/
char WritePhoneNumber(int position, char *phone_number)
{
  char ret_val = -1;

  if (position == 0) return (-3);
  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  ret_val = 0; // phone number was not written yet
  
  //send: AT+CPBW=XY,"00420123456789"
  // where XY = position,
  //       "00420123456789" = phone number string
  serialPuts(gFD, "AT+CPBW=");
  serialPuts(gFD, ChangeIToS((int)position));  
  serialPuts(gFD, ",\"");
  serialPuts(gFD, phone_number);
  serialPuts(gFD, "\"\r");
  delay(400);
  // 5000 msec. for initial comm tmout
  // 50 msec. for inter character timeout
  switch (WaitRespAdd(5000, 50, "OK")) 
  {
    case RX_TMOUT_ERR:
      // response was not received in specific time
      break;
    case RX_FINISHED_STR_RECV:
      // response is OK = has been written
      ret_val = 1;
      break;
    case RX_FINISHED_STR_NOT_RECV:
      // other response: e.g. ERROR
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method del phone number from the specified SIM position

position:     SMS position <1..20>

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        0 - phone number was not deleted
        1 - phone number was deleted
**********************************************************/
char DelPhoneNumber(int position)
{
  char ret_val = -1;

  if (position == 0) return (-3);
  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  ret_val = 0; // phone number was not written yet
  
  //send: AT+CPBW=XY
  // where XY = position
  serialPuts(gFD, "AT+CPBW=");
  serialPuts(gFD, ChangeIToS((int)position));  
  serialPuts(gFD, "\r");
  delay(400);
  // 5000 msec. for initial comm tmout
  // 50 msec. for inter character timeout
  switch (WaitRespAdd(5000, 50, "OK"))
  {
    case RX_TMOUT_ERR:
      // response was not received in specific time
      break;
    case RX_FINISHED_STR_RECV:
      // response is OK = has been written
      ret_val = 1;
      break;
    case RX_FINISHED_STR_NOT_RECV:
      // other response: e.g. ERROR
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Function compares specified phone number string 
with phone number stored at the specified SIM position

position:       SMS position <1..20>
phone_number:   phone number string which should be compare

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        0 - phone numbers are different
        1 - phone numbers are the same


an example of usage:
        if (1 == gsm.ComparePhoneNumber(1, "123456789")) {
          // the phone num. "123456789" is stored on the SIM pos. #1
          // phone number string is copied to the phone_num array
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG phone numbers are the same", 1);
          #endif
        }
        else {
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG phone numbers are different", 1);
          #endif
        }
**********************************************************/
char ComparePhoneNumber(int position, char *phone_number)
{
  char ret_val = -1;
  char sim_phone_number[20];

  ret_val = 0; // numbers are not the same so far
  if (position == 0) return (-3);
  if (1 == GetPhoneNumber(position, sim_phone_number)) 
  {
    // there is a valid number at the spec. SIM position
    // -------------------------------------------------
    if (0 == strcmp(phone_number, sim_phone_number)) 
    {
      // phone numbers are the same
      ret_val = 1;
    }
  }
  return (ret_val);
}

/**********************************************************
Method sets speaker volume

speaker_volume: volume in range 0..14

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module did not answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0..14 current speaker volume 
**********************************************************/
char SetSpeakerVolume(int speaker_volume)
{
  
  char ret_val = -1;

  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  // remember set value as last value
  if (speaker_volume > 14) speaker_volume = 14;
  // select speaker volume (0 to 14)
  // AT+CLVL=X<CR>   X<0..14>
  serialPuts(gFD, "AT+CLVL=");
  serialPuts(gFD, ChangeIToS((int)speaker_volume));    
  serialPuts(gFD, "\r"); // send <CR>
  // 10 sec. for initial comm tmout
  // 50 msec. for inter character timeout
  if (RX_TMOUT_ERR == WaitResp(10000, 50)) 
  {
    ret_val = -2; // ERROR
  }
  else 
  {
    if(IsStringReceived("OK")) 
    {
      last_speaker_volume = speaker_volume;
      ret_val = last_speaker_volume; // OK
    }
    else ret_val = -3; // ERROR
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method increases speaker volume

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module did not answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0..14 current speaker volume 
**********************************************************/
char IncSpeakerVolume()
{
  char ret_val;
  int current_speaker_value;

  current_speaker_value = last_speaker_volume;
  if (current_speaker_value < 14) 
  {
    current_speaker_value++;
    ret_val = SetSpeakerVolume(current_speaker_value);
  }
  else ret_val = 14;

  return (ret_val);
}

/**********************************************************
Method decreases speaker volume

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module did not answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0..14 current speaker volume 
**********************************************************/
char DecSpeakerVolume()
{
  char ret_val;
  int current_speaker_value;

  current_speaker_value = last_speaker_volume;
  if (current_speaker_value > 0) 
  {
    current_speaker_value--;
    ret_val = SetSpeakerVolume(current_speaker_value);
  }
  else ret_val = 0;

  return (ret_val);
}

/**********************************************************
Method sends DTMF signal
This function only works when call is in progress

dtmf_tone: tone to send 0..15

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0.. tone
**********************************************************/
char SendDTMFSignal(int dtmf_tone)
{
  char ret_val = -1;

  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  // e.g. AT+VTS=5<CR>
  serialPuts(gFD, "AT+VTS=");
  serialPuts(gFD, ChangeIToS((int)dtmf_tone));    
  serialPuts(gFD, "\r");
  // 1 sec. for initial comm tmout
  // 50 msec. for inter character timeout
  if (RX_TMOUT_ERR == WaitResp(1000, 50)) 
  {
    ret_val = -2; // ERROR
  }
  else 
  {
    if(IsStringReceived("OK")) 
    {
      ret_val = dtmf_tone; // OK
    }
    else ret_val = -3; // ERROR
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

int IsUserButtonEnable() 
{
	return (module_status & STATUS_USER_BUTTON_ENABLE);
}

void DisableUserButton() 
{
	module_status &= ~STATUS_USER_BUTTON_ENABLE;
}

void EnableUserButton() 
{
	module_status |= STATUS_USER_BUTTON_ENABLE;
}

/**********************************************************
Method returns state of user button


return: 0 - not pushed = released
        1 - pushed
**********************************************************/
int IsUserButtonPushed()
{
  if (CLS_FREE != GetCommLineStatus()) return(0);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  return(0);
}

/**********************************************************
Method picks up an incoming call

return:
**********************************************************/
void PickUp()
{
  if (CLS_FREE != GetCommLineStatus()) return;
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  serialPuts(gFD, "ATA");
  SetCommLineStatus(CLS_FREE);
}

/**********************************************************
Method hangs up incoming or active call

return:
**********************************************************/
void HangUp()
{
  if (CLS_FREE != GetCommLineStatus()) return;
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  serialPuts(gFD, "ATH");
  SetCommLineStatus(CLS_FREE);
}

/**********************************************************
Method checks status of call

return:
      CALL_NONE         - no call activity
      CALL_INCOM_VOICE  - incoming voice
      CALL_ACTIVE_VOICE - active voice
      CALL_NO_RESPONSE  - no response to the AT command 
      CALL_COMM_LINE_BUSY - comm line is not free
**********************************************************/
int CallStatus()
{
  int ret_val = CALL_NONE;

  if (CLS_FREE != GetCommLineStatus()) return (CALL_COMM_LINE_BUSY);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  serialPuts(gFD, "AT+CPAS");

  // 5 sec. for initial comm tmout
  // 50 msec. for inter character timeout
  if (RX_TMOUT_ERR == WaitResp(5000, 50)) 
  {
    // nothing was received (RX_TMOUT_ERR)
    ret_val = CALL_NO_RESPONSE;
  }
  else
  {
    // something was received but what was received?
    // ---------------------------------------------
    // ready (device allows commands from TA/TE)
    // <CR><LF>+CPAS: 0<CR><LF> <CR><LF>OK<CR><LF>
    // unavailable (device does not allow commands from TA/TE)
    // <CR><LF>+CPAS: 1<CR><LF> <CR><LF>OK<CR><LF> 
    // unknown (device is not guaranteed to respond to instructions)
    // <CR><LF>+CPAS: 2<CR><LF> <CR><LF>OK<CR><LF> - NO CALL
    // ringing
    // <CR><LF>+CPAS: 3<CR><LF> <CR><LF>OK<CR><LF> - NO CALL
    // call in progress
    // <CR><LF>+CPAS: 4<CR><LF> <CR><LF>OK<CR><LF> - NO CALL
//    if(IsStringReceived("0"))
//    {
      // ready - there is no call
      // ------------------------
//      ret_val = CALL_NONE;
	//printf("call phone number is 0\n");
//    }
    if(IsStringReceived("3"))
    {
      // incoming call
      ret_val = CALL_INCOM_VOICE;
    }
    else if(IsStringReceived("4"))
    {
      // active call
      ret_val = CALL_ACTIVE_VOICE;
    }
    else if(IsStringReceived("0"))
    {
      ret_val=CALL_NONE;
    }
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

/**********************************************************
Method calls the specific number

number_string: pointer to the phone number string 
               e.g. gsm.Call("+420123456789"); 

**********************************************************/
void CallS(char *number_string)
{
  if (CLS_FREE != GetCommLineStatus()) return;
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  // ATDxxxxxx;<CR>
  serialPuts(gFD, "ATD");
  serialPuts(gFD, number_string);    
  serialPuts(gFD, ";\r");
  // 10 sec. for initial comm tmout
  // 50 msec. for inter character timeout
  WaitResp(10000, 50);
  SetCommLineStatus(CLS_FREE);
}

/**********************************************************
Method calls the number stored at the specified SIM position

sim_position: position in the SIM <1...>
              e.g. gsm.Call(1);
**********************************************************/
void Call(int sim_position)
{
  if (CLS_FREE != GetCommLineStatus()) return;
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  // ATD>"SM" 1;<CR>
  serialPuts(gFD, "ATD>\"SM\" ");
  serialPuts(gFD, ChangeIToS(sim_position));    
  serialPuts(gFD, ";\r");

  // 10 sec. for initial comm tmout
  // 50 msec. for inter character timeout
  WaitResp(10000, 50);

  SetCommLineStatus(CLS_FREE);
}

/**********************************************************
Method checks status of call(incoming or active) 
and makes authorization with specified SIM positions range

phone_number: a pointer where the tel. number string of current call will be placed
              so the space for the phone number string must be reserved - see example
first_authorized_pos: initial SIM phonebook position where the authorization process
                      starts
last_authorized_pos:  last SIM phonebook position where the authorization process
                      finishes

                      Note(important):
                      ================
                      In case first_authorized_pos=0 and also last_authorized_pos=0
                      the received incoming phone number is NOT authorized at all, so every
                      incoming is considered as authorized (CALL_INCOM_VOICE_NOT_AUTH is returned)

return: 
      CALL_NONE                   - no call activity
      CALL_INCOM_VOICE_AUTH       - incoming voice - authorized
      CALL_INCOM_VOICE_NOT_AUTH   - incoming voice - not authorized
      CALL_ACTIVE_VOICE           - active voice
      CALL_INCOM_DATA_AUTH        - incoming data call - authorized
      CALL_INCOM_DATA_NOT_AUTH    - incoming data call - not authorized  
      CALL_ACTIVE_DATA            - active data call
      CALL_NO_RESPONSE            - no response to the AT command 
      CALL_COMM_LINE_BUSY         - comm line is not free
**********************************************************/
int CallStatusWithAuth(char *phone_number, int first_authorized_pos, int last_authorized_pos)
{
  int ret_val = CALL_NONE;
  int search_phone_num = 0;
  int i;
  int status;
  char *p_char; 
  char *p_char1;

  phone_number[0] = 0x00;  // no phonr number so far
  if (CLS_FREE != GetCommLineStatus()) return (CALL_COMM_LINE_BUSY);
  SetCommLineStatus(CLS_ATCMD);
  // Must wait a little bit
  delay(400);
  serialPuts(gFD, "AT+CLCC");

  // 5 sec. for initial comm tmout
  // and max. 1500 msec. for inter character timeout
  RxInit(5000, 1500); 
  // wait response is finished
  do 
  {
    if (IsStringReceived("OK\r\n")) 
    { 
      // perfect - we have some response, but what:

      // there is either NO call:
      // <CR><LF>OK<CR><LF>

      // or there is at least 1 call
      // +CLCC: 1,1,4,0,0,"+420XXXXXXXXX",145<CR><LF>
      // <CR><LF>OK<CR><LF>
      status = RX_FINISHED;
      break; // so finish receiving immediately and let's go to 
             // to check response 
    }
    status = IsRxFinished();
  } while (status == RX_NOT_FINISHED);

  // generate tmout 30msec. before next AT command
  delay(30);

  if (status == RX_FINISHED) 
  {
    // something was received but what was received?
    // example: //+CLCC: 1,1,4,0,0,"+420XXXXXXXXX",145
    if(IsStringReceived("+CLCC: 1,1,4,0,0")) 
    { 
      // incoming VOICE call - not authorized so far
      search_phone_num = 1;
      ret_val = CALL_INCOM_VOICE_NOT_AUTH;
    }
    else if(IsStringReceived("+CLCC: 1,1,4,1,0")) 
    { 
      // incoming DATA call - not authorized so far
      search_phone_num = 1;
      ret_val = CALL_INCOM_DATA_NOT_AUTH;
    }
    else if(IsStringReceived("+CLCC: 1,0,0,0,0")) 
    { 
      // active VOICE call - GSM is caller
      search_phone_num = 1;
      ret_val = CALL_ACTIVE_VOICE;
    }
    else if(IsStringReceived("+CLCC: 1,1,0,0,0"))
    { 
      // active VOICE call - GSM is listener
      search_phone_num = 1;
      ret_val = CALL_ACTIVE_VOICE;
    }
    else if(IsStringReceived("+CLCC: 1,1,0,1,0")) 
    { 
      // active DATA call - GSM is listener
      search_phone_num = 1;
      ret_val = CALL_ACTIVE_DATA;
    }
    else if(IsStringReceived("+CLCC:"))
    { 
      // other string is not important for us - e.g. GSM module activate call
      // etc.
      // IMPORTANT - each +CLCC:xx response has also at the end
      // string <CR><LF>OK<CR><LF>
      ret_val = CALL_OTHERS;
    }
    else if(IsStringReceived("OK"))
    { 
      // only "OK" => there is NO call activity
      ret_val = CALL_NONE;
    }

    
    // now we will search phone num string
    if (search_phone_num) 
    {
      // extract phone number string
      p_char = strchr((char *)(comm_buf),'"');
      p_char1 = p_char+1; // we are on the first phone number character
      p_char = strchr((char *)(p_char1),'"');
      if (p_char != NULL) 
      {
        *p_char = 0; // end of string
        strcpy(phone_number, (char *)(p_char1));
      }
      
      if ( (ret_val == CALL_INCOM_VOICE_NOT_AUTH) || (ret_val == CALL_INCOM_DATA_NOT_AUTH)) 
      {
        if ((first_authorized_pos == 0) && (last_authorized_pos == 0)) 
        {
          // authorization is not required => it means authorization is OK
          if (ret_val == CALL_INCOM_VOICE_NOT_AUTH) ret_val = CALL_INCOM_VOICE_AUTH;
          else ret_val = CALL_INCOM_DATA_AUTH;
        }
        else
        {
          // make authorization
          SetCommLineStatus(CLS_FREE);
          for (i = first_authorized_pos; i <= last_authorized_pos; i++)
          {
            if (ComparePhoneNumber(i, phone_number)) 
            {
              // phone numbers are identical
              // authorization is OK
              if (ret_val == CALL_INCOM_VOICE_NOT_AUTH) ret_val = CALL_INCOM_VOICE_AUTH;
              else ret_val = CALL_INCOM_DATA_AUTH;
              break;  // and finish authorization
            }
          }
        }
      }
    }
  }
  else 
  {
    // nothing was received (RX_TMOUT_ERR)
    ret_val = CALL_NO_RESPONSE;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}