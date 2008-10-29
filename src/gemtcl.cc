//======================================================================
//  gemtcl.cpp - Embedded tcl in gtk example.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Mar 23 09:47:34 2007
//
//  Implementation notes:
//    - The w_button_box_vbox contains two widgets: the notebook on
//      top and the widget being displayed at the bottom. Since the
//      latter is the last widget shown in its container, we can
//      remove it an put another widget there when switching pages.
//----------------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <tcl.h>
#include <string.h>
#include "plis.h"
#include "gem-button-box.h"
#include "gem-button-box-editor.h"
#include "../config.h"
#include "logo_150_inline.i"
#include "logo_64_inline.i"
#include "xnm/xnm.h"
#include "gnet_xmlrpc.h"

using namespace std;
using namespace plis;

#ifdef WIN32
#include <windows.h>
#endif

#define CASE(s) if (S_ == s)

enum
{
  DND_TEXT_PLAIN,
  DND_TEXT_URI_LIST
};

// include change log string
const char changelog_string[] =
#include "changelog_string.i"
;

/* Target types for dropping into the file list */
static const GtkTargetEntry file_list_dest_targets[] = {
    { (gchar*)"text/uri-list", 0, DND_TEXT_URI_LIST }
};

static const int num_file_list_dest_targets = (sizeof (file_list_dest_targets)
					       / sizeof (file_list_dest_targets[0]));

static void cb_menu_quit(gpointer   callback_data,
                         guint      callback_action,
                         GtkWidget *widget);
static void cb_menu_open(gpointer   callback_data,
                         guint      callback_action,
                         GtkWidget *widget);
static void cb_menu_save(gpointer   callback_data,
                         guint      callback_action,
                         GtkWidget *widget);
static void cb_menu_save_as(gpointer   callback_data,
                            guint      callback_action,
                            GtkWidget *widget);
static void cb_menu_file_close(gpointer   callback_data,
                                guint      callback_action,
                                GtkWidget *widget);
static void cb_toggle_view_button_box(gpointer   callback_data,
                                      guint      callback_action,
                                      GtkWidget *widget);
static void cb_menu_button_box_new(gpointer   callback_data,
                                   guint      callback_action,
                                   GtkWidget *widget);
static void cb_menu_edit_button_box(gpointer   callback_data,
                                    guint      callback_action,
                                    GtkWidget *widget);
static void cb_menu_tools_history(gpointer   callback_data,
                                  guint      callback_action,
                                  GtkWidget *widget);
static void cb_menu_tools_xmlrpc(gpointer   callback_data,
                                 guint      callback_action,
                                 GtkWidget *widget);
static int command_tcl_eval_add_newline(const gchar *cmd);
static int command_tcl_eval(const gchar *cmd);

static void cb_menu_help_changelog(gpointer   callback_data,
                                   guint      callback_action,
                                   GtkWidget *widget);
static void cb_menu_about(gpointer   callback_data,
                          guint      callback_action,
                          GtkWidget *widget);
static void cb_menu_help_changelog(gpointer   callback_data,
                                   guint      callback_action,
                                   GtkWidget *widget);
gboolean cb_idle_add_result_to_buffer(gpointer user_data);
gboolean cb_idle_load_file(gpointer user_data);
gboolean cb_idle_add_eval_end(gpointer user_data);
static gboolean cb_idle_tcl_gets(gpointer user_data);
void cb_button_box_name_changed(GtkWidget *widget,
                                const gchar *new_name);
int load_gtf_file(const gchar *filename);

static int tcl_gemtcl_xnm(ClientData clientData,
                          Tcl_Interp *interp,
                          int argc, const char *argv[]);


static void cb_switched_page (GtkNotebook     *notebook,
                              GtkNotebookPage *page,
                              guint            page_num,
                              gpointer         user_data);
static int xmlrpc_cmd_tcl_eval_finish(const gchar *reply);
int xmlrpc_cmd_ping(GNetXmlRpcServer *server,
                    const gchar *command,
                    const gchar *param,
                    gpointer user_data,
                    // output
                    gchar **reply_string);
int xmlrpc_cmd_tcl_eval(GNetXmlRpcServer *server,
                        GConn *gnet_client,
                        const gchar *command,
                        const gchar *param,
                        gpointer user_data);
int xmlrpc_cmd_append_to_log(GNetXmlRpcServer *server,
                             const gchar *command,
                             const gchar *param,
                             gpointer user_data,
                             // output
                             gchar **reply_string);

static GtkItemFactoryEntry menu_items[] = {
    { "/_File",		 NULL,	       0,		      0, "<Branch>" },
    { "/File/_Open",	 "<control>O", (GtkItemFactoryCallback)cb_menu_open,	      0, "<StockItem>", GTK_STOCK_OPEN },
    { "/File/_Save",	 "<control>S", (GtkItemFactoryCallback)cb_menu_save,	      0, "<StockItem>", GTK_STOCK_SAVE },
    { "/File/Save as",	 "<control>S", (GtkItemFactoryCallback)cb_menu_save_as,	      0, "<StockItem>", GTK_STOCK_SAVE_AS },
    { "/File/Close",   NULL,         (GtkItemFactoryCallback)cb_menu_file_close,       0, "<StockItem>", GTK_STOCK_CLOSE },
    { "/File/_New",	 NULL, (GtkItemFactoryCallback)cb_menu_button_box_new,	0, "<Item>" , 0 },
    { "/File/sep1",        NULL,         0,       0, "<Separator>" },
    { "/File/_Quit",	 "<control>Q", (GtkItemFactoryCallback)cb_menu_quit,	      0, "<StockItem>", GTK_STOCK_QUIT },
    { "/_Edit",		 NULL,	       0,		      0, "<Branch>" },
    { "/Edit/ButtonBox",  NULL, (GtkItemFactoryCallback)cb_menu_edit_button_box,       0, "<Item>", 0 },
    { "/Edit/Preferences",   NULL,         0,       0, "<StockItem>", GTK_STOCK_PREFERENCES },
    { "/_Tools",		 NULL,	       0,		      0, "<Branch>" },
    { "/_Tools/_History",  NULL, (GtkItemFactoryCallback)cb_menu_tools_history,   0, "<Item>" },
    { "/_Tools/_XmlRpc",  NULL, (GtkItemFactoryCallback)cb_menu_tools_xmlrpc,       0, "<CheckItem>" },
    { "/_View",		 NULL,	       0,		      0, "<Branch>" },
    { "/_View/_ButtonBox",  NULL, (GtkItemFactoryCallback)cb_toggle_view_button_box,       0, "<CheckItem>", 0 },
    { "/_Help",		 NULL,	       0,		      3, "<LastBranch>" },
    { "/Help/_About",	 NULL,	       (GtkItemFactoryCallback)cb_menu_about,	      0, "<StockItem>", GTK_STOCK_HELP},
    { "/Help/ChangeLog",	 NULL,	       (GtkItemFactoryCallback)cb_menu_help_changelog,	      0, "<StockItem>", GTK_STOCK_INDEX},
    //    { "/Help/ChangeLog",	 NULL,	       (GtkItemFactoryCallback)cb_menu_help_changelog,	      0, "<StockItem>", GTK_STOCK_INDEX},
};

static int nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

Tcl_Interp *interp;
GtkItemFactory *item_factory;
GtkWidget *w_window, *w_text_view, *w_notebook=NULL, *w_button_box_vbox = NULL;
GtkWidget *w_virtual_notebook_page = NULL;
GtkWidget *w_history_window = NULL;
GtkWidget *w_vpaned;
GtkTextIter *current_pos;
vector<slip> history;
slip command_log_message;
bool command_log_message_do_clear = true;
int history_pointer = -1;
GtkTextMark *prompt_mark;
bool doing_readline = false;
GMainLoop *loop;
slip gets_string, tcl_puts_string;
bool readline_abort = false;
bool do_eval = false;
#ifdef WIN32
static HANDLE tcl_eval_mutex, tcl_reply_mutex;
#else
static GMutex *tcl_eval_mutex = NULL, *tcl_reply_mutex = NULL;
#endif
static string tcl_eval_string;
static string button_box_tcl_string="";
vector<GtkWidget*> button_box_list;
slip last_image_path;
static GNetXmlRpcServer *xmlrpc_server;
GConn *xmlrpc_handle = NULL;
double window_width=800, window_height=600;
slip settings_file;
bool is_waiting_for_eval = false;

void move_mark_prompt(GtkTextIter *req_pos);

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt); 
    
    vfprintf(stderr, fmt, ap);
    exit(-1);
}

// A properly casted function for Tcl_SetResult.
static void tcl_str_free(char*buf)
{
    g_free(buf);
}

void dbg_iter(const char *s,
              GtkTextIter *iter)
{
    printf("Tag %s: line.char = %d.%d\n", s,
           gtk_text_iter_get_line(iter),
           gtk_text_iter_get_line_offset(iter));
}

void insert_to_end_of_buffer()
{
    GtkTextIter iter_end;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));
    gtk_text_buffer_get_end_iter(text_buffer,
                                 &iter_end);

    gtk_text_buffer_move_mark_by_name(text_buffer, "insert", &iter_end);
    gtk_text_buffer_move_mark_by_name(text_buffer, "selection_bound", &iter_end);
    
}

void scroll_insert_into_viewer()
{
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));
    GtkTextMark *mark = gtk_text_buffer_get_insert(text_buffer);

    // Get the complete line on screen
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(w_text_view),
                                 mark,
                                 0.3,
                                 FALSE,
                                 0,
                                 1);
}

void replace_cmd(string cmd)
{
    GtkTextIter iter_end, prompt_pos;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));
    gtk_text_buffer_get_end_iter(text_buffer,
                                 &iter_end);

    gtk_text_buffer_get_iter_at_mark(text_buffer,
                                     &prompt_pos,
                                     prompt_mark
                                     );
    gtk_text_buffer_delete(text_buffer, &prompt_pos, &iter_end);
    
    gtk_text_buffer_get_end_iter(text_buffer,
                                 &iter_end);

    gtk_text_buffer_insert (text_buffer,
                            &iter_end,
                            cmd.c_str(), -1);

    scroll_insert_into_viewer();
}

// Return the command at the cursor
slip get_cmd()
{
    GtkTextIter iter_end, iter_prompt;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));
    gtk_text_buffer_get_end_iter(text_buffer,
                                 &iter_end);

    gtk_text_buffer_get_iter_at_mark(text_buffer,
                                     &iter_prompt,
                                     prompt_mark
                                     );
    char *line = gtk_text_buffer_get_text (text_buffer, &iter_prompt, &iter_end,TRUE);
    slip cmd = line;
    cmd.chomp();
    g_free(line);
    return cmd;
}

void history_prev()
{
    string current_cmd = get_cmd();

#if 0
    // TBD add commands in progress
    if (history_pointer == (int)history.size()
        && current_cmd.length()
        && current_cmd != history[history.size()-1]) {
        history.push(current_cmd);
    }
#endif
    if (history_pointer > 0) 
        replace_cmd(history[--history_pointer].c_str());
}

void history_next()
{
    if (history_pointer < (int)history.size()-1) 
        replace_cmd(history[++history_pointer].c_str());
}

void history_set(int new_pos)
{
    if (new_pos>=0 && new_pos < (int)history.size()) {
        history_pointer = new_pos;
        replace_cmd(history[history_pointer].c_str());
    }
}

void move_beginning_of_sentence()
{
    GtkTextIter iter, prompt_pos;
    int prompt_line, insert_line;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));
    GtkTextMark *mark = gtk_text_buffer_get_insert(text_buffer);
    gtk_text_buffer_get_iter_at_mark(text_buffer,
                                     &iter,
                                     mark);
    if (!gtk_text_iter_starts_sentence(&iter))
        gtk_text_iter_backward_sentence_starts (&iter, 1);
    // Check if the iter is at the same line as the prompt. If so forward
    // to the end of the prompt.
    gtk_text_buffer_get_iter_at_mark(text_buffer,
                                     &prompt_pos,
                                     prompt_mark
                                     );
    prompt_line = gtk_text_iter_get_line(&prompt_pos);
    insert_line = gtk_text_iter_get_line(&iter);
    if (prompt_line == insert_line)
        iter = prompt_pos;
    
    gtk_text_buffer_move_mark_by_name(text_buffer, "insert", &iter);
    gtk_text_buffer_move_mark_by_name(text_buffer, "selection_bound", &iter);
}

void move_end_of_sentence()
{
    GtkTextIter iter;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));
    GtkTextMark *mark = gtk_text_buffer_get_insert(text_buffer);
    gtk_text_buffer_get_iter_at_mark(text_buffer,
                                     &iter,
                                     mark);
    if (!gtk_text_iter_ends_sentence(&iter))
        gtk_text_iter_forward_sentence_ends (&iter, 1);
    gtk_text_buffer_move_mark_by_name(text_buffer, "insert", &iter);
    gtk_text_buffer_move_mark_by_name(text_buffer, "selection_bound", &iter);
}

static void
cb_file_list_drag_data_received (GtkWidget          *widget,
				 GdkDragContext     *context,
				 gint                x,
				 gint                y,
				 GtkSelectionData   *selection_data,
				 guint               info,
				 guint               drag_time,
				 gpointer            data)
{
    switch (info) {
    case DND_TEXT_URI_LIST: {
        gchar **uris;
        guint i;
        
        uris = gtk_selection_data_get_uris (selection_data);
        
        // The following works on windows... Should instead recreate
        // uris and then use common loading code...
        if (uris == NULL) {
            uris = g_new0(gchar*, 2);
            slip sdata = (const char*)selection_data->data;
            llip filename_list = sdata.split("\\r?\\n");
            uris[1] = 0;
            
            for (int i=0; i<filename_list; i++) {
                uris[0] = g_strdup(filename_list[i]);
                
                // Get out of loop after loading first image...
                break;
            }
        }
        
        for (i = 0; uris[i] != NULL; i++) {
            gchar *path = g_filename_from_uri(uris[i], NULL, NULL);

            load_gtf_file(path);
            
            g_free(path);
            
            break;
        }
        
        g_strfreev (uris);
        break;
    }
        
    default:
        break;
    }
    
    gtk_drag_finish (context, TRUE, FALSE, drag_time);
}

gboolean cb_key_press_event (GtkWidget   *widget,
                             GdkEventKey *event,
                             gpointer     user_data)
{
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));

    if (event->state & GDK_MOD1_MASK) {
        switch(event->keyval) {
        case 'p':
        case 'P':
        case GDK_Up:
        case GDK_KP_Up:
            history_prev();
            return true;
            break;
        case 'n':
        case 'N':
        case GDK_Down:
        case GDK_KP_Down:
            history_next();
            return true;
            break;
        case 'q':
            exit(0);
        default:
            ;
        }
    }
    else if (event->state & GDK_CONTROL_MASK) {
        switch(event->keyval) {
        case 'q':
            exit(0);
            break;
        case GDK_Up:
        case GDK_KP_Up:
            history_prev();
            return true;
            break;
        case GDK_Down:
        case GDK_KP_Down:
            history_next();
            return true;
            break;
        case 'j':
            do_eval = true;
            break;
        case 'c': {
            if (!gtk_text_buffer_get_has_selection(text_buffer) && doing_readline) {
                readline_abort = true;
                if (g_main_loop_is_running (loop))
                    g_main_loop_quit (loop);
            }
            break;
        }
        case 'r':
            cb_menu_tools_history(NULL, -1, NULL);
            break;
        default:
            ;
        }
    }
    else if (!(event->state & GDK_SHIFT_MASK)) {
        switch(event->keyval) {
        case GDK_F2:
            cb_menu_tools_history(NULL, -1, NULL);
            break;
        case GDK_Return:
            do_eval = true;
            //            printf("Do eval!\n");
            break; // Return false to continue processing
        case GDK_Up:
            history_prev();
            return true;
            break;
        case GDK_Down:
            history_next();
            return true;
            break;
        case GDK_Home:
        case GDK_KP_Home:
            move_beginning_of_sentence();
            return true;
            break;
        case GDK_Escape:
            {
                GtkTextIter iter_end, iter_prompt;

                gtk_text_buffer_get_end_iter(text_buffer,
                                             &iter_end);
                gtk_text_buffer_get_iter_at_mark(text_buffer,
                                                 &iter_prompt,
                                                 prompt_mark
                                                 );
                gtk_text_buffer_delete(text_buffer,
                                       &iter_prompt, &iter_end);
                return true;
            }
        case GDK_End:
        case GDK_KP_End:
            move_end_of_sentence();
            return true;
            break;
        case GDK_BackSpace:
            // Don't do anything if we are in the beginning of the row
            {
                
                GtkTextIter iter_prompt, iter_insert;
                GtkTextMark *insert;
                
                gtk_text_buffer_get_iter_at_mark(text_buffer,
                                                 &iter_prompt,
                                                 prompt_mark
                                                 );
                insert = gtk_text_buffer_get_insert(text_buffer);
                gtk_text_buffer_get_iter_at_mark(text_buffer,
                                                 &iter_insert,
                                                 insert
                                                 );
                if (gtk_text_iter_compare(&iter_insert, &iter_prompt) <= 0) 
                    return true;

                // Don't erase new lines
                if (gtk_text_iter_starts_sentence(&iter_insert))
                    return true;

                {
                    bool ret = false;
                    GtkTextIter iter2 = iter_insert;
                    gtk_text_iter_backward_char(&iter2);
                    
                    char *line = gtk_text_buffer_get_text (text_buffer, &iter2, &iter_insert, TRUE);
                    if (line[0] == '\n')
                        ret = true;
                    g_free(line);
                    return ret;
                }
            }
            
            return false;
        default:
            ;
        }

        // If we are hitting an alphabetic character, move the curser to
        // the end of the buffer.
        GtkTextIter iter_end, iter_prompt, iter_insert;
        GtkTextMark *insert;

        gtk_text_buffer_get_end_iter(text_buffer,
                                     &iter_end);

        gtk_text_buffer_get_iter_at_mark(text_buffer,
                                         &iter_prompt,
                                         prompt_mark
                                         );
        insert = gtk_text_buffer_get_insert(text_buffer);
        gtk_text_buffer_get_iter_at_mark(text_buffer,
                                         &iter_insert,
                                         insert
                                         );

        if (gtk_text_iter_compare(&iter_insert, &iter_prompt) < 0) {
            gtk_text_buffer_move_mark_by_name(text_buffer, "insert", &iter_end);
            gtk_text_buffer_move_mark_by_name(text_buffer, "selection_bound", &iter_end);
        }
    }
            
    return false;
}

void cb_insert_text (GtkTextBuffer *text_buffer,
                     GtkTextIter   *location,
                     gchar         *text,
                     gint           len,
                     gpointer       user_data)
{
    // The de_eval makes sure that we do not enter recursion.
    if (do_eval) {
        do_eval = false;

        // The mangling of the text below is quite a mess and should
        // be cleaned up!

        // Erase all trailing new-lines that were inserted
        GtkTextIter iter_end, iter_prev, iter;
        gtk_text_buffer_get_end_iter(text_buffer,
                                     &iter_end);
        iter = iter_end;
        iter_prev = iter = *location;
        gtk_text_iter_backward_char(&iter);
        gtk_text_buffer_delete(text_buffer, &iter, &iter_prev);


        // Erase new lines at end of line
        gtk_text_buffer_get_end_iter(text_buffer,
                                     &iter_end);
        iter = iter_end;
        gtk_text_iter_backward_char(&iter);
        while(gtk_text_iter_get_char(&iter)=='\n') {
            gtk_text_buffer_delete(text_buffer, &iter, &iter_end);
            gtk_text_buffer_get_end_iter(text_buffer,
                                         &iter);
            gtk_text_iter_backward_char(&iter);
        }
        // Finally insert a single newline.
        gtk_text_buffer_insert (text_buffer,
                                &iter_end,
                                "\n", -1);

        move_end_of_sentence();
        GtkTextIter line_start = iter_end;
        if (!gtk_text_iter_backward_line (&line_start))
            gtk_text_iter_set_offset (&line_start,
                                      0);
        
        GtkTextMark *mark_pos = gtk_text_buffer_create_mark(text_buffer,
                                                            NULL,
                                                            &line_start,
                                                            true);
        GtkTextIter last_char = iter_end;
        GtkTextIter line_end;

        // Remove the added line feed and add it again at the end of the line.
        gtk_text_iter_backward_char(&last_char);
        gtk_text_buffer_delete(text_buffer, &last_char, &iter_end);
        gtk_text_buffer_get_iter_at_mark(text_buffer,
                                         &line_start,
                                         mark_pos
                                         );
        line_end = line_start;
        gtk_text_iter_forward_to_line_end (&line_end);
#if 0
        dbg_iter("line_start",  &line_start);
        dbg_iter("line_end",  &line_end);
#endif
        if (doing_readline) {
            gtk_text_buffer_get_end_iter(text_buffer,
                                         &iter_end);

            gtk_text_buffer_insert (text_buffer,
                                    &iter_end,
                                    "\n", -1);

            gets_string = get_cmd();
            if (g_main_loop_is_running (loop))
                g_main_loop_quit (loop);

            if (!history.size() || history[history.size()-1] != gets_string) {
                gets_string.chomp();
                if (!gets_string.m("^\\s*$"))
                    history.push_back(gets_string);
            }
            history_pointer = history.size();
        }
        else {

            char *line = gtk_text_buffer_get_text (text_buffer, &line_start, &line_end,TRUE);
            
#if 0
            printf("Line = %s\n", line);
#endif
            char *lp = line;

            if (*lp == '%')
                lp ++;  // Skip prompt
            while(*lp == ' ')
                lp++;
            
            gtk_text_iter_forward_to_line_end (&line_end);
            
            iter_end = line_end;
            gtk_text_iter_forward_to_end (&iter_end);
            
#if 0
            dbg_iter("line_end", &line_end);
            dbg_iter("iter_end", &iter_end);
#endif
            if (!gtk_text_iter_equal(&line_end,
                                     &iter_end)) {
                gtk_text_buffer_insert (text_buffer,
                                        &iter_end,
                                        lp, -1);
                gtk_text_buffer_insert (text_buffer,
                                        &iter_end,
                                        "\n", -1);
            }
            else {
                gtk_text_buffer_insert(text_buffer,
                                       &line_end, "\n", -1);
                
            }
            
            gtk_text_buffer_get_iter_at_mark(text_buffer,
                                             &iter_end,
                                             mark_pos
                                             );
            gtk_text_iter_forward_to_end (&iter_end);
            
            // Is there another way to move to the end of the buffer
            gtk_text_buffer_move_mark_by_name(text_buffer, "insert", &iter_end);
            gtk_text_buffer_move_mark_by_name(text_buffer, "selection_bound", &iter_end);
            
            current_pos = &iter_end;
            if (!history.size() || history[history.size()-1] != lp) {
                slip S_ = lp;
                if (!S_.m("^\\s*$"))
                    history.push_back(S_);
            }
            history_pointer = history.size();

            command_tcl_eval(lp);
            g_free(line);
        }
    }
    else {
        //        printf("Cut and paste?\n");
    }
}

// Moves the prompt-end mark to the current position. 
void
move_mark_prompt(GtkTextIter *req_iter)
{
    GtkTextIter iter;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));
    GtkTextMark *mark = gtk_text_buffer_get_insert(text_buffer);

    if (req_iter)
        iter = *req_iter;
    else
        gtk_text_buffer_get_iter_at_mark(text_buffer,
                                         &iter,
                                         mark);
    gtk_text_buffer_move_mark_by_name(text_buffer, "prompt-end", &iter);
    
    // Should scroll so that end of line is visible??
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(w_text_view),
                                 prompt_mark,
                                 0,
                                 FALSE,
                                 0, 0.8);
}
              

void
bind_tcl_commands(GtkWidget *w_text_view)
{
    GtkTextIter iter_end;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));

    g_signal_connect_after (G_OBJECT (text_buffer), "insert-text",
                            G_CALLBACK (cb_insert_text), NULL);
    g_signal_connect (G_OBJECT (w_text_view), "key-press-event",
                      G_CALLBACK (cb_key_press_event), NULL);
    
    gtk_text_buffer_get_end_iter(text_buffer,
                                 &iter_end);
#if 0
    gtk_text_buffer_insert (text_buffer,
                            &iter_end,
                            "% ", -1);
#endif

    prompt_mark = gtk_text_buffer_create_mark(text_buffer,
                                              "prompt-end",
                                              &iter_end,
                                              true);

    move_mark_prompt(NULL);
}

void
load_settings()
{
    GError *error = NULL;
    
    /* Create a new GKeyFile object and a bitwise list of flags. */
    GKeyFile *keyfile = g_key_file_new();
    GKeyFileFlags flags = GKeyFileFlags(0);
    
    /* Load the GKeyFile from keyfile.conf or return. */
    if (!g_key_file_load_from_file (keyfile,
                                    settings_file,
                                    flags,
                                    &error)) {
        return;
    }
    
    char *val = g_key_file_get_string(keyfile,
                                      "settings",
                                      "window_width",
                                      NULL);
    if (val)
        window_width = atoi(val);
    val = g_key_file_get_string(keyfile,
                                "settings",
                                "window_height",
                                NULL);
    if (val)
        window_height = atoi(val);

    g_key_file_free(keyfile);
    
}

void
save_settings()
{
    GKeyFile *keyfile = g_key_file_new();
    GError *error = NULL;
    
    g_key_file_set_integer(keyfile,
                           "settings",
                           "window_width",
                           window_width);
    g_key_file_set_integer(keyfile,
                           "settings",
                           "window_height",
                           window_height);
    
    gchar *data = g_key_file_to_data(keyfile,NULL, NULL);
 
    g_file_set_contents(settings_file,
                        data,
                        -1,
                        &error);
    // Ignore errors
    if (error) {
        g_error_free(error);
    }
    g_free(data);
    g_key_file_free(keyfile);
}

void cb_window_resize(GtkWidget     *widget,
                      GtkAllocation *allocation,
                      gpointer       user_data)
{
    window_width = allocation->width;
    window_height = allocation->height;
    save_settings();
}

void create_widgets()
{
    GtkWidget *w_vbox, *w_text_sw;
    GtkAccelGroup *accel_group;
    GError *err = NULL;
    GdkPixbuf *pixbuf_icon = gdk_pixbuf_new_from_inline(sizeof(logo_64_inline),
                                                          logo_64_inline,
                                                          FALSE,
                                                          &err);

    w_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(w_window), "size-allocate",
                     G_CALLBACK(cb_window_resize), NULL);
                     
    gtk_window_set_icon (GTK_WINDOW(w_window),
                         pixbuf_icon);

    gtk_widget_set_default_direction(GTK_TEXT_DIR_LTR);

    gtk_signal_connect (GTK_OBJECT (w_window), "destroy",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_window_set_policy(GTK_WINDOW(w_window), TRUE, TRUE, TRUE);

    w_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(w_window), w_vbox);

    accel_group = gtk_accel_group_new ();
    item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", accel_group);
    gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);
    gtk_box_pack_start (GTK_BOX (w_vbox),
			gtk_item_factory_get_widget (item_factory, "<main>"),
			FALSE, FALSE, 0);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_item_factory_get_item (item_factory, "/View/ButtonBox")), TRUE);
    gtk_widget_show(gtk_item_factory_get_widget (item_factory, "<main>"));

#if 0
    w_vpaned = gtk_vpaned_new();
    gtk_box_pack_start(GTK_BOX(w_vbox), w_vpaned,
                       TRUE, TRUE, 0);
#endif

    w_button_box_vbox = gtk_vbox_new(FALSE, 0);
    w_notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(w_button_box_vbox), w_notebook, TRUE, TRUE, 0);
#if 0
    gtk_paned_pack1(GTK_PANED(w_vpaned),
                    w_button_box_vbox,
                    FALSE,
                    TRUE);
#endif
    gtk_box_pack_start(GTK_BOX(w_vbox),
                       w_button_box_vbox,
                       FALSE, FALSE, 0); 

    g_signal_connect(G_OBJECT(w_notebook), "switch-page",
                     G_CALLBACK(cb_switched_page), w_vpaned);

    w_text_sw = gtk_scrolled_window_new(NULL,NULL);
    w_text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(w_text_view), GTK_WRAP_CHAR);

    gtk_widget_modify_font(w_text_view, pango_font_description_from_string("Monospace"));

    
#if 0
    gtk_paned_pack2(GTK_PANED(w_vpaned), w_text_sw, TRUE, TRUE);
#endif
    gtk_box_pack_start(GTK_BOX(w_vbox),
                       w_text_sw,
                       TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(w_text_sw), w_text_view);
    gtk_widget_set_size_request(w_window, window_width, window_height);
    bind_tcl_commands(w_text_view);

    // DnD 
    g_signal_connect (G_OBJECT(w_window),
                      "drag-data-received",
                      G_CALLBACK (cb_file_list_drag_data_received),
                      NULL);
    g_signal_connect (G_OBJECT(w_text_view),
                      "drag-data-received",
                      G_CALLBACK (cb_file_list_drag_data_received),
                      NULL);

    gtk_drag_dest_set (GTK_WIDGET (w_window),
                       (GTK_DEST_DEFAULT_ALL),
                       file_list_dest_targets,
                       num_file_list_dest_targets,
                       GDK_ACTION_COPY);
    
    GtkTargetList *target_list;
    target_list = gtk_target_list_new (NULL, 0);
    gtk_target_list_add_uri_targets (target_list, DND_TEXT_URI_LIST);
    gtk_target_list_add_text_targets (target_list, DND_TEXT_PLAIN);
    gtk_drag_dest_set_target_list (GTK_WIDGET (w_text_view), target_list);
    gtk_drag_dest_set_target_list (GTK_WIDGET (w_window), target_list);
    gtk_target_list_unref (target_list);

    gtk_widget_show_all(w_window);
}

static void close_xmlrpc()
{
    gnet_xmlrpc_server_delete(xmlrpc_server);
    xmlrpc_server = NULL;
}

static int create_xmlrpc(int port)
{
    if (port < 0)
        port = 8822; // Default port
    if (xmlrpc_server)
        close_xmlrpc();

    // Create the xmlrpc server
    xmlrpc_server = gnet_xmlrpc_server_new(port);
    if (xmlrpc_server) {
        gnet_xmlrpc_server_register_command(xmlrpc_server,
                                            "Ping",
                                            xmlrpc_cmd_ping,
                                            NULL);
        gnet_xmlrpc_server_register_async_command(xmlrpc_server,
                                                  "TclEval",
                                                  xmlrpc_cmd_tcl_eval,
                                                  NULL);
        gnet_xmlrpc_server_register_command(xmlrpc_server,
                                            "AppendToLog",
                                            xmlrpc_cmd_append_to_log,
                                            NULL);
    }
    else
        return -1;
    return 0;
}

static int tcl_puts(ClientData clientData,
                    Tcl_Interp *interp,
                    int argc, const char *argv[])
{
    bool no_newline = false;
    int argp = 1;

    if (strcmp(argv[argp], "-nonewline") == 0) {
        no_newline = true;
        argp++;
    }

    tcl_puts_string = argv[argp];

    if (!no_newline)
        tcl_puts_string += "\n";
#if 0
    printf("puts = %s\n", tcl_puts_string.c_str());
#endif

    g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                    cb_idle_add_result_to_buffer,
                    g_strdup(tcl_puts_string.c_str()),
                    g_free);

    sprintf(interp->result,"%s","");
    return TCL_OK;
}

static int tcl_load_gtf(ClientData clientData,
                        Tcl_Interp *interp,
                        int argc, const char *argv[])
{
    int argp = 1;

    g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                    cb_idle_load_file,
                    g_strdup(argv[argp]),
                    g_free);

    sprintf(interp->result,"%s","");

    return TCL_OK;
}

static gboolean cb_idle_gemtcl_get(gpointer user_data)
{
    int page_idx = gtk_notebook_get_current_page(GTK_NOTEBOOK(w_notebook));
    GtkWidget *current_bb = GTK_WIDGET(g_object_get_data(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(w_notebook), page_idx)), "button_box"));

    gchar *widget_string = gem_button_box_get_widget_val(GEM_BUTTON_BOX(current_bb),
                                                         (char*)user_data);
    if (widget_string) {
        gets_string = widget_string;
        g_free(widget_string);
    }
    else 
        gets_string = "";

#ifdef WIN32
    SetEvent(tcl_eval_mutex);
#else            
    g_mutex_unlock(tcl_eval_mutex);
#endif
    
    return FALSE;
}

static gboolean cb_idle_gemtcl_set(gpointer user_data)
{
    int page_idx = gtk_notebook_get_current_page(GTK_NOTEBOOK(w_notebook));
    GtkWidget *current_bb = GTK_WIDGET(g_object_get_data(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(w_notebook), page_idx)), "button_box"));

    llip match = slip((char*)user_data).split("\\|");
    if (match.length()<2)
        match.push_back("");

    if (gem_button_box_set_widget_val(GEM_BUTTON_BOX(current_bb),
                                      match[0],
                                      match[1])!=0)
        gets_string = "Failed setting value!";
    else
        gets_string = "";
        
#ifdef WIN32
    SetEvent( tcl_eval_mutex);
#else            
    g_mutex_unlock(tcl_eval_mutex);
#endif
    
    return FALSE;
}

static gboolean cb_idle_gemtcl_show_hide(gpointer user_data)
{
    int page_idx = gtk_notebook_get_current_page(GTK_NOTEBOOK(w_notebook));
    GtkWidget *current_bb = GTK_WIDGET(g_object_get_data(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(w_notebook), page_idx)), "button_box"));

    llip match = slip((char*)user_data).split("\\|");
    if (match.length()<2)
        match.push_back("");

    visibility_t vis;
    if (match[0].m("_show"))
        vis = VISIBILITY_SHOW;
    else if (match[0].m("_hide"))
        vis = VISIBILITY_HIDE;
    else if (match[0].m("_sensitive"))
        vis = VISIBILITY_SENSITIVE;
    else
        vis = VISIBILITY_INSENSITIVE;

    gem_button_box_set_widget_visibility(GEM_BUTTON_BOX(current_bb),
                                         match[1],
                                         vis);
#ifdef WIN32
    SetEvent( tcl_eval_mutex);
#else            
    g_mutex_unlock(tcl_eval_mutex);
#endif
    
    return FALSE;
}

static gboolean cb_idle_gemtcl_serve(gpointer user_data)
{
#ifdef WIN32
    SetEvent(tcl_eval_mutex);
#else            
    g_mutex_unlock(tcl_eval_mutex);
#endif
    
    return FALSE;
}

static int tcl_gemtcl_get(ClientData clientData,
                       Tcl_Interp *interp,
                       int argc, const char *argv[])
{
    g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                    cb_idle_gemtcl_get,
                    g_strdup(argv[1]),
                    g_free);

#ifdef WIN32
    WaitForSingleObject(tcl_eval_mutex, INFINITE );
#else
    g_mutex_lock(tcl_eval_mutex);
#endif
    
    Tcl_SetResult(interp, g_strdup(gets_string.c_str()), tcl_str_free);
    return TCL_OK;
}

static int tcl_gemtcl_widget_show_hide(ClientData clientData,
                                       Tcl_Interp *interp,
                                       int argc, const char *argv[])
{
    g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                    cb_idle_gemtcl_show_hide,
                    g_strdup_printf("%s|%s", argv[0], argv[1]),
                    g_free);

#ifdef WIN32
    WaitForSingleObject(tcl_eval_mutex, INFINITE );
#else
    g_mutex_lock(tcl_eval_mutex);
#endif
    
    Tcl_SetResult(interp, g_strdup(""), tcl_str_free);
    return TCL_OK;
}

static int tcl_gemtcl_serve(ClientData clientData,
                            Tcl_Interp *interp,
                            int argc, const char *argv[])
{
#if 0
    // Give control to the gtk main loop to serve events and lock
    // until we are ready.
    g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                    cb_idle_gemtcl_serve,
                    NULL, NULL);

#ifdef WIN32
    WaitForSingleObject(tcl_eval_mutex, INFINITE );
#else
    g_mutex_lock(tcl_eval_mutex);
#endif

#endif

    if (tcl_eval_string.length()) {
        string s = tcl_eval_string;
        tcl_eval_string = "";
        Tcl_Eval(interp, s.c_str());
    }

    return TCL_OK;
}

static int tcl_gemtcl_set(ClientData clientData,
                          Tcl_Interp *interp,
                          int argc, const char *argv[])
{
    g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                    cb_idle_gemtcl_set,
                    g_strdup_printf("%s|%s", argv[1], argv[2]),
                    g_free);

#ifdef WIN32
    WaitForSingleObject(tcl_eval_mutex, INFINITE );
#else
    g_mutex_lock(tcl_eval_mutex);
#endif

    if (gets_string != "") {
        Tcl_SetResult(interp, g_strdup(gets_string.c_str()), tcl_str_free);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int tcl_gemtcl_xnm(ClientData clientData,
                          Tcl_Interp *tcl,
                          int argc, const char *argv[])
{
    slip res;
    int argp = 1;
    bool do_get = true;
    slip filename;
    slip xnm_string, xnm_key;
    bool do_len = false;
    bool do_ref = false;
    bool do_keylist = false;
    bool do_quiet = false;
    
    while (argp < argc && argv[argp][0] == '-') {
	slip S_ = argv[argp++];

        CASE("-help") {
            slip res = 
                "xnm - Parse strings in the xnm syntax\n"
                "\n"
                "Syntax:\n"
                "    xnm -get [-file file] [-string s] [-key key] [-len]\n"
                "\n"
                "Options:\n"
                "    -file file       Filename of file to parse\n"
                "    -string string   String to parse\n"
                "    -key key         Key to return value of\n"
                "    -len             Return length of an array. Returns -1 for non-arrays.\n"
                "    -ref             Returns type of value or under if not defined.\n"
                "    -keylist         Return a list of keys for a table.\n"
                ;
            Tcl_AppendResult(tcl, res.c_str(), NULL);
            return 0;
        }
        CASE("-file") {
            filename = argv[argp++];
            continue;
        }
        CASE("-string") {
            xnm_string = argv[argp++];
            // Ugly hack... But should be effective
            if (xnm_string.length() == 0)
                xnm_string = " ";
            continue;
        }
        CASE("-key") {
            xnm_key = argv[argp++];
            continue;
        }
        CASE("-len") {
            do_len=true;
            continue;
        }
        CASE("-ref") {
            do_ref=true;
            continue;
        }
        CASE("-keylist") {
            do_keylist = true;
            continue;
        }
        CASE("-q") {
            do_quiet = true;
            continue;
        }
        CASE("-get") {
            do_get = true;
            continue;
        }

        res = "Unknown option " + S_ + "!\n";
        break;
    }

    XnmValue *val = NULL;
    GError *error = NULL;
    if (filename.length()) {
        xnm_parse_file(filename, &val, &error);
        if (error) {
            res = slipprintf("Failed parsing file: %s",
                             error->message);
            g_error_free(error);
        }
    }
    else if (xnm_string.length()) {
        xnm_parse_string(xnm_string,
                         &val,
                         &error);
        if (error) {
            res = slipprintf("Failed parsing string: %s",
                             error->message);
            g_error_free(error);
        }
    }
    else
        res = "Neither string nor file was given!";
    if (res.length()) {
        Tcl_AppendResult(tcl, res.c_str(), NULL);
        return TCL_ERROR;
    }

    if (do_len) {
        int len = xnm_value_get_array_length(val, xnm_key);
        if (do_quiet)
            res = slipprintf("%d", len);
        else
            res = slipprintf("Array len = %d\n", len);
    }
    else if (do_keylist) {
        res = "Sorry! do_keylist is not supported yet!\n";
#if 0
        std::vector<string> key_list = xnm_value_get_keys(val, xnm_key);
        for (int i=0; i<(int)key_list.size(); i++) 
            res += key_list[i] + " ";
        res.chop();
        return res + "\n";
#endif
    }
    else if (do_ref) {
        XnmValue *v = NULL;
        xnm_value_get(val,
                      xnm_key,
                      // output
                      &v);

        slip ref_name;
        if (v == NULL)
            ref_name = "UNDEF";
        else if (v->type == XNM_TABLE)
            ref_name = "TABLE";
        else if (v->type == XNM_ARRAY)
            ref_name = "ARRAY";
        else if (v->type == XNM_STRING)
            ref_name = "STRING";
        res =  slipprintf("%s\n", ref_name.c_str());
        if (v)
            xnm_value_unref(v);
    }
    else {
        gchar *sval=NULL;
        xnm_value_get_string(val,
                             xnm_key,
                             // output
                             &sval);
        if (sval) {
            res += sval;
            g_free(sval);
        }
    }
            
    if (val)
        xnm_value_unref(val);
    res.chomp();
    Tcl_AppendResult(tcl, res.c_str(), NULL);
    
    return 0;
}

static int tcl_gemtcl_get_last_command_log(ClientData clientData,
                                           Tcl_Interp *tcl,
                                           int argc, const char *argv[])
{
    Tcl_SetResult(interp, g_strdup(command_log_message.c_str()), tcl_str_free);
    command_log_message = "";
    return 0;
}

static int tcl_gemtcl_xmlrpc_server(ClientData clientData,
                                    Tcl_Interp *tcl,
                                    int argc, const char *argv[])
{
    int port = -1; // default
    int argp = 1;

    if (argc == 1) {
        const char* on_off_names[] = { "off", "on" };
        int xmlrpc_mode = (xmlrpc_server != NULL);
        Tcl_SetResult(interp, g_strdup_printf("xmlrpc is %s",
                                              on_off_names[xmlrpc_mode]),
                      tcl_str_free);
    }
    else {
        bool new_state = false;
        while(argp < argc) {
            slip S_ = argv[argp++];
            CASE("-on") {
                new_state = true;
                continue;
            }
            CASE("-off") {
                new_state = false;
                continue;
            }
            CASE("-port") {
                port = atoi(argv[argp++]);
                new_state = true;
                continue;
            }
            Tcl_SetResult(interp, g_strdup_printf("Unknown option %s!",
                                                  S_.c_str()),
                          tcl_str_free);
            return TCL_ERROR;
        }
        if (new_state) {
            int ret = create_xmlrpc(port);
            if (ret != 0) {
                Tcl_SetResult(interp,
                              g_strdup("Failed creating xmlrpc server!"),
                              tcl_str_free);

                return TCL_ERROR;
            }
            Tcl_SetResult(interp, g_strdup_printf(""), tcl_str_free);
        }
        else {
            Tcl_SetResult(interp, g_strdup_printf(""), tcl_str_free);

            close_xmlrpc();
        }
    }
    return TCL_OK;
}

static void
app_error (const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    gchar *message = g_strdup_vprintf(format, ap);
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (GTK_WINDOW(w_window),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     message);
    // gtk_window_set_keep_above(GTK_WINDOW(dialog), true);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    g_free(message);

    return;
}

static int tcl_gets(ClientData clientData,
                    Tcl_Interp *interp,
                    int argc, const char *argv[])
{
    g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                    cb_idle_tcl_gets,
                    g_strdup(tcl_puts_string.c_str()),
                    g_free);

#ifdef WIN32
    WaitForSingleObject(tcl_eval_mutex, INFINITE );
#else
    g_mutex_lock(tcl_eval_mutex);
#endif
    
    if (readline_abort) {
        Tcl_SetResult(interp, g_strdup("^C"), tcl_str_free);
        return TCL_ERROR; 
    }
    else {
        Tcl_SetResult(interp, g_strdup(gets_string.c_str()), tcl_str_free);
        return TCL_OK;
    }
}

static gboolean cb_idle_tcl_gets(gpointer user_data)
{
    doing_readline = true;
    readline_abort = false;
    move_mark_prompt(NULL);
    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);
    doing_readline = false;
#ifdef WIN32
    SetEvent( tcl_eval_mutex);
#else            
    g_mutex_unlock(tcl_eval_mutex);
#endif
    
    return FALSE;
}

//======================================================================
// TBD:
//     If the user is at the end of the buffer and has typed text,
//     then that text should still be at the end of the buffer after
//     this procedure.
//----------------------------------------------------------------------
void add_to_end_of_buffer(gboolean do_strip_trailing_newline,
                          gboolean do_add_prompt,
                          gboolean do_before_prompt,
                          const gchar* text)
{
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_text_view));
    GtkTextIter iter;
    string res = text;

    if (do_strip_trailing_newline) {
        // res.chomp();
        if (res.length())
            res += "\n";
    }

    gtk_text_buffer_get_end_iter(text_buffer,
                                 &iter);
    if (!gtk_text_iter_starts_line(&iter)) {
        GtkTextIter end_iter = iter;
        if (do_before_prompt) {
            GtkTextIter iter_end, iter_prompt, iter_insert;
            GtkTextMark *insert;
            
            gtk_text_buffer_get_end_iter(text_buffer,
                                         &iter_end);
            
            gtk_text_buffer_get_iter_at_mark(text_buffer,
                                             &iter_prompt,
                                             prompt_mark
                                             );
            insert = gtk_text_buffer_get_insert(text_buffer);
            gtk_text_buffer_get_iter_at_mark(text_buffer,
                                             &iter_insert,
                                             insert
                                             );

            if (gtk_text_iter_compare(&iter_insert, &iter_prompt) < 1) 
                gtk_text_buffer_place_cursor(text_buffer, &iter_end);

            iter = iter_end;
            
            gtk_text_iter_backward_sentence_starts (&iter, 1);
            // move_mark_prompt(&iter);
        }
            
    }
    gtk_text_buffer_insert (text_buffer,
                            &iter,
                            res.c_str(), res.length());

    if (do_add_prompt) {
        gtk_text_buffer_get_end_iter(text_buffer,
                                     &iter);
        
        if (!gtk_text_iter_starts_line(&iter))
            gtk_text_iter_backward_sentence_starts (&iter, 1);

        // Only insert the "% " if the mark is not already at the
        // end of the buffer
        GtkTextMark *mark_prompt = gtk_text_buffer_get_mark(text_buffer,
                                                            "prompt-end");
        GtkTextIter mark_iter;
        gtk_text_buffer_get_iter_at_mark(text_buffer, &mark_iter, mark_prompt);
        gtk_text_buffer_insert (text_buffer,
                                &iter,
                                "% ", -1);
        // Erase old prompt if it is at the iter pos
        {
            GtkTextIter iter2 = iter;
            gtk_text_iter_forward_chars(&iter2, 2);
            
            char *line = gtk_text_buffer_get_text (text_buffer, &iter, &iter2, TRUE);
            if (strcmp(line, "% ")==0) {
                gtk_text_buffer_delete(text_buffer, &iter, &iter2);
            }
            g_free(line);
        }
            
        move_mark_prompt(&iter);

        // Move the cursor to the end of the line if it is behind the mark
        // prompt. Todo: preserve text that is being edited...
        GtkTextIter iter_end, iter_prompt, iter_insert;
        GtkTextMark *insert;

        gtk_text_buffer_get_end_iter(text_buffer,
                                     &iter_end);

        gtk_text_buffer_get_iter_at_mark(text_buffer,
                                         &iter_prompt,
                                         prompt_mark
                                         );
        insert = gtk_text_buffer_get_insert(text_buffer);
        gtk_text_buffer_get_iter_at_mark(text_buffer,
                                         &iter_insert,
                                         insert
                                         );

        if (gtk_text_iter_compare(&iter_insert, &iter_prompt) < 1) {
            gtk_text_buffer_move_mark_by_name(text_buffer, "insert", &iter_end);
            gtk_text_buffer_move_mark_by_name(text_buffer, "selection_bound", &iter_end);
        }

    }
    scroll_insert_into_viewer();
}

gboolean cb_idle_add_result_to_buffer(gpointer user_data)
{
    add_to_end_of_buffer(false, false, true, (const gchar *)user_data);

    return FALSE;
}

gboolean cb_idle_load_file(gpointer user_data)
{
    gchar *filename = (gchar*)user_data;
    load_gtf_file(filename);

    return FALSE;
}

gboolean cb_idle_add_eval_end(gpointer user_data)
{
    add_to_end_of_buffer(true, true, true, (const gchar *)user_data);
    if (xmlrpc_handle) {
        xmlrpc_cmd_tcl_eval_finish((const gchar*)user_data);
        xmlrpc_handle = NULL;
    }

    return FALSE;
}

void thread_init_tcl()
{
    interp = Tcl_CreateInterp();
    Tcl_Init(interp);
    
    Tcl_CreateCommand(interp, "gemtcl_puts",
                      tcl_puts, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_gets",
                      tcl_gets, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_widget_hide",
                      tcl_gemtcl_widget_show_hide, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_widget_show",
                      tcl_gemtcl_widget_show_hide, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_widget_sensitive",
                      tcl_gemtcl_widget_show_hide, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_widget_insensitive",
                      tcl_gemtcl_widget_show_hide, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_serve",
                      tcl_gemtcl_serve, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_load",
                      tcl_load_gtf, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_get",
                      tcl_gemtcl_get, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_set",
                      tcl_gemtcl_set, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "xnm",
                      tcl_gemtcl_xnm, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_get_last_command_log",
                      tcl_gemtcl_get_last_command_log, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "gemtcl_xmlrpc_server",
                      tcl_gemtcl_xmlrpc_server, (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);
    Tcl_Eval(interp,
             "proc shift {list} {\n"
             "  upvar $list l\n"
             "  set e [lindex $l 0]\n"
             "  set l [lreplace $l 0 0]\n"
             "  return $e\n"
             "}\n"
             "\n"
             "# head LIST  returns the first element of LIST\n"
             "proc head {list} {\n"
             "  return [lindex $list 0]\n"
             "}\n"
             "rename puts ::tcl::puts\n"
             "\n"
             // lassign was introduced in tcl 8.5
             "if {[info procs lassign] == \"\"} {\n"
             "    proc lassign {values args} {\n"
             "        while {[llength $values] < [llength $args]} {\n"
             "            lappend values {}\n"
             "        }\n"
             "        uplevel 1 [list foreach $args $values break]\n"
             "        lrange $values [llength $args] end\n"
             "    }\n"
             "}\n"
             "proc puts args {\n"
             "      set la [llength $args]\n"
             "      if {$la<1 || $la>3} {\n"
             "         error \"usage: puts ?-nonewline? ?channel? string\"\n"
             "      }\n"
             "      set nl \"\\n\"\n"
             "      if {[lindex $args 0]==\"-nonewline\"} {\n"
             "         set nl \"\"\n"
             "         set args [lrange $args 1 end]\n"
             "      }\n"
             "      if {[llength $args]==1} {\n"
             "         set args [list stdout [join $args]] ;# (2)\n"
             "      }\n"
             "      foreach {channel s} $args break\n"
             "      #set s [join $s] ;# (1) prevent braces at leading/tailing spaces\n"
             "      if {$channel==\"stdout\" || $channel==\"stderr\"} {\n"
             "         gemtcl_puts -nonewline $s$nl\n"
             "      } else {\n"
             "         set cmd ::tcl::puts\n"
             "         if {$nl==\"\"} {lappend cmd -nonewline}\n"
             "         lappend cmd $channel $s\n"
             "         eval $cmd\n"
             "      }\n"
             "   }\n"

             "if {[info command tcl::gets] eq \"\"} {rename gets tcl::gets}\n"
             "proc gets {chan {varN \"\"}} {\n"
             "    if {$varN ne \"\"} {\n"
             "       upvar 1 $varN var\n"
             "    }\n"
             "    if {$chan ne \"stdin\"} {\n"
             "        if {$varN ne \"\"} {\n"
             "            tcl::gets $chan var ;# return character count\n"
             "        } else {\n"
             "            tcl::gets $chan ;# return the string\n"
             "        }\n"
             "    } else {\n"
             "        set var [gemtcl_gets]\n"
             "    }\n"
             "}\n"

             "if {[info command tcl::flush] eq \"\"} {rename flush tcl::flush}\n"
             "proc flush chan {\n"
             "    if {$chan ne \"stdout\"} {\n"
             "        tcl::flush $chan\n"
             "    }\n"
             "}\n"
             );

#ifdef WIN32
    slip tcl_lib = Tcl_GetNameOfExecutable();
    tcl_lib.s("/[^\\/]*$","/lib");
    Tcl_Eval(interp, slipprintf("set tcl_library \"%s\"; source $tcl_library/init.tcl", tcl_lib.c_str()).c_str());

    Tcl_Eval(interp, "proc dir args { exec cmd /c dir $args }");
#endif

    // Now sleep until commands are given to this thread
    while(1) {
        is_waiting_for_eval = true;
#ifdef WIN32
        WaitForSingleObject(tcl_eval_mutex, INFINITE );
        //        SetEvent(tcl_reply_mutex);
#else
        g_mutex_lock(tcl_eval_mutex);
        // g_mutex_unlock(tcl_reply_mutex);
#endif
        // The following assignment, stops recursive evaluation of
        // gemtcl_serv.
        string s = tcl_eval_string;
        tcl_eval_string = "";
        is_waiting_for_eval = false;
        Tcl_Eval(interp, s.c_str());
        const char *tcl_eval_result = Tcl_GetStringResult(interp);
#if 0
        printf("%s => %s.\n",
               tcl_eval_string.c_str(),
               tcl_eval_result
               );
#endif

        g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                        cb_idle_add_eval_end,
                        g_strdup(tcl_eval_result),
                        g_free);
#if 0
#ifdef WIN32
        printf("waiting for tcl_reply_mutex.\n");
        WaitForSingleObject(tcl_reply_mutex, INFINITE );
        printf("Done waiting.\n");
#else
        g_mutex_unlock(tcl_reply_mutex);
#endif
#endif
    }
}

static void
cb_history_window_response(GtkWidget *dialog,
                           gint response,
                           gpointer user_data)
{
    w_history_window = NULL;
    gtk_widget_destroy(dialog);
}

static gboolean
cb_history_selection_func(GtkTreeSelection *selection,
                           GtkTreeModel *model,
                           GtkTreePath *path,
                           gboolean path_currently_selected,
                           gpointer data)
{
    if (!path_currently_selected) {
        gchar *spath = gtk_tree_path_to_string(path);
        history_set(history.size()-1-atoi(spath));
        g_free(spath);
    }
    return TRUE;
}
                              
// An idea is not to make this a separate window but instead
// change the main window to display the history when we are in that mode.
// Somehow like collapsing all the output and just showing the input.
static void cb_menu_tools_history(gpointer   callback_data,
                                  guint      callback_action,
                                  GtkWidget *widget)
{
    if (w_history_window)
        return;
    w_history_window = gtk_dialog_new();

    // Store the history in a list
    GtkListStore *store = gtk_list_store_new (1, G_TYPE_STRING);


    // A scrolled window.
    GtkWidget *w_scrolled_win = gtk_scrolled_window_new(NULL,NULL);

    // Create the text tree viewer
    GtkWidget *tree;
    tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    gtk_widget_set_size_request(tree, 550,400);
    gtk_container_add(GTK_CONTAINER(w_scrolled_win), tree);
    // I have to write my own search that searches backwards. :-(
    // gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tree), FALSE);

    /* Create a text column */
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    column = gtk_tree_view_column_new();
    cell = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(cell),
                 "wrap-width", 550,
                 "font", "Monospace",
                 NULL);

    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_attributes (column, cell,
                                         "text", 0,
                                         NULL);
    gtk_tree_view_column_set_title (column, "Commands");
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(w_history_window)->vbox), w_scrolled_win,
                       true, true, 0);
#if 0
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(w_history_window)->vbox), gtk_label_new("Testing"),
                       true, true, 0);
#endif

    gtk_window_set_transient_for(GTK_WINDOW(w_history_window),
                                 GTK_WINDOW(w_window));
    gtk_dialog_add_button (GTK_DIALOG (w_history_window),
                           GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
    g_signal_connect(G_OBJECT (w_history_window), "response",
                     G_CALLBACK(cb_history_window_response), NULL);
    gtk_window_set_title(GTK_WINDOW (w_history_window), "gemtcl history");


    gtk_widget_show_all (w_history_window);

    // Test putting something in the store
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_select_function(selection,
                                           cb_history_selection_func,
                                           NULL,
                                           NULL);
    for (int i=0; i<(int)history.size(); i++) {
        GtkTreeIter   iter;
        gtk_list_store_append (store, &iter);  /* Acquire an iterator */
        gtk_list_store_set (store, &iter, 0, history[history.size()-1-i].c_str(), -1);
        if (i==0)
            gtk_tree_selection_select_iter(selection, &iter);
    }

#if 0
    // Enable this if the history is stored with oldest on top.
    if (history.size()) {
        gtk_tree_selection_select_iter(selection, &last_iter);
        GtkTreePath *p = gtk_tree_path_new_from_indices(history.size()-1,-1);
        gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(tree), p, NULL, FALSE, 0,0);
        gtk_tree_path_free(p);
    }
#endif
}

static void
cb_menu_about(gpointer   callback_data,
	      guint      callback_action,
	      GtkWidget *widget)
{
    GtkWidget *about_window;
    GtkWidget *vbox;
    GtkWidget *label, *w_image;
    GdkPixbuf *icon;
    gchar *markup;
    GError *err = NULL;
  
    about_window = gtk_dialog_new ();
    gtk_window_set_transient_for(GTK_WINDOW(about_window),
                                 GTK_WINDOW(w_window));
    gtk_dialog_add_button (GTK_DIALOG (about_window),
                           GTK_STOCK_OK, GTK_RESPONSE_OK);
    gtk_dialog_set_default_response (GTK_DIALOG (about_window),
                                     GTK_RESPONSE_OK);
  
    gtk_window_set_title(GTK_WINDOW (about_window), "gemtcl");
  
    gtk_window_set_resizable (GTK_WINDOW (about_window), FALSE);

#if 0
    g_signal_connect (G_OBJECT (about_window), "delete-event",
                      G_CALLBACK (gtk_widget_destroy), about_window);
#endif
  
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (about_window)->vbox), vbox, FALSE, FALSE, 0);

    icon = gdk_pixbuf_new_from_inline(sizeof(logo_150_inline),
                                      logo_150_inline,
                                      FALSE,
                                      &err);
    w_image = gtk_image_new_from_pixbuf(icon);

    gtk_box_pack_start (GTK_BOX (vbox), w_image, FALSE, FALSE, 0);

    label = gtk_label_new (NULL);
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
    markup = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">GemTcl "VERSION"</span>\n\n"
                              "%s\n\n"
                              "<span>%s\n%sEmail: <tt>&lt;%s&gt;</tt></span>\n",
                              ("An interactive Tcl-Shell"),
                              ("Copyright &#x00a9; Dov Grobgeld, 2008\n"),
                              ("Programming by: Dov Grobgeld\n"),
                              ("dov.grobgeld@gmail.com"));
    gtk_label_set_markup (GTK_LABEL (label), markup);
    g_free (markup);
    gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  
    gtk_widget_show_all (about_window);
    gtk_dialog_run (GTK_DIALOG (about_window));
    gtk_widget_destroy (about_window);
}

static void
cb_menu_help_changelog(gpointer   callback_data,
                       guint      callback_action,
                       GtkWidget *widget)
{
    GtkWidget *changelog_window;
    GtkWidget *scrolledwin, *w_text;
    GtkTextBuffer *text_buffer;
  
    changelog_window = gtk_dialog_new ();
    gtk_dialog_add_button (GTK_DIALOG (changelog_window),
                           GTK_STOCK_CLOSE, GTK_RESPONSE_OK);
    gtk_dialog_set_default_response (GTK_DIALOG (changelog_window),
                                     GTK_RESPONSE_OK);
  
    gtk_window_set_title(GTK_WINDOW (changelog_window), "salfw-viewer changelog");
  
    gtk_window_set_resizable (GTK_WINDOW (changelog_window), TRUE);
    gtk_window_set_policy(GTK_WINDOW(changelog_window), TRUE, TRUE, TRUE);

    g_signal_connect (G_OBJECT (changelog_window), "delete-event",
                      G_CALLBACK (gtk_widget_destroy), NULL);

    scrolledwin = gtk_scrolled_window_new (NULL, NULL);
  
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (changelog_window)->vbox), scrolledwin, TRUE, TRUE, 0);
    w_text = gtk_text_view_new ();
    gtk_text_view_set_editable      (GTK_TEXT_VIEW(w_text), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(w_text), FALSE);
    gtk_widget_set_size_request (w_text, 800, 500);
    gtk_container_add (GTK_CONTAINER (scrolledwin), w_text);
    text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w_text));
    gtk_widget_modify_font(w_text, pango_font_description_from_string("Monospace"));
    gtk_text_buffer_set_text (text_buffer, changelog_string, -1);
    
    gtk_widget_show_all (changelog_window);
    gtk_dialog_run (GTK_DIALOG (changelog_window));
    gtk_widget_destroy (changelog_window);
}

static void
cb_menu_quit(gpointer   callback_data,
	     guint      callback_action,
	     GtkWidget *widget)
{
    gtk_main_quit();
}

int load_gtf_file(const gchar *filename)
{
    GtkWidget *button_box = gem_button_box_new_from_file(filename,
                                                         command_tcl_eval_add_newline);
    GtkWidget *w_empty_vbox = gtk_vbox_new(FALSE, 0);

    if (!button_box) {
        //app_error("Failed loading button box from \"%s\"\n", filename);
        return -1;
    }

    gtk_widget_show(button_box);

    int page_idx = gtk_notebook_append_page(GTK_NOTEBOOK(w_notebook),
                                            w_empty_vbox,
                                            gtk_label_new(gem_button_box_get_name(GEM_BUTTON_BOX(button_box)))
                                            );
    g_object_ref(button_box);
    g_object_set_data(G_OBJECT(w_empty_vbox),
                      "button_box",
                      button_box);
    gtk_widget_show(w_empty_vbox);

    if (w_virtual_notebook_page)
        gtk_container_remove(GTK_CONTAINER(w_button_box_vbox),
                             w_virtual_notebook_page);
    w_virtual_notebook_page = button_box;
    gtk_box_pack_start(GTK_BOX(w_button_box_vbox),
                       button_box,
                       FALSE, FALSE, 0);
    gtk_notebook_set_current_page (GTK_NOTEBOOK(w_notebook),
                                   page_idx);
    
    g_signal_connect(button_box, "name-changed",
                     G_CALLBACK(cb_button_box_name_changed),
                     NULL);

    return 0;
}

static void
cb_file_chooser_open_response(GtkWidget *dialog,
                              gint response,
                              gpointer user_data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER( dialog));
        last_image_path = filename;
        load_gtf_file(filename);
        
    }
     
    gtk_widget_destroy(dialog);
}

static void
cb_menu_open(gpointer   callback_data,
	     guint      callback_action,
	     GtkWidget *widget)
{
    GtkWidget *file_chooser = gtk_file_chooser_dialog_new ("Open gemtcl file",
                                                           GTK_WINDOW (w_window),
                                                           GTK_FILE_CHOOSER_ACTION_OPEN,
                                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                           GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                           NULL);

    gtk_window_set_transient_for (GTK_WINDOW (file_chooser),
                                  GTK_WINDOW (w_window));
    gtk_dialog_set_default_response ( GTK_DIALOG(file_chooser),
                                      GTK_RESPONSE_ACCEPT);

    g_signal_connect(G_OBJECT (file_chooser), "response",
                     G_CALLBACK(cb_file_chooser_open_response), NULL);

    if (last_image_path.length())
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser),
                                       last_image_path.c_str());

    gtk_widget_show (file_chooser);
}

static void
cb_file_chooser_save_response(GtkWidget *dialog,
                              gint response,
                              gpointer user_data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER( dialog));
        int page_idx = gtk_notebook_get_current_page(GTK_NOTEBOOK(w_notebook));
        GtkWidget *current_bb = GTK_WIDGET(g_object_get_data(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(w_notebook), page_idx)), "button_box"));

        last_image_path = filename;

        if (page_idx >= 0 && current_bb) {
            gem_button_box_save_to_file(GEM_BUTTON_BOX(current_bb),
                                        filename);
        }
    }
    
    gtk_widget_destroy(dialog);
}

static void
cb_menu_save(gpointer   callback_data,
             guint      callback_action,
             GtkWidget *widget)
{
    int page_idx = gtk_notebook_get_current_page(GTK_NOTEBOOK(w_notebook));

    if (page_idx < 0)
        return;
    
    GtkWidget *current_bb = GTK_WIDGET(g_object_get_data(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(w_notebook), page_idx)), "button_box"));

    if (GEM_BUTTON_BOX(current_bb)->filename)
        gem_button_box_save_to_file(GEM_BUTTON_BOX(current_bb), NULL);
    else 
        cb_menu_save_as(callback_data,
                        callback_action,
                        widget);
}

static void
cb_menu_save_as(gpointer   callback_data,
                guint      callback_action,
                GtkWidget *widget)
{
    GtkWidget *file_chooser = gtk_file_chooser_dialog_new ("Save gemtcl file",
                                                           GTK_WINDOW (w_window),
                                                           GTK_FILE_CHOOSER_ACTION_SAVE,
                                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                           GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                           NULL);

    if (last_image_path.length())
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser),
                                       last_image_path.c_str());

    gtk_window_set_transient_for (GTK_WINDOW (file_chooser),
                                  GTK_WINDOW (w_window));

    g_signal_connect(G_OBJECT (file_chooser), "response",
                     G_CALLBACK(cb_file_chooser_save_response), NULL);

    gtk_widget_show (file_chooser);
}

static void
cb_menu_file_close(gpointer   callback_data,
                    guint      callback_action,
                    GtkWidget *widget)
{
    int page_idx = gtk_notebook_get_current_page(GTK_NOTEBOOK(w_notebook));
    GtkWidget *current_bb = GTK_WIDGET(g_object_get_data(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(w_notebook), page_idx)), "button_box"));
    if (page_idx >= 0 && current_bb) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(w_window),
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_QUESTION,
                                                   GTK_BUTTONS_NONE,
                                                   "Buttonbox not saved. Save?");
        gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                               GTK_STOCK_YES,
                               GTK_RESPONSE_YES,
                               GTK_STOCK_NO,
                               GTK_RESPONSE_NO,
                               GTK_STOCK_CANCEL,
                               GTK_RESPONSE_CANCEL,
                               NULL);
        int response = gtk_dialog_run(GTK_DIALOG(dialog));
        
        if (response == GTK_RESPONSE_YES) {
            cb_menu_save(callback_data, callback_action, widget);
        }
        gtk_widget_unref(current_bb);
        gtk_notebook_remove_page(GTK_NOTEBOOK(w_notebook), page_idx);
        gtk_widget_destroy(dialog);
    }
}
    
static void
cb_menu_edit_button_box(gpointer   callback_data,
                        guint      callback_action,
                        GtkWidget *widget)
{
    int page_idx = gtk_notebook_get_current_page(GTK_NOTEBOOK(w_notebook));
    GtkWidget *current_bb = GTK_WIDGET(g_object_get_data(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(w_notebook), page_idx)), "button_box"));
    if (page_idx >= 0 && current_bb) {
        GtkWidget *dialog = gem_button_box_editor_new(GEM_BUTTON_BOX(current_bb));
        
        gtk_widget_show_all(dialog);
    }
}
    
static void cb_menu_button_box_new(gpointer   callback_data,
                                   guint      callback_action,
                                   GtkWidget *widget)
{
    GtkWidget *button_box = gem_button_box_new(command_tcl_eval_add_newline);
    GtkWidget *w_empty_vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(button_box);
    int page_idx = gtk_notebook_append_page(GTK_NOTEBOOK(w_notebook),
                                            w_empty_vbox,
                                            gtk_label_new(gem_button_box_get_name(GEM_BUTTON_BOX(button_box)))
                                            );
    g_object_ref(button_box);
    g_object_set_data(G_OBJECT(w_empty_vbox),
                      "button_box",
                      button_box);
    gtk_widget_show(w_empty_vbox);

    if (w_virtual_notebook_page)
        gtk_container_remove(GTK_CONTAINER(w_button_box_vbox),
                             w_virtual_notebook_page);
    w_virtual_notebook_page = button_box;
    gtk_box_pack_start(GTK_BOX(w_button_box_vbox),
                       button_box,
                       FALSE, FALSE, 0);
    gtk_notebook_set_current_page (GTK_NOTEBOOK(w_notebook),
                                   page_idx);

    g_signal_connect(button_box, "name-changed",
                     G_CALLBACK(cb_button_box_name_changed),
                     NULL);
}

static void cb_toggle_view_button_box(gpointer   callback_data,
                                      guint      callback_action,
                                      GtkWidget *widget)

{
    if (!w_button_box_vbox)
        return;
    bool do_show_buttonbox = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (gtk_item_factory_get_item (item_factory, "/View/ButtonBox")));

    if (do_show_buttonbox) 
        gtk_widget_show(w_button_box_vbox);
    else
        gtk_widget_hide(w_button_box_vbox);
}

static void cb_menu_tools_xmlrpc(gpointer   callback_data,
                                 guint      callback_action,
                                 GtkWidget *widget)

{
    bool do_xmlrpc = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (gtk_item_factory_get_item (item_factory, "/Tools/XmlRpc")));
    if (do_xmlrpc)
        create_xmlrpc(-1);
    else
        close_xmlrpc();
}

void cb_button_box_name_changed(GtkWidget *widget,
                                const gchar *new_name)
{
    // Search for the tab with the corresponding vbox
    gint n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(w_notebook));
    for (int i=0; i<n_pages; i++) {
        GtkWidget *page_vbox
            = gtk_notebook_get_nth_page(GTK_NOTEBOOK(w_notebook), i);
        GtkWidget *bbw = GTK_WIDGET(
            g_object_get_data(G_OBJECT(page_vbox), "button_box"));
        if (bbw == widget) {
            gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(w_notebook),
                                            page_vbox,
                                            new_name);
            break;
        }
    }
}

/**
 * Add a newline to the buffer to start command output on a fresh
 * line and then eval the command. This command is used in all
 * the button press eval callbacks.
 */
static int command_tcl_eval_add_newline(const gchar *cmd)
{
    if (cmd && strlen(cmd))
        add_to_end_of_buffer(false,
                             false,
                             false,
                             "\n");

    command_tcl_eval(cmd);
    return 0;
}

static int command_tcl_eval(const gchar *cmd)
{
    // Clear log of old command. But not if we are doing gemtcl commands!
    command_log_message_do_clear = true;
    
    // Return focus to text_view
    gtk_widget_grab_focus(w_text_view);

    // If this command was used for focusing only, then return.
#if 0
    if (!cmd || strlen(cmd)==0)
        return 0;
#endif
    if (doing_readline) {
        gets_string = cmd;
        if (g_main_loop_is_running (loop))
            g_main_loop_quit (loop);
    }
    else {
        tcl_eval_string = cmd;
        if (is_waiting_for_eval) {
#ifdef WIN32
            SetEvent(tcl_eval_mutex);
#else            
            g_mutex_unlock(tcl_eval_mutex);
#endif
        }
    }
    return 0;
}

static void set_tcl_library_from_executable()
{
#if WIN32
    slip tcl_lib = Tcl_GetNameOfExecutable();
    tcl_lib.s("/[^\\/]*$","/lib");
    Tcl_Eval(interp, slipprintf("set tcl_library %s; source $tcl_library/init.tcl", tcl_lib.c_str()).c_str());
#endif
}

static void cb_switched_page (GtkNotebook     *notebook,
                              GtkNotebookPage *page,
                              guint            page_num,
                              gpointer         user_data)
{
#if 0
    GtkWidget *w_paned = GTK_WIDGET(user_data);
    GtkWidget *w_page = gtk_notebook_get_nth_page(notebook, page_num);
    GtkWidget *tab_label = gtk_notebook_get_tab_label(notebook, w_page);
    GtkRequisition req, req_label;
    gtk_widget_size_request(w_page,
                            &req);
    gtk_widget_size_request(tab_label,
                            &req_label);
    int min_height = req.height+req_label.height+10;
    g_object_set_data(G_OBJECT(w_paned), "min_height", GINT_TO_POINTER(min_height));
#if 0
    gtk_paned_set_position(GTK_PANED(w_paned),
                           min_height);
    // Sorry. Doesn't work. Will have to figure out how to do this manually...
    g_object_set(G_OBJECT(w_paned),
                 "min-position", min_height,
                 NULL);
#endif
#endif
    GtkWidget *w_page = gtk_notebook_get_nth_page(notebook, page_num);
    GtkWidget *button_box = GTK_WIDGET(g_object_get_data(G_OBJECT(w_page), "button_box"));

    if (!button_box)
        return;
    if (w_virtual_notebook_page)
        gtk_container_remove(GTK_CONTAINER(w_button_box_vbox),
                             w_virtual_notebook_page);
    w_virtual_notebook_page = button_box;
    gtk_box_pack_start(GTK_BOX(w_button_box_vbox),
                       button_box,
                       FALSE, FALSE, 0);
}

int xmlrpc_cmd_ping(GNetXmlRpcServer *server,
                    const gchar *command,
                    const gchar *param,
                    gpointer user_data,
                    // output
                    gchar **reply_string)
{
    *reply_string = g_strdup_printf("Pong");

    return 0;
}

int xmlrpc_cmd_append_to_log(GNetXmlRpcServer *server,
                             const gchar *command,
                             const gchar *param,
                             gpointer user_data,
                             // output
                             gchar **reply_string)
{
    slip msg(param);
    msg.chomp();
    msg += "\n";
    if (command_log_message_do_clear) {
        command_log_message = "";
        command_log_message_do_clear = false;
    }
    command_log_message += msg;
    add_to_end_of_buffer(FALSE, FALSE, TRUE, msg);
    *reply_string = g_strdup_printf("Ok");

    return 0;
}

int xmlrpc_cmd_tcl_eval(GNetXmlRpcServer *server,
                        GConn *gnet_client,
                        const gchar *command,
                        const gchar *param,
                        gpointer user_data)
{
    add_to_end_of_buffer(false, false, true,
                         slipprintf(">>> %s\n", param));
    command_tcl_eval(param);
    // TBD wait for command complete

    xmlrpc_handle = gnet_client;

    return 0;
}

static int xmlrpc_cmd_tcl_eval_finish(const gchar *reply)
{
    gnet_xmlrpc_async_client_response(xmlrpc_handle, reply);
    return 0;
}

static int read_config()
{
    
}

int main(int argc, char **argv)
{
    GThread *thread_tcl;
    GError *err = NULL;
    int argp = 1;
    slip init_command = "";

    Tcl_FindExecutable(argv[0]);
    g_thread_init(NULL);
    gdk_threads_init();
    gtk_init(&argc, &argv);
    gdk_threads_enter ();

    settings_file = slip(g_get_home_dir())+"/"+".gemtcl.rc";
    load_settings();

    while(argp < argc && argv[argp][0]=='-') {
        slip S_ = argv[argp++];

        CASE("-help") {
            printf("gemtcl - a tcl interpreter\n"
                   "\n"
                   "Syntax:\n"
                   "    gemtcl [-cmd cmd]\n"
                   );
            exit(0);
        }
        CASE("-cmd") {
            init_command = argv[argp++];
            continue;
        }

        die("Unknown option %s!\n", S_.c_str());
    }

    read_config();
    create_widgets();

#ifdef WIN32
    //    tcl_eval_mutex = CreateMutex( NULL, FALSE, NULL );
    tcl_eval_mutex = CreateEvent (0, FALSE, TRUE, 0);
    tcl_reply_mutex = CreateEvent (0, FALSE, FALSE, 0);
    SetEvent(tcl_reply_mutex);
    WaitForSingleObject(tcl_reply_mutex, INFINITE );
#else
    tcl_eval_mutex = g_mutex_new();
    tcl_reply_mutex = g_mutex_new();
#endif
    
    if ((thread_tcl = g_thread_create((GThreadFunc)thread_init_tcl, NULL, TRUE, &err)) == NULL) {
        printf("Thread create failed: %s!!\n", err->message );
        g_error_free ( err );
    }

    add_to_end_of_buffer(false, false, true,
                         slipprintf("Welcome to gemtcl version %s\n", VERSION));

    if (argp < argc) {
        slip filename = argv[argp++];
        if (filename.m("\\.gtf"))
            load_gtf_file(filename);
        else
            command_tcl_eval(slipprintf("source %s", filename.c_str()));
    }
    if (init_command.length()) {
        add_to_end_of_buffer(false, false, true,
                             slipprintf(">>> %s\n", init_command.c_str()));
        command_tcl_eval(init_command);
    }

    // set_tcl_library_from_executable();

    gtk_main();
    gdk_threads_leave();
    
    exit(0);
}

#ifdef _MSC_VER
int __stdcall WinMain(      

    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    char **argv;
    GError *error = NULL;
    int argc = 0;
    sups lp = (sups) "gemtcl " + lpCmdLine;

    g_shell_parse_argv(lp.c_str(),
                       &argc,
                       &argv,
                       &error);

    int ret = main(argc, argv);

    g_strfreev(argv);

    return ret;
}
#endif
