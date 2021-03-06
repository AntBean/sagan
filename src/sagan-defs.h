/* $Id$ */
/*
** Copyright (C) 2009-2015 Quadrant Information Security <quadrantsec.com>
** Copyright (C) 2009-2015 Champ Clark III <cclark@quadrantsec.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* sagan.h
 *
 * Sagan prototypes and definitions.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"             /* From autoconf */
#endif

#include <syslog.h>

#define PCRE_OVECCOUNT 30

/* Various buffers used during configurations loading */

#define CLASSBUF	1024
#define RULEBUF		5128
#define CONFBUF		1024

#define MAXPATH 	255		/* Max path for files/directories */
#define MAXHOST         128		/* Max host length */
#define MAXPROGRAM	32		/* Max syslog 'program' length */
#define MAXDATE		25		/* Max syslog 'date' length */
#define MAXTIME		10		/* Max syslog 'time length */
#define MAXFACILITY	25		/* Max syslog 'facility' length */
#define MAXPRIORITY	20		/* Max syslog 'priority' length */
#define MAXTAG		32		/* Max syslog 'tag' length */
#define MAXLEVEL	15		/* Max syslog 'level' length */

#define MAX_THREADS     4096            /* Max system threads */
#define MAX_SYSLOGMSG   10240		/* Max length of a syslog message */

#define MAX_VAR_NAME_SIZE  	64	/* Max "var" name size */
#define MAX_VAR_VALUE_SIZE 4096		/* Max "var" value size */

#define MAX_PCRE		10		/* Max PCRE within a rule */
#define MAX_CONTENT		30		/* Max 'content' within a rule */
#define MAX_META_CONTENT	10		/* Max 'meta_content' within a rule */
#define MAX_FLOWBITS		20		/* Max 'flowbits' within a rule */

#define MAX_REFERENCE		10		/* Max references within a rule */
#define MAX_PARSE_IP		10		/* Max IP to collect form log line via parse.c */

#define MAXIP		16		/* Max IP length.  Change to 64 for future IPv6 support */

#define LOCKFILE 	"/var/run/sagan/sagan.pid"
#define SAGANLOG	"/var/log/sagan/sagan.log"
#define ALERTLOG	"/var/log/sagan/alert"
#define SAGANLOGPATH	"/var/log/sagan"
#define FIFO		"/var/run/sagan.fifo"
#define RULE_PATH	"/usr/local/etc/sagan-rules"
#define HOME_NET	"any"
#define EXTERNAL_NET	"any"

#define RUNAS		"sagan"

#define PLOG_INTERFACE	"eth0"
#define PLOG_FILTER	"port 514"
#define PLOG_LOGDEV	"/dev/log"

#define TRACK_TIME	360
#define TRACK_CACHE	"/var/run/sagan/sagan.cache"

#define S_NORMAL	0
#define S_ERROR		1
#define S_WARN		2
#define S_DEBUG		3

#define DEFAULT_SYSLOG_FACILITY	LOG_AUTH
#define DEFAULT_SYSLOG_PRIORITY LOG_ALERT

#define PARSEIP_RETURN_STRING	0

#define DEFAULT_SMTP_SUBJECT 	"[Sagan]"

/* defaults if the user doesn't define */

#define MAX_OUTPUT_THREADS      50
#define MAX_PROCESSOR_THREADS   50

#define SUNDAY		1
#define MONDAY		2
#define TUESDAY		4
#define WEDNESDAY	8
#define THURSDAY	16
#define FRIDAY		32
#define SATURDAY	64

