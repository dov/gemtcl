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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "xnm.h"

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt); 
    
    vfprintf(stderr, fmt, ap);
    exit(-1);
}

void print_xnm_value(XnmValue *xnm_value)
{
    char *exp_string = xnm_value_export_to_string(xnm_value);
    printf("%s\n", exp_string);
    g_free(exp_string);
}

#define CASE(s) if (!strcmp(s, S_))

int main(int argc, char **argv)
{
  const char s[] = 
    "oranges => 3\n"
    "bananas => 5\n"
    "reco => {\n"
    "  peach => 42\n"
    "  array => [1 2 3 \"hello there\"]\n"
    "}\n"
    "array=[3 23 5]\n"
    "deep=[[[[[[[[[[{a=>42}]]]]]]]]]]\n"
    ;
  int argp = 1;
  gchar *filename;
  XnmValue *root;
  GError *error = NULL;
  gchar *key = NULL;
  gboolean do_get_len = FALSE;
  
  while(argp < argc && argv[argp][0] == '-') {
    char *S_ = argv[argp++];
    
    CASE("-help")
      {
        printf("xnm-parse-and-query - Parse and query xnm strings\n\n"
               "Syntax:\n"
               "    xnm-parse-and-query [-key key] [-len] [file]\n"
               "\n"
               "Options:\n"
               "    -key key    Foo\n"
               "    -len        Get len of arrays\n"
               );
        exit(0);
      }
    CASE("-key")
      {
        key = argv[argp++];
        continue;
      }
    CASE("-len")
      {
        do_get_len++;
        continue;
      }
    die("Unknown option %s!\n", S_);
  }
  
  if (argp < argc)
    {
      filename = argv[argp++];
      
      xnm_parse_file(filename,
                     // output
                     &root,
                     &error);
    }
  else
    xnm_parse_string(s,
                     // output
                     &root,
                     &error);
  
  if (error)
    {
      fprintf(stderr, "parse error: %s\n", error->message);
      g_error_free(error);
      exit(-1);
    }

  if (do_get_len)
    {
      gchar *k = key;
      if (k == NULL)
        k = ""; // Root
      int len = xnm_value_get_array_length(root, k);
      printf("len = %d\n", len);
                                           
    }
  else if (key)
    {
      XnmValue *val;
      xnm_value_get(root,
                    key,
                    // output
                    &val);
      print_xnm_value(val);
      xnm_value_unref(val);
    }
  else
    {
      print_xnm_value(root);
    }
  xnm_value_unref(root);
  
  exit(0);
  return(0);
}
