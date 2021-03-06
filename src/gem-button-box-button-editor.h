/* Generated by GOB (v2.0.12)   (do not edit directly) */

#include <glib.h>
#include <glib-object.h>


#include <math.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagesmanager.h>
#include "gem.h"

#ifndef __GEM_BUTTON_BOX_BUTTON_EDITOR_H__
#define __GEM_BUTTON_BOX_BUTTON_EDITOR_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 * Type checking and casting macros
 */
#define GEM_TYPE_BUTTON_BOX_BUTTON_EDITOR	(gem_button_box_button_editor_get_type())
#define GEM_BUTTON_BOX_BUTTON_EDITOR(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), gem_button_box_button_editor_get_type(), GemButtonBoxButtonEditor)
#define GEM_BUTTON_BOX_BUTTON_EDITOR_CONST(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), gem_button_box_button_editor_get_type(), GemButtonBoxButtonEditor const)
#define GEM_BUTTON_BOX_BUTTON_EDITOR_CLASS(klass)	G_TYPE_CHECK_CLASS_CAST((klass), gem_button_box_button_editor_get_type(), GemButtonBoxButtonEditorClass)
#define GEM_IS_BUTTON_BOX_BUTTON_EDITOR(obj)	G_TYPE_CHECK_INSTANCE_TYPE((obj), gem_button_box_button_editor_get_type ())

#define GEM_BUTTON_BOX_BUTTON_EDITOR_GET_CLASS(obj)	G_TYPE_INSTANCE_GET_CLASS((obj), gem_button_box_button_editor_get_type(), GemButtonBoxButtonEditorClass)

/* Private structure type */
typedef struct _GemButtonBoxButtonEditorPrivate GemButtonBoxButtonEditorPrivate;

/*
 * Main object structure
 */
#ifndef __TYPEDEF_GEM_BUTTON_BOX_BUTTON_EDITOR__
#define __TYPEDEF_GEM_BUTTON_BOX_BUTTON_EDITOR__
typedef struct _GemButtonBoxButtonEditor GemButtonBoxButtonEditor;
#endif
struct _GemButtonBoxButtonEditor {
	GtkDialog __parent__;
	/*< private >*/
	GemButtonBoxButtonEditorPrivate *_priv;
};

/*
 * Class definition
 */
typedef struct _GemButtonBoxButtonEditorClass GemButtonBoxButtonEditorClass;
struct _GemButtonBoxButtonEditorClass {
	GtkDialogClass __parent__;
};


/*
 * Public methods
 */
GType	gem_button_box_button_editor_get_type	(void);
GtkWidget * 	gem_button_box_button_editor_new	(GtkWidget * bbb);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
