/* libxnm
 * xnm-retrieve.c:
 *
 * Copyright (C) 2007 Dov Grobgeld
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "xnm.h"
#include "xnm_private.h"

#undef DEBUG_REF 
#undef DEBUG_REF_SHOW_VAL 

static const int xnm_line_wrap = 50;;
static gchar *indent_string(const gchar *s, int num_spaces);

/*======================================================================
//   xnm_value.
//----------------------------------------------------------------------
*/
XnmValue * xnm_value_new()
{
  XnmValue *xnm_value = g_new(XnmValue, 1);
  xnm_value->type = XNM_UNKNOWN;
#if DEBUG_REF
  xnm_value->ref_count = 0;
  xnm_value_ref(xnm_value);
#else
  xnm_value->ref_count = 1;
#endif

    
  return xnm_value;
}

XnmValue * xnm_value_new_from_string(XnmString *string)
{
  XnmValue *xnm_value = g_new(XnmValue, 1);
  xnm_value->type = XNM_STRING;
  xnm_string_ref(string);
  xnm_value->value.string = string;

#if DEBUG_REF
  xnm_value->ref_count = 0;
  xnm_value_ref(xnm_value);
#else
  xnm_value->ref_count = 1;
#endif

  return xnm_value;
}

XnmValue * xnm_value_new_string(const gchar *string)
{
  XnmString *xnm_string = xnm_string_new(string);
  XnmValue *xnm_value = xnm_value_new_from_string(xnm_string);
  xnm_string_unref(xnm_string);
  return xnm_value;
}

XnmValue * xnm_value_new_binary(const gchar *buf, size_t size)
{
  XnmBinary *xnm_binary = xnm_binary_new(buf, size);
  XnmValue *xnm_value = xnm_value_new_from_binary(xnm_binary);
  xnm_binary_unref(xnm_binary);

  return xnm_value;
}

XnmValue *xnm_value_new_from_array(XnmArray *array)
{
  XnmValue *xnm_value = g_new(XnmValue, 1);
  xnm_value->type = XNM_ARRAY;
  xnm_array_ref(array);
  xnm_value->value.array = array;
#if DEBUG_REF
  xnm_value->ref_count = 0;
  xnm_value_ref(xnm_value);
#else
  xnm_value->ref_count = 1;
#endif

  return xnm_value;
}

XnmValue *xnm_value_new_array()
{
  XnmArray *array = xnm_array_new();
  XnmValue *xnm_value = xnm_value_new_from_array(array);
  xnm_array_unref(array);
  return xnm_value;
}

XnmValue *xnm_value_new_from_table(XnmTable *table)
{
  XnmValue *xnm_value = g_new(XnmValue, 1);
  xnm_value->type = XNM_TABLE;
  xnm_value->value.table = table;
  xnm_table_ref(table);
#if DEBUG_REF
  xnm_value->ref_count = 0;
  xnm_value_ref(xnm_value);
#else
  xnm_value->ref_count = 1;
#endif

  return xnm_value;
}

XnmValue *xnm_value_new_table()
{
  XnmTable *table = xnm_table_new();
  XnmValue *xnm_value = xnm_value_new_from_table(table);
  xnm_table_unref(table);
  return xnm_value;
}

void xnm_value_ref     (XnmValue *xnm_value)
{
  xnm_value->ref_count++;
#if DEBUG_REF
  {
    gchar *export_string;

#if DEBUG_REF_SHOW_VAL
    export_string = xnm_value_export_to_string(xnm_value);
#else
    export_string = g_strdup("...");
#endif
    printf("xnm_value_ref  : ref=%d val=0x%08x s=%s\n", xnm_value->ref_count, (int)xnm_value, export_string);
    g_free(export_string);
  }
#endif
}

void xnm_value_unref          (XnmValue *xnm_value)
{
  xnm_value->ref_count--;

#if DEBUG_REF
  {
    gchar *export_string;
#if DEBUG_REF_SHOW_VAL
    export_string = xnm_value_export_to_string(xnm_value);
#else
    export_string = g_strdup("...");
#endif
    printf("xnm_value_unref: ref=%d val=0x%08x s=%s\n", xnm_value->ref_count, (int)xnm_value, export_string);
    g_free(export_string);
  }
#endif

  if (xnm_value->ref_count == 0)
    {
      switch(xnm_value->type) {
      case XNM_TABLE:
	xnm_table_unref(xnm_value->value.table);
	break;
      case XNM_ARRAY:
	xnm_array_unref(xnm_value->value.array);
	break;
      case XNM_STRING:
	xnm_string_unref(xnm_value->value.string);
	break;
      case XNM_BINARY:
        xnm_binary_unref(xnm_value->value.binary);
      default:
	break;
      }

      g_free(xnm_value);
    }
}

char *xnm_value_export_to_string (XnmValue *xnm_value)
{
  char *ret_string;
  
  switch(xnm_value->type) {
  case XNM_TABLE:
    ret_string = xnm_table_export_to_string(xnm_value->value.table);
    break;
  case XNM_ARRAY:
    ret_string = xnm_array_export_to_string(xnm_value->value.array);
    break;
  case XNM_STRING:
    ret_string = xnm_string_export_to_string(xnm_value->value.string);
    break;
  default:
    ret_string = g_strdup_printf("Impossible return value = %d!\n", xnm_value->type);
    break;
  }

  return ret_string;
}

#if 0
char *xnm_value_export_to_xml (XnmValue *xnm_value)
{
  char *ret_string;
  
  switch(xnm_value->type) {
  case XNM_TABLE:
    ret_string = xnm_table_export_to_xml(xnm_value->value.table);
    break;
  case XNM_ARRAY:
    ret_string = xnm_array_export_to_xml(xnm_value->value.array);
    break;
  case XNM_STRING:
    ret_string = xnm_string_export_to_xml(xnm_value->value.string);
    break;
  default:
    ret_string = g_strdup_printf("Impossible return value = %d!\n",
				 xnm_value->type);
    break;
  }

  return ret_string;
}
#endif

/*======================================================================
//   xnm_string
//----------------------------------------------------------------------
*/
XnmString * xnm_string_new(const char *string)
{
  XnmString *xnm_string = g_new(XnmString, 1);
  xnm_string->string = strdup(string);
#ifdef DEBUG_REF
  xnm_string->ref_count = 0;
  xnm_string_ref(xnm_string);
#else
  xnm_string->ref_count = 1;
#endif

  return xnm_string;
}

void xnm_string_ref(XnmString *xnm_string)
{
  xnm_string->ref_count++;
#if DEBUG_REF
  printf("xnm_string_ref  : ref=%d val=0x%08x s=%s\n", xnm_string->ref_count, (int)xnm_string, xnm_string->string);
#endif
}

void xnm_string_unref(XnmString *xnm_string)
{
  xnm_string->ref_count--;
#if DEBUG_REF
  printf("xnm_string_unref: ref=%d val=0x%08x s=%s\n", xnm_string->ref_count, (int)xnm_string, xnm_string->string);
#endif

  if (xnm_string->ref_count == 0)
    {
      g_free(xnm_string->string);
      g_free(xnm_string);
    }
}

gchar *xnm_string_export_to_string(XnmString *xnm_string)
{
  gchar *ret;
  gboolean need_quotes = FALSE;
  GString *exported_string = g_string_new("\"");
  int len = strlen(xnm_string->string);
  int i;
  
  for (i=0; i<len; i++)
    {
      gchar c = xnm_string->string[i];
      switch(c) {
      case ' ':
      case '[':
      case '{':
      case '}':
      case ']':
        need_quotes = TRUE;
        g_string_append_c(exported_string, c);
        break;
      case '"':
        g_string_append_c(exported_string, '\\');
        g_string_append_c(exported_string, c);
        need_quotes = TRUE;
        break;
      default:
        g_string_append_c(exported_string, c);
        break;
      }
    }
  if (need_quotes)
    {
      g_string_append_c(exported_string, '"');
      ret = exported_string->str;
      g_string_free(exported_string, FALSE);
    }
  else
    {
      ret = g_strdup(&exported_string->str[1]);
      g_string_free(exported_string, TRUE);
    }

  return ret;
}

#if 0
char *xnm_string_export_to_xml(XnmString *xnm_string)
{
  return g_strdup_printf("<str val=\"%s\"/>\n", xnm_string->string);
}
#endif

/*======================================================================
//   xnm_binary
//----------------------------------------------------------------------
*/
XnmValue * xnm_value_new_from_binary(XnmBinary *binary)
{
  XnmValue *xnm_value = g_new(XnmValue, 1);
  xnm_value->type = XNM_BINARY;
  xnm_binary_ref(binary);
  xnm_value->value.binary = binary;

#if DEBUG_REF
  xnm_value->ref_count = 0;
  xnm_value_ref(xnm_value);
#else
  xnm_value->ref_count = 1;
#endif

  return xnm_value;
}

XnmBinary * xnm_binary_new(const gchar *data, size_t len)
{
  XnmBinary *xnm_binary = g_new(XnmBinary, 1);
  xnm_binary->buf = g_new(gchar, len);
  xnm_binary->len = len;
  memcpy(xnm_binary->buf, data, len);

#ifdef DEBUG_REF
  xnm_binary->ref_count = 0;
  xnm_binary_ref(xnm_string);
#else
  xnm_binary->ref_count = 1;
#endif

  return xnm_binary;
}

void xnm_binary_ref(XnmBinary *xnm_binary)
{
  xnm_binary->ref_count++;
#if DEBUG_REF
  printf("xnm_string_ref  : ref=%d val=0x%08x s=%s\n", xnm_string->ref_count, (int)xnm_string, xnm_string->string);
#endif
}

void xnm_binary_unref(XnmBinary *xnm_binary)
{
  xnm_binary->ref_count--;
#if DEBUG_REF
  printf("xnm_string_unref: ref=%d val=0x%08x\n", xnm_string->ref_count, (int)xnm_string);
#endif

  if (xnm_binary->ref_count == 0)
    {
      g_free(xnm_binary->buf);
      g_free(xnm_binary);
    }
}


/*======================================================================
//   xnm_array
//----------------------------------------------------------------------
*/
XnmArray *xnm_array_new()
{
  XnmArray *xnm_array = g_new(XnmArray, 1);
  xnm_array->array = g_ptr_array_new();
#if DEBUG_REF
  xnm_array->ref_count = 0;
  xnm_array_ref(xnm_array);
#else
  xnm_array->ref_count = 1;
#endif

  return xnm_array;
}

void xnm_array_ref(XnmArray *xnm_array)
{
  xnm_array->ref_count++;
#if DEBUG_REF
  {
    gchar *export_string;

#if DEBUG_REF_SHOW_VAL
    export_string = xnm_array_export_to_string(xnm_array);
#else
    export_string = g_strdup("...");
#endif
    printf("xnm_array_ref  : ref=%d val=0x%08x s=%s\n", xnm_array->ref_count, (int)xnm_array, export_string);
    g_free(export_string);
  }
#endif  

}

void xnm_array_unref(XnmArray *xnm_array)
{
  xnm_array->ref_count--;
#if DEBUG_REF
  {
    gchar *export_string;
#if DEBUG_REF_SHOW_VAL
    export_string = xnm_array_export_to_string(xnm_array);
#else
    export_string = g_strdup("...");
#endif
    printf("xnm_array_unref: ref=%d val=0x%08x s=%s\n", xnm_array->ref_count, (int)xnm_array, export_string);
    g_free(export_string);
  }
#endif  

  if (xnm_array->ref_count == 0)
    {
      int i;
      for (i=0; i<xnm_array->array->len; i++)
	{
	  xnm_value_unref((XnmValue*)g_ptr_array_index(xnm_array->array, i));
	}
      g_ptr_array_free(xnm_array->array, TRUE);
      g_free(xnm_array);
    }
}

void xnm_array_push(XnmArray *array,
		    XnmValue *value)
{
  g_ptr_array_add(array->array, value);
  xnm_value_ref(value);
}

int xnm_value_array_push_value(XnmValue *xnm_value_array,
                               XnmValue* value)
{
  if (xnm_value_array->type != XNM_ARRAY)
    return -1;
  xnm_array_push(xnm_value_array->value.array, value);
  return 0;
}

int xnm_value_array_push_string(XnmValue *xnm_value_array,
                                const gchar *string)
{
  XnmValue *xnm_value;
  if (xnm_value_array->type != XNM_ARRAY)
    return -1;
  
  xnm_value = xnm_value_new_string(string);
  xnm_array_push(xnm_value_array->value.array,
                 xnm_value);
  xnm_value_unref(xnm_value);

  return 0;
}

int xnm_array_get_key(XnmArray *array,
		      const char *key,
		      /* output */
		      XnmValue **value)
{
  int ret = 0;
  char *key_head = 0, *key_tail = 0;
  gboolean is_leaf, is_array;
  int index;

  if (!array)
    {
      ret = -1;
      goto CATCH;
    }
  xnm_key_split(key,
		/* output */
		&is_leaf,
		&is_array,
		&key_head,
		&key_tail);

  if (!is_array)
    {
      ret = -1;
      goto CATCH;
    }

  index = atoi(key_head);
  if (index >= array->array->len)
    {
      ret = -1;
      goto CATCH;
    }

  *value =  g_ptr_array_index(array->array, index);
  xnm_value_ref(*value);

 CATCH:
  if (key_head)
    g_free(key_head);
  if (key_tail)
    g_free(key_tail);
  return ret;
}

int xnm_array_get_index(XnmArray *array,
			int index,
			/* output */
			XnmValue **value)
{
  if (index >= array->array->len)
    return -1;

  *value =  g_ptr_array_index(array->array, index);
  xnm_value_ref(*value);
  
  return 0;
}
			
char *xnm_array_export_to_string(XnmArray *xnm_array)
{
  GString *exported_string = g_string_new("");
  int i;
  char *ret_string;
  gboolean is_pure_string = TRUE;
  int line_len = 0;

#if 0
  // Look if it is a pure string array so that we can compact it
  for (i=0; i<xnm_array->array->len; i++)
    {
      XnmValue *value = (XnmValue*)g_ptr_array_index(xnm_array->array, i);
      if (value->type == XNM_TABLE || value->type == XNM_ARRAY)
        {
          is_pure_string = FALSE;
          break;
        }
    }
#endif

  is_pure_string = FALSE; // Force at the moment

  for (i=0; i<xnm_array->array->len; i++)
    {
      XnmValue *value = (XnmValue*)g_ptr_array_index(xnm_array->array, i);
      char *s = xnm_value_export_to_string(value);

      if (value->type == XNM_TABLE || value->type == XNM_ARRAY)
        {
          char *indented_s = indent_string(s, 2);
          if (value->type == XNM_TABLE) 
            g_string_sprintfa(exported_string, "{\n%s}\n", indented_s);
          else if (value->type == XNM_ARRAY) 
            g_string_sprintfa(exported_string, "[\n%s]\n", indented_s);
          g_free(indented_s);
        }
      else
        {
          if (is_pure_string)
            {
              int len = strlen(s);
              if (line_len > 0 && line_len + len > xnm_line_wrap)
                {
                  g_string_append(exported_string, "\n");
                  line_len = 0;
                }
              line_len += len + 1;
              g_string_append(exported_string, s);
              if (i < xnm_array->array->len-1)
                g_string_append(exported_string, " ");
            }
          else
            g_string_sprintfa(exported_string, "%s\n", s);
        }

      g_free(s);
    }
  
  ret_string = exported_string->str;
  g_string_free(exported_string, FALSE);

  return ret_string;
}

#if 0
char *xnm_array_export_to_xml(XnmArray *xnm_array)
{
  GString *exported_string = g_string_new("<array>\n");
  int i;
  char *ret_string;

  for (i=0; i<xnm_array->array->len; i++)
    {
      XnmValue *val = (XnmValue*)g_ptr_array_index(xnm_array->array, i);
      char *s = xnm_value_export_to_xml(val);

      g_string_append(exported_string, s);
      g_free(s);
      
      if (i<xnm_array->array->len-1)
	g_string_append(exported_string, " ");
    }
  g_string_append(exported_string, "</array>\n");
  
  ret_string = exported_string->str;
  g_string_free(exported_string, FALSE);

  return ret_string;
}
#endif

/*======================================================================
//   xnm_table
//----------------------------------------------------------------------
*/
XnmTable *xnm_table_new()
{
  XnmTable *xnm_table = g_new(XnmTable, 1);
  xnm_table->table = g_hash_table_new(g_str_hash, g_str_equal);
  xnm_table->key_order = g_ptr_array_new();
#if DEBUG_REF
  xnm_table->ref_count = 0;
  xnm_table_ref(xnm_table);
#else
  xnm_table->ref_count = 1;
#endif

  return xnm_table;
}


void xnm_table_ref(XnmTable *xnm_table)
{
  xnm_table->ref_count++;
#if DEBUG_REF
  {
    gchar *export_string;
#if DEBUG_REF_SHOW_VAL
    export_string = xnm_table_export_to_string(xnm_table);
#else
    export_string = g_strdup("...");
#endif
    printf("xnm_table_ref  : ref=%d val=0x%08x s=%s\n", xnm_table->ref_count, (int)xnm_table, export_string);
    g_free(export_string);
  }
#endif  
}

void xnm_table_unref(XnmTable *xnm_table)
{
  xnm_table->ref_count--;
#if DEBUG_REF
  {
    gchar *export_string;
    
#if DEBUG_REF_SHOW_VAL
    export_string = xnm_table_export_to_string(xnm_table);
#else
    export_string = g_strdup("...");
#endif
    printf("xnm_table_unref: ref=%d val=0x%08x s=%s\n", xnm_table->ref_count, (int)xnm_table, export_string);
    g_free(export_string);
  }
#endif  

  if (xnm_table->ref_count == 0)
    {
      int i;
      for (i=0; i<xnm_table->key_order->len; i++)
	{
	  char *org_key;
	  XnmValue *org_value;
	  g_hash_table_lookup_extended(xnm_table->table,
				       g_ptr_array_index(xnm_table->key_order, i),
				       /* output */
				       (gpointer)&org_key,
				       (gpointer)&org_value);
	  xnm_value_unref(org_value);
	  g_hash_table_remove(xnm_table->table, org_key);
          g_free(g_ptr_array_index(xnm_table->key_order, i));
	}
      g_hash_table_unref(xnm_table->table);
      g_ptr_array_free(xnm_table->key_order, TRUE);
      g_free(xnm_table);
    }
}

int xnm_table_get_key(XnmTable *xnm_table,
                      const char* key,
                      // output
                      XnmValue **ret_xnm_value
                      )
{
  int ret = 0;
  *ret_xnm_value = g_hash_table_lookup(xnm_table->table,
                                       key);

  if (*ret_xnm_value)
    xnm_value_ref(*ret_xnm_value);
  else
    ret = -1;
    
  return ret;
}

gboolean xnm_table_has_key(XnmTable *xnm_table,
                           const char* key)
{
  return g_hash_table_lookup(xnm_table->table, key) != NULL;
}

void xnm_table_set_key_value(XnmTable *xnm_table,
                             const char* key,
                             XnmValue *value)
{
  XnmValue *value_table;
  gchar *key_table;
  

  /* Check if the key exists */
  if (!g_hash_table_lookup_extended(xnm_table->table,
                                    key,
                                    /* output */
                                    (gpointer)&key_table,
                                    (gpointer)&value_table)) 
    {
      key_table = g_strdup(key);
      g_ptr_array_add(xnm_table->key_order,
                      key_table);
      
    }
  else
      xnm_value_unref(value_table);
  
  g_hash_table_insert(xnm_table->table,
                      key_table,
                      value);
  xnm_value_ref(value);
}

char *xnm_table_export_to_string(XnmTable *xnm_table)
{
  GString *exported_string = g_string_new("");
  int i;
  char *ret_string;

  for (i=0; i<xnm_table->key_order->len; i++)
    {
      char *key = (char*)g_ptr_array_index(xnm_table->key_order, i);
      XnmValue *value;

      xnm_table_get_key(xnm_table,
                        key,
                        // output
                        &value);
      char *value_string;

      g_assert(value != NULL);

      value_string = xnm_value_export_to_string(value);
      g_string_sprintfa(exported_string, "%s : ", key);

      if (value->type == XNM_TABLE || value->type == XNM_ARRAY)
        {
          gchar *indented_value_string = indent_string(value_string, 2);
          if (value->type == XNM_TABLE) 
            g_string_sprintfa(exported_string, "{\n%s}\n", indented_value_string);
          else if (value->type == XNM_ARRAY) 
            g_string_sprintfa(exported_string, "[\n%s]\n", indented_value_string);
          g_free(indented_value_string);
        }
      else
        g_string_sprintfa(exported_string, "%s\n", value_string);

      g_free(value_string);
      xnm_value_unref(value);
    }

  ret_string = exported_string->str;
  g_string_free(exported_string, FALSE);
    
  return ret_string;
}

#if 0
char *xnm_table_export_to_xml(XnmTable *xnm_table)
{
  GString *exported_string = g_string_new("<table>\n");
  int i;
  char *ret_string;

  for (i=0; i<xnm_table->key_order->len; i++)
    {
      char *key = (char*)g_ptr_array_index(xnm_table->key_order, i);
      XnmValue *value;
      char *value_string;

      xnm_table_get_key(xnm_table,
                        key,
                        // output
                        &value);

      g_assert(value != NULL);

      value_string = xnm_value_export_to_xml(value);
      
      g_string_sprintfa(exported_string,
			"<keyval key=\"%s\">\n"
			"  %s"
			"</keyval>\n", key, value_string);
      g_free(value_string);
    }
  g_string_append(exported_string, "</table>");

  ret_string = exported_string->str;
  g_string_free(exported_string, FALSE);
    
  return ret_string;
}
#endif

/*======================================================================
//  key_split takes a key string as input and returns the head and
//  the tail of the string.
//----------------------------------------------------------------------
*/
int xnm_key_split(const char *key_string,
		  /* output */
		  gboolean *is_leaf,
		  gboolean *is_array,
		  char **key_head,
		  char **key_tail)
{
    char *colon_pos, *slash_pos, *lbrack_pos;

    /* Allow for both ':' and '/' as separators */
    colon_pos = strchr(key_string, ':');
    slash_pos = strchr(key_string, '/');

    /* Assume only one of them is in use at a time... */
    if (!colon_pos)
        colon_pos = slash_pos;
    
    if (colon_pos == NULL)
      {
	*is_leaf=TRUE;
	*key_head = strdup(key_string);
	*key_tail = NULL;
      }
    else
      {
	int head_len = colon_pos - key_string + 1; /* Including terminating zero */
	*key_head = g_strndup(key_string, head_len-1);
	*key_tail = g_strdup(colon_pos+1);
	*is_leaf=FALSE;
      }

    /* Check if this is array access syntax */
    lbrack_pos = strchr(*key_head, '[');
    if (lbrack_pos != NULL)
      {
	char *rbrack_pos = strchr(*key_head, ']');
	char *ps = lbrack_pos+1;
	char *pd = *key_head;

	while(ps != rbrack_pos)
          *pd++ = *ps++;
	*pd = 0;
	
	*is_array = TRUE;
      }
    else
	*is_array = FALSE;

    return 0;
}
	       
int xnm_value_table_set_key_value(XnmValue * xnm_value_table,
                                  const gchar * key,
                                  XnmValue * xnm_value)
{
  if (xnm_value_table->type != XNM_TABLE)
    return -1;
  xnm_table_set_key_value(xnm_value_table->value.table,
                          key,
                          xnm_value);
  return 0;
}

int
xnm_value_table_set_key_value_string(XnmValue * xnm_value_table,
                                     const gchar * key,
                                     const gchar * string)
{
  XnmValue *value_string;

  if (xnm_value_table->type != XNM_TABLE)
    return -1;

  value_string = xnm_value_new_string(string);

  xnm_table_set_key_value(xnm_value_table->value.table,
                          key,
                          value_string);

  xnm_value_unref(value_string);

  return 0;
}

static gchar *indent_string(const gchar *s, int num_spaces)
{
  gchar *ret;
  GString *indent_space = g_string_new("");
  GString *indented_string = g_string_new("");
  int len = strlen(s);
  int i;

  for (i=0; i<num_spaces; i++)
    g_string_append_c(indent_space, ' ');
    
  for (i=0; i<len; i++)
    {
      if (i==0 || s[i-1] == '\n')
        g_string_append(indented_string, indent_space->str);
      g_string_append_c(indented_string, s[i]);
    }

  ret = indented_string->str;
  g_string_free(indented_string, FALSE);
  g_string_free(indent_space, TRUE);

  return ret;
}

