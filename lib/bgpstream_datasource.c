/*
 * This file is part of bgpstream
 *
 * Copyright (C) 2015 The Regents of the University of California.
 * Authors: Alistair King, Chiara Orsini
 *
 * All rights reserved.
 *
 * This code has been developed by CAIDA at UC San Diego.
 * For more information, contact bgpstream-info@caida.org
 *
 * This source code is proprietary to the CAIDA group at UC San Diego and may
 * not be redistributed, published or disclosed without prior permission from
 * CAIDA.
 *
 * Report any bugs, questions or comments to bgpstream-info@caida.org
 *
 */

#include "config.h"
#include "utils.h"
#include "bgpstream_datasource.h"
#include "bgpstream_debug.h"

#include <inttypes.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errmsg.h>

#define DATASOURCE_BLOCKING_MIN_WAIT 30
#define DATASOURCE_BLOCKING_MAX_WAIT 150


#define GET_DEFAULT_STR_VALUE(var_store, default_value) \
  do {                                                  \
    if(strcmp(default_value, "not-set") == 0)           \
      {                                                 \
        var_store = NULL;                               \
      }                                                 \
    else                                                \
      {                                                 \
        var_store = strdup(default_value);              \
      }                                                 \
  } while(0)

#define GET_DEFAULT_INT_VALUE(var_store, default_value) \
  do {                                                  \
    if(strcmp(default_value, "not-set") == 0)           \
      {                                                 \
        var_store = 0;                                  \
      }                                                 \
    else                                                \
      {                                                 \
        var_store = atoi(default_value);                \
      }                                                 \
  } while(0)


/* datasource mgr related functions */


bgpstream_datasource_mgr_t *bgpstream_datasource_mgr_create(){
  bgpstream_debug("\tBSDS_MGR: create start");
  bgpstream_datasource_mgr_t *datasource_mgr = (bgpstream_datasource_mgr_t*) malloc(sizeof(bgpstream_datasource_mgr_t));
  if(datasource_mgr == NULL) {
    return NULL; // can't allocate memory
  }
  // default values
  datasource_mgr->datasource = BGPSTREAM_DATA_INTERFACE_MYSQL; // default data source
  datasource_mgr->blocking = 0;
  datasource_mgr->backoff_time = DATASOURCE_BLOCKING_MIN_WAIT;
  // datasources (none of them is active at the beginning)
  datasource_mgr->mysql_ds = NULL;
  datasource_mgr->singlefile_ds = NULL;
  datasource_mgr->csvfile_ds = NULL;
  datasource_mgr->sqlite_ds = NULL;
  datasource_mgr->broker_ds = NULL;
  datasource_mgr->status = BGPSTREAM_DATASOURCE_STATUS_OFF;
  // datasource options
  GET_DEFAULT_STR_VALUE(datasource_mgr->mysql_dbname,BGPSTREAM_DS_MYSQL_DB_NAME);
  GET_DEFAULT_STR_VALUE(datasource_mgr->mysql_user, BGPSTREAM_DS_MYSQL_DB_USER);
  GET_DEFAULT_STR_VALUE(datasource_mgr->mysql_password, BGPSTREAM_DS_MYSQL_DB_PASSWORD);
  GET_DEFAULT_STR_VALUE(datasource_mgr->mysql_host, BGPSTREAM_DS_MYSQL_DB_HOST);
  GET_DEFAULT_INT_VALUE(datasource_mgr->mysql_port, BGPSTREAM_DS_MYSQL_DB_PORT);
  GET_DEFAULT_STR_VALUE(datasource_mgr->mysql_socket, BGPSTREAM_DS_MYSQL_DB_SOCKET);
  GET_DEFAULT_STR_VALUE(datasource_mgr->mysql_dump_path, BGPSTREAM_DS_MYSQL_DUMP_PATH);
  GET_DEFAULT_STR_VALUE(datasource_mgr->singlefile_rib_mrtfile, BGPSTREAM_DS_SINGLEFILE_RIB_FILE);
  GET_DEFAULT_STR_VALUE(datasource_mgr->singlefile_upd_mrtfile, BGPSTREAM_DS_SINGLEFILE_UPDATE_FILE);
  GET_DEFAULT_STR_VALUE(datasource_mgr->csvfile_file, BGPSTREAM_DS_CSVFILE_CSV_FILE);
  GET_DEFAULT_STR_VALUE(datasource_mgr->sqlite_file, BGPSTREAM_DS_SQLITE_DB_FILE);
  GET_DEFAULT_STR_VALUE(datasource_mgr->broker_url, BGPSTREAM_DS_BROKER_URL);
  bgpstream_debug("\tBSDS_MGR: create end");
  return datasource_mgr;
}

void bgpstream_datasource_mgr_set_data_interface(bgpstream_datasource_mgr_t *datasource_mgr,
						 const bgpstream_data_interface_id_t datasource) {
  bgpstream_debug("\tBSDS_MGR: set data interface start");
  if(datasource_mgr == NULL) {
    return; // no manager
  }
  datasource_mgr->datasource = datasource;   
  bgpstream_debug("\tBSDS_MGR: set  data interface end");
}


void bgpstream_datasource_mgr_set_data_interface_option(bgpstream_datasource_mgr_t *datasource_mgr,
                                                        const bgpstream_data_interface_option_t *option_type,
							const char *option_value) {
  // this option has no effect if the datasource selected is not
  // using this option
  switch(option_type->if_id)
    {
    case BGPSTREAM_DATA_INTERFACE_MYSQL:
      switch(option_type->id)
        {
        case 0:
          if(datasource_mgr->mysql_dbname!=NULL)
            {
              free(datasource_mgr->mysql_dbname);
            }
          datasource_mgr->mysql_dbname = strdup(option_value);
          break;
        case 1:
          if(datasource_mgr->mysql_user!=NULL)
            {
              free(datasource_mgr->mysql_user);
            }
          datasource_mgr->mysql_user = strdup(option_value);
          break;
        case 2:
          if(datasource_mgr->mysql_password!=NULL)
            {
              free(datasource_mgr->mysql_password);
            }
          datasource_mgr->mysql_password = strdup(option_value);
          break;
        case 3: 
          if(datasource_mgr->mysql_host!=NULL)
            {
              free(datasource_mgr->mysql_host);
            }
          datasource_mgr->mysql_host = strdup(option_value);
          break;
        case 4: 
          datasource_mgr->mysql_port = atoi(option_value);
          break;
        case 5: 
          if(datasource_mgr->mysql_socket!=NULL)
            {
              free(datasource_mgr->mysql_socket);
            }
          datasource_mgr->mysql_socket = strdup(option_value);
          break;
        case 6: 
          if(datasource_mgr->mysql_dump_path!=NULL)
            {
              free(datasource_mgr->mysql_dump_path);
            }
          datasource_mgr->mysql_dump_path = strdup(option_value);
          break;
        }
      break;

    case BGPSTREAM_DATA_INTERFACE_SINGLEFILE:
      switch(option_type->id)
        {
        case 0:
          if(datasource_mgr->singlefile_rib_mrtfile!=NULL)
            {
              free(datasource_mgr->singlefile_rib_mrtfile);
            }
          datasource_mgr->singlefile_rib_mrtfile = strdup(option_value);
          break;
        case 1:
          if(datasource_mgr->singlefile_upd_mrtfile!=NULL)
            {
              free(datasource_mgr->singlefile_upd_mrtfile);
            }
          datasource_mgr->singlefile_upd_mrtfile = strdup(option_value);
          break;
        }
      break;

    case BGPSTREAM_DATA_INTERFACE_CSVFILE:
      switch(option_type->id)
        {
        case 0:
          if(datasource_mgr->csvfile_file!=NULL)
            {
              free(datasource_mgr->csvfile_file);
            }
          datasource_mgr->csvfile_file = strdup(option_value);
          break;
        }
      break;

      case BGPSTREAM_DATA_INTERFACE_SQLITE:
      switch(option_type->id)
        {
        case 0:
          if(datasource_mgr->sqlite_file!=NULL)
            {
              free(datasource_mgr->sqlite_file);
            }
          datasource_mgr->sqlite_file = strdup(option_value);
          break;
        }

    case BGPSTREAM_DATA_INTERFACE_BROKER:
      switch(option_type->id)
        {
        case 0:
          if(datasource_mgr->broker_url!=NULL)
            {
              free(datasource_mgr->broker_url);
            }
          datasource_mgr->broker_url = strdup(option_value);
          break;
        }
      break;
    }
}


void bgpstream_datasource_mgr_init(bgpstream_datasource_mgr_t *datasource_mgr,
				   bgpstream_filter_mgr_t *filter_mgr){
  bgpstream_debug("\tBSDS_MGR: init start");
  if(datasource_mgr == NULL) {
    return; // no manager
  }

  void *ds = NULL;
  
  switch(datasource_mgr->datasource)
    {
   
    case BGPSTREAM_DATA_INTERFACE_SINGLEFILE:
      datasource_mgr->singlefile_ds = bgpstream_singlefile_datasource_create(filter_mgr,
                                                                             datasource_mgr->singlefile_rib_mrtfile,
                                                                             datasource_mgr->singlefile_upd_mrtfile);
      ds = (void *) datasource_mgr->singlefile_ds;
      break;

    case BGPSTREAM_DATA_INTERFACE_CSVFILE:
      datasource_mgr->csvfile_ds = bgpstream_csvfile_datasource_create(filter_mgr,
                                                                       datasource_mgr->csvfile_file);
      ds = (void *) datasource_mgr->csvfile_ds;
      break;

    case BGPSTREAM_DATA_INTERFACE_SQLITE:
      datasource_mgr->sqlite_ds = bgpstream_sqlite_datasource_create(filter_mgr,
                                                                     datasource_mgr->sqlite_file);
      ds = (void *) datasource_mgr->sqlite_ds;
      break;

    case BGPSTREAM_DATA_INTERFACE_BROKER:
      datasource_mgr->broker_ds =
        bgpstream_broker_datasource_create(filter_mgr,
                                           datasource_mgr->broker_url);
      ds = (void *) datasource_mgr->broker_ds;
      break;

    case BGPSTREAM_DATA_INTERFACE_MYSQL:
      datasource_mgr->mysql_ds = bgpstream_mysql_datasource_create(filter_mgr, 
                                                                   datasource_mgr->mysql_dbname,
                                                                   datasource_mgr->mysql_user,
                                                                   datasource_mgr->mysql_password,
                                                                   datasource_mgr->mysql_host,
                                                                   datasource_mgr->mysql_port,
                                                                   datasource_mgr->mysql_socket,
                                                                   datasource_mgr->mysql_dump_path);
      ds = (void *) datasource_mgr->mysql_ds;
      break;

    default:
      ds = NULL;      
    }

  if(ds == NULL)
    {
      datasource_mgr->status = BGPSTREAM_DATASOURCE_STATUS_ERROR;
    } 
    else {
      datasource_mgr->status = BGPSTREAM_DATASOURCE_STATUS_ON;
    }
  bgpstream_debug("\tBSDS_MGR: init end");
}


void bgpstream_datasource_mgr_set_blocking(bgpstream_datasource_mgr_t *datasource_mgr){
  bgpstream_debug("\tBSDS_MGR: set blocking start");
  if(datasource_mgr == NULL) {
    return; // no manager
  }
  datasource_mgr->blocking = 1;
  bgpstream_debug("\tBSDS_MGR: set blocking end");
}


int bgpstream_datasource_mgr_update_input_queue(bgpstream_datasource_mgr_t *datasource_mgr,
						bgpstream_input_mgr_t *input_mgr) {
  bgpstream_debug("\tBSDS_MGR: get data start");
  if(datasource_mgr == NULL) {
    return -1; // no datasource manager
  }
  int results = -1;

  do{
    switch(datasource_mgr->datasource)
      {
      case BGPSTREAM_DATA_INTERFACE_SINGLEFILE:
        results = bgpstream_singlefile_datasource_update_input_queue(datasource_mgr->singlefile_ds, input_mgr);
        break;
      case BGPSTREAM_DATA_INTERFACE_CSVFILE:
        results = bgpstream_csvfile_datasource_update_input_queue(datasource_mgr->csvfile_ds, input_mgr);
        break;
      case BGPSTREAM_DATA_INTERFACE_SQLITE:
        results = bgpstream_sqlite_datasource_update_input_queue(datasource_mgr->sqlite_ds, input_mgr);
        break;
      case BGPSTREAM_DATA_INTERFACE_BROKER:
        results = bgpstream_broker_datasource_update_input_queue(datasource_mgr->broker_ds, input_mgr);
        break;
      case BGPSTREAM_DATA_INTERFACE_MYSQL:
        results = bgpstream_mysql_datasource_update_input_queue(datasource_mgr->mysql_ds, input_mgr);
        break;
      }
    if(results == 0 && datasource_mgr->blocking) {
	// results = 0 => 2+ time and database did not give any error
	sleep(datasource_mgr->backoff_time);
	datasource_mgr->backoff_time = datasource_mgr->backoff_time * 2;
	if(datasource_mgr->backoff_time > DATASOURCE_BLOCKING_MAX_WAIT) {
	  datasource_mgr->backoff_time = DATASOURCE_BLOCKING_MAX_WAIT;
	}
    }
    bgpstream_debug("\tBSDS_MGR: got %d (blocking: %d)", results, datasource_mgr->blocking);
  } while(datasource_mgr->blocking && results == 0);
  
  bgpstream_debug("\tBSDS_MGR: get data end");
  return results; 
}


void bgpstream_datasource_mgr_close(bgpstream_datasource_mgr_t *datasource_mgr) {
  bgpstream_debug("\tBSDS_MGR: close start");
  if(datasource_mgr == NULL) {
    return; // no manager to destroy
  }
  switch(datasource_mgr->datasource)
    {
    case BGPSTREAM_DATA_INTERFACE_SINGLEFILE:
      bgpstream_singlefile_datasource_destroy(datasource_mgr->singlefile_ds);
      datasource_mgr->singlefile_ds = NULL;
      break;
    case BGPSTREAM_DATA_INTERFACE_CSVFILE:
      bgpstream_csvfile_datasource_destroy(datasource_mgr->csvfile_ds);
      datasource_mgr->csvfile_ds = NULL;
      break;
    case BGPSTREAM_DATA_INTERFACE_SQLITE:
      bgpstream_sqlite_datasource_destroy(datasource_mgr->sqlite_ds);
      datasource_mgr->sqlite_ds = NULL;
      break;
    case BGPSTREAM_DATA_INTERFACE_BROKER:
      bgpstream_broker_datasource_destroy(datasource_mgr->broker_ds);
      datasource_mgr->broker_ds = NULL;
      break;
    case BGPSTREAM_DATA_INTERFACE_MYSQL:
      bgpstream_mysql_datasource_destroy(datasource_mgr->mysql_ds);
      datasource_mgr->mysql_ds = NULL;
      break;
    }
  datasource_mgr->status = BGPSTREAM_DATASOURCE_STATUS_OFF;
  bgpstream_debug("\tBSDS_MGR: close end");
}


void bgpstream_datasource_mgr_destroy(bgpstream_datasource_mgr_t *datasource_mgr) {
  bgpstream_debug("\tBSDS_MGR: destroy start");
  if(datasource_mgr == NULL) {
    return; // no manager to destroy
  }
  // destroy any active datasource (if they have not been destroyed before)
  if(datasource_mgr->singlefile_ds != NULL) {
    bgpstream_singlefile_datasource_destroy(datasource_mgr->singlefile_ds);
    datasource_mgr->singlefile_ds = NULL;
  }
  if(datasource_mgr->csvfile_ds != NULL) {
    bgpstream_csvfile_datasource_destroy(datasource_mgr->csvfile_ds);
    datasource_mgr->csvfile_ds = NULL;
  }
  if(datasource_mgr->sqlite_ds != NULL) {
    bgpstream_sqlite_datasource_destroy(datasource_mgr->sqlite_ds);
    datasource_mgr->sqlite_ds = NULL;
  }
  if(datasource_mgr->broker_ds != NULL) {
    bgpstream_broker_datasource_destroy(datasource_mgr->broker_ds);
    datasource_mgr->broker_ds = NULL;
  }
  if(datasource_mgr->mysql_ds != NULL) {
    bgpstream_mysql_datasource_destroy(datasource_mgr->mysql_ds);
    datasource_mgr->mysql_ds = NULL;
  }


  // destroy memory allocated for options
  if(datasource_mgr->singlefile_rib_mrtfile != NULL)
    {
      free(datasource_mgr->singlefile_rib_mrtfile);
    }
  if(datasource_mgr->singlefile_upd_mrtfile != NULL)
    {
      free(datasource_mgr->singlefile_upd_mrtfile);
    }
  if(datasource_mgr->csvfile_file!=NULL)
    {
      free(datasource_mgr->csvfile_file);
    }
  if(datasource_mgr->sqlite_file != NULL)
    {
      free(datasource_mgr->sqlite_file);
    }
  if(datasource_mgr->broker_url != NULL)
    {
      free(datasource_mgr->broker_url);
    }
  if(datasource_mgr->mysql_dbname!=NULL)
    {
      free(datasource_mgr->mysql_dbname);
    }
  if(datasource_mgr->mysql_user!=NULL)
    {
      free(datasource_mgr->mysql_user);
    }
  if(datasource_mgr->mysql_password!=NULL)
    {
      free(datasource_mgr->mysql_password);
    }
  if(datasource_mgr->mysql_host!=NULL)
    {
      free(datasource_mgr->mysql_host);
    }
  if(datasource_mgr->mysql_socket!=NULL)
    {
      free(datasource_mgr->mysql_socket);
    }
  if(datasource_mgr->mysql_dump_path!=NULL)
    {
      free(datasource_mgr->mysql_dump_path);
    }
  free(datasource_mgr);  
  bgpstream_debug("\tBSDS_MGR: destroy end");
}

