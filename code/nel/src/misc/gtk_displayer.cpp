// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdmisc.h"
#include "nel/misc/gtk_displayer.h"

#ifdef NL_USE_GTK

#include <gtk/gtk.h>
#include <gdk/gdk.h>
//#include <gobject/gobject.h>
#include <gdk/gdkkeysyms.h>

#ifdef NL_OS_WINDOWS
// automatically add gtk library
#pragma comment(lib, "gtk-1.3.lib")
#pragma comment(lib, "gdk-1.3.lib")
#pragma comment(lib, "glib-1.3.lib")
#pragma comment(lib, "gthread-1.3.lib")
#endif

#include "nel/misc/app_context.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/thread.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

//
// Variables
//

static vector<string> CommandHistory;
static uint32 CommandHistoryPos = 0;
static CLog *Log = 0;

static GtkWidget *RootWindow = NULL, *OutputText = NULL, *InputText = NULL;
static GtkWidget *hrootbox = NULL, *scrolled_win2 = NULL;

//
// Functions
//

CGtkDisplayer (const char *displayerName) : CWindowDisplayer(displayerName)
{
	needSlashR = false;
	createLabel ("@Clear|CLEAR");

	INelContext::getInstance().setWindowedApplication(true);
}

CGtkDisplayer::~CGtkDisplayer ()
{
	if (_Init)
	{
	}
}

gint ButtonClicked(GtkWidget *Widget, gpointer *Data)
{
	CGtkDisplayer *disp = (CGtkDisplayer *) Data;

	// find the button and execute the command
	CSynchronized<std::vector<CGtkDisplayer::CLabelEntry> >::CAccessor access (&(disp->_Labels));
	for (uint i = 0; i < access.value().size(); i++)
	{
		if (access.value()[i].Hwnd == Widget)
		{
			if(access.value()[i].Value == "@Clear|CLEAR")
			{
				// special commands because the clear must be called by the display thread and not main thread
				disp->clear ();
			}
			else
			{
				// the button was found, add the command in the command stack
				CSynchronized<std::vector<std::string> >::CAccessor accessCommands (&disp->_CommandsToExecute);
				string str;
				nlassert (!access.value()[i].Value.empty());
				nlassert (access.value()[i].Value[0] == '@');

				string::size_type pos = access.value()[i].Value.find ("|");
				if (pos != string::npos)
				{
					str = access.value()[i].Value.substr(pos+1);
				}
				else
				{
					str = access.value()[i].Value.substr(1);
				}
				if (!str.empty())
					accessCommands.value().push_back(str);
			}
			break;
		}
	}
	return TRUE;
}


void CGtkDisplayer::updateLabels ()
{
	{
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access (&_Labels);
		for (uint i = 0; i < access.value().size(); i++)
		{
			if (access.value()[i].NeedUpdate && !access.value()[i].Value.empty())
			{
				string n;

				if (access.value()[i].Value[0] != '@')
					n = access.value()[i].Value;
				else
				{
					string::size_type pos = access.value()[i].Value.find ('|');
					if (pos != string::npos)
					{
						n = access.value()[i].Value.substr (1, pos - 1);
					}
					else
					{
						n = access.value()[i].Value.substr (1);
					}
				}

				if (access.value()[i].Hwnd == NULL)
				{
					// create a button for command and label for variables
					if (access.value()[i].Value[0] == '@')
					{
						access.value()[i].Hwnd = gtk_button_new_with_label (n.c_str());
						nlassert (access.value()[i].Hwnd != NULL);
						gtk_signal_connect (GTK_OBJECT (access.value()[i].Hwnd), "clicked", GTK_SIGNAL_FUNC (ButtonClicked), (gpointer) this);
						GtkLabel *label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(access.value()[i].Hwnd)));
						gtk_label_set_justify (label, GTK_JUSTIFY_LEFT);
						gtk_label_set_line_wrap (label, FALSE);
						gtk_widget_show (GTK_WIDGET (access.value()[i].Hwnd));
						gtk_box_pack_start (GTK_BOX (hrootbox), GTK_WIDGET (access.value()[i].Hwnd), TRUE, TRUE, 0);
					}
					else
					{
						access.value()[i].Hwnd = gtk_label_new ("");
						gtk_label_set_justify (GTK_LABEL (access.value()[i].Hwnd), GTK_JUSTIFY_LEFT);
						gtk_label_set_line_wrap (GTK_LABEL (access.value()[i].Hwnd), FALSE);
						gtk_widget_show (GTK_WIDGET (access.value()[i].Hwnd));
						gtk_box_pack_start (GTK_BOX (hrootbox), GTK_WIDGET (access.value()[i].Hwnd), TRUE, TRUE, 0);
					}
				}

				if (access.value()[i].Value[0] != '@')
					gtk_label_set_text (GTK_LABEL (access.value()[i].Hwnd), n.c_str());

				access.value()[i].NeedUpdate = false;
			}
		}
	}
}

// windows delete event => quit
gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_main_quit();

	exit(1);
	return FALSE;
}

gint KeyIn(GtkWidget *Widget, GdkEventKey *Event, gpointer *Data)
{
	switch (Event->keyval)
	{
	case GDK_Escape :
		gtk_entry_set_text (GTK_ENTRY(Widget), "");
		break;
	case GDK_Up :
		if (CommandHistoryPos > 0) {
			CommandHistoryPos--;
			gtk_entry_set_text (GTK_ENTRY(Widget), CommandHistory[CommandHistoryPos].c_str());
		}
		break;
	case GDK_Down :
		if (CommandHistoryPos + 1 < CommandHistory.size())
		{
			CommandHistoryPos++;
			gtk_entry_set_text (GTK_ENTRY(Widget), CommandHistory[CommandHistoryPos].c_str());
		}
		break;
	case GDK_KP_Enter :
		gtk_signal_emit_by_name(GTK_OBJECT(Widget),"activate");
	default :
		return FALSE;
	}
	gtk_signal_emit_stop_by_name(GTK_OBJECT(Widget),"key_press_event");
	return TRUE;
}

void updateInput ()
{
	gtk_widget_grab_focus (InputText);
}

gint KeyOut(GtkWidget *Widget, GdkEventKey *Event, gpointer *Data)
{
	updateInput();
	gtk_signal_emit_stop_by_name(GTK_OBJECT(Widget),"key_press_event");
	return TRUE;
}


/*gint ButtonClear(GtkWidget *Widget, GdkEventKey *Event, gpointer *Data)
{
	CGtkDisplayer *disp = (CGtkDisplayer *) Data;

	disp->clear ();
	return TRUE;
}
*/


// the user typed  command, execute it
gint cbValidateCommand (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	string cmd = gtk_entry_get_text (GTK_ENTRY(widget));
	CommandHistory.push_back (cmd);
	// execute the command
	if(Log == NULL)
		Log = InfoLog;
	ICommand::execute (cmd, *Log);
	// clear the input text
	gtk_entry_set_text (GTK_ENTRY(widget), "");
	CommandHistoryPos = CommandHistory.size();
	return TRUE;
}


void CGtkDisplayer::setTitleBar (const string &titleBar)
{
	string wn;
	if (!titleBar.empty())
	{
		wn += titleBar;
		wn += ": ";
	}
	wn += "Nel Service Console (compiled " __DATE__ " " __TIME__ " in " + nlMode + " mode)";

	//nlassert (RootWindow != NULL);
	gtk_window_set_title (GTK_WINDOW (RootWindow), wn.c_str());
}

void CGtkDisplayer::open (std::string titleBar, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww, CLog *log)
{
	_HistorySize = hs;

	if (w == -1)
		w = 700;
	if (h == -1)
		h = 300;
	if (hs == -1)
		hs = 10000;

	gtk_init (NULL, NULL);

	Log = log;

	// Root window
	RootWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (RootWindow), w, h);
	gtk_signal_connect (GTK_OBJECT (RootWindow), "delete_event", GTK_SIGNAL_FUNC (delete_event), NULL);

	// Vertical box
	GtkWidget *vrootbox = gtk_vbox_new (FALSE, 0);
	nlassert (vrootbox != NULL);
	gtk_container_add (GTK_CONTAINER (RootWindow), vrootbox);

	// Horizontal box (for labels)
	hrootbox = gtk_hbox_new (FALSE, 0);
	nlassert (hrootbox != NULL);
	gtk_box_pack_start (GTK_BOX (vrootbox), hrootbox, FALSE, FALSE, 0);

/*	// Clear button
    GtkWidget *button = gtk_button_new_with_label ("Clear");
	nlassert (button != NULL);
    gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (ButtonClear), (gpointer) this);
	gtk_box_pack_start (GTK_BOX (hrootbox), button, FALSE, FALSE, 0);
*/
	// Output text
	scrolled_win2 = gtk_scrolled_window_new (NULL, NULL);
	nlassert (scrolled_win2 != NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win2), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_widget_show (scrolled_win2);
	gtk_container_add (GTK_CONTAINER (vrootbox), scrolled_win2);

	OutputText = gtk_text_view_new();
	nlassert (OutputText != NULL);

	PangoFontDescription *fontDesc = pango_font_description_from_string("Monospace 10");
  	gtk_widget_modify_font(OutputText, fontDesc);
  	pango_font_description_free(fontDesc);

	GtkTextBuffer *textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(OutputText));
	GtkTextIter endIter;
	gtk_text_buffer_get_end_iter(textBuffer, &endIter);
	gtk_text_buffer_create_mark(textBuffer, "endmark", &endIter, false);
	gtk_signal_connect(GTK_OBJECT(OutputText),"key_press_event",GTK_SIGNAL_FUNC(KeyOut),NULL);
	gtk_text_view_set_editable (GTK_TEXT_VIEW(OutputText), FALSE);
	gtk_container_add (GTK_CONTAINER (scrolled_win2), OutputText);

	// Input text
	InputText = gtk_entry_new ();
	nlassert (InputText != NULL);
	gtk_signal_connect (GTK_OBJECT(InputText), "activate", GTK_SIGNAL_FUNC(cbValidateCommand), NULL);
	gtk_signal_connect(GTK_OBJECT(InputText),"key_press_event",GTK_SIGNAL_FUNC(KeyIn),NULL);
	gtk_box_pack_start (GTK_BOX (vrootbox), InputText, FALSE, FALSE, 0);

//	gtk_widget_show (button);
	gtk_widget_show (OutputText);
	gtk_widget_show (InputText);

	gtk_widget_show (hrootbox);
	gtk_widget_show (vrootbox);
    gtk_widget_show (RootWindow);

	setTitleBar (titleBar);

	_Init = true;
}

void CGtkDisplayer::clear ()
{
	GtkTextBuffer *buffer;
	GtkTextIter    start, end;

	// who is taking care of the iterators?
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(OutputText));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	gtk_text_buffer_delete(buffer, &start, &end);
}

gint updateInterf (gpointer data)
{
	CGtkDisplayer *disp = (CGtkDisplayer *)data;

	//
	// Update labels
	//

	disp->updateLabels ();

	//
	// Display the bufferized string
	//
	GtkAdjustment *Adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_win2));
	bool Bottom = (Adj->value >= Adj->upper - Adj->page_size - Adj->step_increment);
	bool textChanged = false;

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(OutputText));
	GtkTextIter    end_iter;
	gtk_text_buffer_get_end_iter(buffer, &end_iter);

	std::list<std::pair<uint32, std::string> >::iterator it;
	{
		CSynchronized<std::list<std::pair<uint32, std::string> > >::CAccessor access (&disp->_Buffer);

		for (it = access.value().begin(); it != access.value().end(); it++)
		{
			uint32 col = (*it).first;
			GtkTextTag *tag = NULL;
			if ((col>>24) == 0)
			{
				GdkColor color;
				color.red = (col >> 8) & 0xFF00;
				color.green = col & 0xFF00;
				color.blue = (col << 8) & 0xFF00;
				tag = gtk_text_buffer_create_tag(buffer, NULL, "foreground-gdk", &color, "foreground-set", TRUE, NULL);
			}
			gtk_text_buffer_insert_with_tags(buffer, &end_iter, (*it).second.c_str(), -1, tag, NULL);
			textChanged = true;
		}

		access.value().clear ();
	}

	if (Bottom && textChanged)
	{
		GtkTextMark *mark = gtk_text_buffer_get_mark(buffer, "endmark");
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(OutputText), mark, 0.0, true, 0.0, 1.0);
	}

	return TRUE;
}

void CGtkDisplayer::display_main ()
{
	//
	// Manage windows message
	//

	gtk_timeout_add (10, updateInterf, this);
	gtk_main ();
}


void CGtkDisplayer::getWindowPos (uint32 &x, uint32 &y, uint32 &w, uint32 &h)
{
// todo
	x = y = w = h = 0;
}



} // NLMISC

#else // NL_USE_GTK

// remove stupid VC6 warnings
void foo_gtk_displayer_cpp() {}

#endif // NL_USE_GTK
