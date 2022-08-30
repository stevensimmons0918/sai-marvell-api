/*
 * "$Id: mxml.h 385 2009-03-19 05:38:52Z mike $"
 *
 * Header file for Mini-XML, a small XML-like file parsing library.
 *
 * Copyright 2003-2009 by Michael Sweet.
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
 * Prevent multiple inclusion...
 */

#ifndef _mxml_h_
#  define _mxml_h_

/*
 * Include necessary headers...
 */

#if 0
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <ctype.h>
#  include <errno.h>
#endif


#include <gtOs/gtGenTypes.h>

/*
 * Constants...
 */

#  define MXML_TAB      8   /* Tabs every N columns */

#  define MXML_NO_CALLBACK  0   /* Don't use a type callback */
#  define MXML_INTEGER_CALLBACK mxml_integer_cb
                    /* Treat all data as integers */
#  define MXML_OPAQUE_CALLBACK  mxml_opaque_cb
                    /* Treat all data as opaque */
#if 0
#  define MXML_REAL_CALLBACK    mxml_real_cb
                    /* Treat all data as real numbers */
#endif
#  define MXML_TEXT_CALLBACK    0   /* Treat all data as text */
#  define MXML_IGNORE_CALLBACK  mxml_ignore_cb
                    /* Ignore all non-element content */

#  define MXML_NO_PARENT    0   /* No parent for the node */

#  define MXML_DESCEND      1   /* Descend when finding/walking */
#  define MXML_NO_DESCEND   0   /* Don't descend when finding/walking */
#  define MXML_DESCEND_FIRST    -1  /* Descend for first find */

#  define MXML_WS_BEFORE_OPEN   0   /* Callback for before open tag */
#  define MXML_WS_AFTER_OPEN    1   /* Callback for after open tag */
#  define MXML_WS_BEFORE_CLOSE  2   /* Callback for before close tag */
#  define MXML_WS_AFTER_CLOSE   3   /* Callback for after close tag */

#  define MXML_ADD_BEFORE   0   /* Add node before specified node */
#  define MXML_ADD_AFTER    1   /* Add node after specified node */
#  define MXML_ADD_TO_PARENT    NULL    /* Add node relative to parent */


/*
 * Data types...
 */

typedef enum mxml_sax_event_e       /**** SAX event type. ****/
{
  MXML_SAX_CDATA,           /* CDATA node */
  MXML_SAX_COMMENT,         /* Comment node */
  MXML_SAX_DATA,            /* Data node */
  MXML_SAX_DIRECTIVE,           /* Processing directive node */
  MXML_SAX_ELEMENT_CLOSE,       /* Element closed */
  MXML_SAX_ELEMENT_OPEN         /* Element opened */
} mxml_sax_event_t;

typedef enum mxml_type_e        /**** The XML node type. ****/
{
  MXML_IGNORE = -1,         /* Ignore/throw away node @since Mini-XML 2.3@ */
  MXML_ELEMENT,             /* XML element with attributes */
  MXML_INTEGER,             /* Integer value */
  MXML_OPAQUE,              /* Opaque string */
#if 0
  MXML_REAL,                /* Real value */
#endif
  MXML_TEXT,                /* Text fragment */
  MXML_CUSTOM               /* Custom data @since Mini-XML 2.1@ */
} mxml_type_t;

typedef void (*mxml_custom_destroy_cb_t)(void *);
                    /**** Custom data destructor ****/

typedef void (*mxml_error_cb_t)(const char *);
                    /**** Error callback function ****/

typedef struct mxml_attr_s      /**** An XML element attribute value. ****/
{
  char          *name;      /* Attribute name */
  char          *value;     /* Attribute value */
} mxml_attr_t;

typedef struct mxml_element_s       /**** An XML element value. ****/
{
  char          *name;      /* Name of element */
  int           num_attrs;  /* Number of attributes */
  mxml_attr_t       *attrs;     /* Attributes */
} mxml_element_t;

typedef struct mxml_text_s      /**** An XML text value. ****/
{
  int           whitespace; /* Leading whitespace? */
  char          *string;    /* Fragment string */
} mxml_text_t;

typedef struct mxml_custom_s        /**** An XML custom value. @since Mini-XML 2.1@ ****/
{
  void          *data;      /* Pointer to (allocated) custom data */
  mxml_custom_destroy_cb_t destroy; /* Pointer to destructor function */
} mxml_custom_t;

typedef union mxml_value_u      /**** An XML node value. ****/
{
  mxml_element_t    element;    /* Element */
  int           integer;    /* Integer number */
  char          *opaque;    /* Opaque string */
#if 0
  double        real;       /* Real number */
#endif
  mxml_text_t       text;       /* Text fragment */
  mxml_custom_t     custom;     /* Custom data @since Mini-XML 2.1@ */
} mxml_value_t;

typedef struct mxml_node_s      /**** An XML node. ****/
{
  mxml_type_t       type;       /* Node type */
  struct mxml_node_s    *next;      /* Next node under same parent */
  struct mxml_node_s    *prev;      /* Previous node under same parent */
  struct mxml_node_s    *parent;    /* Parent node */
  struct mxml_node_s    *child;     /* First child node */
  struct mxml_node_s    *last_child;    /* Last child node */
  mxml_value_t      value;      /* Node value */
  int           ref_count;  /* Use count */
  void          *user_data; /* User data */
} mxml_node_t;

typedef struct mxml_index_s     /**** An XML node index. ****/
{
  char          *attr;      /* Attribute used for indexing or NULL */
  int           num_nodes;  /* Number of nodes in index */
  int           alloc_nodes;    /* Allocated nodes in index */
  int           cur_node;   /* Current node */
  mxml_node_t       **nodes;    /* Node array */
} mxml_index_t;

typedef int (*mxml_custom_load_cb_t)(mxml_node_t *, const char *);
                    /**** Custom data load callback function ****/

typedef char *(*mxml_custom_save_cb_t)(mxml_node_t *);
                    /**** Custom data save callback function ****/

typedef int (*mxml_entity_cb_t)(const char *);
                    /**** Entity callback function */

typedef mxml_type_t (*mxml_load_cb_t)(mxml_node_t *);
                    /**** Load callback function ****/

typedef const char *(*mxml_save_cb_t)(mxml_node_t *, int);
                    /**** Save callback function ****/

typedef void (*mxml_sax_cb_t)(mxml_node_t *, mxml_sax_event_t, void *);
                    /**** SAX callback function ****/


/*
 * C++ support...
 */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

/*
 * Prototypes...
 */

extern void     prvCpssMxmlAdd(mxml_node_t *parent, int where,
                    mxml_node_t *child, mxml_node_t *node);
extern void     prvCpssMxmlDelete(mxml_node_t *node);
extern void     prvCpssMxmlElementDeleteAttr(mxml_node_t *node,
                                  const char *name);
extern const char   *prvCpssMxmlElementGetAttr(mxml_node_t *node, const char *name);
extern void     prvCpssMxmlElementSetAttr(mxml_node_t *node, const char *name,
                               const char *value);
extern void     prvCpssMxmlElementSetAttrf(mxml_node_t *node, const char *name,
                                const char *format, ...)
#    ifdef __GNUC__
__attribute__ ((__format__ (__printf__, 3, 4)))
#    endif /* __GNUC__ */
;
extern int      prvCpssMxmlEntityAddCallback(mxml_entity_cb_t cb);
extern const char   *prvCpssMxmlEntityGetName(int val);
extern int      prvCpssMxmlEntityGetValue(const char *name);
extern void     prvCpssMxmlEntityRemoveCallback(mxml_entity_cb_t cb);
extern mxml_node_t  *prvCpssMxmlFindElement(mxml_node_t *node, mxml_node_t *top,
                             const char *name, const char *attr,
                     const char *value, int descend);
extern void     prvCpssMxmlIndexDelete(mxml_index_t *ind);
extern mxml_node_t  *prvCpssMxmlIndexEnum(mxml_index_t *ind);
extern mxml_node_t  *prvCpssMxmlIndexFind(mxml_index_t *ind,
                           const char *element,
                           const char *value);
extern mxml_index_t *prvCpssMxmlIndexNew(mxml_node_t *node, const char *element,
                          const char *attr);
extern mxml_node_t  *prvCpssMxmlIndexReset(mxml_index_t *ind);
extern mxml_node_t  *prvCpssMxmlLoadFd(mxml_node_t *top, CPSS_OS_FILE_TYPE_STC *fd, void *fs,
                        mxml_type_t (*cb)(mxml_node_t *));
#if 0
extern mxml_node_t  *prvCpssMxmlLoadFile(mxml_node_t *top, FILE *fp,
                          mxml_type_t (*cb)(mxml_node_t *));
#endif
extern mxml_node_t  *prvCpssMxmlLoadString(mxml_node_t *top, const char *s,
                            mxml_type_t (*cb)(mxml_node_t *));
extern mxml_node_t  *prvCpssMxmlNewCDATA(mxml_node_t *parent, const char *string);
extern mxml_node_t  *prvCpssMxmlNewCustom(mxml_node_t *parent, void *data,
                           mxml_custom_destroy_cb_t destroy);
extern mxml_node_t  *prvCpssMxmlNewElement(mxml_node_t *parent, const char *name);
extern mxml_node_t  *prvCpssMxmlNewInteger(mxml_node_t *parent, int integer);
extern mxml_node_t  *prvCpssMxmlNewOpaque(mxml_node_t *parent, const char *opaque);
#if 0
extern mxml_node_t  *prvCpssMxmlNewReal(mxml_node_t *parent, double real);
#endif
extern mxml_node_t  *prvCpssMxmlNewText(mxml_node_t *parent, int whitespace,
                         const char *string);
extern mxml_node_t  *prvCpssMxmlNewTextf(mxml_node_t *parent, int whitespace,
                          const char *format, ...)
#    ifdef __GNUC__
__attribute__ ((__format__ (__printf__, 3, 4)))
#    endif /* __GNUC__ */
;
extern mxml_node_t  *prvCpssMxmlNewXML(const char *version);
extern int      prvCpssMxmlRelease(mxml_node_t *node);
extern void     prvCpssMxmlRemove(mxml_node_t *node);
extern int      prvCpssMxmlRetain(mxml_node_t *node);
extern char     *prvCpssMxmlSaveAllocString(mxml_node_t *node,
                             mxml_save_cb_t cb);
extern int      prvCpssMxmlSaveFd(mxml_node_t *node, CPSS_OS_FILE_TYPE_STC *fp, void *fs,
                       mxml_save_cb_t cb);
#if 0
extern int      prvCpssMxmlSaveFile(mxml_node_t *node, FILE *fp,
                         mxml_save_cb_t cb);
#endif
extern int      prvCpssMxmlSaveString(mxml_node_t *node, char *buffer,
                           int bufsize, mxml_save_cb_t cb);
extern mxml_node_t  *prvCpssMxmlSAXLoadFd(mxml_node_t *top, CPSS_OS_FILE_TYPE_STC *fd,
                           mxml_type_t (*cb)(mxml_node_t *),
                           mxml_sax_cb_t sax, void *sax_data);
#if 0
extern mxml_node_t  *prvCpssMxmlSAXLoadFile(mxml_node_t *top, FILE *fp,
                             mxml_type_t (*cb)(mxml_node_t *),
                             mxml_sax_cb_t sax, void *sax_data);
#endif
extern mxml_node_t  *prvCpssMxmlSAXLoadString(mxml_node_t *top, const char *s,
                               mxml_type_t (*cb)(mxml_node_t *),
                               mxml_sax_cb_t sax, void *sax_data);
extern int      prvCpssMxmlSetCDATA(mxml_node_t *node, const char *data);
extern int      prvCpssMxmlSetCustom(mxml_node_t *node, void *data,
                          mxml_custom_destroy_cb_t destroy);
extern void     prvCpssMxmlSetCustomHandlers(mxml_custom_load_cb_t load,
                                  mxml_custom_save_cb_t save);
extern int      prvCpssMxmlSetElement(mxml_node_t *node, const char *name);
extern void     prvCpssMxmlSetErrorCallback(mxml_error_cb_t cb);
extern int      prvCpssMxmlSetInteger(mxml_node_t *node, int integer);
extern int      prvCpssMxmlSetOpaque(mxml_node_t *node, const char *opaque);
#if 0
extern int      prvCpssMxmlSetReal(mxml_node_t *node, double real);
#endif
extern int      prvCpssMxmlSetText(mxml_node_t *node, int whitespace,
                        const char *string);
extern int      prvCpssMxmlSetTextf(mxml_node_t *node, int whitespace,
                         const char *format, ...)
#    ifdef __GNUC__
__attribute__ ((__format__ (__printf__, 3, 4)))
#    endif /* __GNUC__ */
;
extern void     prvCpssMxmlSetWrapMargin(int column);
extern mxml_node_t  *prvCpssMxmlWalkNext(mxml_node_t *node, mxml_node_t *top,
                          int descend);
extern mxml_node_t  *prvCpssMxmlWalkPrev(mxml_node_t *node, mxml_node_t *top,
                          int descend);


/*
 * Semi-private functions...
 */

extern void     mxml_error(const char *format, ...);
extern mxml_type_t  mxml_ignore_cb(mxml_node_t *node);
extern mxml_type_t  mxml_integer_cb(mxml_node_t *node);
extern mxml_type_t  mxml_opaque_cb(mxml_node_t *node);
#if 0
extern mxml_type_t  mxml_real_cb(mxml_node_t *node);
#endif


/*
 * C++ support...
 */

#  ifdef __cplusplus
}
#  endif /* __cplusplus */
#endif /* !_mxml_h_ */


/*
 * End of "$Id: mxml.h 385 2009-03-19 05:38:52Z mike $".
 */
