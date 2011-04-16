/* An XMLRPC client and server library for GNet
 *
 * Copyright (c) 2006 Dov Grobgeld <dov.grobgeld@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gnet.h>
#include "gnet_xmlrpc.h"

typedef struct {
  GNetXmlRpcServer server; 
  GHashTable *command_hash;
} GNetXmlRpcServerPrivate;

typedef struct {
  GNetXmlRpcCommandCallback *callback;
  GNetXmlRpcCommandAsyncCallback *async_callback;
  gpointer user_data;
} command_hash_value_t;

typedef struct {
  GNetXmlRpcServerPrivate *xmlrpc_server;
  GString *connection_string;
} connection_state_t;

static void ob_server_func (GServer* server, GConn* conn, gpointer user_data);
static void ob_client_func (GConn* conn, GConnEvent* event, 
                            gpointer user_data);
gchar *create_fault_response_string(int fault_code,
                                    const gchar *reply);
gchar *create_response_string(const gchar *reply);

int extract_xmlrpc_request(const gchar *str,
                           /* output */
                           gchar **command_name,
                           gchar **parameter
                           );

GNetXmlRpcServer *gnet_xmlrpc_server_new(int port)
{
  GNetXmlRpcServerPrivate *xmlrpc_server = g_new0(GNetXmlRpcServerPrivate, 1);
  
  /* Create the server */
  GServer *gnet_server = gnet_server_new (NULL, port, ob_server_func, xmlrpc_server);
  if (!gnet_server)
    {
      fprintf (stderr, "Error: Could not start server\n");
      return NULL;
    }
  xmlrpc_server->server.gnet_server = gnet_server;
  xmlrpc_server->command_hash = g_hash_table_new(g_str_hash,
                                                 g_str_equal);

  return (GNetXmlRpcServer*)xmlrpc_server;
}

void gnet_xmlrpc_server_delete(GNetXmlRpcServer *xmlrpc_server)
{
  g_hash_table_destroy(((GNetXmlRpcServerPrivate*)xmlrpc_server)->command_hash);
  gnet_server_delete (((GNetXmlRpcServerPrivate*)xmlrpc_server)->server.gnet_server);
  g_free(xmlrpc_server);
}

static void
ob_server_func (GServer* server, GConn* conn, gpointer user_data)
{
  if (conn)
    {
      connection_state_t *conn_state = g_new0(connection_state_t, 1);
      conn_state->xmlrpc_server = (GNetXmlRpcServerPrivate*)user_data;
      conn_state->connection_string = g_string_new("");
      
      gnet_conn_set_callback (conn, ob_client_func, conn_state);
      gnet_conn_set_watch_error (conn, TRUE);
      gnet_conn_readline (conn);
    }
  else  /* Error */
    {
      gnet_server_delete (server);
      exit (EXIT_FAILURE);
    }
}


static void
ob_client_func (GConn* conn, GConnEvent* event, gpointer user_data)
{
  connection_state_t *conn_state = (connection_state_t*)user_data;
  GNetXmlRpcServerPrivate *xmlrpc_server = conn_state->xmlrpc_server;
  GString *req_string = conn_state->connection_string;

  switch (event->type)
    {
    case GNET_CONN_READ:
      {
        event->buffer[event->length-1] = '\n';
        g_string_append_len(req_string, event->buffer, event->length);

        /* Check if we have the whole request */
        if (g_strstr_len(req_string->str,
                         req_string->len,
                         "</methodCall>") != NULL)
          {
            /* Extract command name */
            gchar *command_name=NULL, *parameter=NULL;
            gchar *response=NULL;
            command_hash_value_t *command_val = NULL;
            
            extract_xmlrpc_request(req_string->str,
                                   /* output */
                                   &command_name,
                                   &parameter
                                   );

            /* Call the callback */
            if (command_name)
              command_val = g_hash_table_lookup(xmlrpc_server->command_hash,
                                                command_name);
            if (!command_val)
              response = create_fault_response_string(-2,
                                                      "No such method!");
            else if (command_val->async_callback)
              {
                (*command_val->async_callback)((GNetXmlRpcServer*)xmlrpc_server,
                                               conn,
                                               command_name,
                                               parameter,
                                               command_val->user_data);
              }
            else
              {
                gchar *reply;
                int ret = (*command_val->callback)((GNetXmlRpcServer*)xmlrpc_server,
                                         command_name,
                                         parameter,
                                         command_val->user_data,
                                         /* output */
                                         &reply);
                if (ret == 0)
                  response = create_response_string(reply);
                else
                  response = create_fault_response_string(-1,
                                                          reply);
                g_free(reply);
              }

            if (response)
              {
                /* Send reply */
                gnet_conn_write (conn, response, strlen(response));
                g_free(response);
              }
            
            /* printf("rs = ((%s)) len=%d\n", req_string->str, event->length); */
            g_string_assign(conn_state->connection_string, "");

            g_free(command_name);
            if (parameter)
                g_free(parameter);
          }

        gnet_conn_readline (conn);
        break;
      }

    case GNET_CONN_WRITE:
      {
        ; /* Do nothing */
        break;
      }

    case GNET_CONN_CLOSE:
    case GNET_CONN_TIMEOUT:
    case GNET_CONN_ERROR:
      {
        g_string_free(conn_state->connection_string, TRUE);
        g_free(conn_state);
        gnet_conn_delete (conn);
        break;
      }

    default:
      g_assert_not_reached ();
    }
}

static int
gnet_xmlrpc_server_register_command_full(GNetXmlRpcServer *_xmlrpc_server,
                                         const gchar *command,
                                         GNetXmlRpcCommandCallback *callback,
                                         GNetXmlRpcCommandAsyncCallback *async_callback,
                                         gpointer user_data)
{
  GNetXmlRpcServerPrivate *xmlrpc_server = (GNetXmlRpcServerPrivate*)_xmlrpc_server;
  /* TBD - Check if command exist and override it */
  command_hash_value_t *val = g_new0(command_hash_value_t, 1);
  val->callback = callback;
  val->async_callback = async_callback;
  val->user_data = user_data;
  g_hash_table_insert(xmlrpc_server->command_hash,
                      g_strdup(command),
                      val);

  return 0;
}

int gnet_xmlrpc_server_register_command(GNetXmlRpcServer *_xmlrpc_server,
                                        const gchar *command,
                                        GNetXmlRpcCommandCallback *callback,
                                        gpointer user_data)
{
  gnet_xmlrpc_server_register_command_full(_xmlrpc_server,
                                           command,
                                           callback,
                                           NULL, /* async */
                                           user_data);

  return 0;
}

int gnet_xmlrpc_server_register_async_command(GNetXmlRpcServer *_xmlrpc_server,
                                              const gchar *command,
                                              GNetXmlRpcCommandAsyncCallback *async_callback,
                                              gpointer user_data)
{
  gnet_xmlrpc_server_register_command_full(_xmlrpc_server,
                                           command,
                                           NULL,
                                           async_callback, /* async */
                                           user_data);
  return 0;
}

/** 
 * Reply to an async request
 * 
 * @param gnet_client 
 * @param reply_string 
 * 
 * @return 
 */
int gnet_xmlrpc_async_client_response(GConn *conn,
                                      const gchar *reply)
{
  gchar *response = create_response_string(reply);
  /* Send reply */
  gnet_conn_write (conn, response, strlen(response));
  g_free(response);

  return 0;
}

/**
 * Extract the xmlrpc request. This is done by simple reg exp matching.
 * This should be changed to xml decoding
 */
int extract_xmlrpc_request(const gchar *str,
                           /* output */
                           gchar **command_name,
                           gchar **parameter
                           )
{
  GString *param_string = g_string_new("");
  GError *error = NULL;
  GRegex *re;
  GMatchInfo *match_info;
  const gchar *p;

  re = g_regex_new("<methodName>(\\S*)</methodName>"
                   "(?:.*<param>\\s*"
                   "<value>\\s*"
                   "<string>\\s*"
                   "(.*?)"
                   "</string>)?"
                   ".*</methodCall>"
                   ,
                   G_REGEX_DOTALL,
                   0,
                   &error);

  if (!g_regex_match(re,
                     str,
                     0,
                     &match_info)) {
      g_regex_unref(re);
      return -1;
  }

  *command_name = g_match_info_fetch(match_info, 1);
  gchar *param_name_raw = g_match_info_fetch(match_info, 2);
  g_match_info_free(match_info);
  g_regex_unref(re);
  
  /* Decode and build parameter */
  p = param_name_raw;
  if (p && strlen(p)) {
      while (*p)
        {
          gchar c = *p++;
    
          if (c == '&')
            {
              if (g_strstr_len(p, 4, "amp;") == p)
                {
                  g_string_append_c(param_string, '&');
                  p+= 4;
                }
              if (g_strstr_len(p, 5, "quot;") == p)
                {
                  g_string_append_c(param_string, '"');
                  p+= 5;
                }
              else if (g_strstr_len(p, 3, "lt;") == p)
                {
                  g_string_append_c(param_string, '<');
                  p+= 3;
                }
              else if (g_strstr_len(p, 3, "gt;") == p)
                {
                  g_string_append_c(param_string, '>');
                  p+= 3;
                }
              else
                {
                  /* Don't know what to do. Just add the ampersand.. */
                  g_string_append_c(param_string, '&');
                }
            }
          else
            g_string_append_c(param_string, c);
        }
      *parameter = param_string->str;
      g_string_free(param_string, FALSE);
  }
  else
      *parameter = NULL;

  if (param_name_raw)
    g_free(param_name_raw);

  return 0;
                                   
}

void append_and_encode_string(GString *res,
                              const gchar *s)
{
  const gchar *p = s;
  while(*p) {
    gchar c = *p++;
    switch (c)
      {
      case '&' : g_string_append(res, "&amp;"); break;
      case '<': g_string_append(res, "&lt;"); break;
      case '>': g_string_append(res, "&gt;"); break;
      default:
        g_string_append_c(res, c);
      }
  }
}

gchar *cat_header_contents(GString *content_string)
{
  GString *msg_string = g_string_new("");
  gchar *msg_res = NULL;
  
  g_string_append_printf(msg_string,
                         "HTTP/1.1 200 OK\n"
                         "Connection: close\n"
                         "Content-Length: %d\n"
                         "Content-Type: text/xml\n"
                         "Date: Fri, 1 Jan 2000 00:00:00 GMT\n"
                         "Server: GNetXMLRpc server\n"
                         "\n"
                         "%s",
                         content_string->len,
                         content_string->str
                         );

  msg_res = msg_string->str;
  g_string_free(msg_string, FALSE);

  return msg_res;
}

gchar *create_fault_response_string(int fault_code,
                                    const gchar *reply)
{
  GString *content_string = g_string_new("");
  gchar *res;

  g_string_append_printf(content_string,
                         "<?xml version=\"1.0\"?>\n"
                         "<methodResponse>\n"
                         "  <fault>\n"
                         "    <value>\n"
                         "      <struct>\n"
                         "        <member>\n"
                         "          <name>faultCode</name>\n"
                         "          <value><int>%d</int></value>\n"
                         "        </member>\n"
                         "        <member>\n"
                         "          <name>faultString</name>\n"
                         "          <value><string>",
                         fault_code);

  append_and_encode_string(content_string, reply);
  g_string_append(content_string,
                  "</string></value>\n"
                  "        </member>\n"
                  "      </struct>\n"
                  "    </value>\n"
                  "  </fault>\n"
                  "</methodResponse>\n"
                  );

  res = cat_header_contents(content_string);
  g_string_free(content_string, TRUE);

  return res;
}

gchar *create_response_string(const gchar *reply)
{
  GString *content_string = g_string_new("");
  gchar *res;

  g_string_append(content_string,
                  "<?xml version=\"1.0\"?> <methodResponse> <params> <param> <value><string>");

  append_and_encode_string(content_string, reply);
  g_string_append(content_string,
                  "</string></value> </param> </params> </methodResponse>\n");

  res = cat_header_contents(content_string);
  g_string_free(content_string, TRUE);

  return res;
}

