/* libxnm
 * xnm-parse-and-query.c:
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
%{
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "xnm.h"
#include "xnm_private.h"

static XnmValue *parse_top = NULL;
static GPtrArray *parse_stack = NULL; /* array of XnmValue */
static GPtrArray *key_stack = NULL;   /* array of gchar */
static gchar *xnm_key = NULL;
static XnmValue *xnm_val;
static gchar *lex_string = NULL;
static gint xnm_str_len = 0, xnm_pos = 0;
static gchar *xnm_str = NULL;
static GError **xnm_error;
static int xnm_start_of_line_pos, xnm_prev_start_of_line_pos, xnm_line_num;

int yylex ();
int yyerror (const char *s);  /* Called by yyparse on error */
int xnm_getchar();
void xnm_ungetch();
void print_xnm_value(XnmValue *xnm_value);

%}

%token ID
%token STRING
%token COMMENT

%%

input : {
  xnm_line_num = 1;
  xnm_start_of_line_pos = xnm_prev_start_of_line_pos = 0;

  parse_top = xnm_value_new_table();
  parse_stack = g_ptr_array_new();

  /* The parse top contains the topmost table we are adding to */
  g_ptr_array_add(parse_stack,
                  (gpointer)parse_top);
  xnm_value_ref(parse_top);
  key_stack = g_ptr_array_new();
} assignments {
}
;

assignments : assignment 
            | assignments assignment
            |
;

assignment : COMMENT
           | token {
  xnm_key = g_strdup(lex_string);
  g_ptr_array_add(key_stack, xnm_key);
}
arrow
value {
  xnm_key = g_ptr_array_index(key_stack, key_stack->len-1);
  g_ptr_array_remove_index(key_stack, key_stack->len-1);
  //		     std::cout << "Assigning: " << *xnm_key << " => " << export_string << "\n";
  xnm_table_set_key_value(parse_top->value.table,
                          xnm_key, xnm_val);
  xnm_value_unref(xnm_val);
  xnm_val = NULL;
  g_free(xnm_key);
}
;

arrow : '=' '>'
      | '='
      | ':'
      | '>'
;

token : ID
;

/* TBD - We can be more liberal here than for the key */
barestring : ID
;

token_or_string : barestring | STRING
;

values : value 
       | values value
       |
;

scalar_or_hash_or_array : scalar | hash | array | binary
;

value : scalar_or_hash_or_array {
  if (parse_top->type == XNM_ARRAY)
    {
      // std::string export_string;
      // xnm_val->export_to_string(export_string);
      //       std::cout << "Pushing " << export_string << "\n";
      xnm_array_push(parse_top->value.array, xnm_val);
      xnm_value_unref(xnm_val);
      xnm_val = NULL;
    }
} 
;

scalar : token_or_string  {
  xnm_val = xnm_value_new_string(lex_string);
}
;

array : '[' {
  if (parse_top)
    xnm_value_unref(parse_top);
  
  parse_top = xnm_value_new_array();
  g_ptr_array_add(parse_stack, parse_top);
  xnm_value_ref(parse_top);
}
values
']' {
  if (xnm_val)
    xnm_value_unref(xnm_val);
  xnm_val = parse_top;

  if (parse_top)
    xnm_value_unref(parse_top);
  g_ptr_array_remove_index(parse_stack, parse_stack->len-1);

  parse_top = g_ptr_array_index(parse_stack, parse_stack->len-1);
  xnm_value_ref(parse_top);
}
;

hash : '{' {
  if (parse_top)
    xnm_value_unref(parse_top);
  parse_top = xnm_value_new_table();
  g_ptr_array_add(parse_stack, parse_top);
  xnm_value_ref(parse_top);
}
assignments
'}' {
  if (xnm_val)
    xnm_value_unref(xnm_val);
  xnm_val = parse_top;

  if (parse_top)
    xnm_value_unref(parse_top);
  g_ptr_array_remove_index(parse_stack, parse_stack->len-1);
  
  parse_top = g_ptr_array_index(parse_stack, parse_stack->len-1);
  xnm_value_ref(parse_top);
}
;

binary : '<' {
  if (parse_top)
    xnm_value_unref(parse_top);
  parse_top = xnm_value_new_table();
  g_ptr_array_add(parse_stack, parse_top);
  xnm_value_ref(parse_top);
}
assignments
'>' {
  XnmValue *bin_val; /* Storage of binary value */
  int bin_size=-1;
  if (xnm_val)
    xnm_value_unref(xnm_val);
  xnm_val = parse_top;

  if (parse_top)
    xnm_value_unref(parse_top);
  g_ptr_array_remove_index(parse_stack, parse_stack->len-1);
  
  parse_top = g_ptr_array_index(parse_stack, parse_stack->len-1);
  xnm_value_ref(parse_top);

  /* Extract the size */
  xnm_value_get_int(xnm_val, "size",
                    &bin_size);
  if (bin_size < 0)
    // raise an error. How do you do that?;
    bin_size = 0;
  bin_val = xnm_value_new_binary(xnm_str + xnm_pos, bin_size);
  xnm_pos += bin_size;

  xnm_value_unref(xnm_val);
  xnm_val = bin_val;
}
;

%%

int xnm_getchar()
{
  int c;
  if (xnm_pos > xnm_str_len)
    return -1;
  /* printf("c=%c\n",xnm_str[xnm_pos]); fflush(stdout); */

  c = xnm_str[xnm_pos++];
  if (c == '\n')
    {
      xnm_line_num++;
      xnm_prev_start_of_line_pos = xnm_start_of_line_pos;
      xnm_start_of_line_pos = xnm_pos;
    }
  
  return c;
}

void xnm_ungetch()
{
  if (xnm_pos > 0)
    {
      xnm_pos--;
      if (xnm_str[xnm_pos] == '\n')
        {
          xnm_start_of_line_pos = xnm_prev_start_of_line_pos;
          xnm_line_num--;
        }
    }
}

int yylex ()
{
  int c;

  /* skip white space  */
  c = xnm_getchar();
  while(1) {
      if (c == '#') {
          while ((c = xnm_getchar ()) != '\n' && c>0)
              ;
      }

      // Skip white space
      while (c == ' ' || c == '\t' || c=='\n' || c == '\r') 
          c = xnm_getchar();

      if (c != '#')
          break;
  }

  if (lex_string)
    {
      free(lex_string);
      lex_string = NULL;
    }

  /* Strings */
  if (c == '"' || c == '\'')
    {
      GString *gs = g_string_new("");
      int start_char = c;

      while ((c = xnm_getchar ()) != start_char && c>0)
        {
          if (c == '\\')
            g_string_append_c(gs, xnm_getchar());
          else
            g_string_append_c(gs, c);
        }
      
      lex_string = gs->str;
      g_string_free(gs, FALSE);

      return STRING;
    }

  /* process alphanum  */
  if (isalpha (c) || isdigit(c) || c=='_' || c=='-')
    {
      GString *gs = g_string_new("");
      int ch;

      g_string_append_c(gs, c);
      while ((ch=xnm_getchar())>0 && 
	     (isalpha(ch) || isdigit(ch) || ch == '_' || ch=='-'
	      || ch=='.' || ch=='+'
	      ))
        g_string_append_c(gs, ch);
      xnm_ungetch();

      lex_string = gs->str;
      g_string_free(gs, FALSE);

      /* printf("ID: lex_string = %s\n", lex_string); */
      
      return ID;
    }

  /* return end-of-file  */
  if (c == -1)
    return 0;

  /* return single chars */
  return c;
}

int yyerror (const char *s)  /* Called by yyparse on error */
{
  g_set_error (xnm_error,
               XNM_ERROR,                 /* error domain */
               XNM_ERROR_SYNTAX_ERROR,     /* error code */
               "XNM syntax error at line %d column %d.",
               xnm_line_num, xnm_pos-xnm_start_of_line_pos);
  return 0;
}

int
xnm_parse_string_len(const gchar *string,
                     int len,
                     // output
                     XnmValue **value,
                     GError **error
                     )
{
  xnm_pos = 0;
  xnm_str = (char*)string;
  xnm_str_len = len;
  xnm_error = error;

  yyparse();

  *value = parse_top;

  /* Clean up everything except the parse_top */
  if (*error != NULL)
    {
      int i;
      for (i=0; i<key_stack->len; i++)
        g_free(g_ptr_array_index(key_stack, i));
      for (i=parse_stack->len-1; i>= 0; i--)
        {
          XnmValue *val = g_ptr_array_index(parse_stack, i);
          // Why is this test necessary??
          if (val)
            xnm_value_unref(val);
        }

      *value = 0;
    }

  xnm_value_unref(parse_top);
  parse_top = NULL;

  g_ptr_array_free(key_stack, TRUE);
  key_stack = NULL;
  g_ptr_array_free(parse_stack, TRUE);
  parse_stack = NULL;

  if (*xnm_error != NULL)
    return -1;

  return 0;
}

int
xnm_parse_string(const gchar *string,
                 // output
                 XnmValue **value,
                 GError **error
                 )
{
  return xnm_parse_string_len(string,
                              strlen(string),
                              // output
                              value,
                              error);
}

int
xnm_parse_file(const gchar *filename,
               // output
               XnmValue **xnm_value,
               GError **error)
{
  gchar *xnm_string;
  size_t len;
  g_file_get_contents(filename,
                      &xnm_string,
                      &len,
                      error);
  if (*error)
    return -1;
  
  return xnm_parse_string_len(xnm_string,
                              len,
                              xnm_value,
                              error);
}
