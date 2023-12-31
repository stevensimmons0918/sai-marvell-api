/*
 * "$Id: mxml-private.h 309 2007-09-21 04:46:02Z mike $"
 *
 * Private definitions for Mini-XML, a small XML-like file parsing library.
 *
 * Copyright 2007 by Michael Sweet.
 *
 * Copyright 2011 Marvell International Ltd.
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation,  version 2 of the License.
 * The license can be found at:
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 *
 * 2011 - Adopted to Marvell CPSS LUA CLI - added _mxml_ prefix for all ANSI functions
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * Include necessary headers...
 */

#include <cpssCommon/private/mxml/prvCpssMxml.h>
#include <cpssCommon/private/mxml/prvCpssMxmlConfig.h>

/*
 * Global, per-thread data...
 */

typedef struct _mxml_global_s
{
  void  (*error_cb)(const char *);
  int   num_entity_cbs;
  int   (*entity_cbs[100])(const char *name);
  int   wrap;
  mxml_custom_load_cb_t custom_load_cb;
  mxml_custom_save_cb_t custom_save_cb;
} _mxml_global_t;


/*
 * Functions...
 */

extern _mxml_global_t   *_mxml_global(void);
extern int      _mxml_entity_cb(const char *name);


/*
 * End of "$Id: mxml-private.h 309 2007-09-21 04:46:02Z mike $".
 */
