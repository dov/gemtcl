/* libxnm
 * xnm.h:
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

/**
 * @file   xnm.h
 * @author Dov Grobgeld <dov.grobgeld@gmail.com>
 * @date   Fri Jan 12 09:23:04 2007
 * 
 * @brief  The libxnm header file.
 * 
 * The libxnm library stores the xnm data in a tree of XnmValue's
 * where each XnmValue dynamically may contain a string, a binary
 * buffer, an array or a table.
 *
 * Values are accessed by their keys which is a slash separated
 * path description from the root to a value. Tables are dereferenced
 * by the key name, and arrays by the array index enclosed in square
 * brackets.
 *
 * Given a xnm structure that looks like:
 *
 * \code 
 *
 *   gaim => {
 *     gtk=> {
 *       browsers=> {
 *         place=>F
 * 	   command=>"xterm -e lynx %s"
 * 	   browser=>firefox
 * 	   new_window=>F
 *       }
 *       plugins => [
 *         '/usr/lib/gaim/gaimrc.so'
 * 	   '/usr/lib/gaim/ssl-nss.so'
 * 	   '/usr/lib/gaim/ssl.so'
 *       ]
 *     }
 *     pos => [ {x=>0 y=>0} {x=>100 y=0} {x=>100 y=>50 } ]
 *   }
 * \endcode
 *
 * Here are a couple of valid keys
 *
 *   - gaim/gtk/browser/command
 *   - gaim/pos/[0]/x
 *
 * Most commands are made to ignore undefined keys.
 *
 * libxnm provides several methods for extracting values from an xnm tree
 * that all start with xnm_get .
 *
 */

#ifndef XNM_H
#define XNM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

/**
 * Error definitions
 * 
 */
enum {
  XNM_ERROR = 0,
  XNM_ERROR_SYNTAX_ERROR = 1,
  XNM_ERROR_KEY_NOT_FOUND = 2
};

/**
 * The possible types of a XnmValue.
 * 
 */
typedef enum  {
  XNM_UNKNOWN,
  XNM_TABLE,
  XNM_ARRAY,
  XNM_STRING,
  XNM_BINARY 
} XnmValueType;

/**
 * Values used when retrieving. Note that binary values may not be
 * extracted this way, as they need to have the length of them returned
 * as well.
 * 
 */
typedef enum {
  XNM_GET_UINT8,
  XNM_GET_UINT16,
  XNM_GET_UINT,
  XNM_GET_UINT32,
  XNM_GET_INT8,
  XNM_GET_INT16,
  XNM_GET_INT,
  XNM_GET_INT32,
  XNM_GET_INT64,
  XNM_GET_FLOAT,
  XNM_GET_DOUBLE,
  XNM_GET_STRING,
  XNM_GET_BOOL
} XnmValueGetType;

/* Forward declarations */
struct xnm_string_struct;
struct xnm_table_struct;
struct xnm_array_struct;
struct xnm_bin_struct;

/**
 * The xnmValue should be considered an opaque reference.
 * 
 */
typedef struct {
    int ref_count;              /**< Reference count of the XnmValue */
    XnmValueType type;

    union {
	struct xnm_table_struct *table;
	struct xnm_array_struct *array;
	struct xnm_string_struct *string;
        struct xnm_binary_struct *binary;
    } value;
} XnmValue;

/** 
 * Allocate a new XnmValue with the type string. The string is initialized
 * to a copy of s.
 * 
 * @param s 
 * 
 * @return Newly allocated XnmValue
 */
XnmValue * xnm_value_new_string            (const gchar *s);

/** 
 * Allocate a new XnmValue with the type XNM_TABLE.
 * 
 * 
 * @return 
 */
XnmValue * xnm_value_new_table             ();

/** 
 * Allocate a new XnmValue with the type XNM_ARRAY.
 * 
 * @return 
 */
XnmValue * xnm_value_new_array             ();


/**
 * Allocate a new XnmValue with the type binary and initiate it with
 * the data in buf.
 * 
 */
XnmValue * xnm_value_new_binary            (const gchar *buf, size_t size);


/** 
 * Increase the reference count of xnm_value.
 * 
 * @param xnm_value 
 */
void          xnm_value_ref              (XnmValue *xnm_value);

/** 
 * Decrease the reference count on an xnm_value. If the reference
 * count goes down to 0, the value is destroyed.
 * 
 * @param xnm_value 
 */
void          xnm_value_unref            (XnmValue *xnm_value);

/** 
 * Allocate a new string containing a serialized representation
 * of xnm_value.
 * 
 * @param xnm_value 
 * 
 * @return 
 */
gchar *       xnm_value_export_to_string (XnmValue *xnm_value);

/** 
 * Retrieve an xnm value from an xnm_value_tree. If the key is not
 * found then the value is not assigned. If the key is found then
 * xnm_value is assigned the corresponding string.
 *
 * Note that xnm_value_get does *not* dereference xnm_value before
 * overwriting it with a new value.
 *
 * The returned xnm_value points into the xnm_value_tree with an
 * increased reference count.
 * 
 * @param xnm_value_tree
 * @param key 
 * @param xnm_value 
 * 
 * @return 0 if value retrieved, -1 if value not found, -2 if key syntax error.
 */
int           xnm_value_get              (XnmValue *xnm_value_tree,
					  const char *key,
					  // output
					  XnmValue **xnm_value);

/** 
 * This value retrieves a string from an xnm_value_tree. If the key
 * is found then string_value is freed, and the string is copied
 * into string_val. Otherwise, string_val is left untouched.
 * 
 * @param xnm_value
 * @param key 
 * @param string_val 
 * 
 * @return 0 if value retrieved, -1 if value not found, -2 if key syntax error.
 */
int           xnm_value_get_string       (XnmValue *xnm_value,
					  const char *key,
					  // output
                                          gchar **string_val);

/** 
 * This value retrieves a const pointer to the string within the
 * xnm_value_tree.
 * 
 * @param xnm_value_tree 
 * @param key 
 * @retval const_val_string 
 * 
 * @return 0 if value retrieved, -1 if value not found, -2 if key syntax error.
 */
int           xnm_value_get_const_string  (XnmValue *xnm_value_tree,
                                           const char *key,

                                           /* output */
                                           const gchar **const_val_string);

/** 
 * This value retrieves a value from an xnm_value and turns it
 * into an integer with atoi().
 * 
 * @param xnm_value
 * @param key 
 * @retval val_int
 * 
 * @return 0 if value retrieved, -1 if value not found, -2 if key syntax error.
 */
int           xnm_value_get_int           (XnmValue *xnm_value,
                                           const char *key,
                                           /* output */
                                           int *val_int);


/**
 * Retrieve a boolean value from an xnm_value. The boolean value is
 * considered true if it starts with 't','T','y','Y', or 1. Everything
 * else is false.
 */
int xnm_value_get_bool (XnmValue *xnm_value,
                        const char *key,
                        /* output */
                        gboolean *val_bool);

/** 
 * Add an xnm_value into an XnmValue of type array.
 * 
 * @return 
 */
int xnm_value_array_push_value(XnmValue * xnm_value_array,
                               XnmValue * value);

/** 
 * Push a string into an xnm value of type array.
 * 
 * @param xnm_value_array 
 * @param string 
 * 
 * @return 
 */
int xnm_value_array_push_string(XnmValue * xnm_value_array,
                                const gchar *string);


/** 
 * Push an integer value into an xnm value of type array.
 * 
 * @param xnm_value_array 
 * @param value_int 
 * 
 * @return 
 */
int xnm_value_array_push_int(XnmValue * xnm_value_array,
                             int value_int);


/** 
 * Formatted addition to an array. Use e.g. for doubles.
 * 
 * @param xnm_value_array 
 * @param format 
 * 
 * @return 
 */
int xnm_value_array_push_printf(XnmValue * xnm_value_array,
                                const gchar *format,
                                ...);

/** 
 * Add a xnm_value to a xnm table. This function increases the reference
 * count of the xnm_value.
 *
 * Note! This function does not yet support hierarchical keys. To set an
 * a key that is not a direct parent of xnm_value_tree, the leaf
 * xnm_value table must first be retrieved, and this function then
 * called on that value.
 * 
 * @param xnm_value_tree 
 * @param key 
 * @param xnm_value 
 * 
 * @return 
 */
int xnm_value_table_set_key_value(XnmValue    * xnm_value_tree,
                                  const gchar * key,
                                  XnmValue    * xnm_value);

/** 
 * Convenience function for setting a string key value. If the key
 * already exists in xnm_value_parent, then it is freed and replaced
 * by the new svalue.
 * 
 * @param xnm_value_parent 
 * @param key 
 * @param string 
 * 
 * @return 
 */
int
xnm_value_table_set_key_value_string(XnmValue *xnm_value_parent,
                                     const gchar *key,
                                     const gchar *string);

/** 
 * Set a key value which is formatted with printf.
 * 
 * @param xnm_value_parent 
 * @param key 
 * @param format 
 * 
 * @return 
 */
int
xnm_value_set_key_value_printf(XnmValue *xnm_value_parent,
                               const gchar *key,
                               const gchar *format,
                               ...
                               );

/** 
 * Retrive the length of a XnmValue array.
 * 
 * @param xnm_value 
 * @param key 
 * 
 * @return Length of array if node found. -1 if key not found or if
 *         xnm_value is not an array.
 */
int
xnm_value_get_array_length (XnmValue *xnm_value,
                            const char *key);

/** 
 * Get a list of values in one call. This function is very convenient
 * for extracting multiple values in one call. It takes as argument
 * a NULL terminated vararg list, where each value extraction is
 * controlled by three arguments:
 *
 *   - The key - A still containing the path to the node to get info from
 *   - The type the third argument. This value is of type |XnmValueGetType|.
 *   - A pointer to where to store the value.
 *
 * @param xnm_value 
 * 
 * @return 
 */
int
xnm_value_get_values(XnmValue *xnm_value,
                     ...);

/** 
 * Parse a string in XNM syntax.
 * 
 * @param string A string in xnm format
 * @retval value A newly allocated XnmValue if the parsing was successful.
 * @retval error Error if parsing failed.
 * 
 * @return 
 */
int
xnm_parse_string(const gchar *string,
                 // output
                 XnmValue **value,
                 GError **error
                 );

/** 
 * Parse a string in asf format with a given len.
 * 
 * @param string 
 * @param len 
 * @param value 
 * @param error 
 * 
 * @return 
 */
int
xnm_parse_string_len(const gchar *string,
                     int len,
                     // output
                     XnmValue **value,
                     GError **error
                     );


/** 
 * Read a file and parse it.
 * 
 * @param filenam 
 * @param xnm_value 
 * @param error 
 * 
 * @return 
 */
int
xnm_parse_file(const gchar *filenam,
               // output
               XnmValue **xnm_value,
               GError **error);

#ifdef  __cplusplus
}
#endif

#endif
