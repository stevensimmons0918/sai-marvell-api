/*
 * "$Id: mxml-set.c 270 2007-04-23 21:48:03Z mike $"
 *
 * Node set functions for Mini-XML, a small XML-like file parsing library.
 *
 * Copyright 2003-2007 by Michael Sweet.
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
 *
 * Contents:
 *
 *   prvCpssMxmlSetCustom()  - Set the data and destructor of a custom data node.
 *   prvCpssMxmlSetCDATA()   - Set the element name of a CDATA node.
 *   prvCpssMxmlSetElement() - Set the name of an element node.
 *   prvCpssMxmlSetInteger() - Set the value of an integer node.
 *   prvCpssMxmlSetOpaque()  - Set the value of an opaque node.
 *   --prvCpssMxmlSetReal()    - Set the value of a real number node.
 *   prvCpssMxmlSetText()    - Set the value of a text node.
 *   prvCpssMxmlSetTextf()   - Set the value of a text node to a formatted string.
 */

/*
 * Include necessary headers...
 */
#include <cpssCommon/private/mxml/prvCpssMxml.h>
#include <cpssCommon/private/mxml/prvCpssMxmlConfig.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*
 * 'prvCpssMxmlSetCustom()' - Set the data and destructor of a custom data node.
 *
 * The node is not changed if it is not a custom node.
 *
 * @since Mini-XML 2.1@
 */

int                 /* O - 0 on success, -1 on failure */
prvCpssMxmlSetCustom(
    mxml_node_t              *node, /* I - Node to set */
    void                     *data, /* I - New data pointer */
    mxml_custom_destroy_cb_t destroy)   /* I - New destructor function */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_CUSTOM)
    return (-1);

 /*
  * Free any old element value and set the new value...
  */

  if (node->value.custom.data && node->value.custom.destroy)
    (*(node->value.custom.destroy))(node->value.custom.data);

  node->value.custom.data    = data;
  node->value.custom.destroy = destroy;

  return (0);
}


/*
 * 'prvCpssMxmlSetCDATA()' - Set the element name of a CDATA node.
 *
 * The node is not changed if it is not a CDATA element node.
 *
 * @since Mini-XML 2.3@
 */

int                 /* O - 0 on success, -1 on failure */
prvCpssMxmlSetCDATA(mxml_node_t *node,      /* I - Node to set */
             const char  *data)     /* I - New data string */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !data ||
      _mxml_strncmp(node->value.element.name, "![CDATA[", 8))
    return (-1);

 /*
  * Free any old element value and set the new value...
  */

  if (node->value.element.name)
    _mxml_free(node->value.element.name);

  node->value.element.name = _mxml_strdupf("![CDATA[%s]]", data);

  return (0);
}


/*
 * 'prvCpssMxmlSetElement()' - Set the name of an element node.
 *
 * The node is not changed if it is not an element node.
 */

int                 /* O - 0 on success, -1 on failure */
prvCpssMxmlSetElement(mxml_node_t *node,    /* I - Node to set */
               const char  *name)   /* I - New name string */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name)
    return (-1);

 /*
  * Free any old element value and set the new value...
  */

  if (node->value.element.name)
    _mxml_free(node->value.element.name);

  node->value.element.name = _mxml_strdup(name);

  return (0);
}


/*
 * 'prvCpssMxmlSetInteger()' - Set the value of an integer node.
 *
 * The node is not changed if it is not an integer node.
 */

int                 /* O - 0 on success, -1 on failure */
prvCpssMxmlSetInteger(mxml_node_t *node,    /* I - Node to set */
               int         integer) /* I - Integer value */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_INTEGER)
    return (-1);

 /*
  * Set the new value and return...
  */

  node->value.integer = integer;

  return (0);
}


/*
 * 'prvCpssMxmlSetOpaque()' - Set the value of an opaque node.
 *
 * The node is not changed if it is not an opaque node.
 */

int                 /* O - 0 on success, -1 on failure */
prvCpssMxmlSetOpaque(mxml_node_t *node, /* I - Node to set */
              const char  *opaque)  /* I - Opaque string */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_OPAQUE || !opaque)
    return (-1);

 /*
  * Free any old opaque value and set the new value...
  */

  if (node->value.opaque)
    _mxml_free(node->value.opaque);

  node->value.opaque = _mxml_strdup(opaque);

  return (0);
}


#if 0
/*
 * 'prvCpssMxmlSetReal()' - Set the value of a real number node.
 *
 * The node is not changed if it is not a real number node.
 */

int                 /* O - 0 on success, -1 on failure */
prvCpssMxmlSetReal(mxml_node_t *node,       /* I - Node to set */
            double      real)       /* I - Real number value */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_REAL)
    return (-1);

 /*
  * Set the new value and return...
  */

  node->value.real = real;

  return (0);
}
#endif


/*
 * 'prvCpssMxmlSetText()' - Set the value of a text node.
 *
 * The node is not changed if it is not a text node.
 */

int                 /* O - 0 on success, -1 on failure */
prvCpssMxmlSetText(mxml_node_t *node,       /* I - Node to set */
            int         whitespace, /* I - 1 = leading whitespace, 0 = no whitespace */
        const char  *string)    /* I - String */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_TEXT || !string)
    return (-1);

 /*
  * Free any old string value and set the new value...
  */

  if (node->value.text.string)
    _mxml_free(node->value.text.string);

  node->value.text.whitespace = whitespace;
  node->value.text.string     = _mxml_strdup(string);

  return (0);
}


/*
 * 'prvCpssMxmlSetTextf()' - Set the value of a text node to a formatted string.
 *
 * The node is not changed if it is not a text node.
 */

int                 /* O - 0 on success, -1 on failure */
prvCpssMxmlSetTextf(mxml_node_t *node,      /* I - Node to set */
             int         whitespace,    /* I - 1 = leading whitespace, 0 = no whitespace */
             const char  *format,   /* I - Printf-style format string */
         ...)           /* I - Additional arguments as needed */
{
  va_list   ap;         /* Pointer to arguments */


 /*
  * Range check input...
  */

  if (!node || node->type != MXML_TEXT || !format)
    return (-1);

 /*
  * Free any old string value and set the new value...
  */

  if (node->value.text.string)
    _mxml_free(node->value.text.string);

  va_start(ap, format);

  node->value.text.whitespace = whitespace;
  node->value.text.string     = _mxml_strdupf(format, ap);

  va_end(ap);

  return (0);
}


/*
 * End of "$Id: mxml-set.c 270 2007-04-23 21:48:03Z mike $".
 */
