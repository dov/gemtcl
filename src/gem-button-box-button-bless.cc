//======================================================================
//  gem-button-box-button-bless.cpp
//
//  Blessing a normal button to
//  become a gem-button-box-button.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed May 23 23:35:58 2007
//----------------------------------------------------------------------

#include <gtk/gtk.h>
#include "gem.h"
#include "gem-button-box-button-bless-editor.h"
#include "gem-button-box-button-bless.h"


enum {
    RESPONSE_RUN,
    RESPONSE_NEW,
    RESPONSE_SAVE
};


// Forward declarations to break cyclic dependancies...
extern "C" {
struct GemButtonBox;
void 	gem_button_box_delete_button	(GemButtonBox* self,
                                         GtkWidget * bbb_to_delete);
 
}

static void
cb_button_menu_activate_edit(GtkWidget *menuitem,
                             gpointer user_data);
static void
cb_button_menu_activate_delete(GtkWidget *menuitem,
                               gpointer user_data);

static void
cb_button_clicked(GtkWidget *button);
static int
cb_button_press_event(GtkWidget *button,
                      GdkEventButton *event);
                  
void gem_button_box_button_bless(GtkWidget *button,
                                 const char *label,
                                 const gchar *script,
                                 GtkWidget   *button_box,
                                 GemCmdEval  *cmd_eval,
                                 bool label_is_editable)
{
    g_object_set_data_full(G_OBJECT(button),
                           "script",
                           g_strdup(script),
                           g_free);
    g_object_set_data_full(G_OBJECT(button),
                           "label",
                           g_strdup(label),
                           g_free);
    g_object_set_data     (G_OBJECT(button),
                           "button_box",
                           button_box);
    g_object_set_data     (G_OBJECT(button),
                           "cmd_eval",
                           (gpointer)cmd_eval);
    g_object_set_data     (G_OBJECT(button),
                           "is_editable",
                           GINT_TO_POINTER(label_is_editable));

    // Create a menu and tie it to the button
    GtkWidget *menu_item;
    int row = 0;
    GtkWidget *menu;

    menu = gtk_menu_new();
    g_object_set_data(G_OBJECT(button),
                      "menu",
                      menu);
    
    menu_item =gtk_menu_item_new_with_label("Edit");
    g_signal_connect(G_OBJECT(menu_item), "activate",
                     G_CALLBACK(cb_button_menu_activate_edit),
                     button);
    gtk_menu_attach(GTK_MENU(menu),
                    menu_item,
                    0,1,
                    row,row+1);
    row++;
    
    menu_item =gtk_menu_item_new_with_label("Delete");
    g_signal_connect(G_OBJECT(menu_item), "activate",
                     G_CALLBACK(cb_button_menu_activate_delete),
                     button);
    gtk_menu_attach(GTK_MENU(menu),
                    menu_item,
                    0,1,
                    row,row+1);
    row++;
    
    menu_item =gtk_menu_item_new_with_label("Move back");
    gtk_menu_attach(GTK_MENU(menu),
                    menu_item,
                    0,1,
                    row,row+1);
    row++;
    
    menu_item =gtk_menu_item_new_with_label("Move forward");
    gtk_menu_attach(GTK_MENU(menu),
                    menu_item,
                    0,1,
                    row,row+1);
    row++;

    if (GTK_IS_BUTTON(button)) 
        // Connect the click handler
        g_signal_connect(button, "clicked",
                         G_CALLBACK(cb_button_clicked),
                         NULL);
    else if (GTK_IS_RANGE(button))
        g_signal_connect(button, "value-changed",
                         G_CALLBACK(cb_button_clicked),
                         NULL);
    g_signal_connect(button, "button-press-event",
                     G_CALLBACK(cb_button_press_event),
                     NULL);

    
}
                                 
const gchar *
gem_button_box_button_bless_get_script(GtkWidget *button)
{
    return (const gchar *)g_object_get_data(G_OBJECT(button), "script");
}

const gchar *
gem_button_box_button_bless_get_label(GtkWidget *button)
{
    return (const gchar *)g_object_get_data(G_OBJECT(button), "label");
}

bool
gem_button_box_button_bless_get_label_is_editible(GtkWidget *button)
{
    return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "is_editable"));
}

void
gem_button_box_button_bless_cmd_eval(GtkWidget *button, const char *cmd)
{
    GemCmdEval *cmd_eval = (GemCmdEval*)g_object_get_data(G_OBJECT(button),
                                                          "cmd_eval");
    // Prevent crash - though this shouldn't happen!
    if (!cmd_eval)
        return;

    if (cmd == NULL) {
        const gchar *script = gem_button_box_button_bless_get_script(button);
        cmd_eval(script);
    }
    else
        cmd_eval(cmd);
}


void
gem_button_box_button_bless_set_script(GtkWidget *button, const gchar *script)
{
    g_object_set_data_full(G_OBJECT(button),
                           "script",
                           g_strdup(script),
                           g_free);
}

void
gem_button_box_button_bless_set_label(GtkWidget *button, const gchar *label)
{
    g_object_set_data_full(G_OBJECT(button),
                           "label",
                           g_strdup(label),
                           g_free);
    gtk_button_set_label(GTK_BUTTON(button),
                         label);
}

static void
cb_button_menu_activate_edit(GtkWidget *menuitem,
                             gpointer user_data)
{
    GtkWidget *dialog_edit
        = gem_button_box_button_bless_editor_new(GTK_WIDGET(user_data));

    gtk_widget_show_all(dialog_edit);
}

static void
cb_button_menu_activate_delete(GtkWidget *menuitem,
                               gpointer user_data)
{
    GtkWidget *button = GTK_WIDGET(user_data);
    GemButtonBox *button_box = (GemButtonBox*)g_object_get_data(G_OBJECT(button),
                                                                "button_box");

    gem_button_box_delete_button(button_box,
                                 button);
}

static void
cb_button_clicked(GtkWidget *button)
{
    gem_button_box_button_bless_cmd_eval(GTK_WIDGET(button), NULL);
}    
                  
static
int cb_button_press_event (GtkWidget *button,
                           GdkEventButton *event)
{
    GtkWidget *menu = (GtkWidget*)g_object_get_data(G_OBJECT(button),
                                                    "menu");

    if (event->button == 3) {
        gtk_widget_grab_focus(button);
        gtk_widget_show_all(menu);
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                       event->button, event->time);
        
        gem_button_box_button_bless_cmd_eval(GTK_WIDGET(button), "");
        return TRUE;
    }

    return FALSE;
}
