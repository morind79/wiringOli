#include <wiringOli.h>
#include <wiringOliI2C.h>
#include <oliExt.h>
#include <mcp23008.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <termios.h>
#include <ctype.h>
#include <mysql/mysql.h>
#include "teleinfo.h"

// Some enum
enum parity_e     { P_NONE,   P_EVEN,    P_ODD };
enum flowcntrl_e  { FC_NONE,  FC_RTSCTS, FC_XONXOFF };

// Configuration structure
struct 
{
  char port[128];
  int baud;
  enum flowcntrl_e flow;
  char *flow_str;
  enum parity_e parity;
  char *parity_str;
  int databits;
  int nolock;
  int netport;
  int verbose;
  char value_str[32];

  int  mysql;
  char server[32];
  char user[32];
  char password[32];
  char database[32];
  char table[32];
  unsigned int serverport;
// Configuration structure defaults values
} opts = {
  .port = "/dev/ttyS3",
  .baud = 1200,
  .flow = FC_NONE,
  .flow_str = "none",
  .parity = P_EVEN,
  .parity_str = "even",
  .databits = 7,
  .nolock = false,
  .verbose = true,
  .value_str = "",

  .mysql = true,
  .server = MYSQL_HOST,
  .user = MYSQL_LOGIN,
  .password = MYSQL_PASS,
  .database = MYSQL_DB,
  .table = MYSQL_TABLE,
  .serverport = MYSQL_PORT, 
};


// Statistics and error structure
struct 
{
  unsigned long framesent;
  unsigned long frame;
  unsigned long badchecksum;
  unsigned long frameformaterror;
  unsigned long frameok;
  unsigned long framesizeerror;

  unsigned long mysqlinitok;
  unsigned long mysqliniterror;
  unsigned long mysqlconnectok;
  unsigned long mysqlconnecterror;
  unsigned long mysqlqueryerror;
  unsigned long mysqlqueryok;
} stats;

// ======================================================================
// Global vars 
// ======================================================================
int   g_fd_teleinfo;          // teleinfo serial handle
int   g_exit_pgm;             // indicate en of the program
char  g_lockname[256] = "";   // Lock filename

// ======================================================================
// some func declaration
// ======================================================================
void tlf_close_serial(int);
int uucp_unlock(void);

/* ======================================================================
Function: log_syslog
Purpose : write event to syslog
Input   : stream to write if needed
          string to write in printf format
          printf other arguments
Output  : -
Comments: 
====================================================================== */
void log_syslog( FILE * stream, const char *format, ...)
{
  static char tmpbuff[512]="";
  va_list args;

  // do a style printf style in ou buffer
  va_start (args, format);
  vsnprintf (tmpbuff, sizeof(tmpbuff), format, args);
  tmpbuff[sizeof(tmpbuff) - 1] = '\0';
  va_end (args);

  // Write to logfile
  openlog( PRG_NAME, LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "%s", tmpbuff);
  closelog();
  
  // stream passed ? write also to it
  if (stream && opts.verbose) 
  {
    fprintf(stream, "%s", tmpbuff);
    //fprintf(stream, "\n");
    fflush(stream);
  }
}

/* ======================================================================
Function: show_stats
Purpose : display program statistics 
Input   : -
Output  : -
Comments: 
====================================================================== */
void show_stats(void)
{
  // Print stats
  int old_verb = opts.verbose;
  
  // Fake verbose mode to display info
  // We'll restore it after
  opts.verbose = true;
  
  log_syslog(stderr, "\n"PRG_NAME" "PRG_VERSION_NUMBER" Statistics\n");
  log_syslog(stderr, "==========================\n");
  //log_syslog(stderr, "Frames Sent         : %ld\n", stats.framesent);
  log_syslog(stderr, "Frames checked      : %ld\n", stats.frame);
  log_syslog(stderr, "Frames OK           : %ld\n", stats.frameok);
  //log_syslog(stderr, "Checksum errors     : %ld\n", stats.badchecksum);
  //log_syslog(stderr, "Frame format Errors : %ld\n", stats.frameformaterror);
  //log_syslog(stderr, "Frame size Errors   : %ld\n", stats.framesizeerror);

  //log_syslog(stderr, "MySQL init OK       : %ld\n", stats.mysqlinitok);
  //log_syslog(stderr, "MySQL init errors   : %ld\n", stats.mysqliniterror);
  //log_syslog(stderr, "MySQL connect OK    : %ld\n", stats.mysqlconnectok);
  //log_syslog(stderr, "MySQL connect errors: %ld\n", stats.mysqlconnecterror);
  //log_syslog(stderr, "MySQL queries OK    : %ld\n", stats.mysqlqueryok);
  //log_syslog(stderr, "MySQL queries errors: %ld\n", stats.mysqlqueryerror);
  log_syslog(stderr, "--------------------------\n");
  
  opts.verbose = old_verb;
}

/* ======================================================================
Function: clean_exit
Purpose : exit program 
Input   : exit code
Output  : -
Comments: 
====================================================================== */
void clean_exit (int exit_code)
{

  // close serials
  if (g_fd_teleinfo)
  {
    // then close
    tlf_close_serial(g_fd_teleinfo);
  }

  // display statistics
  show_stats();
    
  if ( exit_code == EXIT_SUCCESS)
      log_syslog(stdout, "Succeded to do my job\n");
  else
      log_syslog(stdout, "Closing teleinfo due to error\n");
  
  if (!opts.nolock)
    uucp_unlock();

  exit(exit_code);
}

/* ======================================================================
Function: fatal
Purpose : exit program due to a fatal error
Input   : string to write in printf format
          printf other arguments
Output  : -
Comments: 
====================================================================== */
void fatal (const char *format, ...)
{
  char tmpbuff[512] = "";
  va_list args;

  va_start(args, format);
  vsnprintf(tmpbuff, sizeof(tmpbuff), format, args);
  tmpbuff[sizeof(tmpbuff) - 1] = '\0';
  va_end(args);

  // Write to logfile
  openlog( PRG_NAME, LOG_PID | LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "%s", tmpbuff);
  closelog();

  fprintf(stderr,"\r\nFATAL: %s \r\n", tmpbuff );
  fflush(stderr);

  clean_exit(EXIT_FAILURE);
}

/* ======================================================================
Function: uucp_lockname
Purpose : create the lock filename
Input   : 
Output  : -
Comments: Routine taken from picocom source code
===================================================================== */
int uucp_lockname(const char *dir, const char *file)
{
  char *p, *cp;
  struct stat sb;

  if ( !dir || *dir=='\0' || stat(dir, &sb) != 0 )
    return -1;

  // cut-off initial "/dev/" from file-name 
  
  p = strchr(file + 1, '/');
  p = p ? p + 1 : (char *)file;
  
  // replace '/'s with '_'s in what remains (after making a copy) 
  p = cp = strdup(p);
  do 
  { 
    if ( *p == '/' ) 
      *p = '_';
  }
  while(*p++);
  
  // build lockname 
  snprintf(g_lockname, sizeof(g_lockname), "%s/LCK..%s", dir, cp);
  
  // destroy the copy 
  free(cp);

  return 0;
}

/* ======================================================================
Function: uucp_lock
Purpose : create lock file
Input   : -
Output  : -
Comments: Routine taken from picocom source code
===================================================================== */
int uucp_lock(void)
{
  int r, fd, pid;
  char buf[16];
  mode_t m;

  // alread a lock ?
  if ( g_lockname[0] == '\0' ) 
    return 0;

  fd = open(g_lockname, O_RDONLY);
  
  if ( fd >= 0 ) 
  {
    r = read(fd, buf, sizeof(buf)); 
    close(fd);
    
    // if r == 4, lock file is binary (old-style) 
    pid = (r == 4) ? *(int *)buf : strtol(buf, NULL, 10);
    if ( pid > 0 && kill((pid_t)pid, 0) < 0 && errno == ESRCH ) 
    {
      // stale lock file
      fprintf(stdout, "Removing stale lock: %s\n", g_lockname);
      sleep(1);
      unlink(g_lockname);
    }
    else
    {
      g_lockname[0] = '\0';
      errno = EEXIST;
      return -1;
    }
  }
  
  // lock it 
  m = umask(022);
  fd = open(g_lockname, O_WRONLY|O_CREAT|O_EXCL, 0666);
  if ( fd < 0 ) 
  { 
    g_lockname[0] = '\0'; 
    return -1; 
  }
  
  umask(m);
  snprintf(buf, sizeof(buf), "%04d\n", getpid());
  write(fd, buf, strlen(buf));
  close(fd);

  return 0;
}

/* ======================================================================
Function: uucp_unlock
Purpose : unlock open lock
Input   : -
Output  : -
Comments: Routine taken from picocom source code
===================================================================== */
int uucp_unlock(void)
{
  if ( g_lockname[0] ) 
    unlink(g_lockname);
  return 0;
}

/* ======================================================================
Function: db_open
Purpose : open database connexion
Input   : pointer to mysql structure
Output  : EXIT_SUCCESS if ok else EXIT_FAILURE
Comments: -
===================================================================== */
int db_open( MYSQL * pmysql) 
{
  MYSQL *mysql_connection;

  // Open MySQL Database and read timestamp of the last record written
  if(!mysql_init(pmysql))
  {
    log_syslog(stderr, "Cannot initialize MySQL");
    stats.mysqliniterror++;
  }
  else
  {
    stats.mysqlinitok++;

    // connect to database
    mysql_connection = mysql_real_connect(pmysql, opts.server, opts.user, opts.password, opts.database, opts.serverport, NULL, 0);

    if(mysql_connection == NULL)
    {
      log_syslog(stderr, "%d: %s \n",   mysql_errno(pmysql), mysql_error(pmysql));
      stats.mysqlconnecterror++;
      return(EXIT_FAILURE);
    }
    else
    {
      stats.mysqlconnectok++;
    }
  }

  return (EXIT_SUCCESS);
}

/* ======================================================================
Function: db_close
Purpose : close database connexion
Input   : pointer to mysql structure
Output  : -
Comments: -
===================================================================== */
void db_close( MYSQL * pmysql) 
{
  // close MySQL Database 
  mysql_close(pmysql);
}

/* ======================================================================
Function: tlf_init_serial
Purpose : initialize serial port for receiving teleinfo
Input   : -
Output  : Serial Port Handle
Comments: -
====================================================================== */
int tlf_init_serial1(void)
{
  int tty_fd;
  struct termios termios;

  // Open serial device
  if ( (tty_fd = open(opts.port, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) < 0 ) 
    fatal( "tlf_init_serial %s: %s", opts.port, strerror(errno));
  else
    log_syslog( stdout, "'%s' opened.\n",opts.port);
    
  // Set descriptor status flags
  fcntl (tty_fd, F_SETFL, O_RDWR ) ;

  // lock serial  ?
  if ( !opts.nolock )
  {
    uucp_lockname(UUCP_LOCK_DIR, opts.port);
    
    if ( uucp_lock() < 0 )
      fatal("cannot lock %s: %s", opts.port, strerror(errno));
  }
    
  // Raw mode
  cfmakeraw(&termios);
  
  // Set serial speed to 1200 bps
  if (cfsetospeed(&termios, B1200) < 0 || cfsetispeed(&termios, B1200) < 0 )
    log_syslog(stderr, "cannot set serial speed to 1200 bps: %s",  strerror(errno));
    
  // Parity Even
  termios.c_cflag &= ~PARODD;
  termios.c_cflag |= PARENB;    
  
  // 7 databits
  termios.c_cflag = (termios.c_cflag & ~CSIZE) | CS7;
  
  // No Flow Control
  termios.c_cflag &= ~(CRTSCTS);
  termios.c_iflag &= ~(IXON | IXOFF | IXANY);
  
  // Local
  termios.c_cflag |= CLOCAL;

  // No minimal char but 8 sec timeout
  termios.c_cc [VMIN]  =  0 ;
  termios.c_cc [VTIME] = 50 ; 

  // now setup the whole parameters
  if ( tcsetattr (tty_fd, TCSANOW | TCSAFLUSH, &termios) <0) 
    log_syslog(stderr, "cannot set current parameters %s: %s",  opts.port, strerror(errno));
    
  // Sleep 50ms
  // trust me don't forget this one, it will remove you some
  // headache to find why serial is not working
  usleep(50000);

  return tty_fd ;
}

int tlf_init_serial(void)
{
	struct termios teleinfo_serial_attr;
	int fd;

	if ((fd = open(opts.port, O_RDONLY | O_NOCTTY)) == -1) {
		syslog(LOG_ERR, "Erreur ouverture du port serie %s !", opts.port);
		return 0;
	}

	tcgetattr(fd, &teleinfo_serial_attr);

	cfsetispeed(&teleinfo_serial_attr, B1200);
	cfsetospeed(&teleinfo_serial_attr, B1200);

	teleinfo_serial_attr.c_cflag |= (CLOCAL | CREAD);

	// Format série "7E1"
	teleinfo_serial_attr.c_cflag |= PARENB;	// Active 7 bits de donnees avec parite pair.
	teleinfo_serial_attr.c_cflag &= ~PARODD;
	teleinfo_serial_attr.c_cflag &= ~CSTOPB;
	teleinfo_serial_attr.c_cflag &= ~CSIZE;
	teleinfo_serial_attr.c_cflag |= CS7;
//     teleinfo_serial_attr.c_cflag &= ~CRTSCTS ;                           // Désactive control de flux matériel. (pas compatible POSIX)

	teleinfo_serial_attr.c_iflag |= (INPCK | ISTRIP);
	teleinfo_serial_attr.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);	// Désactive control de flux logiciel, conversion 0xOD en 0x0A.

	teleinfo_serial_attr.c_oflag &= ~OPOST;	// Pas de mode de sortie particulier (mode raw).

	teleinfo_serial_attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	// Mode non-canonique (mode raw) sans echo.

	teleinfo_serial_attr.c_cc[VTIME] = 80;	// time-out à ~8s.
	teleinfo_serial_attr.c_cc[VMIN] = 0;	// 1 car. attendu.

	tcflush(fd, TCIFLUSH);	// Efface les données reçues mais non lues.
	tcsetattr(fd, TCSANOW, &teleinfo_serial_attr);	// Sauvegarde des nouveaux parametres
	return fd;
}

/* ======================================================================
Function: tlf_close_serial
Purpose : close serial port for receiving teleinfo
Input   : Serial Port Handle
Output  : -
Comments: 
====================================================================== */
void tlf_close_serial(int device)
{
  if (device)
  { 
    close(device) ;
  }
}

/* ======================================================================
Function: tlf_checksum_ok
Purpose : check if teleinfo frame checksum is correct
Input   : -
Output  : true or false
Comments: 
====================================================================== */
int tlf_checksum_ok(char *etiquette, char *valeur, char checksum) 
{
  int i ;
  unsigned char sum = 32 ;    // Somme des codes ASCII du message + un espace

  for (i=0; i < strlen(etiquette); i++) 
    sum = sum + etiquette[i] ;
    
  for (i=0; i < strlen(valeur); i++) 
    sum = sum + valeur[i] ;
    
  sum = (sum & 63) + 32 ;

  // Return 1 si checkum ok.
  if ( sum == checksum) return ( true ); 
  return(false);
}

/* ======================================================================
Function: tlf_treat_label
Purpose : do action when received a correct label / value + checksum line
Input   : plabel : pointer to string containing the label
        : pvalue : pointer to string containing the associated value
Output  : 
Comments: 
====================================================================== */
void tlf_treat_label( char * plabel, char * pvalue) 
{
  if (strcmp(plabel, "OPTARIF")==0 )
  {
    // L'option tarifaire choisie (Groupe "OPTARIF") est codée sur 4 caractères alphanumériques 
    /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
    je mets le 4eme char à 0, trop de possibilités
    BASE => Option Base. 
    HC.. => Option Heures Creuses. 
    EJP. => Option EJP. 
    BBRx => Option Tempo
    */
    pvalue[3] = '\0';
      
         if (strcmp(pvalue, "BAS")==0 ) strcpy (pvalue, "1");
    else if (strcmp(pvalue, "HC.")==0 ) strcpy (pvalue, "2");
    else if (strcmp(pvalue, "EJP")==0 ) strcpy (pvalue, "3");
    else if (strcmp(pvalue, "BBR")==0 ) strcpy (pvalue, "4");
    else strcpy (pvalue, "0");
  }
  else if (strcmp(plabel, "HHPHC")==0 )
  {
    // L'horaire heures pleines/heures creuses (Groupe "HHPHC") est codé par un caractère A à Y 
    // J'ai choisi de prendre son code ASCII
    int code = *pvalue;
    sprintf(pvalue, "%d", code);
  }
  else if (strcmp(plabel, "PTEC")==0 )
  {
    // La période tarifaire en cours (Groupe "PTEC"), est codée sur 4 caractères 
    /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
    TH.. => Toutes les Heures. 
    HC.. => Heures Creuses. 
    HP.. => Heures Pleines. 
    HN.. => Heures Normales. 
    PM.. => Heures de Pointe Mobile. 
    HCJB => Heures Creuses Jours Bleus. 
    HCJW => Heures Creuses Jours Blancs (White). 
    HCJR => Heures Creuses Jours Rouges. 
    HPJB => Heures Pleines Jours Bleus. 
    HPJW => Heures Pleines Jours Blancs (White). 
    HPJR => Heures Pleines Jours Rouges. 
    */
         if (strcmp(pvalue, "TH..")==0 ) strcpy (pvalue, "1");
    else if (strcmp(pvalue, "HC..")==0 ) strcpy (pvalue, "2");
    else if (strcmp(pvalue, "HP..")==0 ) strcpy (pvalue, "3");
    else if (strcmp(pvalue, "HN..")==0 ) strcpy (pvalue, "4");
    else if (strcmp(pvalue, "PM..")==0 ) strcpy (pvalue, "5");
    else if (strcmp(pvalue, "HCJB")==0 ) strcpy (pvalue, "6");
    else if (strcmp(pvalue, "HCJW")==0 ) strcpy (pvalue, "7");
    else if (strcmp(pvalue, "HCJR")==0 ) strcpy (pvalue, "8");
    else if (strcmp(pvalue, "HPJB")==0 ) strcpy (pvalue, "9");
    else if (strcmp(pvalue, "HPJW")==0 ) strcpy (pvalue, "10");
    else if (strcmp(pvalue, "HPJR")==0 ) strcpy (pvalue, "11");
    else strcpy (pvalue, "0");
    
  }
/*  
  // Do we need to get specific value ?
  if (*opts.value_str)
  {
    if (strcmp(plabel, opts.value_str)==0 )
    {
      //fprintf(stdout, "%s\n", pvalue);
    }
  }
  else
  {
    // Display current Power
    if (strcmp(plabel, "PAPP")==0 )
    {
      int power = atoi ( pvalue);
      //fprintf(stdout, "Power = %d W\n", power);
    }
    // Display Index
    if (strcmp(plabel, "HCHC")==0 || strcmp(plabel, "HCHP")==0)
    {
      long index = atol ( pvalue);
      //fprintf(stdout, "%s = %ld KWh\n", plabel, index);
    }
  }
*/
}


/* ======================================================================
Function: tlf_check_frame
Purpose : check for teleinfo frame
Input   : -
Output  : the length of valid buffer, 0 otherwhile
Comments: 
====================================================================== */
int tlf_check_frame( char * pframe) 
{
  char * pstart;
  char * pend;
  char * pnext;
  char * ptok;
  char * pvalue;
  char * pcheck;
  char  buff[TELEINFO_BUFSIZE];

  static char mysql_field[ 512];
  static char mysql_value[ 512];
  static char mysql_job[1024];
  int frame_err , len;
  
  // New frame
  stats.frame++;
  len = strlen(pframe);   
  strncpy( buff, pframe, len+1);
  pstart = &buff[0];
  pend   = pstart + len;

  if (opts.verbose)
  {
    fprintf(stdout, "------------------- Received %d char Frame.%s\n-------------------", len, buff);
    fflush(stdout);
  } 

  // need to put in mysql
  if (opts.mysql)
  {
    // First SQL Fields, use date time from mysql server using NOW()
    strcpy(mysql_field, "DATE");
    strcpy(mysql_value, "NOW()");
  }

  // just one verification, buffer should be at least 100 Char
  if ( pstart && pend && (pend > pstart+100 ) )
  {
    // by default no error
    frame_err = false;
    // ignore STX
    pstart++;
    // Init our pointers
    ptok = pvalue = pnext = pcheck = NULL;
    // Loop in buffer 
    while ( pstart < pend )
    {
      // start of token
      if ( *pstart=='\n' )
      {   
        // Position on token        
        ptok = ++pstart;
      }           
      // start of token value
      if ( *pstart==' ' && ptok)
      {           
        // Isolate token name
        *pstart++ = '\0';

        // no value yet, setit ?
        if (!pvalue)
          pvalue = pstart;
        // we had value, so it's checksum
        else
          pcheck = pstart;
      }           
      // new line ? ok we got all we need ?
      if ( *pstart=='\r' )
      {                   
        *pstart='\0';
        // Good format ?
        if ( ptok && pvalue && pcheck )
        {
          // Checksum OK
          if ( tlf_checksum_ok(ptok, pvalue, *pcheck))
          {
            //fprintf(stdout, "%s=%s\n",ptok, pvalue);            
            // In case we need to do things
            tlf_treat_label(ptok, pvalue);

            // need to send to my sql ?
            if (opts.mysql )
            {
              strcat(mysql_field, ",");
              strcat(mysql_field, ptok);
              
              // Compteur Monophasé IINST et IMAX doivent être reliés à
              // IINST1 et IMAX1 dans la base
              if (strcmp(ptok, "IINST")==0 || strcmp(ptok, "IMAX")==0)
                strcat(mysql_field, "1");

              strcat(mysql_value, ",'");
              strcat(mysql_value, pvalue);
              strcat(mysql_value, "'");
            }
          }
          else
          {
            frame_err = true;
            //fprintf(stdout, "%s=%s BAD",ptok, pvalue);
            log_syslog(stderr, "tlf_checksum_ok('%s','%s','%c') error\n",ptok, pvalue, *pcheck);
            stats.badchecksum++;
          }
        }
        else
        {
          frame_err = true;
          log_syslog(stderr, "tlf_check_frame() no correct frame '%s'\n",ptok);
          stats.frameformaterror++;
        }        
        // reset data
        ptok = pvalue = pnext = pcheck = NULL;
      }                 
      // next
      pstart++;  
    }

    // no error in this frame ?
    if (!frame_err)
    {
      stats.frameok++;
      if (opts.verbose) fprintf(stdout, "Frame OK\n");
      if (opts.mysql)
      {
        MYSQL mysql;
        
        // Ecrit données dans base MySql.
        sprintf(mysql_job, "INSERT INTO %s\n  (%s)\nVALUES\n  (%s);\n", opts.table, mysql_field, mysql_value);
        
        //if (opts.verbose)
          //printf(mysql_job);
        
        if (opts.verbose)
          fprintf(stdout, "%s", mysql_job); 

        if ( db_open(&mysql) != EXIT_SUCCESS )
          log_syslog(stderr, "%d: %s \n", mysql_errno(&mysql), mysql_error(&mysql));
        else
        {
            // execute SQL query
          if (mysql_query(&mysql,mysql_job))
          {
            stats.mysqlqueryerror++;
            log_syslog(stderr, "%d: %s \n", mysql_errno(&mysql), mysql_error(&mysql));
          }
          else
            stats.mysqlqueryok++;

          db_close(&mysql);
          g_exit_pgm = true;
        }
      }
      //debug_dump_sensors(0);
      return (len);
    }
  } 
  else
  {
    log_syslog(stderr, "tlf_check_frame() No correct frame found\n");
    stats.framesizeerror++;
  }
  return (0);
}

/* ======================================================================
Function: trim
Purpose : remove leading en ending space char from a string
Input   : string pointer
Output  : string pointer
Comments: 
====================================================================== */
char* trim(char* pstr)
{
  char* end = 0;
  // Skip leading space
  while(isspace(*pstr)) pstr++;
  // End of string ?
  if (!*pstr) return pstr;
  // Get end of string
  end = pstr + strlen(pstr) - 1;
  // Skip ending space  
  while(isspace(*end)) end--;
  // Note the end of string
  *(end + 1) = '\0';
  return pstr;
}

/* ======================================================================
Function: readEnergyMeter
Purpose : Reads energy meter information
Input   : 
Output  : 
Comments: 
====================================================================== */
void readEnergyMeter()
{
  int n;
  int loop;
  unsigned char c;
  char  rcv_buff[TELEINFO_BUFSIZE];
  int   rcv_idx;

  g_exit_pgm = false;  
  rcv_idx = 0;
  g_fd_teleinfo = 0; 
  
  // clean up our buffer
  bzero(rcv_buff, TELEINFO_BUFSIZE);
  
  // Open serial port
  g_fd_teleinfo = tlf_init_serial();

  loop = 0;
  tcflush(g_fd_teleinfo, TCIFLUSH);  
  // Do while not end
  while ( ! g_exit_pgm ) 
  {
    // If loop is more than 10, then exit
    if (loop > 1000) g_exit_pgm = true;
    // Read from serial port
    n = read(g_fd_teleinfo, &c, 1);
    
    //log_syslog(stderr, "Char c = 0x%02.2X %c\n", c, c);
    //fprintf(stdout, "Char c = 0x%02.2X %c n=%d\n", c, c, n);
       
    if (n < 0) fatal("read failed: %s", strerror(errno));

    // If no energymeter then exit.      
    if (n == 0) g_exit_pgm = 1;

    // What we received ?
    switch (c)
    {
      // start of frame ???
      case  STX:
        // Clear buffer, begin to store in it
        rcv_idx = 0;
        bzero(rcv_buff, TELEINFO_BUFSIZE);
        rcv_buff[rcv_idx++] = c;
        break;
        
      // End of frame ?
      case  ETX:
        // We had STX ?
        if ( rcv_idx )
        {
          // Store in buffer and proceed
          rcv_buff[rcv_idx++] = c;            
          // clear the end of buffer (paranoia inside)
          bzero(&rcv_buff[rcv_idx], TELEINFO_BUFSIZE-rcv_idx);            
          if (opts.mysql) tlf_check_frame(rcv_buff);
        }
        // May be begin of the program or other problem.
        else
        {
          rcv_idx = 0;
          bzero(rcv_buff, TELEINFO_BUFSIZE);
        }
        break;        
      // other char ?
      default:
      {
        // If we are in a frame, store data received
        if (rcv_idx)
        {
          // If buffer is not full
          if ( rcv_idx < TELEINFO_BUFSIZE)
          {
            // Store data received
            rcv_buff[rcv_idx++] = c;
          }
          else
          {
            // clear buffer & restart
            rcv_idx=0;
            bzero(rcv_buff, TELEINFO_BUFSIZE);
          }
        }
      }
      break;
    }
    loop++;
  } // while not exit program
  
  //clean_exit(EXIT_SUCCESS);
}
