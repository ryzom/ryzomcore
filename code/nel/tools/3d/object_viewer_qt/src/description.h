/**
@mainpage Object Viewer Qt
@author Dzmitry Kamiahin <dnk-88@tut.by>, (C) 2010

@section introduce Introduce
@details
The Object Viewer Qt (ovqt) is a tool for working with graphics technology NeL data, 
which can be useful durung the development and preparation of the game content. 
The tool can be used in different tasks:
- Loading and viewing NeL data (.shape, .ps) files with the ability to set wind power, color light.
- Viewing animation shape (loading .anim, .skel, .swt files) using playlist or slot manager(mixing animation).
- Create and modify NeL particle system. 
- Viewing water shape with the ability to set day/night.
- Modify skeleton (.skel files, .scale file).
- Create and modify micro-vegetation material (.vegetset files)
- Viewing landscape (.zonel files)
- Dialog allows to specify graphical, sound, search path and landscape settings.

@section project_structure Overview of the Object Viewer Qt Project Structure
@details
OVQT - consists of several major subsystems. Each subsystem performs its defined part of the task. 
Through Modules:: provides access to all other program subsystems. 

Program has the following subsystems:
- @ref Modules - Main modules aggregated all parts of the program.
- @ref NLQT::CConfiguration - is responsible for loading and saving settings from the configuration file. As well as search path of data.
- @ref NLQT::CObjectViewer - main subsystem of the program, which initializes the driver, creates a scene and other supporting elements. 
It is a container for loaded models, which can further be viewed and animated.
- @ref NLQT::CMainWindow - is responsible for the GUI.
- @ref NLQT::CParticleEditor - is responsible for the particle systems and provides access to a container that keeps all the loaded particle systems. 
And also allows you to view an animation of particle systems, with the ability to control its parameters.
- @ref NLQT::CVegetableEditor - is responsible for the landscape zones and the editor of vegetation. 
Allows you to load and view the landscape. Also has the ability to create and edit the parameters of the micro-vegetation.
- @ref NLQT::CSoundSystem -  is responsible for the sound in the program.

<b>
The structure of the GUI in the editor of the particles. 
</b><br>
This can be useful for new developers who want to add more new dialogs or to improve the functionality of existing dialogues.
<img src="gui_struct.png" alt="Particle Workspace">
ParticleWorkspace dialogue uses the technique of model-view. 
Using the signal/slot link QTreeView with QStackWidget, Editor properties dialog. 
When you select an item in QTreeView, QStackWidget displays the necessary page(PageWidget) where you can edit the element of the particles system. 
Each page is made in the designer and has a separate forms(.ui). 
In accordance with the recommendations of dialogues design, with a large number of items,every page uses QTabWidget. 
In case when there is a great number of controls, and not everything you want to display, 
it's used the dynamic creation of tabs. Moreover, each tab uses a separate forms(.ui).

@section for_new_developer Guide for new developers of the Object Viewer Qt.
@details
 First of all, to begin developing dialogues that add new features ovqt,
it is needed to read the documentation Qt libs (http://doc.qt.nokia.com/) and NeL documentation.
 
 In order to have convenient using of the tool and its further development, 
it is expected to make a unified interface that is why all dialogs should adhere to a standard design, 
which will be written further. For this goals program provides some additional widgets, 
which are recommended to use. As in the development Qt Designer is actively used. 
To get access to founded here widgets from the designer,the technique promotion is used, 
which can be found <a href="http://doc.qt.nokia.com/4.5/designer-using-custom-widgets.html"> here </a>.
<ol>
<li><b>
CEditRangeUIntWidget / CEditRangeIntWidget / CEditRangeFloatWidget
</b><br>
Widgets provides a slider that allows you to specify an integer (or float, depending on which widget used) number within a set range. 
The range is also can be set by the user, or for a more accurate selection of numbers, either for receiving large values. 
As there may be situations when the range that a user requests,has to be restricted, and widget provides methods that allow you to do so.
<br><br>
<img src="cedit_range_int_widget.png" alt="CEditRangeIntWidget">
<br>
<img src="cedit_range_float_widget.png" alt="CEditRangeFloatWidget">
@see
@ref NLQT::CEditRangeUIntWidget, @ref NLQT::CEditRangeIntWidget, @ref NLQT::CEditRangeFloatWidget 

<li><b>
CColorEditWidget
</b><br>
Using this widget you can set the color(RGBA) using the four sliders or through the color selection dialog.
<br><br>
<img src="cedit_color_widget.png" alt="CColorEditWidget">
@see
@ref NLQT::CColorEditWidget

<li><b>
CDirectionWidget 
</b><br>
This widget helps to choose from several preset directions, or to choose a custom one. 
To use it you have to create a wrapper.
<br><br>
<img src="cdirection_widget.png" alt="CDirectionWidget">
@see
@ref NLQT::CDirectionWidget
</ol><br><br>

To further convinience of instrument using dialogues interface is recommended to make as follows. 
The most sophisticated tools of the program, should take the form of three dock windows (example shown in the screenshot).
<img src="object_viewer_qt_window.png" alt="Object Viewer Qt">
<ol>
<li>
The first dock window - is a control dock window, it should focus all of the control functions of general purpose 
(for example: start/stop/pause animations or particles system). 
It is recommended to perform of the horizontal type window and placed in the bottom of the main window.
<li>
The second dock window - is a list or a tree of items. In which selecting of the current element, 
which assumes to modify or viewis possible.Operations "add/remove/insert or other" items are recommended to make as a popur menu. 
It is recommended to perform of the vertical type window and placed in the left of the main window.
<li>
The third dock - is an editor for properties of the element that was selected in the list or in the tree of elements. 
As all the controls occupy much space or do not fit at all, you have to use tabs QTabWidget, 
which in total can contains quite a lot of elements. 
For a small number of tabs it is allowed to use both horizontal and vertical location. 
But with a large number of tabs, it is necessary to apply the vertical arrangement. 
It is recommended to perform of the vertical type of window and placed in the right of the main window.
</ol>
In the simple dialogues do not necessary to use all three windows, but user has to adhere to the recommendations given above as well.
Also, <b>all dialogs must use the qt layout manager</b>. And if you do not use the designer, 
make sure you use the qt tools internationalization applications.
<br><br>
In ovqt for most dialogs their owner is NLQT::CMainWindow and in its methods creating and initializing all dependent dialogs occur. 
For the convenience of the program using, most dialogues are created in the form of docking windows. 
Creating all the dialogues are carried out in a private method NLQT::CMainWindow::createDialogs(). 
Hence, it is necessary to add operations in this method to create new dialogues. 
But we must take into account that at this moment is fully available only one component of the program CConfiguration, 
to read the settings from the configuration file. The remaining components of the program are available only after the main window stays visible. 
Calling the dialogues going through the menu or the toolbar, usually it is a checkable item. 
Adding of the new menu items or items toolbars is need in methods NLQT::CMainWindow::createMenus(), NLQT::CMainWindow::createToolBars().

@section license_ovqt License Object Viewer Qt
@details
    Object Viewer Qt
<br>
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>
<br><br>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
<br><br>
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
<br><br>
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/