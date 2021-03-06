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

/* sagan-bro-intel.c
*
* This allows Sagan to read in Bro Intel files,  like those from Critical
* Stack (https://intel.brointel.com).
*
*/

/* TODO:  needs stats and perfmon! */


#ifdef HAVE_CONFIG_H
#include "config.h"             /* From autoconf */
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


#include "sagan.h"
#include "sagan-defs.h"
#include "sagan-config.h"

#include "parsers/parsers.h"

#include "processors/sagan-bro-intel.h"

#define MAX_BROINTEL_LINE_SIZE 10240

struct _SaganConfig *config;
struct _SaganCounters *counters;
struct _SaganDebug *debug;

struct _Sagan_Processor_Info *processor_info_brointel = NULL;

struct _Sagan_BroIntel_Intel_Addr *Sagan_BroIntel_Intel_Addr;
struct _Sagan_BroIntel_Intel_Domain *Sagan_BroIntel_Intel_Domain;
struct _Sagan_BroIntel_Intel_File_Hash *Sagan_BroIntel_Intel_File_Hash;
struct _Sagan_BroIntel_Intel_URL *Sagan_BroIntel_Intel_URL;
struct _Sagan_BroIntel_Intel_Software *Sagan_BroIntel_Intel_Software;
struct _Sagan_BroIntel_Intel_Email *Sagan_BroIntel_Intel_Email;
struct _Sagan_BroIntel_Intel_User_Name *Sagan_BroIntel_Intel_User_Name;
struct _Sagan_BroIntel_Intel_File_Name *Sagan_BroIntel_Intel_File_Name;
struct _Sagan_BroIntel_Intel_Cert_Hash *Sagan_BroIntel_Intel_Cert_Hash;

/*****************************************************************************
 * Sagan_BroIntel_Init - Sets up globals.  Not really used yet.
 *****************************************************************************/

void Sagan_BroIntel_Init(void)
{

    processor_info_brointel = malloc(sizeof(struct _Sagan_Processor_Info));
    memset(processor_info_brointel, 0, sizeof(_Sagan_Processor_Info));

    /* This really isn't being used (yet)? */

    processor_info_brointel->processor_name          =       BROINTEL_PROCESSOR_NAME;
    processor_info_brointel->processor_generator_id  =       BROINTEL_PROCESSOR_GENERATOR_ID;
    processor_info_brointel->processor_name          =       BROINTEL_PROCESSOR_NAME;
    processor_info_brointel->processor_facility      =       BROINTEL_PROCESSOR_FACILITY;
    processor_info_brointel->processor_priority      =       BROINTEL_PROCESSOR_PRIORITY;
    processor_info_brointel->processor_pri           =       BROINTEL_PROCESSOR_PRI;
    processor_info_brointel->processor_class         =       BROINTEL_PROCESSOR_CLASS;
    processor_info_brointel->processor_tag           =       BROINTEL_PROCESSOR_TAG;
    processor_info_brointel->processor_rev           =       BROINTEL_PROCESSOR_REV;

    /* Init memory for processor / storage of Bro Intel data */

    Sagan_BroIntel_Intel_Addr = malloc(sizeof(_Sagan_BroIntel_Intel_Addr));
    Sagan_BroIntel_Intel_Domain = malloc(sizeof(_Sagan_BroIntel_Intel_Domain));
    Sagan_BroIntel_Intel_File_Hash = malloc(sizeof(_Sagan_BroIntel_Intel_File_Hash));
    Sagan_BroIntel_Intel_URL = malloc(sizeof(_Sagan_BroIntel_Intel_URL));
    Sagan_BroIntel_Intel_Software = malloc(sizeof(_Sagan_BroIntel_Intel_Software));
    Sagan_BroIntel_Intel_Email = malloc(sizeof(_Sagan_BroIntel_Intel_Email));
    Sagan_BroIntel_Intel_User_Name = malloc(sizeof(_Sagan_BroIntel_Intel_User_Name));
    Sagan_BroIntel_Intel_File_Name = malloc(sizeof(_Sagan_BroIntel_Intel_File_Name));
    Sagan_BroIntel_Intel_Cert_Hash = malloc(sizeof(_Sagan_BroIntel_Intel_Cert_Hash));


}

/*****************************************************************************
 * Sagan_BroIntel_Load_File - Loads BroIntel data and splits it up
 * into different arrays.
 * ***************************************************************************/

void Sagan_BroIntel_Load_File ( void )
{

    FILE *brointel_file;

    char *value;
    char *type;
    char *description;

    sbool found_flag;
    sbool found_flag_array;

    char *tok = NULL; ;
    char *ptmp = NULL;

    int line_count;
    int i;

    uint32_t u32_ip;

    char *brointel_filename = NULL;
    char brointelbuf[MAX_BROINTEL_LINE_SIZE] = { 0 };

    counters->brointel_dups = 0;

    brointel_filename = strtok_r(config->brointel_files, ",", &ptmp);

    while ( brointel_filename != NULL )
        {

            Sagan_Log(S_NORMAL, "Bro Intel Processor Loading File: %s.", brointel_filename);

            if (( brointel_file = fopen(brointel_filename, "r")) == NULL )
                {
                    Sagan_Log(S_ERROR, "[%s, line %d] Could not load Bro Intel file! (%s - %s)", __FILE__, __LINE__, brointel_filename, strerror(errno));
                }

            while(fgets(brointelbuf, MAX_BROINTEL_LINE_SIZE, brointel_file) != NULL)
                {

                    /* Skip comments and blank linkes */

                    if (brointelbuf[0] == '#' || brointelbuf[0] == 10 || brointelbuf[0] == ';' || brointelbuf[0] == 32 )
                        {
                            line_count++;
                            continue;
                        }
                    else
                        {

                            Remove_Return(brointelbuf);

                            value = strtok_r(brointelbuf, "\t", &tok);
                            type = strtok_r(NULL, "\t", &tok);
                            description = strtok_r(NULL, "\t", &tok);

                            if ( value == NULL || type == NULL || description == NULL )
                                {
                                    Sagan_Log(S_WARN, "[%s, line %d] Got invalid line at %d in %s", __FILE__, __LINE__, line_count, brointel_filename);
                                }

                            found_flag = 0;

                            if (!strcmp(type, "Intel::ADDR"))
                                {

                                    u32_ip = IP2Bit(value);

                                    found_flag = 1; 			/* Used to short circuit other 'type' lookups */
                                    found_flag_array = 0;		/* Used to short circuit/warn when dups are found.  This way we don't waste memory/CPU */

                                    /* Check for duplicates. */

                                    for (i=0; i < counters->brointel_addr_count; i++)
                                        {

                                            if ( u32_ip == Sagan_BroIntel_Intel_Addr[i].u32_ip )
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::ADDR address %s in %s on line %d.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }

                                    if ( found_flag_array == 0 )
                                        {

                                            Sagan_BroIntel_Intel_Addr = (_Sagan_BroIntel_Intel_Addr *) realloc(Sagan_BroIntel_Intel_Addr, (counters->brointel_addr_count+1) * sizeof(_Sagan_BroIntel_Intel_Addr));
                                            Sagan_BroIntel_Intel_Addr[counters->brointel_addr_count].u32_ip = IP2Bit(value);
                                            counters->brointel_addr_count++;
                                        }

                                }

                            if (!strcmp(type, "Intel::DOMAIN") && found_flag == 0)
                                {
                                    To_LowerC(value);

                                    found_flag = 1;
                                    found_flag_array = 0;


                                    for (i=0; i < counters-> brointel_domain_count; i++)
                                        {
                                            if (!strcmp(Sagan_BroIntel_Intel_Domain[i].domain, value))
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::DOMAIN '%s' in %s on line %d.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }

                                    if ( found_flag_array == 0 )
                                        {
                                            Sagan_BroIntel_Intel_Domain = (_Sagan_BroIntel_Intel_Domain *) realloc(Sagan_BroIntel_Intel_Domain, (counters->brointel_domain_count+1) * sizeof(_Sagan_BroIntel_Intel_Domain));
                                            strlcpy(Sagan_BroIntel_Intel_Domain[counters->brointel_domain_count].domain, value, sizeof(Sagan_BroIntel_Intel_Domain[counters->brointel_domain_count].domain));
                                            counters->brointel_domain_count++;
                                        }

                                }

                            if (!strcmp(type, "Intel::FILE_HASH") && found_flag == 0)
                                {
                                    To_LowerC(value);

                                    found_flag = 1;
                                    found_flag_array = 0;

                                    for (i=0; i < counters->brointel_file_hash_count; i++)
                                        {
                                            if (!strcmp(Sagan_BroIntel_Intel_File_Hash[i].hash, value))
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::FILE_HASH '%s' in %s on line %d.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }

                                    if ( found_flag_array == 0 )
                                        {

                                            Sagan_BroIntel_Intel_File_Hash = (_Sagan_BroIntel_Intel_File_Hash *) realloc(Sagan_BroIntel_Intel_File_Hash, (counters->brointel_file_hash_count+1) * sizeof(_Sagan_BroIntel_Intel_File_Hash));
                                            strlcpy(Sagan_BroIntel_Intel_File_Hash[counters->brointel_file_hash_count].hash, value, sizeof(Sagan_BroIntel_Intel_File_Hash[counters->brointel_file_hash_count].hash));
                                            counters->brointel_file_hash_count++;

                                        }
                                }


                            if (!strcmp(type, "Intel::URL") && found_flag == 0)
                                {

                                    To_LowerC(value);

                                    found_flag = 1;
                                    found_flag_array = 0;

                                    for (i=0; i < counters->brointel_url_count; i++)
                                        {
                                            if (!strcmp(Sagan_BroIntel_Intel_URL[i].url, value))
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::URL '%s' in %s on line %d.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }


                                    if ( found_flag_array == 0 )
                                        {
                                            Sagan_BroIntel_Intel_URL = (_Sagan_BroIntel_Intel_URL *) realloc(Sagan_BroIntel_Intel_URL, (counters->brointel_url_count+1) * sizeof(_Sagan_BroIntel_Intel_URL));
                                            strlcpy(Sagan_BroIntel_Intel_URL[counters->brointel_url_count].url, value, sizeof(Sagan_BroIntel_Intel_URL[counters->brointel_url_count].url));
                                            counters->brointel_url_count++;
                                        }

                                }


                            if (!strcmp(type, "Intel::SOFTWARE") && found_flag == 0)
                                {


                                    To_LowerC(value);

                                    found_flag = 1;
                                    found_flag_array = 0;

                                    for (i=0; i < counters->brointel_software_count++; i++)
                                        {
                                            if (!strcmp(Sagan_BroIntel_Intel_Software[i].software, value))
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::SOFTWARE '%s' in %s on line %d.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }

                                    if ( found_flag_array == 0 )
                                        {

                                            Sagan_BroIntel_Intel_Software = (_Sagan_BroIntel_Intel_Software *) realloc(Sagan_BroIntel_Intel_Software, (counters->brointel_software_count+1) * sizeof(_Sagan_BroIntel_Intel_Software));
                                            strlcpy(Sagan_BroIntel_Intel_Software[counters->brointel_software_count].software, value, sizeof(Sagan_BroIntel_Intel_Software[counters->brointel_software_count].software));
                                            counters->brointel_software_count++;

                                        }
                                }

                            if (!strcmp(type, "Intel::EMAIL") && found_flag == 0)
                                {

                                    To_LowerC(value);

                                    found_flag_array = 0;

                                    for (i=0; i < counters->brointel_email_count; i++)
                                        {
                                            if (!strcmp(Sagan_BroIntel_Intel_Email[i].email, value))
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::EMAIL '%s' in %s on line %d.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }

                                    if ( found_flag_array == 0 )
                                        {

                                            Sagan_BroIntel_Intel_Email = (_Sagan_BroIntel_Intel_Email *) realloc(Sagan_BroIntel_Intel_Email, (counters->brointel_email_count+1) * sizeof(_Sagan_BroIntel_Intel_Email));

                                            strlcpy(Sagan_BroIntel_Intel_Email[counters->brointel_email_count].email, value, sizeof(Sagan_BroIntel_Intel_Email[counters->brointel_email_count].email));
                                            counters->brointel_email_count++;
                                            found_flag = 1;
                                        }

                                }


                            if (!strcmp(type, "Intel::USER_NAME") && found_flag == 0)
                                {
                                    To_LowerC(value);

                                    found_flag = 1;
                                    found_flag_array = 0;

                                    for (i=0; i < counters->brointel_user_name_count; i++)
                                        {
                                            if (!strcmp(Sagan_BroIntel_Intel_User_Name[i].username, value))
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::USER_NAME '%s' in %s on line %.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }

                                    if ( found_flag_array == 0 )
                                        {


                                            Sagan_BroIntel_Intel_User_Name = (_Sagan_BroIntel_Intel_User_Name *) realloc(Sagan_BroIntel_Intel_User_Name, (counters->brointel_user_name_count+1) * sizeof(_Sagan_BroIntel_Intel_User_Name));


                                            strlcpy(Sagan_BroIntel_Intel_User_Name[counters->brointel_user_name_count].username, value, sizeof(Sagan_BroIntel_Intel_User_Name[counters->brointel_user_name_count].username));
                                            counters->brointel_user_name_count++;
                                        }
                                }

                            if (!strcmp(type, "Intel::FILE_NAME") && found_flag == 0)
                                {

                                    To_LowerC(value);

                                    found_flag = 1;
                                    found_flag_array = 0;

                                    for (i=0; i < counters->brointel_file_name_count; i++)
                                        {
                                            if (!strcmp(Sagan_BroIntel_Intel_File_Name[i].file_name, value))
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::FILE_NAME '%s' in %s on line %d.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }


                                    if ( found_flag_array == 0 )
                                        {

                                            Sagan_BroIntel_Intel_File_Name = (_Sagan_BroIntel_Intel_File_Name *) realloc(Sagan_BroIntel_Intel_File_Name, (counters->brointel_file_name_count+1) * sizeof(_Sagan_BroIntel_Intel_File_Name));

                                            strlcpy(Sagan_BroIntel_Intel_File_Name[counters->brointel_file_name_count].file_name, value, sizeof(Sagan_BroIntel_Intel_File_Name[counters->brointel_file_name_count].file_name));
                                            counters->brointel_file_name_count++;
                                        }

                                }

                            if (!strcmp(type, "Intel::CERT_HASH") && found_flag == 0)
                                {
                                    To_LowerC(value);

                                    found_flag = 1;
                                    found_flag_array = 0;

                                    for (i=0; i < counters->brointel_cert_hash_count; i++)
                                        {
                                            if (!strcmp(Sagan_BroIntel_Intel_Cert_Hash[i].cert_hash, value))
                                                {
                                                    Sagan_Log(S_WARN, "[%s, line %d] Got duplicate Intel::CERT_HASH '%s' in %s on line %d.", __FILE__, __LINE__, value, brointel_filename, line_count + 1);
                                                    counters->brointel_dups++;
                                                    found_flag_array = 1;
                                                }
                                        }

                                    if ( found_flag_array == 0 )
                                        {
                                            Sagan_BroIntel_Intel_Cert_Hash = (_Sagan_BroIntel_Intel_Cert_Hash *) realloc(Sagan_BroIntel_Intel_Cert_Hash, (counters->brointel_cert_hash_count+1) * sizeof(_Sagan_BroIntel_Intel_Cert_Hash));


                                            strlcpy(Sagan_BroIntel_Intel_Cert_Hash[counters->brointel_cert_hash_count].cert_hash, value, sizeof(Sagan_BroIntel_Intel_Cert_Hash[counters->brointel_cert_hash_count].cert_hash));
                                            counters->brointel_cert_hash_count++;
                                        }
                                }


                        }

                    line_count++;

                }
            fclose(brointel_file);
            brointel_filename = strtok_r(NULL, ",", &ptmp);
            line_count = 0;
        }

}

/*****************************************************************************
 * Sagan_BroIntel_IPADDR - Search array for blacklisted IP addresses
 *****************************************************************************/

sbool Sagan_BroIntel_IPADDR ( uint32_t ip )
{

    int i;

    /* If RFC1918,  we can short circuit here */

    if ( is_rfc1918(ip))
        {

            if ( debug->debugbrointel )
                {
                    Sagan_Log(S_DEBUG, "[%s, line %d] %u is RFC1918.", __FILE__, __LINE__, ip);
                }

            return(false);
        }

    /* Search array for for the IP address */

    for ( i = 0; i < counters->brointel_addr_count; i++)
        {

            if ( Sagan_BroIntel_Intel_Addr[i].u32_ip == ip )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found IP %u.", __FILE__, __LINE__, ip);
                        }

                    return(true);
                }

        }

    return(false);

}

/*****************************************************************************
 * Sagan_BroIntel_IPADDR_All - Search and tests _all_ IP addresses within
 * a syslog_message (reguardless of lognorm/parse ip)!
 *****************************************************************************/

sbool Sagan_BroIntel_IPADDR_All ( char *syslog_message )
{

    int i;
    int b;

    uint32_t ip;

    char *results = NULL;


    for (i = 1; i < MAX_PARSE_IP; i++)
        {

            results = parse_ip(syslog_message, i);

            /* Failed to find next IP,  short circuit the process */

            if (!strcmp(results, "0"))
                {
                    return(false);
                }

            ip = IP2Bit(results);

            for ( b = 0; b < counters->brointel_addr_count; b++ )
                {

                    if ( Sagan_BroIntel_Intel_Addr[b].u32_ip == ip )
                        {
                            return(true);
                        }
                }

        }

    return(false);
}

/*****************************************************************************
 * Sagan_BroIntel_DOMAIN - Search DOMAIN array
 *****************************************************************************/

sbool Sagan_BroIntel_DOMAIN ( char *syslog_message )
{

    int i;

    for ( i = 0; i < counters->brointel_domain_count; i++)
        {

            if ( Sagan_stristr(syslog_message, Sagan_BroIntel_Intel_Domain[i].domain, false) )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found domain %s.", __FILE__, __LINE__, Sagan_BroIntel_Intel_Domain[i].domain);
                        }

                    return(true);
                }

        }

    return(false);

}

/*****************************************************************************
 * Sagan_BroIntel_FILE_HASH - Search FILE_HASH array
 *****************************************************************************/

sbool Sagan_BroIntel_FILE_HASH ( char *syslog_message )
{

    int i;

    for ( i = 0; i < counters->brointel_file_hash_count; i++)
        {

            if ( Sagan_stristr(syslog_message, Sagan_BroIntel_Intel_File_Hash[i].hash, false) )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found file hash %s.", __FILE__, __LINE__, Sagan_BroIntel_Intel_File_Hash[i].hash);
                        }

                    return(true);
                }

        }

    return(false);

}

/*****************************************************************************
 * Sagan_BroIntel_URL - Search URL array
 *****************************************************************************/

sbool Sagan_BroIntel_URL ( char *syslog_message )
{

    int i;

    for ( i = 0; i < counters->brointel_url_count; i++)
        {

            if ( Sagan_stristr(syslog_message, Sagan_BroIntel_Intel_URL[i].url, false) )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found URL \"%s\".", __FILE__, __LINE__, Sagan_BroIntel_Intel_URL[i].url);
                        }

                    return(true);
                }

        }

    return(false);
}

/*****************************************************************************
 * Sagan_BroIntel_SOFTWARE - Search SOFTWARE array
 ****************************************************************************/

sbool Sagan_BroIntel_SOFTWARE ( char *syslog_message )
{

    int i;

    for ( i = 0; i < counters->brointel_software_count; i++)
        {

            if ( Sagan_stristr(syslog_message, Sagan_BroIntel_Intel_Software[i].software, false) )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found software \"%s\".", __FILE__, __LINE__, Sagan_BroIntel_Intel_Software[i].software);
                        }

                    return(true);
                }

        }

    return(false);
}

/*****************************************************************************
 * Sagan_BroIntel_EMAIL - Search EMAIL array
 *****************************************************************************/

sbool Sagan_BroIntel_EMAIL ( char *syslog_message )
{

    int i;

    for ( i = 0; i < counters->brointel_email_count; i++)
        {

            if ( Sagan_stristr(syslog_message, Sagan_BroIntel_Intel_Email[i].email, false) )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found e-mail address \"%s\".", __FILE__, __LINE__, Sagan_BroIntel_Intel_Email[i].email);
                        }

                    return(true);
                }

        }

    return(false);
}

/*****************************************************************************
 * Sagan_BroIntel_USER_NAME - Search USER_NAME array
 ****************************************************************************/

sbool Sagan_BroIntel_USER_NAME ( char *syslog_message )
{

    int i;

    for ( i = 0; i < counters->brointel_user_name_count; i++)
        {

            if ( Sagan_stristr(syslog_message, Sagan_BroIntel_Intel_User_Name[i].username, false) )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found the username \"%s\".", __FILE__, __LINE__, Sagan_BroIntel_Intel_User_Name[i].username);
                        }

                    return(true);
                }

        }

    return(false);
}

/****************************************************************************
 * Sagan_BroIntel_FILE_NAME - Search FILE_NAME array
 ****************************************************************************/

sbool Sagan_BroIntel_FILE_NAME ( char *syslog_message )
{

    int i;

    for ( i = 0; i < counters->brointel_file_name_count; i++)
        {

            if ( Sagan_stristr(syslog_message, Sagan_BroIntel_Intel_File_Name[i].file_name, false) )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found the file name \"%s\".", __FILE__, __LINE__, Sagan_BroIntel_Intel_File_Name[i].file_name);
                        }

                    return(true);
                }

        }

    return(false);
}

/***************************************************************************
 * Sagan_BroIntel_CERT_HASH - Search CERT_HASH array
 ***************************************************************************/

sbool Sagan_BroIntel_CERT_HASH ( char *syslog_message )
{

    int i;

    for ( i = 0; i < counters->brointel_cert_hash_count; i++)
        {

            if ( Sagan_stristr(syslog_message, Sagan_BroIntel_Intel_Cert_Hash[i].cert_hash, false) )
                {
                    if ( debug->debugbrointel )
                        {
                            Sagan_Log(S_DEBUG, "[%s, line %d] Found the CERT_HASH \"%s\".", __FILE__, __LINE__, Sagan_BroIntel_Intel_Cert_Hash[i].cert_hash);
                        }

                    return(true);
                }

        }

    return(false);
}

