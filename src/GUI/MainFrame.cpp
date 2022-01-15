/* SampleHive
 * Copyright (C) 2021  Apoorv Singh
 * A simple, modern audio sample browser/manager for GNU/Linux.
 *
 * This file is a part of SampleHive
 *
 * SampleHive is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SampleHive is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "GUI/MainFrame.hpp"
#include "GUI/Dialogs/Settings.hpp"
#include "GUI/Dialogs/TagEditor.hpp"
#include "Database/Database.hpp"
#include "Utility/ControlID_Enums.hpp"
#include "Utility/Paths.hpp"
#include "Utility/Tags.hpp"
#include "Utility/Sample.hpp"
#include "Utility/Log.hpp"
#include "SampleHiveConfig.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <exception>
#include <ios>

#include <stdlib.h>

#include <wx/aboutdlg.h>
#include <wx/accel.h>
#include <wx/arrstr.h>
#include <wx/artprov.h>
#include <wx/busyinfo.h>
#include <wx/dataview.h>
#include <wx/debug.h>
#include <wx/defs.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/dnd.h>
#include <wx/dvrenderers.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/fswatcher.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/dataobj.h>
#include <wx/headercol.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/object.h>
#include <wx/progdlg.h>
#include <wx/stringimpl.h>
#include <wx/textdlg.h>
#include <wx/valtext.h>
#include <wx/variant.h>
#include <wx/vector.h>
#include <wx/utils.h>

MainFrame::MainFrame()
    : wxFrame(NULL, wxID_ANY, "SampleHive", wxDefaultPosition)
{
    // Initialize statusbar with 4 sections
    m_StatusBar = CreateStatusBar(4);

    // Set width for each section of the statusbar
    int status_width[4] = { 300, -6, -1, -2 };
    m_StatusBar->SetStatusWidths(4, status_width);

    m_HiveBitmap = new wxStaticBitmap(m_StatusBar, wxID_ANY, wxBitmap(ICON_HIVE_24px));

    // Initialize menubar and menus
    m_MenuBar = new wxMenuBar();
    m_FileMenu = new wxMenu();
    m_EditMenu = new wxMenu();
    m_ViewMenu = new wxMenu();
    m_HelpMenu = new wxMenu();

    // File menu items
    m_AddFile = new wxMenuItem(m_FileMenu, MN_AddFile, _("Add a file\tCtrl+F"), _("Add a file"));
    m_AddFile->SetBitmap(wxArtProvider::GetBitmap(wxART_NORMAL_FILE));
    m_FileMenu->Append(m_AddFile);

    m_AddDirectory = new wxMenuItem(m_FileMenu, MN_AddDirectory,
                                    _("Add a directory\tCtrl+D"), _("Add a directory"));
    m_AddDirectory->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER));
    m_FileMenu->Append(m_AddDirectory);

    m_FileMenu->AppendSeparator();

    m_FileMenu->Append(wxID_EXIT, wxEmptyString, _("Exits the application"));

    // Edit menu items
    m_EditMenu->Append(wxID_PREFERENCES, _("Preferences\tCtrl+P"), _("Open preferences dialog"));

    // View menu items
    m_ToggleExtension = new wxMenuItem(m_ViewMenu, MN_ToggleExtension,
                                       _("Toggle Extension\tCtrl+E"), _("Show/Hide Extension"), wxITEM_CHECK);
    m_ToggleMenuBar = new wxMenuItem(m_ViewMenu, MN_ToggleMenuBar,
                                     _("Toggle Menu Bar\tCtrl+M"), _("Show/Hide Menu Bar"), wxITEM_CHECK);
    m_ToggleStatusBar = new wxMenuItem(m_ViewMenu, MN_ToggleStatusBar,
                                       _("Toggle Status Bar\tCtrl+B"), _("Show/Hide Status Bar"), wxITEM_CHECK);

    m_ViewMenu->Append(m_ToggleExtension)->Check(true);
    m_ViewMenu->Append(m_ToggleMenuBar)->Check(m_MenuBar->IsShown());
    m_ViewMenu->Append(m_ToggleStatusBar)->Check(m_StatusBar->IsShown());

    // Help menu items
    m_HelpMenu->Append(wxID_REFRESH, _("Reset app data"),
                       _("Clear the application data revert to default configuration"));
    m_HelpMenu->Append(wxID_ABOUT, wxEmptyString, _("Show about the application"));

    // Append all menus to menubar
    m_MenuBar->Append(m_FileMenu, _("&File"));
    m_MenuBar->Append(m_EditMenu, _("&Edit"));
    m_MenuBar->Append(m_ViewMenu, _("&View"));
    m_MenuBar->Append(m_HelpMenu, _("&Help"));

    // Set the menu bar to use
    SetMenuBar(m_MenuBar);

    // Initializing Sizers
    m_MainSizer = new wxBoxSizer(wxVERTICAL);

    m_TopSizer = new wxBoxSizer(wxVERTICAL);

    m_BottomLeftPanelMainSizer = new wxBoxSizer(wxVERTICAL);
    m_TopPanelMainSizer = new wxBoxSizer(wxVERTICAL);
    m_BottomRightPanelMainSizer = new wxBoxSizer(wxVERTICAL);

    m_BrowserControlSizer = new wxBoxSizer(wxHORIZONTAL);
    m_WaveformDisplaySizer = new wxBoxSizer(wxHORIZONTAL);

    m_HivesMainSizer = new wxBoxSizer(wxVERTICAL);
    m_HivesFavoritesSizer = new wxBoxSizer(wxVERTICAL);
    m_HivesButtonSizer = new wxBoxSizer(wxHORIZONTAL);

    m_TrashMainSizer = new wxBoxSizer(wxVERTICAL);
    m_TrashItemSizer = new wxBoxSizer(wxVERTICAL);
    m_TrashButtonSizer = new wxBoxSizer(wxHORIZONTAL);

    // Main panel of the app
    m_MainPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    // Creating top splitter window
    m_TopSplitter = new wxSplitterWindow(m_MainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                         wxSP_NOBORDER | wxSP_LIVE_UPDATE | wxSP_THIN_SASH);
    m_TopSplitter->SetMinimumPaneSize(200);
    m_TopSplitter->SetSashGravity(0);

    // Top half of TopSplitter window
    m_TopPanel = new wxPanel(m_TopSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    // Bottom half of the TopSplitter window
    m_BottomSplitter = new wxSplitterWindow(m_TopSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                            wxSP_NOBORDER | wxSP_LIVE_UPDATE | wxSP_THIN_SASH);
    m_BottomSplitter->SetMinimumPaneSize(300);
    m_BottomSplitter->SetSashGravity(0);

    // Left half of the BottomSplitter window
    m_BottomLeftPanel = new wxPanel(m_BottomSplitter, wxID_ANY,
                                    wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    // Initializing wxNotebook
    m_Notebook = new wxNotebook(m_BottomLeftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);

    // Initializing wxGenericDirCtrl as one of the wxNotebook page.
    m_DirCtrl = new wxGenericDirCtrl(m_Notebook, BC_DirCtrl, wxDirDialogDefaultFolderStr, wxDefaultPosition,
                                     wxDefaultSize, wxDIRCTRL_SHOW_FILTERS,
                                     _("All files|*|Ogg files (*.ogg)|*.ogg|Wav files (*.wav)|*.wav|"
                                       "Flac files (*.flac)|*.flac"), 0);

    m_DirCtrl->SetPath(USER_HOME_DIR);

    // This panel will hold 2nd page of wxNotebook
    m_HivesPanel = new wxPanel(m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    m_AddHiveButton = new wxButton(m_HivesPanel, BC_HiveAdd, "+", wxDefaultPosition, wxDefaultSize, 0);
    m_AddHiveButton->SetToolTip(_("Create new hive"));
    m_RemoveHiveButton = new wxButton(m_HivesPanel, BC_HiveRemove, "-", wxDefaultPosition, wxDefaultSize, 0);
    m_RemoveHiveButton->SetToolTip(_("Delete selected hive"));

    // Initializing wxTreeCtrl as another page of wxNotebook
    m_Hives = new wxDataViewTreeCtrl(m_HivesPanel, BC_Hives, wxDefaultPosition, wxDefaultSize,
                                     wxDV_NO_HEADER | wxDV_SINGLE);

    // Adding default hive
    favorites_hive = m_Hives->AppendContainer(wxDataViewItem(wxNullPtr), _("Favorites"));

    // Setting m_Hives to accept files to be dragged and dropped on it
    m_Hives->DragAcceptFiles(true);

    m_TrashPanel = new wxPanel(m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    m_Trash = new wxTreeCtrl(m_TrashPanel, BC_Trash, wxDefaultPosition, wxDefaultSize,
                             wxTR_NO_BUTTONS | wxTR_HIDE_ROOT | wxTR_MULTIPLE);

    // Setting m_Trash to accept files to be dragged and dropped on it
    m_Trash->DragAcceptFiles(true);

    m_RestoreTrashedItemButton = new wxButton(m_TrashPanel, BC_RestoreTrashedItem, _("Restore sample"),
                                              wxDefaultPosition, wxDefaultSize, 0);
    m_RestoreTrashedItemButton->SetToolTip(_("Restore selected sample"));

    // Addubg root to TrashedItems
    trash_root = m_Trash->AddRoot("Trash");

    // Adding the pages to wxNotebook
    m_Notebook->AddPage(m_DirCtrl, _("Browse"), false);
    
    // fastbrowse mode
    /* m_Notebook->AddPage(m_HivesPanel, _("Hives"), false);
        m_Notebook->AddPage(m_TrashPanel, _("Trash"), false); */

    // Right half of BottomSlitter window
    m_BottomRightPanel = new wxPanel(m_BottomSplitter, wxID_ANY,
                                     wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    // Set split direction
    m_TopSplitter->SplitHorizontally(m_TopPanel, m_BottomSplitter);
    m_BottomSplitter->SplitVertically(m_BottomLeftPanel, m_BottomRightPanel);

    m_TopControlsPanel = new wxPanel(m_TopPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                     wxTAB_TRAVERSAL | wxNO_BORDER);

    // Looping region controls
    if (m_Theme.IsDark())
        m_LoopABButton = new wxBitmapToggleButton(m_TopControlsPanel, BC_LoopABButton,
                                                  static_cast<wxString>(ICON_AB_LIGHT_16px),
                                                  wxDefaultPosition, wxDefaultSize, 0);
    else
        m_LoopABButton = new wxBitmapToggleButton(m_TopControlsPanel, BC_LoopABButton,
                                                  static_cast<wxString>(ICON_AB_DARK_16px),
                                                  wxDefaultPosition, wxDefaultSize, 0);

    m_LoopABButton->SetToolTip(_("Loop selected region"));

    // Initializing browser controls on top panel.
    m_AutoPlayCheck = new wxCheckBox(m_TopControlsPanel, BC_Autoplay, _("Autoplay"),
                                     wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_AutoPlayCheck->SetToolTip(_("Autoplay"));
    m_VolumeSlider = new wxSlider(m_TopControlsPanel, BC_Volume, 100, 0, 100,
                                  wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
    m_VolumeSlider->SetToolTip(_("Volume"));
    m_VolumeSlider->SetMinSize(wxSize(120, -1));
    m_VolumeSlider->SetMaxSize(wxSize(120, -1));
    m_SamplePosition = new wxStaticText(m_TopControlsPanel, BC_SamplePosition, "--:--/--:--",
                                        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);

    // Initialize browser control buttons
    if (m_Theme.IsDark())
    {
        m_PlayButton = new wxBitmapButton(m_TopControlsPanel, BC_Play,
                                          wxBitmapBundle::FromBitmap(static_cast<wxString>
                                                                     (ICON_PLAY_LIGHT_16px)),
                                          wxDefaultPosition, wxDefaultSize, 0);
        m_LoopButton = new wxBitmapToggleButton(m_TopControlsPanel, BC_Loop,
                                                static_cast<wxString>(ICON_LOOP_LIGHT_16px),
                                                wxDefaultPosition, wxDefaultSize, 0);
        m_StopButton = new wxBitmapButton(m_TopControlsPanel, BC_Stop,
                                          wxBitmapBundle::FromBitmap(static_cast<wxString>
                                                                     (ICON_STOP_LIGHT_16px)),
                                          wxDefaultPosition, wxDefaultSize, 0);
        m_MuteButton = new wxBitmapToggleButton(m_TopControlsPanel, BC_Mute,
                                                static_cast<wxString>(ICON_MUTE_LIGHT_16px),
                                                wxDefaultPosition, wxDefaultSize, 0);
    }
    else
    {
        m_PlayButton = new wxBitmapButton(m_TopControlsPanel, BC_Play,
                                          wxBitmapBundle::FromBitmap(static_cast<wxString>
                                                                     (ICON_PLAY_DARK_16px)),
                                          wxDefaultPosition, wxDefaultSize, 0);
        m_LoopButton = new wxBitmapToggleButton(m_TopControlsPanel, BC_Loop,
                                                static_cast<wxString>(ICON_LOOP_DARK_16px),
                                                wxDefaultPosition, wxDefaultSize, 0);
        m_StopButton = new wxBitmapButton(m_TopControlsPanel, BC_Stop,
                                          wxBitmapBundle::FromBitmap(static_cast<wxString>
                                                                     (ICON_STOP_DARK_16px)),
                                          wxDefaultPosition, wxDefaultSize, 0);
        m_MuteButton = new wxBitmapToggleButton(m_TopControlsPanel, BC_Mute,
                                                static_cast<wxString>(ICON_MUTE_DARK_16px),
                                                wxDefaultPosition, wxDefaultSize, 0);
    }

    m_PlayButton->SetToolTip(_("Play"));
    m_LoopButton->SetToolTip(_("Loop"));
    m_StopButton->SetToolTip(_("Stop"));
    m_MuteButton->SetToolTip(_("Mute"));

    m_SettingsButton = new wxButton(m_TopControlsPanel, BC_Settings, _("Settings"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    m_SettingsButton->SetToolTip(_("Settings"));

    // Initializing wxSearchCtrl on bottom panel.
    m_SearchBox = new wxSearchCtrl(m_BottomRightPanel, BC_Search, _("Search for samples.."),
                                   wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    // Set minimum and maximum size of m_SearchBox
    // so it doesn't expand too wide when resizing the main frame.
    m_SearchBox->SetMinSize(wxSize(-1, 38));
    m_SearchBox->SetMaxSize(wxSize(-1, 38));

    m_SearchBox->ShowSearchButton(true);
    m_SearchBox->ShowCancelButton(true);

    // Initializing wxDataViewListCtrl on bottom panel.
    m_Library = new wxDataViewListCtrl(m_BottomRightPanel, BC_Library, wxDefaultPosition, wxDefaultSize,
                                       wxDV_MULTIPLE | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES);

    // Adding columns to wxDataViewListCtrl.
    m_Library->AppendBitmapColumn(wxBitmap(ICON_STAR_FILLED_16px),
                                  0,
                                  wxDATAVIEW_CELL_ACTIVATABLE,
                                  30,
                                  wxALIGN_CENTER,
                                  !wxDATAVIEW_COL_RESIZABLE);
    m_Library->AppendTextColumn(_("Filename"),
                                wxDATAVIEW_CELL_INERT,
                                250,
                                wxALIGN_LEFT,
                                wxDATAVIEW_COL_RESIZABLE |
                                wxDATAVIEW_COL_SORTABLE |
                                wxDATAVIEW_COL_REORDERABLE);
    m_Library->AppendTextColumn(_("Sample Pack"),
                                wxDATAVIEW_CELL_INERT,
                                180,
                                wxALIGN_LEFT,
                                wxDATAVIEW_COL_RESIZABLE |
                                wxDATAVIEW_COL_SORTABLE |
                                wxDATAVIEW_COL_REORDERABLE);
    m_Library->AppendTextColumn(_("Type"),
                                wxDATAVIEW_CELL_INERT,
                                120,
                                wxALIGN_LEFT,
                                wxDATAVIEW_COL_RESIZABLE |
                                wxDATAVIEW_COL_SORTABLE |
                                wxDATAVIEW_COL_REORDERABLE);
    m_Library->AppendTextColumn(_("Channels"),
                                wxDATAVIEW_CELL_INERT,
                                90,
                                wxALIGN_RIGHT,
                                wxDATAVIEW_COL_RESIZABLE |
                                wxDATAVIEW_COL_SORTABLE |
                                wxDATAVIEW_COL_REORDERABLE);
    m_Library->AppendTextColumn(_("Length"),
                                wxDATAVIEW_CELL_INERT,
                                80,
                                wxALIGN_RIGHT,
                                wxDATAVIEW_COL_RESIZABLE |
                                wxDATAVIEW_COL_SORTABLE |
                                wxDATAVIEW_COL_REORDERABLE);
    m_Library->AppendTextColumn(_("Sample Rate"),
                                wxDATAVIEW_CELL_INERT,
                                120,
                                wxALIGN_RIGHT,
                                wxDATAVIEW_COL_RESIZABLE |
                                wxDATAVIEW_COL_SORTABLE |
                                wxDATAVIEW_COL_REORDERABLE);
    m_Library->AppendTextColumn(_("Bitrate"),
                                wxDATAVIEW_CELL_INERT,
                                80,
                                wxALIGN_RIGHT,
                                wxDATAVIEW_COL_RESIZABLE |
                                wxDATAVIEW_COL_SORTABLE |
                                wxDATAVIEW_COL_REORDERABLE);
    m_Library->AppendTextColumn(_("Path"),
                                wxDATAVIEW_CELL_INERT,
                                250,
                                wxALIGN_LEFT,
                                wxDATAVIEW_COL_RESIZABLE |
                                wxDATAVIEW_COL_SORTABLE |
                                wxDATAVIEW_COL_REORDERABLE);

    // Enable Library to accept files to be dropped on it
    m_Library->DragAcceptFiles(true);

    // Enable dragging a file from Library
    m_Library->EnableDragSource(wxDF_FILENAME);

    // Initialize wxInfoBar for showing information inside application
    m_InfoBar = new wxInfoBar(m_BottomRightPanel);

    // Initializing wxMediaCtrl.
    m_MediaCtrl = new wxMediaCtrl(this, BC_MediaCtrl, wxEmptyString, wxDefaultPosition,
                                  wxDefaultSize, 0, wxEmptyString);

    // Intializing wxTimer
    m_Timer = new wxTimer(this);

    // Load default yaml config file.
    LoadConfigFile();

    // Initialize the database
    try
    {
        m_Database = std::make_unique<Database>();
        m_Database->CreateTableSamples();
        m_Database->CreateTableHives();
    }
    catch (std::exception& e)
    {
        SH_LOG_ERROR("Error! Cannot initialize database {}", e.what());
    }

    m_TopWaveformPanel = new WaveformViewer(m_TopPanel, *m_Library, *m_MediaCtrl, *m_Database);

    // Binding events.
    Bind(wxEVT_MENU, &MainFrame::OnSelectAddFile, this, MN_AddFile);
    Bind(wxEVT_MENU, &MainFrame::OnSelectAddDirectory, this, MN_AddDirectory);
    Bind(wxEVT_MENU, &MainFrame::OnSelectToggleExtension, this, MN_ToggleExtension);
    Bind(wxEVT_MENU, &MainFrame::OnSelectToggleMenuBar, this, MN_ToggleMenuBar);
    Bind(wxEVT_MENU, &MainFrame::OnSelectToggleStatusBar, this, MN_ToggleStatusBar);
    Bind(wxEVT_MENU, &MainFrame::OnSelectExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainFrame::OnSelectPreferences, this, wxID_PREFERENCES);
    Bind(wxEVT_MENU, &MainFrame::OnSelectResetAppData, this, wxID_REFRESH);
    Bind(wxEVT_MENU, &MainFrame::OnSelectAbout, this, wxID_ABOUT);

    Bind(wxEVT_SPLITTER_SASH_POS_CHANGING, &MainFrame::OnTopSplitterSashPosChanged,
         this, m_TopSplitter->GetId());
    Bind(wxEVT_SPLITTER_SASH_POS_CHANGING, &MainFrame::OnBottomSplitterSashPosChanged,
         this, m_BottomSplitter->GetId());

    this->Connect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::OnResizeFrame), NULL, this);
    m_StatusBar->Connect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::OnResizeStatusBar), NULL, this);

    //Bind(wxEVT_DIRCTRL_FILEACTIVATED, &MainFrame::OnClickDirCtrl, this, BC_DirCtrl);
    //Bind(wxEVT_TREE_BEGIN_DRAG, &MainFrame::OnDragFromDirCtrl, this, m_DirCtrl->GetTreeCtrl()->GetId());
    
    
    // fastbroewsemode
    Bind(wxEVT_TREE_ITEM_EXPANDED, &MainFrame::OnDirTreeOpenAutoImport, this, m_DirCtrl->GetTreeCtrl()->GetId());

    Bind(wxEVT_BUTTON, &MainFrame::OnClickPlay, this, BC_Play);
    Bind(wxEVT_TOGGLEBUTTON, &MainFrame::OnClickLoop, this, BC_Loop);
    Bind(wxEVT_BUTTON, &MainFrame::OnClickStop, this, BC_Stop);
    Bind(wxEVT_TOGGLEBUTTON, &MainFrame::OnClickMute, this, BC_Mute);
    Bind(wxEVT_MEDIA_FINISHED, &MainFrame::OnMediaFinished, this, BC_MediaCtrl);
    Bind(wxEVT_BUTTON, &MainFrame::OnClickSettings, this, BC_Settings);
    Bind(wxEVT_CHECKBOX, &MainFrame::OnCheckAutoplay, this, BC_Autoplay);
    Bind(wxEVT_SCROLL_THUMBTRACK, &MainFrame::OnSlideVolume, this, BC_Volume);
    Bind(wxEVT_SCROLL_THUMBRELEASE, &MainFrame::OnReleaseVolumeSlider, this, BC_Volume);

    Bind(wxEVT_TIMER, &MainFrame::UpdateElapsedTime, this);

    m_Trash->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(MainFrame::OnDragAndDropToTrash), NULL, this);
    Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &MainFrame::OnShowTrashContextMenu, this, BC_Trash);
    Bind(wxEVT_BUTTON, &MainFrame::OnClickRestoreTrashItem, this, BC_RestoreTrashedItem);

    Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &MainFrame::OnClickLibrary, this, BC_Library);
    Bind(wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, &MainFrame::OnDragFromLibrary, this);
    m_Library->Connect(wxEVT_DROP_FILES,
                       wxDropFilesEventHandler(MainFrame::OnDragAndDropToLibrary), NULL, this); 
    
    //Bind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, &MainFrame::OnShowLibraryContextMenu, this, BC_Library);
    
    Bind(wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK,
         &MainFrame::OnShowLibraryColumnHeaderContextMenu, this, BC_Library);

    Bind(wxEVT_TEXT, &MainFrame::OnDoSearch, this, BC_Search);
    Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &MainFrame::OnDoSearch, this, BC_Search);
    Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &MainFrame::OnCancelSearch, this, BC_Search);

    m_Hives->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(MainFrame::OnDragAndDropToHives), NULL, this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, &MainFrame::OnShowHivesContextMenu, this, BC_Hives);
    Bind(wxEVT_DATAVIEW_ITEM_START_EDITING, &MainFrame::OnHiveStartEditing, this, BC_Hives);
    Bind(wxEVT_BUTTON, &MainFrame::OnClickAddHive, this, BC_HiveAdd);
    Bind(wxEVT_BUTTON, &MainFrame::OnClickRemoveHive, this, BC_HiveRemove);

    Bind(SampleHive::SH_EVT_LOOP_POINTS_UPDATED, &MainFrame::OnRecieveLoopPoints, this);
    Bind(SampleHive::SH_EVT_STATUSBAR_STATUS_PUSH, &MainFrame::OnRecievePushStatusBarStatus, this);
    Bind(SampleHive::SH_EVT_STATUSBAR_STATUS_POP, &MainFrame::OnRecievePopStatusBarStatus, this);
    Bind(SampleHive::SH_EVT_STATUSBAR_STATUS_SET, &MainFrame::OnRecieveSetStatusBarStatus, this);
    Bind(SampleHive::SH_EVT_INFOBAR_MESSAGE_SHOW, &MainFrame::OnRecieveInfoBarStatus, this);
    Bind(SampleHive::SH_EVT_TIMER_STOP, &MainFrame::OnRecieveTimerStopStatus, this);

    // Adding widgets to their sizers
    m_MainSizer->Add(m_MainPanel, 1, wxALL | wxEXPAND, 0);

    m_TopSizer->Add(m_TopSplitter, 1, wxALL | wxEXPAND, 0);

    m_BrowserControlSizer->Add(m_PlayButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_BrowserControlSizer->Add(m_StopButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_BrowserControlSizer->Add(m_LoopButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_BrowserControlSizer->Add(m_LoopABButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_BrowserControlSizer->Add(m_SettingsButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_BrowserControlSizer->Add(0,0,1, wxALL | wxEXPAND, 0);
    m_BrowserControlSizer->Add(m_SamplePosition, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_BrowserControlSizer->Add(30,0,0, wxALL | wxEXPAND, 0);
    m_BrowserControlSizer->Add(m_MuteButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_BrowserControlSizer->Add(m_VolumeSlider, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_BrowserControlSizer->Add(m_AutoPlayCheck, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_TopPanelMainSizer->Add(m_TopWaveformPanel, 1, wxALL | wxEXPAND, 2);
    m_TopPanelMainSizer->Add(m_TopControlsPanel, 0, wxALL | wxEXPAND, 2);

    m_BottomLeftPanelMainSizer->Add(m_Notebook, 1, wxALL | wxEXPAND, 0);

    m_HivesFavoritesSizer->Add(m_Hives, 1, wxALL | wxEXPAND, 0);

    m_HivesButtonSizer->Add(m_AddHiveButton, 1, wxALL | wxEXPAND, 0);
    m_HivesButtonSizer->Add(m_RemoveHiveButton, 1, wxALL | wxEXPAND, 0);

    m_HivesMainSizer->Add(m_HivesFavoritesSizer, 1, wxALL | wxEXPAND, 0);
    m_HivesMainSizer->Add(m_HivesButtonSizer, 0, wxALL | wxEXPAND, 0);

    m_TrashItemSizer->Add(m_Trash, 1, wxALL | wxEXPAND, 0);
    m_TrashButtonSizer->Add(m_RestoreTrashedItemButton, 1, wxALL | wxEXPAND, 0);

    m_TrashMainSizer->Add(m_TrashItemSizer, 1, wxALL | wxEXPAND, 0);
    m_TrashMainSizer->Add(m_TrashButtonSizer, 0, wxALL | wxEXPAND, 0);

    m_BottomRightPanelMainSizer->Add(m_SearchBox, 1, wxALL | wxEXPAND, 2);
    m_BottomRightPanelMainSizer->Add(m_InfoBar, 0, wxALL | wxEXPAND, 0);
    m_BottomRightPanelMainSizer->Add(m_Library, 1, wxALL | wxEXPAND, 0);

    // Sizer for the frame
    this->SetSizer(m_MainSizer);
    this->Layout();
    this->Center(wxBOTH);

    // Sizer for the main panel
    m_MainPanel->SetSizer(m_TopSizer);
    m_TopSizer->Fit(m_MainPanel);
    m_TopSizer->SetSizeHints(m_MainPanel);
    m_TopSizer->Layout();

    m_TopControlsPanel->SetSizer(m_BrowserControlSizer);
    m_BrowserControlSizer->Fit(m_TopControlsPanel);
    m_BrowserControlSizer->SetSizeHints(m_TopControlsPanel);
    m_BrowserControlSizer->Layout();

    m_TopWaveformPanel->SetSizer(m_WaveformDisplaySizer);
    m_WaveformDisplaySizer->Fit(m_TopWaveformPanel);
    m_WaveformDisplaySizer->SetSizeHints(m_TopWaveformPanel);
    m_WaveformDisplaySizer->Layout();

    // Sizer for TopPanel
    m_TopPanel->SetSizer(m_TopPanelMainSizer);
    m_TopPanelMainSizer->Fit(m_TopPanel);
    m_TopPanelMainSizer->SetSizeHints(m_TopPanel);
    m_TopPanelMainSizer->Layout();

    // Sizer for bottom left panel
    m_BottomLeftPanel->SetSizer(m_BottomLeftPanelMainSizer);
    m_BottomLeftPanelMainSizer->Fit(m_BottomLeftPanel);
    m_BottomLeftPanelMainSizer->SetSizeHints(m_BottomLeftPanel);
    m_BottomLeftPanelMainSizer->Layout();

    // Sizer for Hives page for wxNotebook
    m_HivesPanel->SetSizer(m_HivesMainSizer);
    m_HivesMainSizer->Fit(m_HivesPanel);
    m_HivesMainSizer->SetSizeHints(m_HivesPanel);
    m_HivesMainSizer->Layout();

    // Sizer for trash pane
    m_TrashPanel->SetSizer(m_TrashMainSizer);
    m_TrashMainSizer->Fit(m_TrashPanel);
    m_TrashMainSizer->SetSizeHints(m_TrashPanel);
    m_TrashMainSizer->Layout();

    // Sizer for bottom right panel
    m_BottomRightPanel->SetSizer(m_BottomRightPanelMainSizer);
    m_BottomRightPanelMainSizer->Fit(m_BottomRightPanel);
    m_BottomRightPanelMainSizer->SetSizeHints(m_BottomRightPanel);
    m_BottomRightPanelMainSizer->Layout();

    // Restore the data previously added to Library
    //LoadDatabase();

    // Set some properites after the frame has been created
    CallAfter(&MainFrame::SetAfterFrameCreate);
}

void MainFrame::OnClickSettings(wxCommandEvent& event)
{
    Settings* settings = new Settings(this);

    switch (settings->ShowModal())
    {
        case wxID_OK:
            if (settings->CanAutoImport())
            {
                OnAutoImportDir(settings->GetImportDirPath());
                RefreshDatabase();
            }
            if (settings->IsWaveformColourChanged())
            {
                m_TopWaveformPanel->ResetDC();
            }
            break;
        case wxID_CANCEL:
            break;
        default:
            return;
    }
}

void MainFrame::AddSamples(wxArrayString& files)
{
    Serializer serializer;
    
    wxBusyCursor busy_cursor;
    wxWindowDisabler window_disabler;

    wxProgressDialog* progressDialog = new wxProgressDialog(_("Adding files.."),
                                                            _("Adding files, please wait..."),
                                                            static_cast<int>(files.size()), this,
                                                            wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT |
                                                            wxPD_AUTO_HIDE);
    progressDialog->CenterOnParent(wxBOTH);

    std::vector<Sample> sample_array;

    std::string path;
    std::string artist;
    std::string filename_with_extension; 
    std::string filename_without_extension;
    std::string extension;
    std::string filename;
    
    //Check All Files At Once
    wxArrayString sorted_files;

    sorted_files = m_Database->CheckDuplicates(files);
    files = sorted_files;

    if(files.size() < 1)
    {
        progressDialog->Destroy();
        return;
    }

    progressDialog->SetRange(files.size());

    for(unsigned int i = 0; i < files.size(); i++) 
    {
        progressDialog->Update(i, wxString::Format(_("Getting Data For %s"), files[i].AfterLast('/')));

        if(progressDialog->WasCancelled()) 
        {
            progressDialog->Destroy();
            return;
        }

        path = files[i].ToStdString();
        filename_with_extension = files[i].AfterLast('/').ToStdString();
        filename_without_extension = files[i].AfterLast('/').BeforeLast('.').ToStdString();
        extension = files[i].AfterLast('.').ToStdString();

        filename = serializer.DeserializeShowFileExtension() ?
            filename_with_extension : filename_without_extension;
        
        Sample sample;

        sample.SetPath(path);
        sample.SetFilename(filename_without_extension);
        sample.SetFileExtension(extension);
        
        Tags tags(path);

        artist = tags.GetAudioInfo().artist.ToStdString();

        sample.SetSamplePack(artist);
        sample.SetChannels(tags.GetAudioInfo().channels);
        sample.SetLength(tags.GetAudioInfo().length);
        sample.SetSampleRate(tags.GetAudioInfo().sample_rate);
        sample.SetBitrate(tags.GetAudioInfo().bitrate);

        wxLongLong llLength = sample.GetLength();
        int total_min = static_cast<int>((llLength / 60000).GetValue());
        int total_sec = static_cast<int>(((llLength % 60000) / 1000).GetValue());

        wxVector<wxVariant> data;

        wxVariant icon = wxVariant(wxBitmap(ICON_STAR_EMPTY_16px));

        if (tags.IsFileValid())
        {
            data.clear();
            data.push_back(icon);
            data.push_back(filename);
            data.push_back(sample.GetSamplePack());
            data.push_back("");
            data.push_back(wxString::Format("%d", sample.GetChannels()));
            data.push_back(wxString::Format("%2i:%02i", total_min, total_sec));
            data.push_back(wxString::Format("%d", sample.GetSampleRate()));
            data.push_back(wxString::Format("%d", sample.GetBitrate()));
            data.push_back(path);
    
            SH_LOG_INFO("Adding file: {}, Extension: {}", sample.GetFilename(), sample.GetFileExtension());
    
            m_Library->AppendItem(data);

            sample_array.push_back(sample);
        }
        else
        {
            wxString msg = wxString::Format(_("Error! Cannot open %s, Invalid file type."),
                                            filename_with_extension);
            m_InfoBar->ShowMessage(msg, wxICON_ERROR);
        }
    }

    progressDialog->Pulse(_("Updating Database.."), NULL);

    m_Database->InsertIntoSamples(sample_array);

    progressDialog->Destroy();
}

void MainFrame::OnClickDirCtrl(wxCommandEvent& event)
{
    wxArrayString path;
    path.push_back(m_DirCtrl->GetFilePath());
    
    AddSamples(path);
}

void MainFrame::OnDragAndDropToLibrary(wxDropFilesEvent& event)
{
    SH_LOG_DEBUG("Start Inserting Samples");

    if (event.GetNumberOfFiles() > 0)
    {
        wxString* dropped = event.GetFiles();
        wxASSERT(dropped);

        wxBusyCursor busy_cursor;
        wxWindowDisabler window_disabler;

        wxString name;
        wxString filepath;
        wxArrayString filepath_array;

        wxProgressDialog* progressDialog = new wxProgressDialog(_("Reading files.."),
                                                                _("Reading files, please wait..."),
                                                                event.GetNumberOfFiles(), this,
                                                                wxPD_APP_MODAL | wxPD_SMOOTH |
                                                                wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

        progressDialog->CenterOnParent(wxBOTH);

        wxYield();

        for (int i = 0; i < event.GetNumberOfFiles(); i++)
        {
            filepath = dropped[i];

            if (wxFileExists(filepath))
            {
                filepath_array.push_back(filepath);
            }
            else if (wxDirExists(filepath))
            {
                wxDir::GetAllFiles(filepath, &filepath_array);
            }

            progressDialog->Pulse(_("Reading Samples"), NULL);
        }

        progressDialog->Destroy();

        AddSamples(filepath_array);

        SH_LOG_DEBUG("Done Inserting Samples");
    }
}

void MainFrame::OnDragAndDropToHives(wxDropFilesEvent& event)
{
    Serializer serializer;

    if (event.GetNumberOfFiles() > 0)
    {
        wxFileDataObject file_data;
        wxArrayString files;

        wxDataViewItemArray items;
        int rows = m_Library->GetSelections(items);

        wxDataViewItem drop_target;;
        wxDataViewColumn* column;
        wxPoint position = event.GetPosition();

        m_Hives->HitTest(position, drop_target, column);

        wxString hive_name = m_Hives->GetItemText(drop_target);

        wxString msg;

        for (int i = 0; i < rows; i++)
        {
            int row = m_Library->ItemToRow(items[i]);

            wxString name = m_Library->GetTextValue(row, 1);

            file_data.AddFile(name);

            files = file_data.GetFilenames();

            wxString file_name = serializer.DeserializeShowFileExtension() ?
                files[i].BeforeLast('.') : files[i];

            SH_LOG_DEBUG("Dropping {} file(s) {} on {}", rows - i, files[i], m_Hives->GetItemText(drop_target));

            if (drop_target.IsOk() && m_Hives->IsContainer(drop_target) &&
                m_Database->GetFavoriteColumnValueByFilename(file_name.ToStdString()) == 0)
            {
                m_Hives->AppendItem(drop_target, files[i]);

                m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_FILLED_16px)), row, 0);

                m_Database->UpdateFavoriteColumn(file_name.ToStdString(), 1);
                m_Database->UpdateHiveName(file_name.ToStdString(), hive_name.ToStdString());

                msg = wxString::Format(_("%s added to %s."), files[i], hive_name);
            }
            else
            {
                if (m_Database->GetFavoriteColumnValueByFilename(file_name.ToStdString()) == 1)
                {
                    wxMessageBox(wxString::Format(_("%s is already added to %s hive"), files[i],
                                                  m_Database->GetHiveByFilename(file_name.ToStdString())),
                                 _("Error!"), wxOK | wxICON_ERROR | wxCENTRE, this);
                }
                else
                {
                    if (m_Hives->GetItemText(drop_target) == "")
                        wxMessageBox(_("Cannot drop item outside of a hive, try dropping on a hive."),
                                     _("Error!"), wxOK | wxICON_ERROR | wxCENTRE, this);
                    else
                        wxMessageBox(wxString::Format(_("%s is not a hive, try dropping on a hive."),
                                                      m_Hives->GetItemText(drop_target)), _("Error!"),
                                     wxOK | wxICON_ERROR | wxCENTRE, this);
                }
            }

            if (!msg.IsEmpty())
                m_InfoBar->ShowMessage(msg, wxICON_ERROR);
        }
    }
}

void MainFrame::OnDirTreeOpenAutoImport(wxTreeEvent& event) {
    

    
    wxBusyCursor busy_cursor;
    wxWindowDisabler window_disabler;

    wxString filepath;
    wxArrayString filepath_array;

    const wxString pathToDirectory = m_DirCtrl->GetPath(event.GetItem());


    size_t number_of_files = wxDir::GetAllFiles(pathToDirectory, &filepath_array,
                                                wxEmptyString, wxDIR_FILES);

    // Delete all Files
    m_Database->DeleteAllSamples();
    m_Library->DeleteAllItems();
    AddSamples(filepath_array);
}

void MainFrame::OnAutoImportDir(const wxString& pathToDirectory)
{
    SH_LOG_DEBUG("Start Importing Samples");

    wxBusyCursor busy_cursor;
    wxWindowDisabler window_disabler;

    wxString filepath;
    wxArrayString filepath_array;

    size_t number_of_files = wxDir::GetAllFiles(pathToDirectory, &filepath_array,
                                                wxEmptyString, wxDIR_DEFAULT);

    wxProgressDialog* progressDialog = new wxProgressDialog(_("Adding files.."),
                                                            _("Adding files, please wait..."),
                                                            static_cast<int>(number_of_files), this,
                                                            wxPD_APP_MODAL | wxPD_SMOOTH |
                                                            wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

    progressDialog->CenterOnParent(wxBOTH);

    for (size_t i = 0; i < number_of_files; i++)
    {
        filepath = filepath_array[i];

        if (wxFileExists(filepath))
        {
            filepath_array.push_back(filepath);
        }
        else if (wxDirExists(filepath))
        {
            wxDir::GetAllFiles(filepath, &filepath_array);
        }

        progressDialog->Pulse(_("Reading Samples"), NULL);
    }

    progressDialog->Destroy();

    //AddSamples(filepath_array);

    SH_LOG_DEBUG("Done Importing Samples");
}

// Temporary function to check drag and drop result
void LogDragResult(wxDragResult result)
{
    wxString msg;
    switch (result)
    {
       case wxDragError:   msg = "Error!";    break;
       case wxDragNone:    msg = "Nothing";   break;
       case wxDragCopy:    msg = "Copied";    break;
       case wxDragMove:    msg = "Moved";     break;
       case wxDragCancel:  msg = "Cancelled"; break;
       default:            msg = "Huh?";      break;
    }

    SH_LOG_DEBUG("Drag result: {}", msg);
}

void MainFrame::OnDragFromDirCtrl(wxTreeEvent& event)
{
    wxFileDataObject file_data;
    file_data.AddFile(m_DirCtrl->GetPath(event.GetItem()));

    wxDropSource drop_source(this);
    drop_source.SetData(file_data);

    LogDragResult(drop_source.DoDragDrop());
}

void MainFrame::OnDragFromLibrary(wxDataViewEvent& event)
{
    int selected_row = m_Library->ItemToRow(event.GetItem());

    if (selected_row < 0)
        return;

    wxString selection = m_Library->GetTextValue(selected_row, 1);

    wxString sample_path = GetFilenamePathAndExtension(selection).Path;

    wxFileDataObject* fileData = new wxFileDataObject();

    fileData->AddFile(sample_path);
    event.SetDataObject(fileData);

    SH_LOG_DEBUG("Started dragging '{}'.", sample_path);
}

void MainFrame::OnClickPlay(wxCommandEvent& event)
{
    bStopped = false;

    int selected_row = m_Library->GetSelectedRow();

    if (selected_row < 0)
        return;

    wxString selection = m_Library->GetTextValue(selected_row, 1);

    wxString sample_path = GetFilenamePathAndExtension(selection).Path;

    if (bLoopPointsSet && m_LoopABButton->GetValue())
        PlaySample(sample_path.ToStdString(), selection.ToStdString(), true, m_LoopA.ToDouble(), wxFromStart);
    else
        PlaySample(sample_path.ToStdString(), selection.ToStdString());
}

void MainFrame::OnClickLoop(wxCommandEvent& event)
{
    Serializer serializer;

    bLoop = m_LoopButton->GetValue();

    serializer.SerializeMediaOptions("loop", bLoop);
}

void MainFrame::OnClickStop(wxCommandEvent& event)
{
    m_MediaCtrl->Stop();

    bStopped = true;

    if (m_Timer->IsRunning())
        m_Timer->Stop();

    m_SamplePosition->SetLabel("--:--/--:--");

    this->SetStatusText(_("Stopped"), 1);
}

void MainFrame::OnClickMute(wxCommandEvent& event)
{
    Serializer serializer;

    if (m_MuteButton->GetValue())
    {
        m_MediaCtrl->SetVolume(0.0);
        bMuted = true;

        serializer.SerializeMediaOptions("muted", bMuted);
    }
    else
    {
        m_MediaCtrl->SetVolume(double(m_VolumeSlider->GetValue()) / 100);
        bMuted = false;

        serializer.SerializeMediaOptions("muted", bMuted);
    }
}

void MainFrame::OnMediaFinished(wxMediaEvent& event)
{
    if (bLoop)
    {
        if (!m_MediaCtrl->Play())
        {
            wxMessageDialog msgDialog(NULL, _("Error! Cannot loop media."), _("Error"), wxOK | wxICON_ERROR);
            msgDialog.ShowModal();
        }
        else
            m_MediaCtrl->Play();
    }
    else
    {
        if (m_Timer->IsRunning())
        {
            m_Timer->Stop();
            SH_LOG_DEBUG("TIMER STOPPED");
        }

        m_SamplePosition->SetLabel("--:--/--:--");
        PopStatusText(1);
        this->SetStatusText(_("Stopped"), 1);
    }
}

void MainFrame::UpdateElapsedTime(wxTimerEvent& event)
{
    SH_LOG_DEBUG("TIMER IS RUNNING..");

    wxString duration, position;
    wxLongLong llLength, llTell;

    llLength = m_MediaCtrl->Length();
    int total_min = static_cast<int>((llLength / 60000).GetValue());
    int total_sec = static_cast<int>(((llLength % 60000) / 1000).GetValue());

    llTell = m_MediaCtrl->Tell();
    int current_min = static_cast<int>((llTell / 60000).GetValue());
    int current_sec = static_cast<int>(((llTell % 60000) / 1000).GetValue());

    duration.Printf(wxT("%2i:%02i"), total_min, total_sec);
    position.Printf(wxT("%2i:%02i"), current_min, current_sec);

    m_SamplePosition->SetLabel(wxString::Format(wxT("%s/%s"), position.c_str(), duration.c_str()));

    m_TopControlsPanel->Refresh();
    m_TopWaveformPanel->Refresh();

    if (bLoopPointsSet && m_LoopABButton->GetValue())
        if (static_cast<double>(m_MediaCtrl->Tell()) >= m_LoopB.ToDouble())
            m_MediaCtrl->Seek(m_LoopA.ToDouble(), wxFromStart);
}

void MainFrame::OnCheckAutoplay(wxCommandEvent& event)
{
    Serializer serializer;

    if (m_AutoPlayCheck->GetValue())
    {
        bAutoplay = true;

        serializer.SerializeMediaOptions("autoplay", bAutoplay);
    }
    else
    {
        bAutoplay = false;

        serializer.SerializeMediaOptions("autoplay", bAutoplay);
    }
}

void MainFrame::OnSlideVolume(wxScrollEvent& event)
{
    m_MediaCtrl->SetVolume(double(m_VolumeSlider->GetValue()) / 100);

    PushStatusText(wxString::Format(_("Volume: %d"), m_VolumeSlider->GetValue()), 1);
}

void MainFrame::OnReleaseVolumeSlider(wxScrollEvent& event)
{
    Serializer serializer;

    serializer.SerializeMediaVolume(m_VolumeSlider->GetValue());

    int selected_row = m_Library->GetSelectedRow();

    if (selected_row < 0)
        return;

    wxString selection = m_Library->GetTextValue(selected_row, 1);

    // Wait a second then remove the status from statusbar
    wxSleep(1);
    PopStatusText(1);

    if (m_MediaCtrl->GetState() == wxMEDIASTATE_STOPPED)
        this->SetStatusText(_("Stopped"), 1);
    else
        PushStatusText(wxString::Format(_("Now playing: %s"), selection), 1);
}

void MainFrame::OnClickLibrary(wxDataViewEvent& event)
{
    int selected_row = m_Library->ItemToRow(event.GetItem());
    int current_row = m_Library->ItemToRow(m_Library->GetCurrentItem());

    if (selected_row < 0 || !event.GetItem().IsOk())
        return;

    if (selected_row != current_row)
    {
        m_Library->SetCurrentItem(event.GetItem());
        return;
    }

    // Update the waveform bitmap
    m_TopWaveformPanel->ResetDC();

    m_LoopABButton->SetValue(false);

    if (m_Timer->IsRunning())
    {
        m_Timer->Stop();
        SH_LOG_DEBUG("TIMER STOPPED");
    }

    wxString selection = m_Library->GetTextValue(selected_row, 1);

    // Get curremt column
    wxDataViewColumn* CurrentColumn = m_Library->GetCurrentColumn();

    // Get favorite column
    wxDataViewColumn* FavoriteColumn = m_Library->GetColumn(0);

    if (!CurrentColumn)
        return;

    wxString sample_path = GetFilenamePathAndExtension(selection).Path;
    std::string filename = GetFilenamePathAndExtension(selection).Filename;
    std::string extension = GetFilenamePathAndExtension(selection).Extension;

    if (CurrentColumn != FavoriteColumn)
    {
        ClearLoopPoints();

        if (bAutoplay)
        {
            if (bLoopPointsSet && m_LoopABButton->GetValue())
                PlaySample(sample_path.ToStdString(), selection.ToStdString(),
                           true, m_LoopA.ToDouble(), wxFromStart);
            else
                PlaySample(sample_path.ToStdString(), selection.ToStdString());
        }
        else
            m_MediaCtrl->Stop();
    }
    else
    {
        wxString msg;

        // Get hive name and location
        std::string hive_name = m_Hives->GetItemText(favorites_hive).ToStdString();
        wxDataViewItem hive_selection = m_Hives->GetSelection();

        if (hive_selection.IsOk() && m_Hives->IsContainer(hive_selection))
            hive_name = m_Hives->GetItemText(hive_selection).ToStdString();

        wxString name = m_Library->GetTextValue(selected_row, 1);

        // Get root
        wxDataViewItem root = wxDataViewItem(wxNullPtr);
        wxDataViewItem container;
        wxDataViewItem child;

        if (m_Database->GetFavoriteColumnValueByFilename(filename) == 0)
        {
            m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_FILLED_16px)), selected_row, 0);

            m_Database->UpdateFavoriteColumn(filename, 1);
            m_Database->UpdateHiveName(filename, hive_name);

            for (int i = 0; i < m_Hives->GetChildCount(root); i++)
            {
                container = m_Hives->GetNthChild(root, i);

                if (m_Hives->GetItemText(container).ToStdString() == hive_name)
                {
                    m_Hives->AppendItem(container, name);
                    break;
                }
            }

            msg = wxString::Format(_("Added %s to %s"), name, hive_name);
        }
        else
        {
            m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_EMPTY_16px)), selected_row, 0);

            m_Database->UpdateFavoriteColumn(filename, 0);
            m_Database->UpdateHiveName(filename, m_Hives->GetItemText(favorites_hive).ToStdString());

            for (int i = 0; i < m_Hives->GetChildCount(root); i++)
            {
                container = m_Hives->GetNthChild(root, i);

                for (int j = 0; j < m_Hives->GetChildCount(container); j++)
                {
                    child = m_Hives->GetNthChild(container, j);

                    if (m_Hives->GetItemText(child) == name)
                    {
                        m_Hives->DeleteItem(child);
                        break;
                    }
                }
            }

            msg = wxString::Format(_("Removed %s from %s"), name, hive_name);
        }

        if (!msg.IsEmpty())
            m_InfoBar->ShowMessage(msg, wxICON_INFORMATION);
    }
}

void MainFrame::OnShowHivesContextMenu(wxDataViewEvent& event)
{
    Serializer serializer;

    wxDataViewItem selected_hive = event.GetItem();

    wxString hive_name = m_Hives->GetItemText(selected_hive);

    wxMenu menu;

    if (m_Hives->IsContainer(selected_hive))
    {
        // Container menu items
        menu.Append(MN_RenameHive, _("Rename hive"), _("Rename selected hive"));
        menu.Append(MN_DeleteHive, _("Delete hive"), _("Delete selected hive"));

        if (!bFiltered)
            menu.Append(MN_FilterLibrary, _("Filter library"),
                        _("Show only samples from current hive in library"));
        else
            menu.Append(MN_FilterLibrary, _("Clear filter"), _("Clear the filter"));
    }
    else
    {
        // Child menu items
        menu.Append(MN_RemoveSample, _("Remove sample"), _("Remove the selected sample(s)"));
        menu.Append(MN_ShowInLibrary, _("Show sample in library"), _("Show the selected in library"));
    }

    if (selected_hive.IsOk() && m_Hives->IsContainer(selected_hive))
    {
        switch (m_Hives->GetPopupMenuSelectionFromUser(menu, event.GetPosition()))
        {
            case MN_RenameHive:
            {
                std::deque<wxDataViewItem> nodes;
                nodes.push_back(m_Hives->GetNthChild(wxDataViewItem(wxNullPtr), 0));

                wxDataViewItem current_item, found_item;

                int row = 0;
                int hive_count = m_Hives->GetChildCount(wxDataViewItem(wxNullPtr));

                wxString msg;

                wxTextEntryDialog renameEntry(this, _("Enter new name"), wxGetTextFromUserPromptStr,
                                              wxEmptyString, wxTextEntryDialogStyle, wxDefaultPosition);

                renameEntry.SetTextValidator(wxFILTER_EMPTY);

                switch (renameEntry.ShowModal())
                {
                    case wxID_OK:
                    {
                        wxString hive_name = renameEntry.GetValue();

                        while(!nodes.empty())
                        {
                            current_item = nodes.front();
                            nodes.pop_front();

                            if (m_Hives->GetItemText(current_item) == hive_name)
                            {
                                found_item = current_item;
                                SH_LOG_DEBUG("Found item: {}", m_Hives->GetItemText(current_item));
                                break;
                            }

                            wxDataViewItem child = m_Hives->GetNthChild(wxDataViewItem(wxNullPtr), 0);

                            while (row < (hive_count - 1))
                            {
                                row ++;

                                child = m_Hives->GetNthChild(wxDataViewItem(wxNullPtr), row);
                                nodes.push_back(child);
                            }
                        }

                        nodes.clear();

                        if (found_item.IsOk())
                        {
                            wxMessageBox(wxString::Format(_("Another hive by the name %s already exist. "
                                                            "Please try with a different name."),
                                                          hive_name),
                                         _("Error!"), wxOK | wxCENTRE, this);
                        }
                        else
                        {
                            wxString selected_hive_name = m_Hives->GetItemText(selected_hive);

                            int sample_count = m_Hives->GetChildCount(selected_hive);

                            if (sample_count <= 0)
                            {
                                m_Hives->SetItemText(selected_hive, hive_name);
                                m_Database->UpdateHive(selected_hive_name.ToStdString(),
                                                       hive_name.ToStdString());
                            }
                            else
                            {
                                for (int i = 0; i < sample_count; i++)
                                {
                                    wxDataViewItem sample_item = m_Hives->GetNthChild(selected_hive, i);

                                    wxString sample_name = serializer.DeserializeShowFileExtension() ?
                                        m_Hives->GetItemText(sample_item).BeforeLast('.') :
                                        m_Hives->GetItemText(sample_item);

                                    m_Database->UpdateHiveName(sample_name.ToStdString(),
                                                               hive_name.ToStdString());
                                    m_Database->UpdateHive(selected_hive_name.ToStdString(),
                                                           hive_name.ToStdString());

                                    m_Hives->SetItemText(selected_hive, hive_name);
                                }
                            }

                            msg = wxString::Format(_("Successfully changed hive name to %s."), hive_name);
                        }
                    }
                    break;
                    case wxID_CANCEL:
                        break;
                    default:
                        return;
                }

                if (!msg.IsEmpty())
                    m_InfoBar->ShowMessage(msg, wxICON_INFORMATION);
            }
            break;
            case MN_DeleteHive:
            {
                wxString msg;

                wxMessageDialog deleteEmptyHiveDialog(this, wxString::Format(_("Are you sure you want to "
                                                                               "delete %s from hives?"),
                                                                             hive_name),
                                                      wxMessageBoxCaptionStr,
                                                      wxYES_NO | wxNO_DEFAULT |
                                                      wxICON_QUESTION | wxSTAY_ON_TOP);

                wxMessageDialog deleteFilledHiveDialog(this, wxString::Format(_("Are you sure you want to "
                                                                                "delete %s and all samples "
                                                                                "inside %s from hives?"),
                                                                              hive_name, hive_name),
                                                       wxMessageBoxCaptionStr,
                                                       wxYES_NO | wxNO_DEFAULT |
                                                       wxICON_QUESTION | wxSTAY_ON_TOP);

                if (hive_name == m_Hives->GetItemText(favorites_hive))
                {
                    wxMessageBox(wxString::Format(_("Error! Default hive %s cannot be deleted."), hive_name),
                                 _("Error!"), wxOK | wxCENTRE, this);
                    return;
                }
                else if (!selected_hive.IsOk())
                {
                    wxMessageBox(_("No hive selected, try selecting a hive first"), _("Error!"),
                                 wxOK | wxCENTRE, this);
                    return;
                }
                else if (selected_hive.IsOk() && !m_Hives->IsContainer(selected_hive))
                {
                    wxMessageBox(wxString::Format(_("Error! %s is not a hive, cannot delete from hives."),
                                                  hive_name),
                                 _("Error!"), wxOK | wxCENTRE, this);
                    return;
                }

                if(m_Hives->GetChildCount(selected_hive) <= 0)
                {
                    switch (deleteEmptyHiveDialog.ShowModal())
                    {
                        case wxID_YES:
                            if (selected_hive.IsOk() && m_Hives->IsContainer(selected_hive) &&
                                hive_name != m_Hives->GetItemText(favorites_hive))
                            {
                                m_Hives->DeleteItem(selected_hive);

                                m_Database->RemoveHiveFromDatabase(hive_name.ToStdString());

                                msg = wxString::Format(_("%s deleted from hives successfully."), hive_name);
                            }
                            break;
                        case wxID_NO:
                            break;
                        default:
                            return;
                    }
                }
                else
                {
                    switch (deleteFilledHiveDialog.ShowModal())
                    {
                        case wxID_YES:
                            if (selected_hive.IsOk() && m_Hives->IsContainer(selected_hive) &&
                                hive_name != m_Hives->GetItemText(favorites_hive))
                            {
                                wxDataViewItem child_item;

                                for (int i = 0; i < m_Library->GetItemCount(); i++)
                                {
                                    wxString matched_sample =
                                        serializer.DeserializeShowFileExtension() ?
                                        m_Library->GetTextValue(i, 1).BeforeLast('.') :
                                        m_Library->GetTextValue(i, 1);

                                    for (int j = 0; j < m_Hives->GetChildCount(selected_hive); j++)
                                    {
                                        child_item = m_Hives->GetNthChild(selected_hive, j);

                                        wxString child_name =
                                            serializer.DeserializeShowFileExtension() ?
                                            m_Hives->GetItemText(child_item).BeforeLast('.') :
                                            m_Hives->GetItemText(child_item);

                                        if (child_name == matched_sample)
                                        {
                                            SH_LOG_DEBUG("Found match");

                                            m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_EMPTY_16px)),
                                                                i, 0);

                                            m_Database->UpdateFavoriteColumn(matched_sample.ToStdString(), 0);
                                            m_Database->UpdateHiveName(matched_sample.ToStdString(),
                                                                       m_Hives->GetItemText(favorites_hive).ToStdString());

                                            break;
                                        }
                                        else
                                            SH_LOG_DEBUG("No match found");
                                    }
                                }

                                m_Hives->DeleteChildren(selected_hive);
                                m_Hives->DeleteItem(selected_hive);

                                m_Database->RemoveHiveFromDatabase(hive_name.ToStdString());

                                msg = wxString::Format(_("%s and all samples inside %s "
                                                         "have been deleted from hives successfully."),
                                                       hive_name, hive_name);
                            }
                            break;
                        case wxID_NO:
                            break;
                        default:
                            return;
                    }
                }

                if (!msg.IsEmpty())
                    m_InfoBar->ShowMessage(msg, wxICON_INFORMATION);
            }
                break;
            case MN_FilterLibrary:
            {
                if (!bFiltered)
                {
                    try
                    {
                        const auto dataset = m_Database->FilterDatabaseByHiveName(hive_name.ToStdString(),
                                                                                  serializer.DeserializeShowFileExtension(),
                                                                                  ICON_STAR_FILLED_16px,
                                                                                  ICON_STAR_EMPTY_16px);

                        if (dataset.empty())
                        {
                            wxMessageBox(_("Error! Database is empty."), _("Error!"),
                                         wxOK | wxICON_ERROR | wxCENTRE, this);
                            return;
                        }
                        else
                        {
                            m_Library->DeleteAllItems();

                            for (auto data : dataset)
                            {
                                m_Library->AppendItem(data);
                            }
                        }
                    }
                    catch (...)
                    {
                        wxMessageBox(_("Error loading data, cannot filter sample view"), _("Error!"),
                                     wxOK | wxICON_ERROR | wxCENTRE, this);
                    }

                    bFiltered = true;
                }
                else
                {
                    try
                    {
                        const auto dataset = m_Database->FilterDatabaseBySampleName("", serializer.DeserializeShowFileExtension(),
                                                                                    ICON_STAR_FILLED_16px, ICON_STAR_EMPTY_16px);

                        if (dataset.empty())
                        {
                            wxMessageBox(_("Error! Database is empty."), _("Error!"),
                                         wxOK | wxICON_ERROR | wxCENTRE, this);
                        }
                        else
                        {
                            m_Library->DeleteAllItems();

                            for (auto data : dataset)
                            {
                                m_Library->AppendItem(data);
                            }
                        }
                    }
                    catch (...)
                    {
                        wxMessageBox(_("Error loading data, cannot filter sample view"), _("Error!"),
                                     wxOK | wxICON_ERROR | wxCENTRE, this);
                    }

                    bFiltered = false;
                }
            }
                break;
            default:
                return;
        }
    }
    else if (selected_hive.IsOk() && !m_Hives->IsContainer(selected_hive))
    {
        switch (m_Hives->GetPopupMenuSelectionFromUser(menu, event.GetPosition()))
        {
            case MN_RemoveSample:
                for(int i = 0; i < m_Library->GetItemCount(); i++)
                {
                    wxString matched_sample = serializer.DeserializeShowFileExtension() ?
                        m_Library->GetTextValue(i, 1).BeforeLast('.') :
                        m_Library->GetTextValue(i, 1);

                    wxString selected_sample_name = serializer.DeserializeShowFileExtension() ?
                        m_Hives->GetItemText(event.GetItem()).BeforeLast('.') :
                        m_Hives->GetItemText(event.GetItem());

                    if(selected_sample_name == matched_sample)
                    {
                        SH_LOG_DEBUG("Found match");

                        m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_EMPTY_16px)), i, 0);

                        m_Database->UpdateFavoriteColumn(matched_sample.ToStdString(), 0);
                        m_Database->UpdateHiveName(matched_sample.ToStdString(),
                                                   m_Hives->GetItemText(favorites_hive).ToStdString());

                        m_Hives->DeleteItem(selected_hive);

                        break;
                    }

                    m_InfoBar->ShowMessage(wxString::Format(_("Removed %s from %s"),
                                                            m_Hives->GetItemText(event.GetItem()),
                                                            m_Database->GetHiveByFilename(matched_sample.ToStdString())),
                                           wxICON_INFORMATION);
                }
                break;
            case MN_ShowInLibrary:
                for(int i = 0; i < m_Library->GetItemCount(); i++)
                {
                    wxString matched_sample = serializer.DeserializeShowFileExtension() ?
                        m_Library->GetTextValue(i, 1).BeforeLast('.') :
                        m_Library->GetTextValue(i, 1);

                    wxString selected_sample_name = serializer.DeserializeShowFileExtension() ?
                        m_Hives->GetItemText(event.GetItem()).BeforeLast('.') :
                        m_Hives->GetItemText(event.GetItem());

                    if(selected_sample_name == matched_sample)
                    {
                        SH_LOG_DEBUG("Found match");

                        wxDataViewItem matched_item = m_Library->RowToItem(i);

                        m_Library->UnselectAll();
                        m_Library->SelectRow(i);
                        m_Library->EnsureVisible(matched_item);

                        break;
                    }
                }
                break;
            default:
                return;
        }
    }
}

void MainFrame::OnShowLibraryContextMenu(wxDataViewEvent& event)
{
    TagEditor* tagEditor;
    Serializer serializer;

    wxString msg;

    wxDataViewItem item = event.GetItem();
    int selected_row;

    if (item.IsOk())
        selected_row = m_Library->ItemToRow(item);
    else
        return;

    wxString selection = m_Library->GetTextValue(selected_row, 1);

    wxString sample_path = GetFilenamePathAndExtension(selection).Path;
    std::string filename = GetFilenamePathAndExtension(selection).Filename;
    std::string extension = GetFilenamePathAndExtension(selection).Extension;

    wxMenu menu;

    //true = add false = remove
    bool favorite_add = false;

    if (m_Database->GetFavoriteColumnValueByFilename(filename) == 1)
        menu.Append(MN_FavoriteSample, _("Remove from hive"), _("Remove the selected sample(s) from hive"));
    else
    {
        menu.Append(MN_FavoriteSample, _("Add to hive"), _("Add selected sample(s) to hive"));
        favorite_add = true;
    }

    menu.Append(MN_DeleteSample, _("Delete"), _("Delete the selected sample(s) from database"));
    menu.Append(MN_TrashSample, _("Trash"), _("Send the selected sample(s) to trash"));

    if (m_Library->GetSelectedItemsCount() <= 1)
    {
        menu.Append(MN_EditTagSample, _("Edit tags"),
                    _("Edit the tags for the selected sample"))->Enable(true);
        menu.Append(MN_OpenFile, _("Open in file manager"),
                    _("Open the selected sample in system's file manager"))->Enable(true);
    }
    else
    {
        menu.Append(MN_EditTagSample, _("Edit tags"),
                    _("Edit the tags for the selected sample"))->Enable(false);
        menu.Append(MN_OpenFile, _("Open in file manager"),
                    _("Open the selected sample in system's file manager"))->Enable(false);
    }

    switch (m_Library->GetPopupMenuSelectionFromUser(menu, event.GetPosition()))
    {
        case MN_FavoriteSample:
        {
            std::string hive_name = m_Hives->GetItemText(favorites_hive).ToStdString();

            wxDataViewItem hive_selection = m_Hives->GetSelection();

            if (hive_selection.IsOk() && m_Hives->IsContainer(hive_selection))
                hive_name = m_Hives->GetItemText(hive_selection).ToStdString();

            wxDataViewItem root = wxDataViewItem(wxNullPtr);
            wxDataViewItem container;
            wxDataViewItem child;

            wxDataViewItemArray samples;
            int sample_count = m_Library->GetSelections(samples);
            int selected_row = 0;
            int db_status = 0;

            for (int k = 0; k < sample_count; k++)
            {
                selected_row = m_Library->ItemToRow(samples[k]);

                if (selected_row < 0)
                    continue;

                wxString name = m_Library->GetTextValue(selected_row, 1);

                filename = serializer.DeserializeShowFileExtension() ?
                    name.BeforeLast('.').ToStdString() : name.ToStdString();

                db_status = m_Database->GetFavoriteColumnValueByFilename(filename);

                // Aleady Added, Do Nothing
                if (favorite_add && db_status == 1)
                    continue;

                // Already Removed, Do Nothing
                if (!favorite_add && db_status == 0)
                    continue;

                // Add To Favorites
                if (favorite_add && db_status == 0)
                {
                    m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_FILLED_16px)), selected_row, 0);

                    m_Database->UpdateFavoriteColumn(filename, 1);
                    m_Database->UpdateHiveName(filename, hive_name);

                    for (int i = 0; i < m_Hives->GetChildCount(root); i++)
                    {
                        container = m_Hives->GetNthChild(root, i);

                        if (m_Hives->GetItemText(container).ToStdString() == hive_name)
                        {
                            m_Hives->AppendItem(container, name);

                            msg = wxString::Format(_("Added %s to %s"), name, hive_name);
                            break;
                        }
                    }
                }
                else
                {
                    //Remove From Favorites
                    m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_EMPTY_16px)), selected_row, 0);

                    m_Database->UpdateFavoriteColumn(filename, 0);
                    m_Database->UpdateHiveName(filename,
                                               m_Hives->GetItemText(favorites_hive).ToStdString());

                    for (int i = 0; i < m_Hives->GetChildCount(root); i++)
                    {
                        container = m_Hives->GetNthChild(root, i);

                        for (int j = 0; j < m_Hives->GetChildCount(container); j++)
                        {
                            child = m_Hives->GetNthChild(container, j);

                            if (m_Hives->GetItemText(child) == name)
                            {
                                m_Hives->DeleteItem(child);

                                msg = wxString::Format(_("Removed %s from %s"), name, hive_name);
                                break;
                            }
                        }
                    }
                }
            }
            break;
        }
        case MN_DeleteSample:
        {
            wxDataViewItemArray items;
            int rows = m_Library->GetSelections(items);

            wxMessageDialog singleMsgDialog(this, wxString::Format(_("Are you sure you want to delete "
                                                                     "%s from database? "
                                                                     "Warning this change is "
                                                                     "permanent, and cannot be undone."),
                                                                   sample_path.AfterLast('/')),
                                            wxMessageBoxCaptionStr,
                                            wxYES_NO | wxNO_DEFAULT |
                                            wxICON_QUESTION | wxSTAY_ON_TOP |
                                            wxCENTER);

            wxMessageDialog multipleMsgDialog(this, wxString::Format(_("Are you sure you want to delete "
                                                                       "%d selected samples from database? "
                                                                       "Warning this change is "
                                                                       "permanent, and cannot be "
                                                                       "undone."), rows),
                                              wxMessageBoxCaptionStr,
                                              wxYES_NO | wxNO_DEFAULT |
                                              wxICON_QUESTION | wxSTAY_ON_TOP |
                                              wxCENTER);

            wxDataViewItem root = wxDataViewItem(wxNullPtr);
            wxDataViewItem container;
            wxDataViewItem child;

            if (m_Library->GetSelectedItemsCount() <= 1)
            {
                switch (singleMsgDialog.ShowModal())
                {
                    case wxID_YES:
                    {
                        m_Database->RemoveSampleFromDatabase(filename);
                        m_Library->DeleteItem(selected_row);

                        for (int j = 0; j < m_Hives->GetChildCount(root); j++)
                        {
                            container = m_Hives->GetNthChild(root, j);

                            for (int k = 0; k < m_Hives->GetChildCount(container); k++)
                            {
                                child = m_Hives->GetNthChild(container, k);

                                wxString child_text = serializer.DeserializeShowFileExtension() ?
                                    m_Hives->GetItemText(child).BeforeLast('.') :
                                    m_Hives->GetItemText(child);

                                if (child_text == filename)
                                {
                                    m_Hives->DeleteItem(child);

                                    break;
                                }
                            }
                        }

                        msg = wxString::Format(_("Deleted %s from database successfully"), selection);
                    }
                    break;
                    case wxID_NO:
                        msg = _("Cancel delete");
                        break;
                    default:
                        wxMessageBox(_("Unexpected wxMessageDialog return code!"), _("Error!"),
                                     wxOK | wxICON_ERROR | wxCENTRE, this);
                }
            }
            else
            {
                switch (multipleMsgDialog.ShowModal())
                {
                    case wxID_YES:
                    {
                        for (int i = 0; i < rows; i++)
                        {
                            int row = m_Library->ItemToRow(items[i]);

                            wxString text_value = m_Library->GetTextValue(row, 1);

                            std::string multi_selection = serializer.DeserializeShowFileExtension() ?
                                text_value.BeforeLast('.').ToStdString() : text_value.ToStdString() ;

                            m_Database->RemoveSampleFromDatabase(multi_selection);
                            m_Library->DeleteItem(row);

                            for (int j = 0; j < m_Hives->GetChildCount(root); j++)
                            {
                                container = m_Hives->GetNthChild(root, j);

                                for (int k = 0; k < m_Hives->GetChildCount(container); k++)
                                {
                                    child = m_Hives->GetNthChild(container, k);

                                    wxString child_text = serializer.DeserializeShowFileExtension() ?
                                        m_Hives->GetItemText(child).BeforeLast('.') :
                                        m_Hives->GetItemText(child);

                                    if (child_text == multi_selection)
                                    {
                                        m_Hives->DeleteItem(child);
                                        break;
                                    }
                                }
                            }

                            msg = wxString::Format(_("Deleted %s from database successfully"), text_value);
                        }
                    }
                    break;
                    case wxID_NO:
                        msg = _("Cancel delete");
                        break;
                    default:
                        wxMessageBox(_("Unexpected wxMessageDialog return code!"), _("Error!"),
                                     wxOK | wxICON_ERROR | wxCENTRE, this);
                }
            }
        }
        break;
        case MN_TrashSample:
        {
            wxDataViewItem root = wxDataViewItem(wxNullPtr);
            wxDataViewItem container, child;

            if (m_Database->IsTrashed(filename))
                SH_LOG_INFO("{} already trashed", filename);
            else
            {
                wxDataViewItemArray items;
                int rows = m_Library->GetSelections(items);

                wxString name;
                wxFileDataObject file_data;
                wxArrayString files;

                for (int i = 0; i < rows; i++)
                {
                    int item_row = m_Library->ItemToRow(items[i]);

                    wxString text_value = m_Library->GetTextValue(item_row, 1);

                    std::string multi_selection = serializer.DeserializeShowFileExtension() ?
                        m_Library->GetTextValue(item_row, 1).BeforeLast('.').ToStdString() :
                        m_Library->GetTextValue(item_row, 1).ToStdString() ;

                    file_data.AddFile(multi_selection);

                    files = file_data.GetFilenames();

                    if (m_Database->GetFavoriteColumnValueByFilename(files[i].ToStdString()))
                    {
                        m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_EMPTY_16px)), item_row, 0);

                        m_Database->UpdateFavoriteColumn(files[i].ToStdString(), 0);

                        for (int j = 0; j < m_Hives->GetChildCount(root); j++)
                        {
                            container = m_Hives->GetNthChild(root, j);

                            for (int k = 0; k < m_Hives->GetChildCount(container); k++)
                            {
                                child = m_Hives->GetNthChild(container, k);

                                wxString child_text = serializer.DeserializeShowFileExtension() ?
                                    m_Hives->GetItemText(child).BeforeLast('.') :
                                    m_Hives->GetItemText(child);

                                if (child_text == files[i])
                                {
                                    m_Hives->DeleteItem(child);
                                    break;
                                }
                            }
                        }
                    }

                    m_Database->UpdateTrashColumn(files[i].ToStdString(), 1);
                    m_Database->UpdateHiveName(files[i].ToStdString(),
                                               m_Hives->GetItemText(favorites_hive).ToStdString());

                    m_Trash->AppendItem(trash_root, text_value);

                    m_Library->DeleteItem(item_row);

                    msg = wxString::Format(_("%s sent to trash"), text_value);
                }
            }
        }
        break;
        case MN_EditTagSample:
        {
            tagEditor = new TagEditor(this, static_cast<std::string>(sample_path));

            switch (tagEditor->ShowModal())
            {
                case wxID_OK:
                    SH_LOG_DEBUG("tags dialog ok, Return code: {}", tagEditor->GetReturnCode());
                    break;
                case wxID_APPLY:
                    SH_LOG_DEBUG("tags dialog apply, Return code: {}", tagEditor->GetReturnCode());
                    break;
                case wxID_CANCEL:
                    SH_LOG_DEBUG("tags dialog cancel, Return code: {}", tagEditor->GetReturnCode());
                    break;
                default:
                    msg = _("Unexpected TagEditor return code!");
            }
        }
        break;
        case MN_OpenFile:
            wxExecute(wxString::Format("xdg-open '%s'", sample_path.BeforeLast('/')));
            break;
        case wxID_NONE:
            return;
        default:
            wxMessageBox(_("Unexpected wxMenu return code!"), _("Error!"),
                         wxOK | wxICON_ERROR | wxCENTRE, this);
            break;
    }

    if(!msg.IsEmpty())
        m_InfoBar->ShowMessage(msg);
}

void MainFrame::OnShowLibraryColumnHeaderContextMenu(wxDataViewEvent& event)
{
    wxMenu menu;

    wxDataViewColumn* FavoriteColumn = m_Library->GetColumn(0);
    wxDataViewColumn* FilenameColumn = m_Library->GetColumn(1);
    wxDataViewColumn* SamplePackColumn = m_Library->GetColumn(2);
    wxDataViewColumn* TypeColumn = m_Library->GetColumn(3);
    wxDataViewColumn* ChannelsColumn = m_Library->GetColumn(4);
    wxDataViewColumn* LengthColumn = m_Library->GetColumn(5);
    wxDataViewColumn* SampleRateColumn = m_Library->GetColumn(6);
    wxDataViewColumn* BitrateColumn = m_Library->GetColumn(7);
    wxDataViewColumn* PathColumn = m_Library->GetColumn(8);

    menu.AppendCheckItem(MN_ColumnFavorite, _("Favorites"),
                         _("Toggle favorites column"))->Check(FavoriteColumn->IsShown());
    menu.AppendCheckItem(MN_ColumnFilename, _("Filename"),
                         _("Toggle filename column"))->Check(FilenameColumn->IsShown());
    menu.AppendCheckItem(MN_ColumnSamplePack, _("Sample Pack"),
                         _("Toggle sample pack column"))->Check(SamplePackColumn->IsShown());
    menu.AppendCheckItem(MN_ColumnType, _("Type"),
                         _("Toggle type column"))->Check(TypeColumn->IsShown());
    menu.AppendCheckItem(MN_ColumnChannels, _("Channels"),
                         _("Toggle channels column"))->Check(ChannelsColumn->IsShown());
    menu.AppendCheckItem(MN_ColumnLength, _("Length"),
                         _("Toggle length column"))->Check(LengthColumn->IsShown());
    menu.AppendCheckItem(MN_ColumnSampleRate, _("Sample Rate"),
                         _("Toggle sample rate column"))->Check(SampleRateColumn->IsShown());
    menu.AppendCheckItem(MN_ColumnBitrate, _("Bitrate"),
                         _("Toggle bitrate column"))->Check(BitrateColumn->IsShown());
    menu.AppendCheckItem(MN_ColumnPath, _("Path"),
                         _("Toggle path column"))->Check(PathColumn->IsShown());

    switch (m_Library->GetPopupMenuSelectionFromUser(menu, event.GetPosition()))
    {
        case MN_ColumnFavorite:
            FavoriteColumn->SetHidden(!menu.IsChecked(MN_ColumnFavorite));
            break;
        case MN_ColumnFilename:
            FilenameColumn->SetHidden(!menu.IsChecked(MN_ColumnFilename));
            break;
        case MN_ColumnSamplePack:
            SamplePackColumn->SetHidden(!menu.IsChecked(MN_ColumnSamplePack));
            break;
        case MN_ColumnType:
            TypeColumn->SetHidden(!menu.IsChecked(MN_ColumnType));
            break;
        case MN_ColumnChannels:
            ChannelsColumn->SetHidden(!menu.IsChecked(MN_ColumnChannels));
            break;
        case MN_ColumnLength:
            LengthColumn->SetHidden(!menu.IsChecked(MN_ColumnLength));
            break;
        case MN_ColumnSampleRate:
            SampleRateColumn->SetHidden(!menu.IsChecked(MN_ColumnSampleRate));
            break;
        case MN_ColumnBitrate:
            BitrateColumn->SetHidden(!menu.IsChecked(MN_ColumnBitrate));
            break;
        case MN_ColumnPath:
            PathColumn->SetHidden(!menu.IsChecked(MN_ColumnPath));
            break;
        default:
            break;
    }
}

void MainFrame::LoadDatabase()
{
    Serializer serializer;

    try
    {
        const auto dataset = m_Database->LoadSamplesDatabase(*m_Hives, favorites_hive,
                                                             *m_Trash, trash_root,
                                                             serializer.DeserializeShowFileExtension(),
                                                             ICON_STAR_FILLED_16px, ICON_STAR_EMPTY_16px);

        if (dataset.empty())
            SH_LOG_INFO("Error! Database is empty.");
        else
        {
            for (auto data : dataset)
                m_Library->AppendItem(data);
        }

        m_Database->LoadHivesDatabase(*m_Hives);
    }
    catch(...)
    {
        std::cerr << "Error loading data." << std::endl;
    }
}

void MainFrame::OnShowTrashContextMenu(wxTreeEvent& event)
{
    Serializer serializer;

    wxTreeItemId selected_trashed_item = event.GetItem();

    wxMenu menu;

    menu.Append(MN_DeleteTrash, _("Delete from database"), _("Delete the selected sample(s) from database"));
    menu.Append(MN_RestoreTrashedItem, _("Restore sample"), _("Restore the selected sample(s) back to library"));

    if (selected_trashed_item.IsOk())
    {
        switch (m_Trash->GetPopupMenuSelectionFromUser(menu, event.GetPoint()))
        {
            case MN_DeleteTrash:
            {
                wxString trashed_item_name = serializer.DeserializeShowFileExtension() ?
                    m_Trash->GetItemText(selected_trashed_item).BeforeLast('.') :
                    m_Trash->GetItemText(selected_trashed_item);

                m_Database->RemoveSampleFromDatabase(trashed_item_name.ToStdString());

                m_Trash->Delete(selected_trashed_item);

                SH_LOG_INFO("{} deleted from trash and databse", trashed_item_name);
            }
            break;
            case MN_RestoreTrashedItem:
            {
                wxArrayTreeItemIds selected_item_ids;
                m_Trash->GetSelections(selected_item_ids);

                wxFileDataObject file_data;
                wxArrayString files;

                wxString selected_item_text;
                std::string filename;

                for (size_t i = 0; i < selected_item_ids.GetCount(); i++)
                {
                    selected_item_text = m_Trash->GetItemText(selected_item_ids[i]);

                    filename = GetFilenamePathAndExtension(selected_item_text).Filename;

                    file_data.AddFile(filename);

                    files = file_data.GetFilenames();

                    m_Database->UpdateTrashColumn(files[i].ToStdString(), 0);

                    try
                    {
                        wxVector<wxVector<wxVariant>> dataset;

                        if (m_Database->RestoreFromTrashByFilename(files[i].ToStdString(),
                                                                   dataset,
                                                                   serializer.DeserializeShowFileExtension(),
                                                                   ICON_STAR_FILLED_16px,
                                                                   ICON_STAR_EMPTY_16px).empty())
                        {
                            SH_LOG_INFO("Error! Database is empty.");
                        }
                        else
                        {
                            for (auto data : dataset)
                            {
                                m_Library->AppendItem(data);
                            }
                        }
                    }
                    catch (...)
                    {
                        std::cerr << "Error loading data." << std::endl;
                    }

                    m_Trash->Delete(selected_item_ids[i]);

                    SH_LOG_INFO("{} restored from trash", files[i]);
                }
            }
            break;
            default:
                break;
        }
    }
}

void MainFrame::OnDragAndDropToTrash(wxDropFilesEvent& event)
{
    Serializer serializer;

    if (event.GetNumberOfFiles() > 0)
    {
        wxFileDataObject file_data;
        wxArrayString files;

        wxDataViewItemArray items;
        int rows = m_Library->GetSelections(items);

        wxString msg;

        wxDataViewItem root = wxDataViewItem(wxNullPtr);
        wxDataViewItem container, child;

        for (int i = 0; i < rows; i++)
        {
            int item_row = m_Library->ItemToRow(items[i]);

            wxString text_value = m_Library->GetTextValue(item_row, 1);

            std::string multi_selection = serializer.DeserializeShowFileExtension() ?
                m_Library->GetTextValue(item_row, 1).BeforeLast('.').ToStdString() :
                m_Library->GetTextValue(item_row, 1).ToStdString() ;

            file_data.AddFile(multi_selection);

            files = file_data.GetFilenames();

            if (m_Database->GetFavoriteColumnValueByFilename(files[i].ToStdString()))
            {
                m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_EMPTY_16px)), item_row, 0);

                m_Database->UpdateFavoriteColumn(files[i].ToStdString(), 0);

                for (int j = 0; j < m_Hives->GetChildCount(root); j++)
                {
                    container = m_Hives->GetNthChild(root, j);

                    for (int k = 0; k < m_Hives->GetChildCount(container); k++)
                    {
                        child = m_Hives->GetNthChild(container, k);

                        wxString child_text = serializer.DeserializeShowFileExtension() ?
                            m_Hives->GetItemText(child).BeforeLast('.') :
                            m_Hives->GetItemText(child);

                        if (child_text == files[i])
                        {
                            m_Hives->DeleteItem(child);
                            break;
                        }
                    }
                }
            }

            m_Database->UpdateTrashColumn(files[i].ToStdString(), 1);
            m_Database->UpdateHiveName(files[i].ToStdString(),
                                       m_Hives->GetItemText(favorites_hive).ToStdString());

            m_Trash->AppendItem(trash_root, text_value);

            m_Library->DeleteItem(item_row);

            msg = wxString::Format(_("%s sent to trash"), text_value);
        }

        if (!msg.IsEmpty())
            m_InfoBar->ShowMessage(msg, wxICON_ERROR);
    }
}

void MainFrame::OnClickAddHive(wxCommandEvent& event)
{
    std::deque<wxDataViewItem> nodes;
    nodes.push_back(m_Hives->GetNthChild(wxDataViewItem(wxNullPtr), 0));

    wxDataViewItem current_item, found_item;

    int row = 0;
    int hive_count = m_Hives->GetChildCount(wxDataViewItem(wxNullPtr));

    wxString msg;

    wxTextEntryDialog hiveEntry(this, _("Enter hive name"), _("Create new hive"), wxEmptyString,
                                wxTextEntryDialogStyle, wxDefaultPosition);

    hiveEntry.SetTextValidator(wxFILTER_EMPTY);

    switch (hiveEntry.ShowModal())
    {
        case wxID_OK:
        {
            wxString hive_name = hiveEntry.GetValue();

            while(!nodes.empty())
            {
                current_item = nodes.front();
                nodes.pop_front();

                if (m_Hives->GetItemText(current_item) == hive_name)
                {
                    found_item = current_item;
                    SH_LOG_DEBUG("Found item: {}", m_Hives->GetItemText(current_item));
                    break;
                }

                wxDataViewItem child = m_Hives->GetNthChild(wxDataViewItem(wxNullPtr), 0);

                while (row < (hive_count - 1))
                {
                    row ++;

                    child = m_Hives->GetNthChild(wxDataViewItem(wxNullPtr), row);
                    nodes.push_back(child);
                }
            }

            nodes.clear();

            if (found_item.IsOk())
            {
                wxMessageBox(wxString::Format(_("Another hive by the name %s already exist. "
                                                "Please try with a different name."),
                                              hive_name),
                             _("Error!"), wxOK | wxCENTRE, this);
            }
            else
            {
                m_Hives->AppendContainer(wxDataViewItem(wxNullPtr), hive_name);
                m_Database->InsertIntoHives(hive_name.ToStdString());

                msg = wxString::Format(_("%s added to Hives."), hive_name);
            }
            break;
        }
        case wxID_CANCEL:
            break;
        default:
            return;
    }

    if (!msg.IsEmpty())
        m_InfoBar->ShowMessage(msg, wxICON_INFORMATION);
}

void MainFrame::OnClickRemoveHive(wxCommandEvent& event)
{
    Serializer serializer;

    wxDataViewItem selected_item = m_Hives->GetSelection();
    wxString hive_name = m_Hives->GetItemText(selected_item);

    wxString msg;

    wxMessageDialog deleteEmptyHiveDialog(this, wxString::Format(_("Are you sure you want to delete "
                                                                   "%s from hives?"),
                                                                 hive_name),
                                          wxMessageBoxCaptionStr,
                                          wxYES_NO | wxNO_DEFAULT |
                                          wxICON_QUESTION | wxSTAY_ON_TOP);

    wxMessageDialog deleteFilledHiveDialog(this, wxString::Format(_("Are you sure you want to delete "
                                                                    "%s and all sample inside %s from hives?"),
                                                                  hive_name, hive_name),
                                           wxMessageBoxCaptionStr,
                                           wxYES_NO | wxNO_DEFAULT |
                                           wxICON_QUESTION | wxSTAY_ON_TOP);

    if (hive_name == m_Hives->GetItemText(favorites_hive))
    {
        wxMessageBox(wxString::Format(_("Error! Default hive %s cannot be deleted."), hive_name),
                     _("Error!"), wxOK | wxCENTRE, this);
        return;
    }
    else if (!selected_item.IsOk())
    {
        wxMessageBox(_("No hive selected, try selecting a hive first"), _("Error!"), wxOK | wxCENTRE, this);
        return;
    }
    else if (selected_item.IsOk() && !m_Hives->IsContainer(selected_item))
    {
        wxMessageBox(wxString::Format(_("Error! %s is not a hive, cannot delete from hives."), hive_name),
                     _("Error!"), wxOK | wxCENTRE, this);
        return;
    }

    if (m_Hives->GetChildCount(selected_item) <= 0)
    {
        switch (deleteEmptyHiveDialog.ShowModal())
        {
            case wxID_YES:
                if (selected_item.IsOk() && m_Hives->IsContainer(selected_item) &&
                    hive_name != m_Hives->GetItemText(favorites_hive))
                {
                    m_Hives->DeleteItem(selected_item);

                    m_Database->RemoveHiveFromDatabase(hive_name.ToStdString());
                    msg = wxString::Format(_("%s deleted from hives successfully."), hive_name);
                }
                break;
            case wxID_NO:
                break;
            default:
                return;
        }
    }
    else
    {
        switch (deleteFilledHiveDialog.ShowModal())
        {
            case wxID_YES:
                if (selected_item.IsOk() && m_Hives->IsContainer(selected_item) &&
                    hive_name != m_Hives->GetItemText(favorites_hive))
                {
                    wxDataViewItem child_item;

                    for (int i = 0; i < m_Library->GetItemCount(); i++)
                    {
                        wxString matched_sample = serializer.DeserializeShowFileExtension() ?
                            m_Library->GetTextValue(i, 1).BeforeLast('.') :
                            m_Library->GetTextValue(i, 1);

                        for (int j = 0; j < m_Hives->GetChildCount(selected_item); j++)
                        {
                            child_item = m_Hives->GetNthChild(selected_item, j);

                            wxString child_name = serializer.DeserializeShowFileExtension() ?
                                m_Hives->GetItemText(child_item).BeforeLast('.') :
                                m_Hives->GetItemText(child_item);

                            if (child_name == matched_sample)
                            {
                                SH_LOG_DEBUG("Found match");

                                m_Library->SetValue(wxVariant(wxBitmap(ICON_STAR_EMPTY_16px)), i, 0);

                                m_Database->UpdateFavoriteColumn(matched_sample.ToStdString(), 0);
                                m_Database->UpdateHiveName(matched_sample.ToStdString(),
                                                           m_Hives->GetItemText(favorites_hive).ToStdString());

                                break;
                            }
                            else
                                SH_LOG_DEBUG("No match found");
                        }
                    }

                    m_Hives->DeleteChildren(selected_item);
                    m_Hives->DeleteItem(selected_item);

                    m_Database->RemoveHiveFromDatabase(hive_name.ToStdString());

                    msg = wxString::Format(_("%s and all samples inside %s have been deleted "
                                             "from hives successfully."),
                                           hive_name, hive_name);
                }
                break;
            case wxID_NO:
                break;
            default:
                return;
        }
    }

    if (!msg.IsEmpty())
        m_InfoBar->ShowMessage(msg, wxICON_INFORMATION);
}

void MainFrame::OnClickRestoreTrashItem(wxCommandEvent& event)
{
    Serializer serializer;

    wxArrayTreeItemIds selected_item_ids;
    m_Trash->GetSelections(selected_item_ids);

    wxFileDataObject file_data;
    wxArrayString files;

    wxString selected_item_text;
    std::string filename;

    if (m_Trash->GetChildrenCount(trash_root) == 0)
    {
        wxMessageBox(_("Trash is empty, nothing to restore!"), wxMessageBoxCaptionStr, wxOK | wxCENTRE, this);
        return;
    }

    if (selected_item_ids.IsEmpty())
    {
        wxMessageBox(_("No item selected, try selected a item first."), wxMessageBoxCaptionStr,
                     wxOK | wxCENTRE, this);
        return;
    }

    for (size_t i = 0; i < selected_item_ids.GetCount(); i++)
    {
        selected_item_text = m_Trash->GetItemText(selected_item_ids[i]);

        filename = GetFilenamePathAndExtension(selected_item_text).Filename;

        file_data.AddFile(filename);

        files = file_data.GetFilenames();

        m_Database->UpdateTrashColumn(files[i].ToStdString(), 0);

        try
        {
            wxVector<wxVector<wxVariant>> dataset;

            if (m_Database->RestoreFromTrashByFilename(files[i].ToStdString(), dataset,
                                                       serializer.DeserializeShowFileExtension(),
                                                       ICON_STAR_FILLED_16px, ICON_STAR_EMPTY_16px).empty())
            {
                SH_LOG_INFO("Error! Database is empty.");
            }
            else
            {
                for (auto data : dataset)
                {
                    m_Library->AppendItem(data);
                }
            }
        }
        catch (...)
        {
            std::cerr << "Error loading data." << std::endl;
        }

        m_Trash->Delete(selected_item_ids[i]);
    }
}

void MainFrame::OnDoSearch(wxCommandEvent& event)
{
    Serializer serializer;

    const auto search = m_SearchBox->GetValue().ToStdString();

    try
    {
        const auto dataset = m_Database->FilterDatabaseBySampleName(search,
                                                                    serializer.DeserializeShowFileExtension(),
                                                                    ICON_STAR_FILLED_16px,
                                                                    ICON_STAR_EMPTY_16px);

        if (dataset.empty())
        {
            SH_LOG_INFO("Error! Database is empty.");
        }
        else
        {
            m_Library->DeleteAllItems();

            std::cout << search << std::endl;

            for (const auto& data : dataset)
            {
                m_Library->AppendItem(data);
            }
        }
    }
    catch (...)
    {
        std::cerr << "Error loading data." << std::endl;
    }
}

void MainFrame::OnCancelSearch(wxCommandEvent& event)
{
    m_SearchBox->Clear();
}

void MainFrame::LoadConfigFile()
{
    // Check if SampleHive configuration directory exist and create it if not
    if (!wxDirExists(APP_CONFIG_DIR))
    {
        if (wxFileName::Mkdir(APP_CONFIG_DIR, wxPOSIX_USER_READ | wxPOSIX_USER_WRITE | wxPOSIX_USER_EXECUTE |
                              wxPOSIX_GROUP_READ | wxPOSIX_GROUP_EXECUTE |
                              wxPOSIX_OTHERS_READ | wxPOSIX_OTHERS_EXECUTE, wxPATH_MKDIR_FULL))
        {
            SH_LOG_INFO("Successfully created configuratin directory at {}", APP_CONFIG_DIR);
        }
        else
        {
            wxMessageBox(wxString::Format(_("Error! Could not create configuration directory %s"),
                                          APP_CONFIG_DIR), _("Error!"), wxOK | wxCENTRE, this);
        }
    }

    // Check if SampleHive data directory exist and create it if not
    if (!wxDirExists(APP_DATA_DIR))
    {
        if (wxFileName::Mkdir(APP_DATA_DIR, wxPOSIX_USER_READ | wxPOSIX_USER_WRITE | wxPOSIX_USER_EXECUTE |
                              wxPOSIX_GROUP_READ | wxPOSIX_GROUP_EXECUTE |
                              wxPOSIX_OTHERS_READ | wxPOSIX_OTHERS_EXECUTE, wxPATH_MKDIR_FULL))
        {
            SH_LOG_INFO("Successfully created data directory at {}", APP_DATA_DIR);
        }
        else
        {
            wxMessageBox(wxString::Format(_("Error! Could not create data directory %s"), APP_DATA_DIR),
                         _("Error!"), wxOK | wxCENTRE, this);
        }
    }

    Serializer serializer;

    SH_LOG_INFO("Reading configuration file..");

    int height = 600, width = 800;

    bAutoplay = serializer.DeserializeMediaOptions("autoplay");
    bLoop = serializer.DeserializeMediaOptions("loop");
    bMuted = serializer.DeserializeMediaOptions("muted");

    m_AutoPlayCheck->SetValue(bAutoplay);
    m_LoopButton->SetValue(bLoop);
    m_MuteButton->SetValue(bMuted);

    m_VolumeSlider->SetValue(serializer.DeserializeMediaVolume());

    if (!bMuted)
        m_MediaCtrl->SetVolume(double(m_VolumeSlider->GetValue()) / 100);
    else
        m_MediaCtrl->SetVolume(0.0);

    width = serializer.DeserializeWinSize().first;
    height = serializer.DeserializeWinSize().second;

    int min_width = 960, min_height = 540;

    bShowMenuBar = serializer.DeserializeShowMenuAndStatusBar("menubar");
    bShowStatusBar = serializer.DeserializeShowMenuAndStatusBar("statusbar");

    m_ToggleMenuBar->Check(bShowMenuBar);
    m_MenuBar->Show(bShowMenuBar);
    m_ToggleStatusBar->Check(bShowStatusBar);
    m_StatusBar->Show(bShowStatusBar);

    m_ToggleExtension->Check(serializer.DeserializeShowFileExtension());

    this->SetFont(serializer.DeserializeFontSettings());
    this->SetSize(width, height);
    this->SetMinSize(wxSize(min_width, min_height));
    this->CenterOnScreen(wxBOTH);
    this->SetIcon(wxIcon(ICON_HIVE_256px, wxICON_DEFAULT_TYPE, -1, -1));
    this->SetTitle(PROJECT_NAME);
    this->SetStatusText(wxString::Format("%s %s", PROJECT_NAME, PROJECT_VERSION), 3);
    this->SetStatusText(_("Stopped"), 1);
}

void MainFrame::RefreshDatabase()
{
    m_Library->DeleteAllItems();

    if (m_Hives->GetChildCount(wxDataViewItem(wxNullPtr)) < 1 &&
        m_Hives->GetItemText(wxDataViewItem(wxNullPtr)) == m_Hives->GetItemText(favorites_hive))
        return;
    else
        m_Hives->DeleteAllItems();

    m_Trash->DeleteAllItems();

    LoadDatabase();
}

bool MainFrame::CreateWatcherIfNecessary()
{
    if (m_FsWatcher)
        return false;

    CreateWatcher();
    Bind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);

    return true;
}

void MainFrame::CreateWatcher()
{
    Serializer serializer;

    wxCHECK_RET(!m_FsWatcher, _("Watcher already initialized"));

    m_FsWatcher = new wxFileSystemWatcher();
    m_FsWatcher->SetOwner(this);

    wxString path = serializer.DeserializeAutoImport().second;

    if (serializer.DeserializeAutoImport().first)
    {
        SH_LOG_INFO("Adding watch entry: {}", path);

        if (serializer.DeserializeRecursiveImport())
            AddWatchEntry(wxFSWPath_Tree, path.ToStdString());
        else
            AddWatchEntry(wxFSWPath_Dir, path.ToStdString());
    }
}

void MainFrame::AddWatchEntry(wxFSWPathType type, std::string path)
{
    Serializer serializer;

    if (path.empty())
    {
        path = wxDirSelector(_("Choose a directory to watch"), "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

        if (path.empty())
            return;
    }

    wxCHECK_RET(m_FsWatcher, _("Watcher not initialized"));

    SH_LOG_INFO("Adding {}: '{}'", path, type == wxFSWPath_Dir ? "directory" : "directory tree");

    wxFileName filename = wxFileName::DirName(path);

    if (!filename.IsOk() || !filename.IsDir() || !filename.IsDirReadable())
    {
        SH_LOG_ERROR("Error! Something wrong with {} path", filename.GetFullPath());
        return;
    }

    if (!serializer.DeserializeFollowSymLink())
    {
        filename.DontFollowLink();
    }

    bool ok = false;

    switch (type)
    {
        case wxFSWPath_Dir:
            ok = m_FsWatcher->Add(filename);
            break;
        case wxFSWPath_Tree:
            ok = m_FsWatcher->AddTree(filename);
            break;
        case wxFSWPath_File:
            break;
        case wxFSWPath_None:
            wxFAIL_MSG(_("Error! Unexpected path type."));
    }

    if (!ok)
    {
        SH_LOG_ERROR("Error! Cannot add '{}' to watched paths", filename.GetPath());
        return;
    }
}

void MainFrame::OnFileSystemEvent(wxFileSystemWatcherEvent& event)
{
    wxLogTrace(wxTRACE_FSWATCHER, "*** %s ***", event.ToString());

    int type = event.GetChangeType();

    wxString path = event.GetPath().GetFullPath();

    wxArrayString files;
    files.push_back(path);

    SH_LOG_DEBUG("{} {}", path, event.ToString());

    switch (type)
    {
        case wxFSW_EVENT_CREATE:
            SH_LOG_INFO("NEW FILES DETECTED, ADDING: {}", path);
            //AddSamples(files);
            break;
        case wxFSW_EVENT_ACCESS:
            SH_LOG_INFO("ACCESSING DIRECTORY: {}", path);
            break;
        case wxFSW_EVENT_DELETE:
            SH_LOG_INFO("FILES DELETED IN DIRECTORY: {}", path);
            break;
        case wxFSW_EVENT_MODIFY:
            SH_LOG_INFO("DIRECTORY MODIFIED: {}", path);
            break;
        case wxFSW_EVENT_RENAME:
            SH_LOG_INFO("FILES RENAMED IN DIRECTORY: {}", event.GetNewPath().GetFullPath());
            break;
        case wxFSW_EVENT_WARNING:
            SH_LOG_INFO("Filesystem watcher warning: {}", event.GetWarningType());
            break;
        case wxFSW_EVENT_ERROR:
            SH_LOG_INFO("Error! Filesystem watcher: {}", event.GetErrorDescription());
            break;
        default:
            break;
    }
}

FileInfo MainFrame::GetFilenamePathAndExtension(const wxString& selected,
                                                bool checkExtension, bool doGetFilename) const
{
    Serializer serializer;

    wxString path;
    std::string extension, filename;

    wxString filename_with_extension = m_Database->GetSamplePathByFilename(selected.BeforeLast('.').ToStdString());
    wxString filename_without_extension = m_Database->GetSamplePathByFilename(selected.ToStdString());

    if (checkExtension)
    {
        extension = serializer.DeserializeShowFileExtension() ?
            m_Database->GetSampleFileExtension(selected.ToStdString()) :
            m_Database->GetSampleFileExtension(selected.BeforeLast('.').ToStdString());
    }

    path = selected.Contains(wxString::Format(".%s", extension)) ?
        filename_with_extension : filename_without_extension;

    if (doGetFilename)
        filename = path.AfterLast('/').BeforeLast('.').ToStdString();

    return { path, extension, filename };
}

void MainFrame::OnHiveStartEditing(wxDataViewEvent &event)
{
    SH_LOG_INFO("Right click on a hive and select rename to rename it..");
    event.Veto();
}

void MainFrame::OnSelectAddFile(wxCommandEvent& event)
{
    wxFileDialog file_dialog(this, wxFileSelectorPromptStr, USER_HOME_DIR,
                             wxEmptyString, wxFileSelectorDefaultWildcardStr,
                             wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE | wxFD_PREVIEW,
                             wxDefaultPosition, wxDefaultSize);

    switch (file_dialog.ShowModal())
    {
        case wxID_OK:
        {
            wxArrayString paths;
            file_dialog.GetPaths(paths);

            for (size_t i = 0; i < (size_t)paths.size(); i++)
                AddSamples(paths);
        }
            break;
        default:
            break;
    }
}

void MainFrame::OnSelectAddDirectory(wxCommandEvent& event)
{
    wxDirDialog dir_dialog(this, wxDirSelectorPromptStr, USER_HOME_DIR,
                           wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST, wxDefaultPosition, wxDefaultSize);

    switch (dir_dialog.ShowModal())
    {
        case wxID_OK:
        {
            wxString path = dir_dialog.GetPath();

            OnAutoImportDir(path);
        }
        break;
        default:
            break;
    }
}

void MainFrame::OnSelectToggleExtension(wxCommandEvent& event)
{
    Serializer serializer;

    if (m_ToggleExtension->IsChecked())
    {
        serializer.SerializeShowFileExtension(true);
        m_InfoBar->ShowMessage(_("Extension showing, restart the application to view changes "
                                 "or press CTRL+E to toggle show/hide."), wxICON_INFORMATION);
    }
    else
    {
        serializer.SerializeShowFileExtension(false);
        m_InfoBar->ShowMessage(_("Extension hidden, restart the application to view changes "
                                 "or press CTRL+E to toggle show/hide."), wxICON_INFORMATION);
    }
}

void MainFrame::OnSelectToggleMenuBar(wxCommandEvent& event)
{
    Serializer serializer;

    if (m_ToggleMenuBar->IsChecked())
    {
        m_MenuBar->Show();
        m_InfoBar->ShowMessage(_("MenuBar showing, press CTRL+M to toggle show/hide."), wxICON_INFORMATION);

        bShowMenuBar = true;

        serializer.SerializeShowMenuAndStatusBar("menubar", bShowMenuBar);
    }
    else
    {
        m_MenuBar->Hide();
        m_InfoBar->ShowMessage(_("MenuBar hidden, press CTRL+M to toggle show/hide."), wxICON_INFORMATION);

        bShowMenuBar = false;

        serializer.SerializeShowMenuAndStatusBar("menubar", bShowMenuBar);
    }
}

void MainFrame::OnSelectToggleStatusBar(wxCommandEvent& event)
{
    Serializer serializer;

    if (m_ToggleStatusBar->IsChecked())
    {
        m_StatusBar->Show();
        m_InfoBar->ShowMessage(_("StatusBar showing, press CTRL+B to toggle show/hide."), wxICON_INFORMATION);

        bShowStatusBar = true;

        serializer.SerializeShowMenuAndStatusBar("statusbar", bShowStatusBar);
    }
    else
    {
        m_StatusBar->Hide();
        m_InfoBar->ShowMessage(_("StatusBar hidden, press CTRL+B to toggle show/hide."), wxICON_INFORMATION);

        bShowStatusBar = false;

        serializer.SerializeShowMenuAndStatusBar("statusbar", bShowStatusBar);
    }
}

void MainFrame::OnSelectExit(wxCommandEvent& event)
{
    Close();
}

void MainFrame::OnSelectPreferences(wxCommandEvent& event)
{
    Settings* settings = new Settings(this);

    switch (settings->ShowModal())
    {
        case wxID_OK:
            if (settings->CanAutoImport())
            {
                OnAutoImportDir(settings->GetImportDirPath());
                RefreshDatabase();
            }
            if (settings->IsWaveformColourChanged())
            {
                m_TopWaveformPanel->ResetDC();
            }
            break;
        case wxID_CANCEL:
            break;
        default:
            return;
    }
}

void MainFrame::OnSelectResetAppData(wxCommandEvent& event)
{
    wxMessageDialog clearDataDialog(this, wxString::Format(_("Warning! This will delete configuration file "
                                                             "\"%s\" and database file \"%s\" permanently, "
                                                             "are you sure you want to delete these files?"),
                                                           CONFIG_FILEPATH, DATABASE_FILEPATH),
                                    _("Clear app data?"),
                                    wxYES_NO | wxNO_DEFAULT | wxCENTRE, wxDefaultPosition);

    bool remove = false;

    switch (clearDataDialog.ShowModal())
    {
        case wxID_YES:
            remove = true;

            if (remove)
            {
                if (!wxFileExists(CONFIG_FILEPATH))
                {
                    SH_LOG_ERROR("Error! File {} doesn't exist.", CONFIG_FILEPATH);
                    return;
                }

                bool config_is_deleted = wxRemoveFile(CONFIG_FILEPATH);

                if (config_is_deleted)
                    SH_LOG_INFO("Deleted {}", CONFIG_FILEPATH);
                else
                    SH_LOG_ERROR("Could not delete {}", CONFIG_FILEPATH);

                if (!wxFileExists(DATABASE_FILEPATH))
                {
                    SH_LOG_ERROR("Error! File {} doesn't exist.", DATABASE_FILEPATH);
                    return;
                }

                bool db_is_deleted = wxRemoveFile(DATABASE_FILEPATH);

                if (db_is_deleted)
                    SH_LOG_INFO("Deleted {}", DATABASE_FILEPATH);
                else
                    SH_LOG_ERROR("Could not delete {}", DATABASE_FILEPATH);

                if (config_is_deleted && db_is_deleted)
                    m_InfoBar->ShowMessage(_("Successfully cleared app data"), wxICON_INFORMATION);
                else
                    wxMessageBox(_("Error! Could not clear app data"), _("Error!"),
                                 wxOK | wxCENTRE | wxICON_ERROR, this);
            }
            break;
        case wxID_NO:
            break;
        default:
            break;
    }
}

void MainFrame::OnSelectAbout(wxCommandEvent& event)
{
    wxAboutDialogInfo aboutInfo;

    aboutInfo.SetName(PROJECT_NAME);
    aboutInfo.SetIcon(wxIcon(ICON_HIVE_64px));
    aboutInfo.AddArtist(PROJECT_AUTHOR);
    aboutInfo.SetVersion(PROJECT_VERSION, _("Version 0.9.0_alpha.1"));
    aboutInfo.SetDescription(_(PROJECT_DESCRIPTION));
    aboutInfo.SetCopyright("(C)" PROJECT_COPYRIGHT_YEARS);
    aboutInfo.SetWebSite(PROJECT_WEBSITE);
    aboutInfo.AddDeveloper(PROJECT_AUTHOR);
    aboutInfo.SetLicence(wxString::Format(wxString::FromAscii(
                             "%s %s\n"
                             "Copyright (C) %s  Apoorv Singh\n"
                             "\n"
                             "%s is free software: you can redistribute it and/or modify\n"
                             "it under the terms of the GNU General Public License as published by\n"
                             "the Free Software Foundation, either version 3 of the License, or\n"
                             "(at your option) any later version.\n"
                             "\n"
                             "%s is distributed in the hope that it will be useful,\n"
                             "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                             "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                             "GNU General Public License for more details.\n"
                             "\n"
                             "You should have received a copy of the GNU General Public License\n"
                             "along with this program.  If not, see <https://www.gnu.org/licenses/>.\n"
                             ), PROJECT_NAME, PROJECT_VERSION, PROJECT_COPYRIGHT_YEARS,
                                PROJECT_NAME, PROJECT_NAME));

    wxAboutBox(aboutInfo);
}

void MainFrame::OnResizeStatusBar(wxSizeEvent& event)
{
    wxRect rect;
    m_StatusBar->GetFieldRect(2, rect);

    wxSize bitmap_size = m_HiveBitmap->GetSize();

    m_HiveBitmap->Move(rect.x + (rect.width - bitmap_size.x),
                       rect.y + (rect.height - bitmap_size.y));

    event.Skip();
}

void MainFrame::OnResizeFrame(wxSizeEvent& event)
{
    Serializer serializer;

    SH_LOG_DEBUG("Frame resized to {}, {}", GetSize().GetWidth(), GetSize().GetHeight());

    serializer.SerializeWinSize(GetSize().GetWidth(), GetSize().GetHeight());

    event.Skip();
}

void MainFrame::OnTopSplitterSashPosChanged(wxSplitterEvent& event)
{
    Serializer serializer;

    SH_LOG_DEBUG("TopSplitter at {}", m_TopSplitter->GetSashPosition());

    serializer.SerializeSplitterSashPos("top", m_TopSplitter->GetSashPosition());
}

void MainFrame::OnBottomSplitterSashPosChanged(wxSplitterEvent& event)
{
    Serializer serializer;

    SH_LOG_DEBUG("BottomSplitter at {}", m_BottomSplitter->GetSashPosition());

    serializer.SerializeSplitterSashPos("bottom", m_BottomSplitter->GetSashPosition());
}

void MainFrame::SetAfterFrameCreate()
{
    Serializer serializer;

    m_TopSplitter->SetSashPosition(serializer.DeserializeSplitterSashPos("top"));
    m_BottomSplitter->SetSashPosition(serializer.DeserializeSplitterSashPos("bottom"));
}

void MainFrame::OnRecieveLoopPoints(SampleHive::SH_LoopPointsEvent& event)
{
    std::pair<double, double> loop_points = event.GetLoopPoints();

    m_LoopA = wxLongLong(loop_points.first);
    m_LoopB = wxLongLong(loop_points.second);

    int loopA_min = static_cast<int>((m_LoopA / 60000).GetValue());
    int loopA_sec = static_cast<int>(((m_LoopA % 60000) / 1000).GetValue());
    int loopB_min = static_cast<int>((m_LoopB / 60000).GetValue());
    int loopB_sec = static_cast<int>(((m_LoopB % 60000) / 1000).GetValue());

    SH_LOG_INFO(wxString::Format(_("Loop points set: A: %2i:%02i, B: %2i:%02i"),
                                loopA_min, loopA_sec, loopB_min, loopB_sec));

    m_LoopABButton->SetValue(true);

    bLoopPointsSet = true;
}

void MainFrame::OnRecievePushStatusBarStatus(SampleHive::SH_StatusBarStatusEvent& event)
{
    std::pair<wxString, int> status = event.GetPushMessageAndSection();

    m_StatusBar->PushStatusText(status.first, status.second);
}

void MainFrame::OnRecievePopStatusBarStatus(SampleHive::SH_StatusBarStatusEvent& event)
{
    m_StatusBar->PopStatusText(event.GetPopMessageSection());
}

void MainFrame::OnRecieveSetStatusBarStatus(SampleHive::SH_StatusBarStatusEvent& event)
{
    std::pair<wxString, int> status = event.GetStatusTextAndSection();

    m_StatusBar->SetStatusText(status.first, status.second);
}

void MainFrame::OnRecieveInfoBarStatus(SampleHive::SH_InfoBarMessageEvent& event)
{
    SH_LOG_INFO("{} called..", __FUNCTION__);

    std::pair<wxString, int> info = event.GetInfoBarMessage();

    m_InfoBar->ShowMessage(info.first, info.second);

    SH_LOG_INFO("{} event processed", __FUNCTION__);
}

void MainFrame::OnRecieveTimerStopStatus(SampleHive::SH_TimerEvent& event)
{
    if (m_Timer->IsRunning())
        m_Timer->Stop();
}

void MainFrame::ClearLoopPoints()
{
    m_LoopA = 0;
    m_LoopB = 0;

    bLoopPointsSet = false;
}

void MainFrame::PlaySample(const std::string& filepath, const std::string& sample,
                           bool seek, wxFileOffset where, wxSeekMode mode)
{
    if (m_MediaCtrl->Load(filepath))
    {
        if (seek)
            m_MediaCtrl->Seek(where, mode);

        if (!m_MediaCtrl->Play())
            SH_LOG_ERROR("Error! Cannot play sample.");

        PushStatusText(wxString::Format(_("Now playing: %s"), sample), 1);

        if (!m_Timer->IsRunning())
            m_Timer->Start(20, wxTIMER_CONTINUOUS);
    }
    else
        SH_LOG_ERROR("Error! Cannot load sample.");
}

MainFrame::~MainFrame()
{
    // Delete wxTimer
    delete m_Timer;

    // Delete wxFilesystemWatcher
    delete m_FsWatcher;
}
