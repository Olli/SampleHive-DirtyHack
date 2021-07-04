#pragma once

#include <string>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/collpane.h>
#include <wx/dataview.h>
#include <wx/dirctrl.h>
#include <wx/dirdlg.h>
#include <wx/event.h>
#include <wx/frame.h>
#include <wx/fswatcher.h>
#include <wx/infobar.h>
#include <wx/listctrl.h>
#include <wx/mediactrl.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/setup.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/splitter.h>
#include <wx/string.h>
#include <wx/stringimpl.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/toplevel.h>
#include <wx/treebase.h>
#include <wx/treectrl.h>
#include <wx/window.h>

#include <wxSVG/svgctrl.h>
#include <wxSVG/SVGDocument.h>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>

struct FileInfo
{
    wxString Path;
    std::string Extension;
    std::string Filename;
};

class MainFrame : public wxFrame
{
    public:
        MainFrame();
        ~MainFrame();

    private:
        // -------------------------------------------------------------------
        // Main Panel
        wxPanel* m_MainPanel;
        wxBoxSizer* m_MainSizer;

    private:
        // -------------------------------------------------------------------
        // Splitter windows
        wxSplitterWindow* m_TopSplitter;
        wxSplitterWindow* m_BottomSplitter;

        // -------------------------------------------------------------------
        // Top panel controls
        wxPanel* m_TopPanel;
        wxBoxSizer* m_TopSizer;
        wxBoxSizer* m_TopPanelMainSizer;
        wxBoxSizer* m_WaveformDisplaySizer;
        wxSVGCtrl* m_WaveformViewer;
        wxBoxSizer* m_BrowserControlSizer;
        wxButton* m_PlayButton;
        wxToggleButton* m_LoopButton;
        wxButton* m_StopButton;
        wxButton* m_SettingsButton;
        wxToggleButton* m_MuteButton;
        wxStaticText* m_SamplePosition;
        wxSlider* m_VolumeSlider;
        wxCheckBox* m_AutoPlayCheck;

        // -------------------------------------------------------------------
        // Left panel controls
        wxPanel* m_BottomLeftPanel;
        wxPanel* m_HivesPanel;
        wxWindow* m_TrashPaneWindow;
        wxNotebook* m_Notebook;
        wxBoxSizer* m_BottomLeftPanelMainSizer;
        wxBoxSizer* m_HivesMainSizer;
        wxBoxSizer* m_HivesFavoritesSizer;
        wxBoxSizer* m_HivesViewTrashSizer;
        wxBoxSizer* m_HivesButtonSizer;
        wxBoxSizer* m_TrashItemSizer;
        wxSizerItem* m_HivesViewTrashSizerItem;
        wxDirCtrl* m_DirCtrl;
        wxDataViewTreeCtrl* m_Hives;
        wxDataViewItem favorites_hive;
        wxTreeItemId trash_root_node;
        wxCollapsiblePane* m_TrashPane;
        wxTreeCtrl* m_TrashedItems;
        wxButton* m_AddHiveButton;
        wxButton* m_RemoveHiveButton;
        wxButton* m_RestoreTrashedItemButton;

        // -------------------------------------------------------------------
        // Right panel controls
        wxPanel* m_BottomRightPanel;
        wxBoxSizer* m_BottomRightPanelMainSizer;
        wxSearchCtrl* m_SearchBox;
        wxInfoBar* m_InfoBar;
        wxDataViewListCtrl* m_SampleListView;

        // -------------------------------------------------------------------
        // MediaCtrl
        wxMediaCtrl* m_MediaCtrl;

        // -------------------------------------------------------------------
        // Timer
        wxTimer* m_Timer;

        // -------------------------------------------------------------------
        // FileSystemWatcher
        wxFileSystemWatcher* m_FsWatcher;

    private:
        // -------------------------------------------------------------------
        bool bAutoplay = false;
        bool bLoop = false;
        bool bMuted = false;
        bool bStopped = false;
        bool bFiltered = false;

        // -------------------------------------------------------------------
        const std::string m_ConfigFilepath;
        const std::string m_DatabaseFilepath;

    private:
        // -------------------------------------------------------------------
        // Top panel control handlers
        void OnClickPlay(wxCommandEvent& event);
        void OnClickLoop(wxCommandEvent& event);
        void OnClickStop(wxCommandEvent& event);
        void OnClickMute(wxCommandEvent& event);
        void OnMediaFinished(wxMediaEvent& event);
        void OnCheckAutoplay(wxCommandEvent& event);
        void OnSlideVolume(wxScrollEvent& event);
        void OnClickSettings(wxCommandEvent& event);

        // -------------------------------------------------------------------
        // DirCtrl event handlers
        void OnClickDirCtrl(wxCommandEvent& event);
        void OnDragFromDirCtrl(wxTreeEvent& event);

        // -------------------------------------------------------------------
        // TrashPane event handlers
        void OnExpandTrash(wxCollapsiblePaneEvent& event);
        void OnClickRestoreTrashItem(wxCommandEvent& event);

        // -------------------------------------------------------------------
        // Hives panel button event handlers
        void OnDragAndDropToHives(wxDropFilesEvent& event);
        void OnClickAddHive(wxCommandEvent& event);
        void OnClickRemoveHive(wxCommandEvent& event);
        void OnShowHivesContextMenu(wxDataViewEvent& event);
        void OnHiveStartEditing(wxDataViewEvent& event);

        // -------------------------------------------------------------------
        // SearchCtrl event handlers
        void OnDoSearch(wxCommandEvent& event);
        void OnCancelSearch(wxCommandEvent& event);

        // -------------------------------------------------------------------
        // SampleListView event handlers
        void OnClickSampleView(wxDataViewEvent& event);
        void OnDragAndDropToSampleListView(wxDropFilesEvent& event);
        void OnDragFromSampleView(wxDataViewEvent& event);
        void OnShowSampleListViewContextMenu(wxDataViewEvent& event);

        // -------------------------------------------------------------------
        // Timer update event handler
        void UpdateElapsedTime(wxTimerEvent& event);

        // -------------------------------------------------------------------
        void AddSamples(wxArrayString& files);
        void OnAutoImportDir();

        // -------------------------------------------------------------------
        void LoadDatabase();
        void RefreshDatabase();
        void LoadConfigFile();

        // -------------------------------------------------------------------
        // Getters
        FileInfo GetFilenamePathAndExtension(const wxString& selected,
                                             bool checkExtension = true, bool doGetFilename = true) const;

        // -------------------------------------------------------------------
        // Directory watchers
        bool CreateWatcherIfNecessary();
        void CreateWatcher();

        // wxString TagLibTowx(const TagLib::String& in);

        // -------------------------------------------------------------------
        friend class App;
};
