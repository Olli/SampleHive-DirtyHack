#include <wx/defs.h>

enum ControlIDs
{
    /*
    ** BC = Browser control
    ** SD = Settings dialog
    ** MN = Popup menu
    ** ET = Edit tag dialog
    */

    // -------------------------------------------------------------------
    // Browser controls
    BC_Play = wxID_HIGHEST + 1,
    BC_Settings,
    BC_Loop,
    BC_Stop,
    BC_Mute,
    BC_Autoplay,
    BC_Volume,
    BC_SamplePosition,
    BC_Hives,
    BC_DirCtrl,
    BC_SampleListView,
    BC_Search,
    BC_MediaCtrl,
    BC_TrashPane,
    BC_RestoreTrashedItemButton,
    BC_HiveAdd,
    BC_HiveRemove,

    // -------------------------------------------------------------------
    // Setting dialog controls
    SD_BrowseConfigDir,
    SD_BrowseDatabaseDir,
    SD_AutoImport,
    SD_ShowFileExtension,
    SD_BrowseAutoImportDir,
    SD_FontType,
    SD_FontSize,
    SD_FontBrowseButton,

    // -------------------------------------------------------------------
    // SampleView Menu items
    MN_FavoriteSample,
    MN_DeleteSample,
    MN_TrashSample,
    MN_EditTagSample,
    MN_OpenFile,

    // -------------------------------------------------------------------
    // Hives Menu items
    MN_CreateHive,
    MN_DeleteHive,
    MN_RemoveSample,
    MN_FilterSampleView,
    MN_ShowInLibrary,

    // -------------------------------------------------------------------
    // Edit tags dialog controls
    ET_TitleCheck,
    ET_ArtistCheck,
    ET_AlbumCheck,
    ET_GenreCheck,
    ET_CommentsCheck,
    ET_TypeCheck,
    ET_CustomTag,
};
