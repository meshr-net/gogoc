/*
-----------------------------------------------------------------------------
 $Id: config.c,v 1.3 2010/03/07 20:12:49 carl Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

/*  Configuration file handling. */

#include "platform.h"
#include "gogoc_status.h"

#include "config.h"
#include "log.h"
#include "hex_strings.h"
#include "cli.h"

/* gogoCLIENT Configuration Subsystem */
#define TBOOLEAN_DECLARED
#include <gogocconfig/gogoc_c_wrapper.h>
#include <gogocconfig/gogocuistrings.h>
#undef TBOOLEAN_DECLARED


#if !(defined(WIN32) || defined(WINCE))
static syslog_facility_t syslog_facilities[] = {
  { STR_CONFIG_SLOG_FACILITY_USER, LOG_USER },
  { STR_CONFIG_SLOG_FACILITY_LOCAL0, LOG_LOCAL0 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL1, LOG_LOCAL1 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL2, LOG_LOCAL2 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL3, LOG_LOCAL3 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL4, LOG_LOCAL4 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL5, LOG_LOCAL5 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL6, LOG_LOCAL6 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL7, LOG_LOCAL7 },
  { NULL, 0 }
};


/* ----------------------------------------------------------------------- */
/* Function: ParseSyslogFacility                                           */
/*                                                                         */
/* Description:                                                            */
/*   Parse the configuration file's 'syslog_facility' directive.           */
/*                                                                         */
/* Arguments:                                                              */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*   facility: char* [IN], The input syslog facility.                      */
/*                                                                         */
/* Return Values:                                                          */
/*   1 on error.                                                           */
/*   0 on successful completion.                                           */
/*                                                                         */
/* ----------------------------------------------------------------------- */
static sint32_t ParseSyslogFacility( tConf *pConf, char *facility )
{
  sint32_t index = 0;

  /* Loop through the known facility strings, and compare with the one we found. */
  while( (syslog_facilities != NULL) && (syslog_facilities[index].string != NULL) )
  {
    if (strcmp(facility, syslog_facilities[index].string) == 0)
    {
      pConf->syslog_facility = syslog_facilities[index].value;
      return 0;
    }
    index++;
  }

  return 1;
}
#endif


/* ----------------------------------------------------------------------- */
/* Function: tspReadConfigFile                                             */
/*                                                                         */
/* Description:                                                            */
/*   Will extract the configuration data from the configuration file.      */
/*                                                                         */
/* Arguments:                                                              */
/*   szFile: char* [IN], The input configuration filename.                 */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*                                                                         */
/* Return Values:                                                          */
/*   gogoc_err value.                                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
gogoc_status tspReadConfigFile( char* szFile, tConf* pConf )
{
  sint32_t i, nErrors, iRet;
  uint32_t* tErrors = NULL;
  char* szValue = NULL;


  /* Check input parameters. */
  if( szFile == NULL  ||  pConf == NULL )
  {
    return make_status(CTX_CFGVALIDATION, ERR_INVAL_CFG_FILE);
  }


  /* --------------------------------------------------------------------- */
  /* Read and load the configuration file.                                 */
  /* Will also perform thorough validation.                                */
  /* --------------------------------------------------------------------- */
  if( (iRet = initialize( szFile )) != 0 )
  {
    if( iRet == -1 )
    {
      /* Retrieve confguration error(s). */
      get_config_errors( &nErrors, &tErrors );

      for( i=0; i<nErrors; i++ )
        DirectErrorMessage( (char*)get_ui_string( tErrors[i] ) );
    }
    else
    {
      /* Initialization error */
      DirectErrorMessage( (char*)get_ui_string( iRet ) );
    }

    return make_status(CTX_CFGVALIDATION, ERR_INVAL_CFG_FILE);
  }


  /* --------------------------------------------------------------------- */
  /* Fill in the tConf structure from the file contents.                   */
  /* --------------------------------------------------------------------- */

  // Server is facultative in the gogoc-config validation routine, but not here.
  get_server( &(pConf->server) );
  if( strlen( pConf->server ) == 0 )
  {
    free( pConf->server );
    DirectErrorMessage( (char*)get_ui_string( GOGOC_UIS__G6V_SERVERMUSTBESPEC ) );
    return make_status(CTX_CFGVALIDATION, ERR_INVAL_CFG_FILE);
  }

  get_gogoc_dir( &(pConf->tsp_dir) );

#ifdef DSLITE_SUPPORT
  get_dslite_server( &(pConf->dslite_server) );
  get_dslite_client( &(pConf->dslite_client) );
#endif
  
  get_client_v4( &(pConf->client_v4) );

#ifdef V4V6_SUPPORT
  get_client_v6( &(pConf->client_v6) );
#endif /* V4V6_SUPPORT  */

  get_user_id( &(pConf->userid) );

  get_passwd( &(pConf->passwd) );

  get_auth_method( &(pConf->auth_method) );

  get_host_type( &(pConf->host_type) );

  get_template( &(pConf->template) );

  get_if_tun_v6v4( &(pConf->if_tunnel_v6v4) );

  get_if_tun_v6udpv4( &(pConf->if_tunnel_v6udpv4) );

#ifdef V4V6_SUPPORT
  get_if_tun_v4v6( &(pConf->if_tunnel_v4v6) );
#endif /* V4V6_SUPPORT */

  get_tunnel_mode( &szValue );

  if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6ANYV4) == 0) {
    pConf->tunnel_mode = V6ANYV4;
  }
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6V4) == 0) {
    pConf->tunnel_mode = V6V4;
  }
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0) {
    pConf->tunnel_mode = V6UDPV4;
  }
#ifdef V4V6_SUPPORT
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V4V6) == 0) {
    pConf->tunnel_mode = V4V6;
  }
#endif /* V4V6_SUPPORT */
#ifdef DSLITE_SUPPORT
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_DSLITE) == 0) {
    pConf->tunnel_mode = DSLITE;
  }
#endif /* V4V6_SUPPORT */
  free( szValue );  szValue = NULL;

  get_dns_server( &(pConf->dns_server) );

  get_ifprefix( &(pConf->if_prefix) );

  get_prefixlen( &(pConf->prefixlen) );

  get_retry_delay( &(pConf->retry_delay) );

  get_retry_delay_max( &(pConf->retry_delay_max) );

  get_keepalive( &(pConf->keepalive) );

  get_keepalive_interval( &(pConf->keepalive_interval) );

  get_proxy_client( &(pConf->proxy_client) );

#if !(defined(WIN32) || defined(WINCE))
  get_syslog_facility( &szValue );
  ParseSyslogFacility( pConf, szValue );
  free( szValue );  szValue = NULL;
#endif

  get_log_filename( &(pConf->log_filename) );

  get_log_rotation( &(pConf->log_rotation) );

  get_log_rotation_sz( &(pConf->log_rotation_size) );

  get_log_rotation_del( &(pConf->log_rotation_delete) );

  get_log( STR_CONFIG_LOG_DESTINATION_STDERR, &(pConf->log_level_stderr) );

  get_log( STR_CONFIG_LOG_DESTINATION_SYSLOG, &(pConf->log_level_syslog) );

  get_log( STR_CONFIG_LOG_DESTINATION_CONSOLE, &(pConf->log_level_console) );

  get_log( STR_CONFIG_LOG_DESTINATION_FILE, &(pConf->log_level_file) );

  get_auto_retry_connect( &(pConf->auto_retry_connect) );

  get_last_server_file( &(pConf->last_server_file) );

  get_always_use_last_server( &(pConf->always_use_same_server) );

  get_broker_list_file( &(pConf->broker_list_file) );

  get_haccess_web_enabled( &(pConf->haccess_web_enabled) );

  get_haccess_proxy_enabled( &(pConf->haccess_proxy_enabled) );

  get_haccess_document_root( &(pConf->haccess_document_root) );

  /* Close the gogoCLIENT configuration object. */
  un_initialize();

  /* Successful completion. */
  return make_status(CTX_CFGVALIDATION, SUCCESS);
}


/* ----------------------------------------------------------------------- */
/* Function: tspInitialize                                                 */
/*                                                                         */
/* Description:                                                            */
/*   Initialize with default values, read configuration file and override  */
/*   defaults with config file values.                                     */
/*                                                                         */
/* Arguments:                                                              */
/*   argc: char* [IN], Number of arguments passed on command line.         */
/*   argv: char*[] [IN], The command-line arguments.                       */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*                                                                         */
/* Return Values:                                                          */
/*   gogoc_err value                                                        */
/*                                                                         */
/* ----------------------------------------------------------------------- */
gogoc_status tspInitialize(sint32_t argc, char *argv[], tConf *pConf)
{
  tConf CmdLine;
  gogoc_status status = STATUS_SUCCESS_INIT;
  const char* cszTemplDir = "template";


  // Hard-coded parameters. Not configurable anymore.
  pConf->syslog = FALSE;
  pConf->protocol = pal_strdup( "default_route" );
  pConf->routing_info = pal_strdup("");


  /* --------------------------------------------------------------------- */
  /* Read configuration data from the command-line arguments.              */
  /* --------------------------------------------------------------------- */
  memset(&CmdLine, 0, sizeof(CmdLine));
  if( argc > 1 )
  {
    ParseArguments(argc, argv, &CmdLine);
  }

  /* --------------------------------------------------------------------- */
  /* Read configuration data from the file.                                */
  /* --------------------------------------------------------------------- */
  status = tspReadConfigFile(FileName, pConf);
  if( status_number(status) != SUCCESS )
  {
    return status;
  }

  /* --------------------------------------------------------------------- */
  /* Override the config file with parameters from the command line.       */
  /* --------------------------------------------------------------------- */
  if(CmdLine.if_tunnel_v6v4)
    pConf->if_tunnel_v6v4 = CmdLine.if_tunnel_v6v4;

  if(CmdLine.if_tunnel_v6udpv4)
    pConf->if_tunnel_v6udpv4 = CmdLine.if_tunnel_v6udpv4;

#ifdef V4V6_SUPPORT
  if(CmdLine.if_tunnel_v4v6)
    pConf->if_tunnel_v4v6 = CmdLine.if_tunnel_v4v6;
#endif /* V4V6_SUPPORT  */

  if(CmdLine.client_v4)
    pConf->client_v4 = CmdLine.client_v4;

  pConf->boot_mode = CmdLine.boot_mode;
  pConf->nodaemon = CmdLine.nodaemon;
  pConf->no_questions = CmdLine.no_questions;

  /* --------------------------------------------------------------------- */
  /* Extrapolate directory in which template scripts are located.          */
  /* --------------------------------------------------------------------- */
  if( strlen(pConf->tsp_dir) != 0 )
  {
    TspHomeDir = pConf->tsp_dir;
    if( (ScriptDir = (char*)malloc( (size_t)(strlen(pConf->tsp_dir)+strlen(cszTemplDir)+2)) ) == NULL )
    {
      DirectErrorMessage( STR_GEN_MALLOC_ERROR );
      return make_status(CTX_CFGVALIDATION, ERR_MEMORY_STARVATION);
    }
    sprintf(ScriptDir, "%s%c%s", pConf->tsp_dir, DirSeparator, cszTemplDir);
  }
  else
  {
    if((ScriptDir = (char *)malloc((size_t)(strlen(TspHomeDir)+strlen(cszTemplDir)+2)))==NULL)
    {
      DirectErrorMessage( STR_GEN_MALLOC_ERROR );
      return make_status(CTX_CFGVALIDATION, ERR_MEMORY_STARVATION);
    }
    sprintf(ScriptDir, "%s%c%s", TspHomeDir, DirSeparator, cszTemplDir);
  }

  return make_status(CTX_CFGVALIDATION, SUCCESS);
}
