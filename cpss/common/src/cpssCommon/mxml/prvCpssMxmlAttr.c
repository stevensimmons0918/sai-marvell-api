/*
 * "$Id: mxml-attr.c 308 2007-09-15 20:04:56Z mike $"
 *
 * Attribute support code for Mini-XML, a small XML-like file parsing library.
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
 *   prvCpssMxmlElementDeleteAttr() - Delete an attribute.
 *   prvCpssMxmlElementGetAttr()    - Get an attribute.
 *   prvCpssMxmlElementSetAttr()    - Set an attribute.
 *   prvCpssMxmlElementSetAttrf()   - Set an attribute with a formatted value.
 *   mxml_set_attr()         - Set or add an attribute name/value pair.
 */

/*
 * Include necessary headers...
 */

#include <cpssCommon/private/mxml/prvCpssMxml.h>
#include <cpssCommon/private/mxml/prvCpssMxmlConfig.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*
 * Local functions...
 */

static int  mxml_set_attr(mxml_node_t *node, const char *name,
                      char *value);


/*
 * 'prvCpssMxmlElementDeleteAttr()' - Delete an attribute.
 *
 * @since Mini-XML 2.4@
 */

void
prvCpssMxmlElementDeleteAttr(mxml_node_t *node,/* I - Element */
                      const char  *name)/* I - Attribute name */
{
  int       i;          /* Looping var */
  mxml_attr_t   *attr;          /* Cirrent attribute */


#ifdef MXML_DEBUG
  fprintf(stderr, "prvCpssMxmlElementDeleteAttr(node=%p, name=\"%s\")\n",
          node, name ? name : "(null)");
#endif /* MXML_DEBUG */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name)
    return;

 /*
  * Look for the attribute...
  */

  for (i = node->value.element.num_attrs, attr = node->value.element.attrs;
       i > 0;
       i --, attr ++)
  {
#ifdef MXML_DEBUG
    printf("    %s=\"%s\"\n", attr->name, attr->value);
#endif /* MXML_DEBUG */

    if (!_mxml_strcmp(attr->name, name))
    {
     /*
      * Delete this attribute...
      */

      _mxml_free(attr->name);
      _mxml_free(attr->value);

      i --;
      if (i > 0)
        _mxml_memmove(attr, attr + 1, i * sizeof(mxml_attr_t));

      node->value.element.num_attrs --;
      return;
    }
  }
}


/*
 * 'prvCpssMxmlElementGetAttr()' - Get an attribute.
 *
 * This function returns NULL if the node is not an element or the
 * named attribute does not exist.
 */

const char *                /* O - Attribute value or NULL */
prvCpssMxmlElementGetAttr(mxml_node_t *node,   /* I - Element node */
                   const char  *name)   /* I - Name of attribute */
{
  int       i;          /* Looping var */
  mxml_attr_t   *attr;          /* Cirrent attribute */


#ifdef MXML_DEBUG
  fprintf(stderr, "prvCpssMxmlElementGetAttr(node=%p, name=\"%s\")\n",
          node, name ? name : "(null)");
#endif /* MXML_DEBUG */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name)
    return (NULL);

 /*
  * Look for the attribute...
  */

  for (i = node->value.element.num_attrs, attr = node->value.element.attrs;
       i > 0;
       i --, attr ++)
  {
#ifdef MXML_DEBUG
    printf("    %s=\"%s\"\n", attr->name, attr->value);
#endif /* MXML_DEBUG */

    if (!_mxml_strcmp(attr->name, name))
    {
#ifdef MXML_DEBUG
      printf("    Returning \"%s\"!\n", attr->value);
#endif /* MXML_DEBUG */
      return (attr->value);
    }
  }

 /*
  * Didn't find attribute, so return NULL...
  */

#ifdef MXML_DEBUG
  puts("    Returning NULL!\n");
#endif /* MXML_DEBUG */

  return (NULL);
}


/*
 * 'prvCpssMxmlElementSetAttr()' - Set an attribute.
 *
 * If the named attribute already exists, the value of the attribute
 * is replaced by the new string value. The string value is copied
 * into the element node. This function does nothing if the node is
 * not an element.
 */

void
prvCpssMxmlElementSetAttr(mxml_node_t *node,   /* I - Element node */
                   const char  *name,   /* I - Name of attribute */
                   const char  *value)  /* I - Attribute value */
{
  char  *valuec;            /* Copy of value */


#ifdef MXML_DEBUG
  fprintf(stderr, "prvCpssMxmlElementSetAttr(node=%p, name=\"%s\", value=\"%s\")\n",
          node, name ? name : "(null)", value ? value : "(null)");
#endif /* MXML_DEBUG */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name)
    return;

  if (value)
    valuec = _mxml_strdup(value);
  else
    valuec = NULL;

  if (mxml_set_attr(node, name, valuec))
    _mxml_free(valuec);
}


/*
 * 'prvCpssMxmlElementSetAttrf()' - Set an attribute with a formatted value.
 *
 * If the named attribute already exists, the value of the attribute
 * is replaced by the new formatted string. The formatted string value is
 * copied into the element node. This function does nothing if the node
 * is not an element.
 *
 * @since Mini-XML 2.3@
 */

void
prvCpssMxmlElementSetAttrf(mxml_node_t *node,  /* I - Element node */
                    const char  *name,  /* I - Name of attribute */
                    const char  *format,/* I - Printf-style attribute value */
            ...)        /* I - Additional arguments as needed */
{
  va_list   ap;         /* Argument pointer */
  char      *value;         /* Value */


#ifdef MXML_DEBUG
  fprintf(stderr,
          "prvCpssMxmlElementSetAttrf(node=%p, name=\"%s\", format=\"%s\", ...)\n",
          node, name ? name : "(null)", format ? format : "(null)");
#endif /* MXML_DEBUG */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name || !format)
    return;

 /*
  * Format the value...
  */

  va_start(ap, format);
  value = _mxml_vstrdupf(format, ap);
  va_end(ap);

  if (!value)
    mxml_error("Unable to allocate memory for attribute '%s' in element %s!",
               name, node->value.element.name);
  else if (mxml_set_attr(node, name, value))
    _mxml_free(value);
}


/*
 * 'mxml_set_attr()' - Set or add an attribute name/value pair.
 */

static int              /* O - 0 on success, -1 on failure */
mxml_set_attr(mxml_node_t *node,    /* I - Element node */
              const char  *name,    /* I - Attribute name */
              char        *value)   /* I - Attribute value */
{
  int       i;          /* Looping var */
  mxml_attr_t   *attr;          /* New attribute */


 /*
  * Look for the attribute...
  */

  for (i = node->value.element.num_attrs, attr = node->value.element.attrs;
       i > 0;
       i --, attr ++)
    if (!_mxml_strcmp(attr->name, name))
    {
     /*
      * Free the old value as needed...
      */

      if (attr->value)
        _mxml_free(attr->value);

      attr->value = value;

      return (0);
    }

 /*
  * Add a new attribute...
  */

  if (node->value.element.num_attrs == 0)
    attr = _mxml_malloc(sizeof(mxml_attr_t));
  else
    attr = _mxml_realloc(node->value.element.attrs,
                   (node->value.element.num_attrs + 1) * sizeof(mxml_attr_t));

  if (!attr)
  {
    mxml_error("Unable to allocate memory for attribute '%s' in element %s!",
               name, node->value.element.name);
    return (-1);
  }

  node->value.element.attrs = attr;
  attr += node->value.element.num_attrs;

  if ((attr->name = _mxml_strdup(name)) == NULL)
  {
    mxml_error("Unable to allocate memory for attribute '%s' in element %s!",
               name, node->value.element.name);
    return (-1);
  }

  attr->value = value;

  node->value.element.num_attrs ++;

  return (0);
}


/*
 * End of "$Id: mxml-attr.c 308 2007-09-15 20:04:56Z mike $".
 */
