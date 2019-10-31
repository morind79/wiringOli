/*
 * teleinfo.h:
 *	Extend wiringOli with the MCP 23008 I2C GPIO expander chip
 ***********************************************************************
 * This file is part of wiringOli:
 *	https://
 *
 *    wiringOli is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringOli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringOli.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

// ----------------
// Constants
// ----------------
#define true 1
#define false 0

#define PRG_NAME            "teleinfo"
#define PRG_VERSION_NUMBER  "1.0.9"

#define UUCP_LOCK_DIR "/var/lock"

// Local File for teleinfo
//#define TELEINFO_DATALOG  "/tmp/teleinfo.log"
//#define TELEINFO_TRAMELOG "/tmp/teleinfotrame."
#define TELEINFO_BUFSIZE  512

// Teleinfo start and end of frame characters
#define STX 0x02
#define ETX 0x03

// Define mysql
#define MYSQL_HOST  "192.168.1.50"
#define MYSQL_LOGIN "morind79"
#define MYSQL_PASS  "DMO12lip6"
#define MYSQL_DB    "Domotic"
#define MYSQL_TABLE "DbiTeleinfo"
#define MYSQL_PORT  3306

void log_syslog( FILE * stream, const char *format, ...);
void show_stats(void);
void clean_exit (int exit_code);
void fatal (const char *format, ...);
int uucp_lockname(const char *dir, const char *file);
int uucp_lock(void);
int uucp_unlock(void);
int db_open( MYSQL * pmysql);
void db_close( MYSQL * pmysql) ;
int tlf_init_serial(void);
void tlf_close_serial(int device);
int tlf_checksum_ok(char *etiquette, char *valeur, char checksum);
void tlf_treat_label( char * plabel, char * pvalue);
int tlf_check_frame( char * pframe);
char* trim(char* pstr);
extern void readEnergyMeter();

#ifdef __cplusplus
}
#endif
