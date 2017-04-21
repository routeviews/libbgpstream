/*
 * This file is part of bgpstream
 *
 * CAIDA, UC San Diego
 * bgpstream-info@caida.org
 *
 * Copyright (C) 2012 The Regents of the University of California.
 * Authors: Alistair King, Chiara Orsini
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "utils.h"
#include "bgpstream_resource.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/** A Type/Value structure for extra information about a specific resource */
typedef struct {

  /** The type of this attribute */
  bgpstream_resource_attr_type_t type;

  /** The value of this attribute */
  char *value;

} attr_t;

bgpstream_resource_t *
bgpstream_resource_create(bgpstream_transport_type_t transport_type,
                          bgpstream_format_type_t format_type,
                          const char *uri,
                          uint32_t initial_time,
                          uint32_t duration,
                          const char *project, const char *collector,
                          bgpstream_record_dump_type_t record_type)
{
  bgpstream_resource_t *res;

  if ((res = malloc_zero(sizeof(bgpstream_resource_t))) == NULL) {
    return NULL;
  }

  res->transport_type = transport_type;
  res->format_type = format_type;
  if ((res->uri = strdup(uri)) == NULL) {
    goto err;
  }
  res->initial_time = res->current_time = initial_time;
  res->duration = duration;
  if ((res->project = strdup(project)) == NULL) {
    goto err;
  }
  if ((res->collector = strdup(collector)) == NULL) {
    goto err;
  }
  res->record_type = record_type;

  return res;

 err:
  bgpstream_resource_destroy(res);
  return NULL;
}

void bgpstream_resource_destroy(bgpstream_resource_t *resource)
{
  free(resource->uri);
  resource->uri = NULL;

  free(resource->project);
  resource->project = NULL;

  free(resource->collector);
  resource->collector = NULL;

  free(resource);
}

/* ========== PUBLIC FUNCTIONS BELOW HERE ========== */

#if 0
int bgpstream_resource_set_attr(bgpstream_resource_t *resource,
                                bgpstream_resource_attr_type_t type,
                                const char *value)
{
  assert(type >= 0 && type < _BGPSTREAM_RESOURCE_ATTR_CNT);
  resource->attrs[type].type = type;
  if ((resource->attrs[type].value = strdup(value)) == NULL) {
    return -1;
  }

  return 0;
}

const char *
bgpstream_resource_get_attr(bgpstream_resource_t *resource,
                            bgpstream_resource_attr_type_t type)
{
  assert(type >= 0 && type < _BGPSTREAM_RESOURCE_ATTR_CNT);
  return resource->attrs[type].value;
}
#endif

bgpstream_transport_type_t
bgpstream_resource_get_transport_type(bgpstream_resource_t *resource)
{
  return resource->transport_type;
}

bgpstream_format_type_t
bgpstream_resource_get_format_type(bgpstream_resource_t *resource)
{
  return resource->format_type;
}

const char *
bgpstream_resource_get_uri(bgpstream_resource_t *resource)
{
  return resource->uri;
}

uint32_t
bgpstream_resource_get_initial_time(bgpstream_resource_t *resource)
{
  return resource->initial_time;
}

uint32_t
bgpstream_resource_get_duration(bgpstream_resource_t *resource)
{
  return resource->duration;
}

const char *
bgpstream_resource_get_project(bgpstream_resource_t *resource)
{
  return resource->project;
}

const char *
bgpstream_resource_get_collector(bgpstream_resource_t *resource)
{
  return resource->collector;
}

bgpstream_record_dump_type_t
bgpstream_resource_get_record_type(bgpstream_resource_t *resource)
{
  return resource->record_type;
}