/* Example program for the gnet_xmlrpc server functionality.
 *
 * This server simulates a simple microscope, that has the
 * following functionality:
 *
 * * zoom-in and zoom-out
 * * light control
 * * motion
 * * get-image
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
#include <string.h>
#include "gnet_xmlrpc.h"

#define CASE(s) if (strcmp(s, S_) == 0)

Tcl_Interp *tcl_interp;
double cam_pos_x = 0;
double cam_pos_y = 0;
double scale = 4.0;
double brightness = 10;
char *microscope_bright_filename = NULL;
gchar *gs_path = NULL;

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

// Move the camera to a position.
static int tcl_move_to(ClientData clientData,
                       Tcl_Interp *interp,
                       int argc, const char *argv[])
{
    gboolean do_rel = FALSE;
    int argp = 1;

    while (argp < argc
           && argv[argp][0] == '-'
           && (argv[argp][1] < '0'
               || argv[argp][1] > '9')
           
           ) {
        const char *S_ = argv[argp++];

        CASE("-help") {
            Tcl_SetResult(interp,
                          "move_to - move to a position\n"
                          "\n"
                          "Syntax:\n"
                          "    move_to [-rel] x y\n",
                          TCL_VOLATILE);
            return TCL_OK;
        }
        CASE("-rel") {
            do_rel = TRUE;
            continue;
        }

        Tcl_AppendResult(interp,
                         "Unknown option ",
                         S_,
                         0);
        return -1;
    }

    if (argp >= argc) {
        char res[30];
        sprintf(res, "%f %f", cam_pos_x, cam_pos_y);
        
        Tcl_SetResult(interp, res, TCL_VOLATILE);
        return TCL_OK;
    }
    else {
        double x = atof(argv[argp++]);
        double y = atof(argv[argp++]);
        if (do_rel) {
            cam_pos_x +=x;
            cam_pos_y +=y;
        }
        else {
            cam_pos_x = x;
            cam_pos_y = y;
        }
    }

    Tcl_SetResult(interp, "", TCL_VOLATILE);

    return TCL_OK;
}

static int tcl_light(ClientData clientData,
                     Tcl_Interp *interp,
                     int argc, const char *argv[])
{
    int argp = 1;

    while (argp < argc
           && argv[argp][0] == '-'
           && (argv[argp][1] < '0'
               || argv[argp][1] > '9')
           
           ) {
        const char *S_ = argv[argp++];

        CASE("-help") {
            Tcl_SetResult(interp,
                          "light - set light control\n"
                          "\n"
                          "Syntax:\n"
                          "    light [level]\n",
                          TCL_VOLATILE);
            return TCL_OK;
        }

        Tcl_AppendResult(interp,
                         "Unknown option ",
                         S_,
                         0);
        return -1;
    }

    if (argp >= argc) {
        char res[30];
        sprintf(res, "%.0f", brightness);
        
        Tcl_SetResult(interp, res, TCL_VOLATILE);
        return TCL_OK;
    }
    else {
        brightness = atoi(argv[argp++]);
    }

    Tcl_SetResult(interp, "", TCL_VOLATILE);

    return TCL_OK;
}

static int tcl_scale(ClientData clientData,
                     Tcl_Interp *interp,
                     int argc, const char *argv[])
{
    int argp = 1;

    while (argp < argc
           && argv[argp][0] == '-'
           && (argv[argp][1] < '0'
               || argv[argp][1] > '9')
           
           ) {
        const char *S_ = argv[argp++];

        CASE("-help") {
            Tcl_SetResult(interp,
                          "scale - set scale control\n"
                          "\n"
                          "Syntax:\n"
                          "    scale [level]\n",
                          TCL_VOLATILE);
            return TCL_OK;
        }

        Tcl_AppendResult(interp,
                         "Unknown option ",
                         S_,
                         0);
        return -1;
    }

    if (argp >= argc) {
        char res[30];
        sprintf(res, "%.0f", scale);
        
        Tcl_SetResult(interp, res, TCL_VOLATILE);
        return TCL_OK;
    }
    else {
        scale = atof(argv[argp++]);
    }

    Tcl_SetResult(interp, "", TCL_VOLATILE);

    return TCL_OK;
}

static int tcl_get_image(ClientData clientData,
                         Tcl_Interp *interp,
                         int argc, const char *argv[])
{
    int argp = 1;
    FILE *PS;
    const gchar* tmp_dir = g_get_tmp_dir();
    gchar *target_file = g_strdup_printf("%s/target.ps", tmp_dir);
    gchar *filename = g_strdup_printf("%s/image.png", tmp_dir);
    double brightness_factor = 1.0/10 * brightness;

    while (argp < argc && argv[argp][0] == '-') {
        const char *S_ = argv[argp++];

        CASE("-help") {
            Tcl_SetResult(interp,
                          "get_image - get an image from the microscope\n"
                          "\n"
                          "Syntax:\n"
                          "    get_image [-filename fn]\n",
                          TCL_VOLATILE);
            return TCL_OK;
        }
        CASE("-filename") {
            g_free(filename);
            filename = g_strdup(argv[argp++]);
            continue;
        }

        Tcl_AppendResult(interp,
                         "Unknown option ",
                         S_,
                         0);
        return -1;

    }

#ifdef G_OS_WIN32
    gchar *p;
    gchar *creatures_path = g_strdup_printf("%s/examples/microscope-creatures.ps",
                                            g_win32_get_package_installation_directory_of_module (NULL)
                                            );

    p = creatures_path;
    while(*p) { 
        if (*p == '\\')
            *p = '/';
        p++;
    }
#else
    gchar *creatures_path = g_strdup("microscope-creatures.ps");
#endif
    // Generate an image and save it in filename
    PS = fopen(target_file, "w");
    fprintf(PS,
            "%%!\n"
            "%% half image shift before scale and after scale\n"
            "/xyscale %f def\n"
            "-160 xyscale mul -120 xyscale mul translate\n"
            "xyscale dup scale\n"
            "160.0 xyscale div 120.0 xyscale div translate\n"
            "%f xyscale div %f xyscale div translate\n"
            "%% Overload setrgb to simulate microscope brightness.\n"
            "/org-setrgbcolor /setrgbcolor load def\n"
            "/brightness %f def\n"
            "/setrgbcolor {\n"
            "   brightness mul\n"
            "   3 1 roll\n"
            "   brightness mul\n"
            "   3 1 roll\n"
            "   brightness mul\n"
            "   3 1 roll\n"
            "   org-setrgbcolor\n"
            "} def\n"
            "/org-setgray /setgray load def\n"
            "(%s) run\n"
            "showpage\n"
            ,
            scale,
            -cam_pos_x,
            -cam_pos_y,
            brightness_factor,
            creatures_path
            );
    fclose(PS);

    char *cmd = g_strdup_printf("\"%s\" -q -dNOPAUSE -g320x240 -r72x72 -sDEVICE=pngalpha"
                                " -sOutputFile=\"%s\" \"%s\" quit.ps",
                                gs_path,
                                filename,
                                target_file);

    gchar *std_out = NULL;
    gchar *std_err = NULL;
    gint exit_status = 0;
    GError *error = NULL;
    Tcl_AppendResult(interp,
                     "Image saved in ",
                     target_file,
                     0);
    //    fprintf(stderr, "cmd = %s\n", cmd);

    g_spawn_command_line_sync(cmd,
                              &std_out,
                              &std_err,
                              &exit_status,
                              // output
                              &error);

    if (error) {
        fprintf(stderr, "Error: %s\n", error->message);
        g_free(error);
    }

    if (std_out)
        g_free(std_out);
    if (std_err)
        g_free(std_err);
    g_free(creatures_path);
    g_free(filename);
    g_free(target_file);

    return TCL_OK;
}

int
main(int argc, char** argv)
{
  int port = 8124;
  GNetXmlRpcServer *server;
  GMainLoop* main_loop;

  Tcl_FindExecutable(argv[0]);
  gnet_init ();

  if (argc > 1)
      port = atoi(argv[1]);

  /* Create the main loop */
  main_loop = g_main_new (FALSE);

#ifdef G_OS_WIN32
  if (getenv("GSPATH"))
      gs_path = g_strdup(getenv("GSPATH"));

  gchar *search_dir[] = { "c:/Program Files/gplgs",
                          "c:/Program Files/gs/gs8.63/bin",
                          NULL };

  int i=0;
  while(!gs_path && search_dir[i]) {
      gchar *gstest = g_strdup_printf("%s/gswin32c", search_dir[i]);
      if (g_file_test(gstest, G_FILE_TEST_IS_EXECUTABLE)) 
          gs_path = g_strdup(gstest);
      g_free(gstest);
      i++;
  }
  if (!gs_path)
      gs_path = g_find_program_in_path("gswin32c");

  // Todo: check that ghostscript path exists!

  if (!gs_path) {
      fprintf(stderr,
              "Error: Failed finding ghostscript. Define environment\n"
              "variable GSPATH to point to ghostscript executable.\n"
              );
      getch();
      exit(EXIT_FAILURE);
  }

  // Get rid of backslashes
  gchar *p;
  p = gs_path;
  while(*p) {
      if (*p == '\\')
          *p = '/';
      p++;
  }

  printf("Found ghostscript at: %s\n", gs_path);
#else
  gs_path = g_strdup("gs");
#endif
  
  server = gnet_xmlrpc_server_new(port);

  printf("Listening to port %d\n", port);
  
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
  
  Tcl_CreateCommand(tcl_interp,
                    "move_to",
                    tcl_move_to,
                    (ClientData) NULL,
                    (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(tcl_interp,
                    "get_image",
                    tcl_get_image,
                    (ClientData) NULL,
                    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(tcl_interp,
                    "light",
                    tcl_light,
                    (ClientData) NULL,
                    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(tcl_interp,
                    "scale",
                    tcl_scale,
                    (ClientData) NULL,
                    (Tcl_CmdDeleteProc *) NULL);

  gnet_xmlrpc_server_register_command(server,
				      "ping",
				      xmlrpc_cmd_ping,
				      NULL);

  gnet_xmlrpc_server_register_command(server,
				      "cmd",
				      xmlrpc_cmd_tcl,
				      NULL);

  const gchar* tmp_dir = g_get_tmp_dir();
  microscope_bright_filename = g_strdup_printf("%s/microscope_bright_filename", tmp_dir);

  /* Start the main loop */
  g_main_run(main_loop);

  exit (EXIT_SUCCESS);
  return 0;
}

