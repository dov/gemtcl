/* Generated by GOB (v2.0.12) on Thu May 24 00:12:10 2007
   (do not edit directly) */

/* End world hunger, donate to the World Food Programme, http://www.wfp.org */

#define GOB_VERSION_MAJOR 2
#define GOB_VERSION_MINOR 0
#define GOB_VERSION_PATCHLEVEL 12

#define selfp (self->_priv)

#include <string.h> /* memset() */

#include "gem-button-box-button-editor.h"

#include "gem-button-box-button-editor-private.h"

#ifdef G_LIKELY
#define ___GOB_LIKELY(expr) G_LIKELY(expr)
#define ___GOB_UNLIKELY(expr) G_UNLIKELY(expr)
#else /* ! G_LIKELY */
#define ___GOB_LIKELY(expr) (expr)
#define ___GOB_UNLIKELY(expr) (expr)
#endif /* G_LIKELY */

#line 17 "src/gem-button-box-button-bless-editor.gob"

#define DO_HOMOGENOUS TRUE
#define DO_NOT_HOMOGENOUS FALSE
#define DO_EXPAND TRUE
#define DO_NOT_EXPAND FALSE
#define DO_FILL TRUE
#define DO_NOT_FILL FALSE

/* Define some macros defining all space distance used in programs */
#define SPACE_ZERO 0
#define PADDING_ZERO 0

enum {
    RESPONSE_RUN,
    RESPONSE_NEW,
    RESPONSE_SAVE
};


#line 47 "gem-button-box-button-editor.cc"
/* self casting macros */
#define SELF(x) GEM_BUTTON_BOX_BUTTON_EDITOR(x)
#define SELF_CONST(x) GEM_BUTTON_BOX_BUTTON_EDITOR_CONST(x)
#define IS_SELF(x) GEM_IS_BUTTON_BOX_BUTTON_EDITOR(x)
#define TYPE_SELF GEM_TYPE_BUTTON_BOX_BUTTON_EDITOR
#define SELF_CLASS(x) GEM_BUTTON_BOX_BUTTON_EDITOR_CLASS(x)

#define SELF_GET_CLASS(x) GEM_BUTTON_BOX_BUTTON_EDITOR_GET_CLASS(x)

/* self typedefs */
typedef GemButtonBoxButtonEditor Self;
typedef GemButtonBoxButtonEditorClass SelfClass;

/* here are local prototypes */
static void gem_button_box_button_editor_init (GemButtonBoxButtonEditor * o) G_GNUC_UNUSED;
static void gem_button_box_button_editor_class_init (GemButtonBoxButtonEditorClass * c) G_GNUC_UNUSED;
static void ___2_gem_button_box_button_editor_response (GtkDialog * dialog, gint response_id) G_GNUC_UNUSED;

/* pointer to the class of our parent */
static GtkDialogClass *parent_class = NULL;

/* Short form macros */
#define self_new gem_button_box_button_editor_new
GType
gem_button_box_button_editor_get_type (void)
{
	static GType type = 0;

	if ___GOB_UNLIKELY(type == 0) {
		static const GTypeInfo info = {
			sizeof (GemButtonBoxButtonEditorClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gem_button_box_button_editor_class_init,
			(GClassFinalizeFunc) NULL,
			NULL /* class_data */,
			sizeof (GemButtonBoxButtonEditor),
			0 /* n_preallocs */,
			(GInstanceInitFunc) gem_button_box_button_editor_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_DIALOG, "GemButtonBoxButtonEditor", &info, (GTypeFlags)0);
	}

	return type;
}

/* a macro for creating a new object of our type */
#define GET_NEW ((GemButtonBoxButtonEditor *)g_object_new(gem_button_box_button_editor_get_type(), NULL))

/* a function for creating a new object of our type */
#include <stdarg.h>
static GemButtonBoxButtonEditor * GET_NEW_VARG (const char *first, ...) G_GNUC_UNUSED;
static GemButtonBoxButtonEditor *
GET_NEW_VARG (const char *first, ...)
{
	GemButtonBoxButtonEditor *ret;
	va_list ap;
	va_start (ap, first);
	ret = (GemButtonBoxButtonEditor *)g_object_new_valist (gem_button_box_button_editor_get_type (), first, ap);
	va_end (ap);
	return ret;
}


static void
___finalize(GObject *obj_self)
{
#define __GOB_FUNCTION__ "Gem:Button:Box:Button:Editor::finalize"
	GemButtonBoxButtonEditor *self G_GNUC_UNUSED = GEM_BUTTON_BOX_BUTTON_EDITOR (obj_self);
	gpointer priv G_GNUC_UNUSED = self->_priv;
	if(G_OBJECT_CLASS(parent_class)->finalize) \
		(* G_OBJECT_CLASS(parent_class)->finalize)(obj_self);
}
#undef __GOB_FUNCTION__

static void 
gem_button_box_button_editor_init (GemButtonBoxButtonEditor * o)
{
#define __GOB_FUNCTION__ "Gem:Button:Box:Button:Editor::init"
	o->_priv = G_TYPE_INSTANCE_GET_PRIVATE(o,GEM_TYPE_BUTTON_BOX_BUTTON_EDITOR,GemButtonBoxButtonEditorPrivate);
}
#undef __GOB_FUNCTION__
static void 
gem_button_box_button_editor_class_init (GemButtonBoxButtonEditorClass * c)
{
#define __GOB_FUNCTION__ "Gem:Button:Box:Button:Editor::class_init"
	GObjectClass *g_object_class G_GNUC_UNUSED = (GObjectClass*) c;
	GtkDialogClass *gtk_dialog_class = (GtkDialogClass *)c;

	g_type_class_add_private(c,sizeof(GemButtonBoxButtonEditorPrivate));

	parent_class = (GtkDialogClass *)g_type_class_ref (GTK_TYPE_DIALOG);

#line 121 "src/gem-button-box-button-bless-editor.gob"
	gtk_dialog_class->response = ___2_gem_button_box_button_editor_response;
#line 145 "gem-button-box-button-editor.cc"
	g_object_class->finalize = ___finalize;
}
#undef __GOB_FUNCTION__



#line 46 "src/gem-button-box-button-bless-editor.gob"
GtkWidget * 
gem_button_box_button_editor_new (GtkWidget * bbb)
#line 155 "gem-button-box-button-editor.cc"
{
#define __GOB_FUNCTION__ "Gem:Button:Box:Button:Editor::new"
{
#line 48 "src/gem-button-box-button-bless-editor.gob"
	
        GtkWidget *widget =  (GtkWidget *)GET_NEW;
        GemButtonBoxButtonEditor *self = GEM_BUTTON_BOX_BUTTON_EDITOR(widget);

        selfp->button_box_button = bbb;
        gtk_window_set_transient_for(GTK_WINDOW(widget),
                                     GTK_WINDOW(gtk_widget_get_toplevel(bbb)));

        gtk_dialog_add_buttons(GTK_DIALOG(widget),
                               GTK_STOCK_APPLY,
                               GTK_RESPONSE_APPLY,
                               GTK_STOCK_CLOSE,
                               GTK_RESPONSE_CLOSE,
                               "gtk-execute",
                               RESPONSE_RUN,
                               NULL
                               );

        GtkWidget *hbox = gtk_hbox_new(FALSE,FALSE);
        gtk_box_pack_start(GTK_BOX(hbox),
                           gtk_label_new("Label: "),
                           FALSE, FALSE, 0);
        selfp->w_entry_label = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(selfp->w_entry_label),
                           gem_button_box_button_bless_get_label(GTK_BUTTON(bbb)));
        gtk_box_pack_start(GTK_BOX(hbox),
                           selfp->w_entry_label, FALSE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widget)->vbox),
                           hbox, FALSE, FALSE, 0);

        GtkWidget *w_text_sw = gtk_scrolled_window_new(FALSE, FALSE);
        /*        GtkWidget *w_text_view = gtk_text_view_new(); */

        GtkSourceLanguagesManager *lang_manager = gtk_source_languages_manager_new();
        GtkSourceLanguage *tcl_language = gtk_source_languages_manager_get_language_from_mime_type (lang_manager, "text/x-tcl");
        selfp->text_buffer = GTK_TEXT_BUFFER(gtk_source_buffer_new_with_language(tcl_language));
        GtkWidget *w_text_view = gtk_source_view_new_with_buffer(GTK_SOURCE_BUFFER(selfp->text_buffer));
        gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW(w_text_view),
                                         TRUE);
        
        gtk_source_buffer_set_highlight (GTK_SOURCE_BUFFER(selfp->text_buffer),
                                         true);
        
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(selfp->text_buffer),
                                 gem_button_box_button_bless_get_script(GTK_BUTTON(bbb)), -1);

        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(w_text_view),
                                     GTK_WRAP_NONE);
        gtk_source_view_set_insert_spaces_instead_of_tabs
            (GTK_SOURCE_VIEW(w_text_view),
             true);

        gtk_widget_modify_font(w_text_view,
                               pango_font_description_from_string("Monospace"));
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widget)->vbox),
                           w_text_sw,
                           TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(w_text_sw), w_text_view);
        gtk_widget_set_size_request(w_text_view, 500, 300);
        
#if 0
        g_signal_connect(G_OBJECT(widget), "response",
                         G_CALLBACK(cb_dialog_edit_response),
                         bbb);
#endif

        gtk_widget_grab_focus(w_text_view);
        
        gtk_widget_show_all(widget);

        return widget;
    }}
#line 232 "gem-button-box-button-editor.cc"
#undef __GOB_FUNCTION__

#line 121 "src/gem-button-box-button-bless-editor.gob"
static void 
___2_gem_button_box_button_editor_response (GtkDialog * dialog, gint response_id)
#line 238 "gem-button-box-button-editor.cc"
#define PARENT_HANDLER(___dialog,___response_id) \
	{ if(GTK_DIALOG_CLASS(parent_class)->response) \
		(* GTK_DIALOG_CLASS(parent_class)->response)(___dialog,___response_id); }
{
#define __GOB_FUNCTION__ "Gem:Button:Box:Button:Editor::response"
#line 121 "src/gem-button-box-button-bless-editor.gob"
	g_return_if_fail (dialog != NULL);
#line 121 "src/gem-button-box-button-bless-editor.gob"
	g_return_if_fail (GTK_IS_DIALOG (dialog));
#line 248 "gem-button-box-button-editor.cc"
{
#line 125 "src/gem-button-box-button-bless-editor.gob"
	
        GemButtonBoxButtonEditor *self = GEM_BUTTON_BOX_BUTTON_EDITOR(dialog);
        GtkTextIter iter_start, iter_end;
        
        gtk_text_buffer_get_start_iter(selfp->text_buffer, &iter_start);
        gtk_text_buffer_get_end_iter(selfp->text_buffer, &iter_end);
        gchar *contents = gtk_text_buffer_get_text(selfp->text_buffer,
                                                           &iter_start, &iter_end, TRUE);
        switch (response_id) {
        case GTK_RESPONSE_CLOSE:
            {
                const gchar *current_script = gem_button_box_button_bless_get_script(GTK_BUTTON(selfp->button_box_button));
                if (strcmp(current_script, contents) != 0) {
                    GtkWidget *qdialog
                        = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 "Current script not stored. Apply?"
                                                 );
                    gtk_widget_show_all(qdialog);
                    if (gtk_dialog_run(GTK_DIALOG(qdialog)) == GTK_RESPONSE_YES) {
                        gem_button_box_button_bless_set_script(GTK_BUTTON(selfp->button_box_button),
                                                         contents);
                        gem_button_box_button_bless_set_label(GTK_BUTTON(selfp->button_box_button),
                                                        gtk_entry_get_text(GTK_ENTRY(selfp->w_entry_label)));
                    }
                    
                    gtk_widget_destroy(GTK_WIDGET(qdialog));
                }
                gtk_widget_destroy(GTK_WIDGET(self));
            }
            break;
        case RESPONSE_RUN:
            {
                gem_button_box_button_bless_cmd_eval(selfp->button_box_button,
                                                     contents);
                break;
            }
        case GTK_RESPONSE_APPLY:
            gem_button_box_button_bless_set_label(GTK_BUTTON(selfp->button_box_button),
                                                  gtk_entry_get_text(GTK_ENTRY(selfp->w_entry_label)));
            gem_button_box_button_bless_set_script(GTK_BUTTON(selfp->button_box_button),
                                                   contents);
            break;
        default:
            ;
        }
        g_free(contents);

        PARENT_HANDLER (GTK_DIALOG(self), response_id);
    }}
#line 303 "gem-button-box-button-editor.cc"
#undef __GOB_FUNCTION__
#undef PARENT_HANDLER
