/*======================================================================
//  gem-button-box-button-bless.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Thu May 24 00:20:25 2007
//----------------------------------------------------------------------
*/
#ifndef GEM_BUTTON_BOX_BUTTON_BLESS_H
#define GEM_BUTTON_BOX_BUTTON_BLESS_H

#include <gtk/gtk.h>
#include "gem.h"

extern "C" {

void gem_button_box_button_bless(GtkWidget *button,
                                 const char *label,
                                 const gchar *script,
                                 GtkWidget   *button_box,
                                 GemCmdEval  *cmd_eval,
                                 bool label_is_editable);

const gchar *
gem_button_box_button_bless_get_script(GtkWidget *button);

const gchar *
gem_button_box_button_bless_get_label(GtkWidget *button);

void
gem_button_box_button_bless_cmd_eval(GtkWidget *button, const char *cmd);

void
gem_button_box_button_bless_set_script(GtkWidget *button, const gchar *script);

void
gem_button_box_button_bless_set_label(GtkWidget *button, const gchar *script);

bool
gem_button_box_button_bless_get_label_is_editible(GtkWidget *button);

}

#endif /* GEM-BUTTON-BOX-BUTTON-BLESS */
