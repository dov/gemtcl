/* libxnm
 * xnm_private.h:
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
 *
 * Dov Grobgeld <dov.grobgeld@gmail.com>
 * Thu Jan 11 23:44:32 2007
 */

#ifndef XNM_PRIVATE_H
#define XNM_PRIVATE_H

#define XNM_UNKNOWN -1

/* The strings are simply gchar* */
typedef struct xnm_string_struct {
    int ref_count;
    gchar *string;
} XnmString;

XnmString *xnm_string_new              (const gchar *string);
void          xnm_string_ref              (XnmString *xnm_string);
void          xnm_string_unref            (XnmString *xnm_string);
char *        xnm_string_export_to_string (XnmString *xnm_string);

/* Binaries are similar, but contain length as well* */
typedef struct xnm_binary_struct {
    int ref_count;
    gchar *buf;
    int len;
} XnmBinary;

XnmBinary *xnm_binary_new                 (const gchar *data, size_t len);
void          xnm_binary_ref              (XnmBinary *xnm_string);
void          xnm_binary_unref            (XnmBinary *xnm_string);

/* No export to string for binary as it is meaningless. */

/* Arrays are GPtrArray */
typedef struct xnm_array_struct {
    int ref_count;
    GPtrArray *array;
} XnmArray;

XnmArray *xnm_array_new             ();
void         xnm_array_ref             (XnmArray *xnm_array);
void         xnm_array_unref           (XnmArray *xnm_array);
char        *xnm_array_export_to_string(XnmArray *xnm_array);
char        *xnm_array_export_to_xml   (XnmArray *xnm_array);
void         xnm_array_push            (XnmArray *xnm_array,
					XnmValue *xnm_value);
int          xnm_array_get_idx         (XnmArray *xnm_array,
					int idx,
					/* output */
					XnmValue *xnm_value);
int          xnm_array_get_key         (XnmArray *xnm_array,
					const char *key,
					/* output */
					XnmValue **xnm_value);
int          xnm_array_get_index       (XnmArray *array,
					int index,
					/* output */
					XnmValue **value);

/* Tables are GHashes with string keys. It also has an extra
   order array to keep track of the order of the keys. */
typedef struct xnm_table_struct {
    int ref_count;
    GHashTable *table;
    GPtrArray  *key_order;
} XnmTable;

XnmTable*  xnm_table_new             ();
void          xnm_table_ref             (XnmTable *xnm_table);
void          xnm_table_unref           (XnmTable *xnm_table);
int           xnm_table_get_key         (XnmTable *xnm_table,
					 const char* key,
                                         // output
                                         XnmValue **ret_xnm_value
                                         );
gboolean      xnm_table_has_key         (XnmTable *xnm_table,
                                         const char* key);
void          xnm_table_set_key_value   (XnmTable *xnm_table,
					 const char* key,
					 XnmValue *xnm_value);
char         *xnm_table_export_to_string(XnmTable *xnm_table);
char         *xnm_table_export_to_xml   (XnmTable *xnm_table);    

/* Declarations placed here when all types are defined */
XnmValue * xnm_value_new_from_string       (XnmString *string);
XnmValue * xnm_value_new_from_array        (XnmArray *array);
XnmValue * xnm_value_new_from_table        (XnmTable *table);
XnmValue * xnm_value_new_from_binary       (XnmBinary *binary);

/* Utility routine for extracting keys */
int xnm_key_split(const char *key_string,
		  /* output */
		  gboolean *is_leaf,
		  gboolean *is_array,
		  char **key_head,
		  char **key_tail);


#endif /* XNM_PRIVATE */
