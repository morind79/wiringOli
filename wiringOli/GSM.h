#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Length for the internal communication buffer
#define COMM_BUF_LEN        300

// Some constants for the IsRxFinished() method
#define RX_NOT_STARTED      0
#define RX_ALREADY_STARTED  1

// Some constants for the InitParam() method
#define PARAM_SET_0   0
#define PARAM_SET_1   1

// Status bits definition
#define STATUS_NONE                 0
#define STATUS_INITIALIZED          1
#define STATUS_REGISTERED           2
#define STATUS_USER_BUTTON_ENABLE   4

// Communication SIM900
int module_status;
int comm_line_status;
void Echo(int state);
char SendATCmdWaitResp(char *AT_cmd_string, uint16_t start_comm_tmout, uint16_t max_interchar_tmout, char const *response_string, int no_of_attempts);
int WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout);
int WaitRespAdd(uint16_t start_comm_tmout, uint16_t max_interchar_tmout, char const *expected_resp_string);
void RxInit(uint16_t start_comm_tmout, uint16_t max_interchar_tmout);
int IsRxFinished(void);
int IsStringReceived(char const *compare_string);

// Serial port
void serialBegin(long baud);
void serialEnd();
long checkBaud();

// Others
char *ChangeIToS(int IntNU);

// Power
void powerOn();
void reset();
int isOn();

// GSM
void init();
void InitParam(int group);
char InitSMSMemory();
char SendSMS(char *number_str, char *message_str);
char SendSMSSpecified(int sim_phonebook_position, char *message_str);
char IsSMSPresent(int required_status);
char GetSMS(int position, char *phone_number, char *SMS_text, int max_SMS_len);
char GetAuthorizedSMS(int position, char *phone_number, char *SMS_text, int max_SMS_len, int first_authorized_pos, int last_authorized_pos);
char DeleteSMS(int position);
char DeleteAllSMS();
char GetPhoneNumber(int position, char *phone_number);
char WritePhoneNumber(int position, char *phone_number);
char DelPhoneNumber(int position);
char ComparePhoneNumber(int position, char *phone_number);

char SetSpeakerVolume(int speaker_volume);
char IncSpeakerVolume();
char DecSpeakerVolume();

char SendDTMFSignal(int dtmf_tone);

int IsUserButtonEnable();
void DisableUserButton();
void EnableUserButton();
int IsUserButtonPushed();

void PickUp();
void HangUp();
int CallStatus();
void CallS(char *number_string);
void Call(int sim_position);
int CallStatusWithAuth(char *phone_number, int first_authorized_pos, int last_authorized_pos);

enum at_resp_enum
{
	AT_RESP_ERR_NO_RESP = -1,   // Nothing received
	AT_RESP_ERR_DIF_RESP = 0,   // Response_string is different from the response
	AT_RESP_OK = 1,             // Response_string was included in the response

	AT_RESP_LAST_ITEM
};

enum rx_state_enum
{
	RX_NOT_FINISHED = 0,      // not finished yet
	RX_FINISHED,              // finished, some character was received
	RX_FINISHED_STR_RECV,     // finished and expected string received
	RX_FINISHED_STR_NOT_RECV, // finished, but expected string not received
	RX_TMOUT_ERR,             // finished, no character received
	// initial communication tmout occurred
	RX_LAST_ITEM
};

enum comm_line_status_enum
{
	// CLS like CommunicationLineStatus
	CLS_FREE,   // line is free - not used by the communication and can be used
	CLS_ATCMD,  // line is used by AT commands, includes also time for response
	CLS_DATA,   // for the future - line is used in the CSD or GPRS communication
	CLS_LAST_ITEM
};

enum sms_type_enum
{
	SMS_UNREAD,
	SMS_READ,
	SMS_ALL,
	SMS_LAST_ITEM
};

enum getsms_ret_val_enum
{
	GETSMS_NO_SMS   = 0,
	GETSMS_UNREAD_SMS,
	GETSMS_READ_SMS,
	GETSMS_OTHER_SMS,
	GETSMS_NOT_AUTH_SMS,
	GETSMS_AUTH_SMS,
	GETSMS_LAST_ITEM
};

enum call_ret_val_enum
{
	CALL_NONE = 0,
	CALL_INCOM_VOICE,
	CALL_ACTIVE_VOICE,
	CALL_INCOM_VOICE_AUTH,
	CALL_INCOM_VOICE_NOT_AUTH,
	CALL_INCOM_DATA_AUTH,
	CALL_INCOM_DATA_NOT_AUTH,
	CALL_ACTIVE_DATA,
	CALL_OTHERS,
	CALL_NO_RESPONSE,
	CALL_COMM_LINE_BUSY,
	CALL_LAST_ITEM
};