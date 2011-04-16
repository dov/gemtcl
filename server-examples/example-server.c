/* Example program for the gnet_xmlrpc server functionality.
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
 */
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include "gnet_xmlrpc.h"

Tcl_Interp *tcl_interp;

int xmlrpc_cmd_ping(GNetXmlRpcServer *server,
                    const gchar *command,
                    const gchar *param,
                    gpointer user_data,
                    // output
                    gchar **reply_string);
int xmlrpc_cmd_tcl(GNetXmlRpcServer *server,
                   const gchar *command,
                   const gchar *param,
                   gpointer user_data,
                   // output
                   gchar **reply_string);

int xmlrpc_cmd_ping(GNetXmlRpcServer *server,
                   const gchar *command,
                   const gchar *param,
                   gpointer user_data,
                   // output
                   gchar **reply_string)
{
    *reply_string = g_strdup("pong");
    return 0;
}

int xmlrpc_cmd_tcl(GNetXmlRpcServer *server,
                   const gchar *command,
                   const gchar *param,
                   gpointer user_data,
                   // output
                   gchar **reply_string)
{
    int ret = 0;

    if (!param) {
        *reply_string = g_strdup("");
        return 0;
    }
    
    ret = Tcl_Eval(tcl_interp, param);

    if (ret == TCL_ERROR) 
        ret = -1;

    *reply_string = g_strdup(Tcl_GetStringResult(tcl_interp));

    return ret;
}

static int tcl_ping(ClientData clientData,
                    Tcl_Interp *interp,
                    int argc, const char *argv[])
{
    Tcl_SetResult(interp, "pong", TCL_VOLATILE);

    return TCL_OK;
}


int
main(int argc, char** argv)
{
  int port = 8123;
  GNetXmlRpcServer *server;
  GMainLoop* main_loop;

  Tcl_FindExecutable(argv[0]);
  gnet_init ();

  if (argc > 1)
      port = atoi(argv[1]);

  /* Create the main loop */
  main_loop = g_main_new (FALSE);

  server = gnet_xmlrpc_server_new(port);

  printf("port = %d\n", port);
  
  if (!server)
    {
      fprintf (stderr, "Error: Could not start server\n");
      exit (EXIT_FAILURE);
    }

  tcl_interp = Tcl_CreateInterp();
  Tcl_Init(tcl_interp);
  Tcl_CreateCommand(tcl_interp, "ping",
                    tcl_ping, (ClientData) NULL,
                    (Tcl_CmdDeleteProc *) NULL);

  gnet_xmlrpc_server_register_command(server,
				      "ping",
				      xmlrpc_cmd_ping,
				      NULL);

  gnet_xmlrpc_server_register_command(server,
				      "cmd",
				      xmlrpc_cmd_tcl,
				      NULL);

  /* Start the main loop */
  g_main_run(main_loop);

  exit (EXIT_SUCCESS);
  return 0;
}

