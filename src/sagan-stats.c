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

/* sagan-stats.c
 *
 * Simply dumps statistics of Sagan to the user or via sagan.log
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"             /* From autoconf */
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

#include "sagan.h"
#include "sagan-defs.h"
#include "sagan-stats.h"
#include "sagan-config.h"

struct _SaganCounters *counters;
struct _SaganConfig *config;

void sagan_statistics( void )
{

    char timet[20];

    time_t t;
    struct tm *now;
//    unsigned long seconds = 0;
    int seconds = 0;
    unsigned long total=0;

    int uptime_days;
    int uptime_abovedays;
    int uptime_hours;
    int uptime_abovehours;
    int uptime_minutes;
    int uptime_seconds;

#ifdef WITH_WEBSENSE
    unsigned long websense_total=0;
#endif

    /* This is used to calulate the events per/second */
    /* Champ Clark III - 11/17/2011 */

    t = time(NULL);
    now=localtime(&t);
    strftime(timet, sizeof(timet), "%s",  now);
    seconds = atol(timet) - atol(config->sagan_startutime);

    /* if statement prevents floating point exception */

    if ( seconds != 0 )
        {
            total = counters->sagantotal / seconds;

#ifdef WITH_WEBSENSE
            websense_total = counters->websense_total / seconds;
#endif
        }


    if ((isatty(1)))
        {


            Sagan_Log(S_NORMAL, " ,-._,-.  -[ Sagan Version %s - Engine Statistics ]-", VERSION);
            Sagan_Log(S_NORMAL, " \\/)\"(\\/");
            Sagan_Log(S_NORMAL, "  (_o_)    Events processed         : %" PRIu64 "", counters->sagantotal);
            Sagan_Log(S_NORMAL, "  /   \\/)  Signatures matched       : %" PRIu64 " (%.3f%%)", counters->saganfound, CalcPct(counters->saganfound, counters->sagantotal ) );
            Sagan_Log(S_NORMAL, " (|| ||)   Alerts                   : %" PRIu64 " (%.3f%%)",  counters->alert_total, CalcPct( counters->alert_total, counters->sagantotal) );
            Sagan_Log(S_NORMAL, "  oo-oo    After                    : %" PRIu64 " (%.3f%%)",  counters->after_total, CalcPct( counters->after_total, counters->sagantotal) );
            Sagan_Log(S_NORMAL, "           Threshold                : %" PRIu64 " (%.3f%%)", counters->threshold_total, CalcPct( counters->threshold_total, counters->sagantotal) );
            Sagan_Log(S_NORMAL, "           Dropped                  : %" PRIu64 " (%.3f%%)", counters->sagan_processor_drop + counters->sagan_output_drop + counters->sagan_log_drop, CalcPct(counters->sagan_processor_drop + counters->sagan_output_drop + counters->sagan_log_drop, counters->sagantotal) );

//	    Sagan_Log(S_NORMAL, "           Malformed                : h:%" PRIu64 "|f:%" PRIu64 "|p:%" PRIu64 "|l:%" PRIu64 "|T:%" PRIu64 "|d:%" PRIu64 "|T:%" PRIu64 "|P:%" PRIu64 "|M:%" PRIu64 "", counters->malformed_host, counters->malformed_facility, counters->malformed_priority, counters->malformed_level, counters->malformed_tag, counters->malformed_date, counters->malformed_time, counters->malformed_program, counters->malformed_message);

            Sagan_Log(S_NORMAL, "           Thread Exhaustion        : %" PRIu64 " (%.3f%%)", counters->worker_thread_exhaustion,  CalcPct( counters->worker_thread_exhaustion, counters->sagantotal) );


            if (config->sagan_droplist_flag)
                {
                    Sagan_Log(S_NORMAL, "           Ignored Input            : %" PRIu64 " (%.3f%%)", counters->ignore_count, CalcPct(counters->ignore_count, counters->sagantotal) );
                }

#ifdef HAVE_LIBGEOIP
            Sagan_Log(S_NORMAL, "           GeoIP Hits:              : %" PRIu64 " (%.3f%%)", counters->geoip_hit, CalcPct( counters->geoip_hit, counters->sagantotal) );
            Sagan_Log(S_NORMAL, "           GeoIP Lookups:           : %" PRIu64 "", counters->geoip_lookup);
            Sagan_Log(S_NORMAL, "           GeoIP Misses             : %" PRIu64 "", counters->geoip_miss);
#endif

            uptime_days = seconds / 86400;
            uptime_abovedays = seconds % 86400;
            uptime_hours = uptime_abovedays / 3600;
            uptime_abovehours = uptime_abovedays % 3600;
            uptime_minutes = uptime_abovehours / 60;
            uptime_seconds = uptime_abovehours % 60;

            Sagan_Log(S_NORMAL, "           Uptime                   : %d days, %d hours, %d minutes, %d seconds.", uptime_days, uptime_hours, uptime_minutes, uptime_seconds);

            /* If processing from a file,  don't display events per/second */

            if ( config->sagan_fifo_flag == 0 )
                {

                    if ( seconds < 60 || seconds == 0 )
                        {
                            Sagan_Log(S_NORMAL, "           Avg. events per/second   : %lu [%lu of 60 seconds. Calculating...]", total, seconds);
                        }
                    else
                        {
                            Sagan_Log(S_NORMAL, "           Avg. events per/second   : %lu", total);
                        }
                }

            Sagan_Log(S_NORMAL, "");
            Sagan_Log(S_NORMAL, "          -[ Sagan Processor Statistics ]-");
            Sagan_Log(S_NORMAL, "");
            Sagan_Log(S_NORMAL, "           Dropped                  : %" PRIu64 " (%.3f%%)", counters->sagan_processor_drop, CalcPct(counters->sagan_processor_drop, counters->sagantotal) );

            if (config->blacklist_flag)
                {
                    Sagan_Log(S_NORMAL, "           Blacklist Hits           : %" PRIu64 " (%.3f%%)", counters->blacklist_hit_count, CalcPct(counters->blacklist_hit_count, counters->sagantotal) );
                }

            if (config->sagan_track_clients_flag)
                {
                    Sagan_Log(S_NORMAL, "           Tracking/Down            : %" PRIu64 " / %"PRIu64 " [%d minutes]" , counters->track_clients_client_count, counters->track_clients_down, config->pp_sagan_track_clients);
                }


            if (config->output_thread_flag)
                {
                    Sagan_Log(S_NORMAL, "");
                    Sagan_Log(S_NORMAL, "          -[ Sagan Output Plugin Statistics ]-");
                    Sagan_Log(S_NORMAL, "");
                    Sagan_Log(S_NORMAL,"           Dropped                  : %" PRIu64 " (%.3f%%)", counters->sagan_output_drop, CalcPct(counters->sagan_output_drop, counters->sagantotal) );
                }

#ifdef HAVE_LIBESMTP
            if ( config->sagan_esmtp_flag )
                {
                    Sagan_Log(S_NORMAL, "           Email Success/Failed     : %" PRIu64 " / %" PRIu64 "" , counters->esmtp_count_success, counters->esmtp_count_failed);
                }
#endif


            if (config->syslog_src_lookup)
                {
                    Sagan_Log(S_NORMAL, "");
                    Sagan_Log(S_NORMAL, "          -[ Sagan DNS Cache Statistics ]-");
                    Sagan_Log(S_NORMAL, "");
                    Sagan_Log(S_NORMAL, "           Cached                   : %" PRIu64 "", counters->dns_cache_count);
                    Sagan_Log(S_NORMAL, "           Missed                   : %" PRIu64 " (%.3f%%)", counters->dns_miss_count, CalcPct(counters->dns_miss_count, counters->dns_cache_count));
                }

#ifdef WITH_WEBSENSE

            if (config->websense_flag)
                {
                    Sagan_Log(S_NORMAL, "");
                    Sagan_Log(S_NORMAL, "          -[ Sagan Websense Processor ]-");
                    Sagan_Log(S_NORMAL, "");
                    Sagan_Log(S_NORMAL, "          Entries in cache          : %" PRIu64 " (%.3f%%)", counters->websense_cache_count, CalcPct(counters->websense_cache_count, config->websense_max_cache));
                    Sagan_Log(S_NORMAL, "          Hits from cache           : %" PRIu64 " (%.3f%%)", counters->websense_cache_hit, CalcPct(counters->websense_cache_hit, counters->websense_cache_count));
                    Sagan_Log(S_NORMAL, "          Errors                    : %" PRIu64 "", counters->websense_error_count);
                    Sagan_Log(S_NORMAL, "          Websense hits in logs     : %" PRIu64 "", counters->websense_postive_hit);
                    Sagan_Log(S_NORMAL, "          Queries per/second        : %lu", websense_total);

//if ( config->websense_flag ) Sagan_Log(S_NORMAL, "Websense Cache Statistics - Cached: %" PRIu64 " Hits: %" PRIu64 " Total: %" PRIu64 " Queries Per/Sec: %lu", counters->websense_cache_count, counters->websense_cache_hit, counters->websense_total, websense_total );
                }
#endif

            Sagan_Log(S_NORMAL, "-------------------------------------------------------------------------------");


        }
}
