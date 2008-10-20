/* libxnm
 * test-libxnm.c:
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
 * test-libxnm.c - Testing by design of the xnm library.
 *
 * Dov Grobgeld <dov.grobgeld@gmail.com>
 * Thu Jan 11 23:44:32 2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xnm.h"

gboolean test_ok(gboolean is_ok,
                 int test_number);

const char test_string[] =
    "oranges => 3\n"
    "bananas => 5\n"
    "junk => {\n"
    "  tires => 42\n"
    "  bolts => [1 2 3 \"hello there\"]\n"
    "}\n"
    "array=[3 23 5]\n"
    "deep=[[[[[[[[[[{a=>42}]]]]]]]]]]\n"
    "pineapple => ananas\n"
    "reflection=> T"
    ;

const gchar *create_test_file()
{
    const gchar *test_file = "xnm-test.xnm";
    FILE *XNM = fopen(test_file, "w");
    fprintf(XNM, "%s", test_string);
    fclose(XNM);
    return test_file;
}

static int num_tests = 0;

int main()
{
    int count_ok = 0, count_failed;
    XnmValue *val = NULL;
    int ret;
    int val_int, len;
    gboolean val_bool = FALSE;
    const gchar *val_const_gchar;
    GError *error = NULL;
    GString *gs;

    ret = xnm_parse_string(test_string,
                           // output
                           &val,
                           &error);

    count_ok += test_ok(error == NULL && val != NULL,                       1);
    count_ok += test_ok(val->ref_count == 1,                                2);
    count_ok += test_ok(val->type == XNM_TABLE,                             3);
    ret = xnm_value_get_int(val, "junk/tires", &val_int);
    count_ok += test_ok(ret == 0,                                           4);
    count_ok += test_ok(val_int == 42,                                      5);
    xnm_value_table_set_key_value_string(val, "foo", "bar");
    ret = xnm_value_get_const_string(val, "foo", &val_const_gchar);
    count_ok += test_ok(strcmp(val_const_gchar, "bar")==0,                  6);
    ret = xnm_value_get_values(val,
                               "junk/tires", XNM_GET_INT, &val_int,
                               "pineapple", XNM_GET_STRING, &val_const_gchar,
                               "reflection", XNM_GET_BOOL, &val_bool,
                               NULL);
    count_ok += test_ok(ret== 0,                                            7);
    count_ok += test_ok(val_int == 42,                                      8);
    count_ok += test_ok(strcmp(val_const_gchar, "ananas")==0,               9);
    count_ok += test_ok(val_bool == TRUE,                                  10);

    count_ok += test_ok(xnm_value_get_array_length(val, "array") == 3,     11);
    
    xnm_value_table_set_key_value_string(val, "junk", "43");
    ret = xnm_value_get_const_string(val, "junk", &val_const_gchar);
    count_ok += test_ok(strcmp(val_const_gchar, "43")==0,                  12);
    
    xnm_value_unref(val);

    // Testing binary
    gs = g_string_new("");
    g_string_append(gs, "bin = <size=5>abcd");
    g_string_append_c(gs, 0);
    g_string_append(gs, "foo = bar\n");
    ret = xnm_parse_string_len(gs->str,
                               gs->len,
                               // output
                               &val,
                               &error);
    g_string_free(gs, TRUE);
    count_ok += test_ok(error == NULL && val != NULL,                      13);
    xnm_value_get_binary(val,
                         "bin",
                         &val_const_gchar,
                         &len);
    count_ok += test_ok(len == 5,                                          14);
    count_ok += test_ok(val_const_gchar[0]=='a'
                        && val_const_gchar[1]=='b'
                        && val_const_gchar[2]=='c'
                        && val_const_gchar[3]=='d'
                        && val_const_gchar[4]==0,                          15);
    xnm_value_get_const_string(val,
                               "foo",
                               &val_const_gchar);
    count_ok += test_ok(strcmp(val_const_gchar, "bar") == 0,               16);

    xnm_value_unref(val);
    
    /*======================================================================
    //  End of tests. Create summary.
    //----------------------------------------------------------------------*/
    count_failed = num_tests-count_ok;

    if (count_failed == 0)
        printf("All tests passed. :-)\n");
    else
        printf("%d out of %d tests failed.\n", num_tests-count_ok, num_tests);

    if (count_failed == 0)
        exit(0);
    else
        exit(-1);
}

gboolean test_ok(gboolean is_ok,
                 int test_number)
{
    if (is_ok)
        printf("Passed %d\n", test_number);
    else
        printf("Failed %d\n", test_number);

    num_tests++;
    return is_ok;
}

