// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// Scintilla source code edit control
/** @file ScintillaWidget.h
 ** Definition of Scintilla widget for GTK+.
 ** Only needed by GTK+ code but is harmless on other platforms.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLAWIDGET_H
#define SCINTILLAWIDGET_H

#if PLAT_GTK

#ifdef __cplusplus
extern "C" {
#endif

#define SCINTILLA(obj)          GTK_CHECK_CAST (obj, scintilla_get_type (), ScintillaObject)
#define SCINTILLA_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, scintilla_get_type (), ScintillaClass)
#define IS_SCINTILLA(obj)       GTK_CHECK_TYPE (obj, scintilla_get_type ())

typedef struct _ScintillaObject ScintillaObject;
typedef struct _ScintillaClass  ScintillaClass;

struct _ScintillaObject {
	GtkContainer cont;
	void *pscin;
};

struct _ScintillaClass {
	GtkFixedClass parent_class;

	void (* command) (ScintillaObject *ttt);
	void (* notify) (ScintillaObject *ttt);
};

guint		scintilla_get_type	(void);
GtkWidget*	scintilla_new		(void);
void		scintilla_set_id	(ScintillaObject *sci,int id);
sptr_t	scintilla_send_message	(ScintillaObject *sci,unsigned int iMessage, uptr_t wParam, sptr_t lParam);

#ifdef __cplusplus
}
#endif

#endif

#endif
