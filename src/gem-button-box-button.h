/* Generated by GOB (v2.0.12)   (do not edit directly) */

#include <glib.h>
#include <glib-object.h>


#include <math.h>
#include <gtk/gtk.h>
#include "gem.h"
#include "gem-button-box-button-editor.h"


#ifndef __GEM_BUTTON_BOX_BUTTON_H__
#define __GEM_BUTTON_BOX_BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 * Type checking and casting macros
 */
#define GEM_TYPE_BUTTON_BOX_BUTTON	(gem_button_box_button_get_type())
#define GEM_BUTTON_BOX_BUTTON(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), gem_button_box_button_get_type(), GemButtonBoxButton)
#define GEM_BUTTON_BOX_BUTTON_CONST(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), gem_button_box_button_get_type(), GemButtonBoxButton const)
#define GEM_BUTTON_BOX_BUTTON_CLASS(klass)	G_TYPE_CHECK_CLASS_CAST((klass), gem_button_box_button_get_type(), GemButtonBoxButtonClass)
#define GEM_IS_BUTTON_BOX_BUTTON(obj)	G_TYPE_CHECK_INSTANCE_TYPE((obj), gem_button_box_button_get_type ())

#define GEM_BUTTON_BOX_BUTTON_GET_CLASS(obj)	G_TYPE_INSTANCE_GET_CLASS((obj), gem_button_box_button_get_type(), GemButtonBoxButtonClass)

/* Private structure type */
typedef struct _GemButtonBoxButtonPrivate GemButtonBoxButtonPrivate;

/*
 * Main object structure
 */
#ifndef __TYPEDEF_GEM_BUTTON_BOX_BUTTON__
#define __TYPEDEF_GEM_BUTTON_BOX_BUTTON__
typedef struct _GemButtonBoxButton GemButtonBoxButton;
#endif
struct _GemButtonBoxButton {
	GtkButton __parent__;
	/*< private >*/
	GemButtonBoxButtonPrivate *_priv;
};

/*
 * Class definition
 */
typedef struct _GemButtonBoxButtonClass GemButtonBoxButtonClass;
struct _GemButtonBoxButtonClass {
	GtkButtonClass __parent__;
};


/*
 * Public methods
 */
GType	gem_button_box_button_get_type	(void);
GtkWidget * 	gem_button_box_button_new	(const gchar * label,
					const gchar * script,
					GtkWidget * button_box,
					GemCmdEval * cmd_eval);
const gchar * 	gem_button_box_button_get_script	(GemButtonBoxButton * self);
void 	gem_button_box_button_set_script	(GemButtonBoxButton * self,
					const gchar * script);
const gchar * 	gem_button_box_button_get_label	(GemButtonBoxButton * self);
void 	gem_button_box_button_set_label	(GemButtonBoxButton * self,
					const gchar * label);
void 	gem_button_box_button_cmd_eval	(GemButtonBoxButton * self,
					const char * cmd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
