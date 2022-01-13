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

#include "GUI/Dialogs/Settings.hpp"
#include "Utility/ControlID_Enums.hpp"
#include "Utility/Serialize.hpp"
#include "Utility/Log.hpp"
#include "Utility/Paths.hpp"

#include <wx/defs.h>
#include <wx/gdicmn.h>
#include <wx/stringimpl.h>

Settings::Settings(wxWindow *window)
    : wxDialog(window, wxID_ANY, "Settings", wxDefaultPosition,
               wxSize(720, 270), wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP),
      m_Window(window)
{
    m_Panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    m_MainSizer = new wxBoxSizer(wxVERTICAL);
    m_NotebookSizer = new wxBoxSizer(wxVERTICAL);
    m_ButtonSizer = new wxBoxSizer(wxHORIZONTAL);

    m_Notebook = new wxNotebook(m_Panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _T("NOTEBOOK"));

    m_DisplaySettingPanel = new wxPanel(m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    m_DisplayTopSizer = new wxBoxSizer(wxVERTICAL);
    m_DisplayFontSizer = new wxBoxSizer(wxHORIZONTAL);
    m_WaveformColourSizer = new wxBoxSizer(wxHORIZONTAL);

    Serializer serializer;

    wxString fontChoices[] = { "System default" };

    m_FontTypeText = new wxStaticText(m_DisplaySettingPanel, wxID_ANY, "Font",
                                      wxDefaultPosition, wxDefaultSize, 0);
    m_FontType = new wxChoice(m_DisplaySettingPanel, SD_FontType,
                              wxDefaultPosition, wxDefaultSize, 1, fontChoices, 0);
    m_FontType->SetSelection(0);
    m_FontSize = new wxSpinCtrl(m_DisplaySettingPanel, SD_FontSize, "Default",
                                wxDefaultPosition, wxDefaultSize);
    m_FontSize->SetValue(window->GetFont().GetPointSize());
    m_FontBrowseButton = new wxButton(m_DisplaySettingPanel, SD_FontBrowseButton, "Select font",
                                      wxDefaultPosition, wxDefaultSize, 0);
    m_WaveformColourLabel = new wxStaticText(m_DisplaySettingPanel, wxID_ANY, "Waveform colour",
                                             wxDefaultPosition, wxDefaultSize, 0);
    m_WaveformColourPickerCtrl = new wxColourPickerCtrl(m_DisplaySettingPanel, SD_WaveformColourPickerCtrl,
                                                        serializer.DeserializeWaveformColour(),
                                                        wxDefaultPosition, wxDefaultSize,
                                                        wxCLRP_DEFAULT_STYLE);

    m_CollectionSettingPanel = new wxPanel(m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    m_CollectionMainSizer = new wxBoxSizer(wxVERTICAL);
    m_CollectionImportDirSizer = new wxBoxSizer(wxHORIZONTAL);
    m_CollectionImportOptionsSizer = new wxBoxSizer(wxHORIZONTAL);
    m_CollectionShowExtensionSizer = new wxBoxSizer(wxVERTICAL);

    wxString defaultDir = wxGetHomeDir();

    m_AutoImportCheck = new wxCheckBox(m_CollectionSettingPanel, SD_AutoImport, "Auto import",
                                       wxDefaultPosition, wxDefaultSize, 0);
    
    m_ImportDirLocation = new wxTextCtrl(m_CollectionSettingPanel, wxID_ANY, defaultDir,
                                         wxDefaultPosition, wxDefaultSize, 0);
    m_ImportDirLocation->Disable();
    m_BrowseAutoImportDirButton = new wxButton(m_CollectionSettingPanel, SD_BrowseAutoImportDir, "Browse",
                                               wxDefaultPosition, wxDefaultSize, 0);
    m_BrowseAutoImportDirButton->Disable();
    m_FollowSymLinksCheck = new wxCheckBox(m_CollectionSettingPanel, SD_FollowSymLinks,
                                           "Follow symbolic links", wxDefaultPosition, wxDefaultSize, 0);
    m_FollowSymLinksCheck->SetToolTip("Wheather to follow symbolic links");
    m_FollowSymLinksCheck->Disable();
    m_RecursiveImportCheck = new wxCheckBox(m_CollectionSettingPanel, SD_RecursiveImport,
                                           "Recursive search", wxDefaultPosition, wxDefaultSize, 0);
    m_RecursiveImportCheck->SetToolTip("Recursively search for samples in the directory");
    m_RecursiveImportCheck->Disable();
    m_ShowFileExtensionCheck = new wxCheckBox(m_CollectionSettingPanel, SD_ShowFileExtension,
                                              "Show file extension", wxDefaultPosition, wxDefaultSize, 0);
    m_ShowFileExtensionCheck->SetToolTip("Weather to show file extension");

    m_ConfigurationSettingPanel = new wxPanel(m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    m_GeneralMainSizer = new wxFlexGridSizer(2, 3, 0, 0);
    m_GeneralMainSizer->AddGrowableCol(1);
    m_GeneralMainSizer->SetFlexibleDirection(wxBOTH);
    m_GeneralMainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    m_ConfigLabel = new wxStaticText(m_ConfigurationSettingPanel, wxID_ANY,
                                     "Default configuration file location", wxDefaultPosition, wxDefaultSize);
    m_ConfigText = new wxTextCtrl(m_ConfigurationSettingPanel, wxID_ANY, CONFIG_FILEPATH,
                                  wxDefaultPosition, wxDefaultSize);
    m_ConfigBrowse = new wxButton(m_ConfigurationSettingPanel, SD_BrowseConfigDir, "Browse",
                                  wxDefaultPosition, wxDefaultSize, 0);
    m_DatabaseLabel = new wxStaticText(m_ConfigurationSettingPanel, wxID_ANY, "Default database location",
                                       wxDefaultPosition, wxDefaultSize);
    m_DatabaseText = new wxTextCtrl(m_ConfigurationSettingPanel, wxID_ANY, DATABASE_FILEPATH,
                                    wxDefaultPosition, wxDefaultSize);
    m_DatabaseBrowse = new wxButton(m_ConfigurationSettingPanel, SD_BrowseDatabaseDir, "Browse",
                                    wxDefaultPosition, wxDefaultSize, 0);

    m_Notebook->AddPage(m_DisplaySettingPanel, "Display");
    m_Notebook->AddPage(m_CollectionSettingPanel, "Collection");
    m_Notebook->AddPage(m_ConfigurationSettingPanel, "General");

    m_OkButton = new wxButton(m_Panel, wxID_OK, "OK", wxDefaultPosition, wxDefaultSize);
    m_CancelButton = new wxButton(m_Panel, wxID_CANCEL, "Cancel", wxDefaultPosition, wxDefaultSize);

    LoadDefaultConfig();

    // Bind events
    Bind(wxEVT_CHECKBOX, &Settings::OnCheckAutoImport, this, SD_AutoImport);
    Bind(wxEVT_CHECKBOX, &Settings::OnCheckFollowSymLinks, this, SD_FollowSymLinks);
    Bind(wxEVT_CHECKBOX, &Settings::OnCheckRecursiveImport, this, SD_RecursiveImport);
    Bind(wxEVT_CHECKBOX, &Settings::OnCheckShowFileExtension, this, SD_ShowFileExtension);
    Bind(wxEVT_SPINCTRL, &Settings::OnChangeFontSize, this, SD_FontSize);
    Bind(wxEVT_BUTTON, &Settings::OnSelectFont, this, SD_FontBrowseButton);
    Bind(wxEVT_BUTTON, &Settings::OnClickBrowseAutoImportDir, this, SD_BrowseAutoImportDir);
    Bind(wxEVT_BUTTON, &Settings::OnClickConfigBrowse, this, SD_BrowseConfigDir);
    Bind(wxEVT_BUTTON, &Settings::OnClickDatabaseBrowse, this, SD_BrowseDatabaseDir);
    Bind(wxEVT_COLOURPICKER_CHANGED, &Settings::OnChangeWaveformColour, this, SD_WaveformColourPickerCtrl);

    // Adding controls to sizers
    m_NotebookSizer->Add(m_Notebook, 1, wxALL | wxEXPAND, 2);

    m_GeneralMainSizer->Add(m_ConfigLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_GeneralMainSizer->Add(m_ConfigText, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2);
    m_GeneralMainSizer->Add(m_ConfigBrowse, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_GeneralMainSizer->Add(m_DatabaseLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_GeneralMainSizer->Add(m_DatabaseText, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2);
    m_GeneralMainSizer->Add(m_DatabaseBrowse, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_DisplayFontSizer->Add(m_FontTypeText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_DisplayFontSizer->Add(m_FontType, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_DisplayFontSizer->Add(m_FontSize, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_DisplayFontSizer->Add(m_FontBrowseButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_WaveformColourSizer->Add(m_WaveformColourLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_WaveformColourSizer->Add(m_WaveformColourPickerCtrl, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_DisplayTopSizer->Add(m_DisplayFontSizer, 0, wxALL | wxEXPAND, 2);
    m_DisplayTopSizer->Add(m_WaveformColourSizer, 0, wxALL | wxEXPAND, 2);

    m_CollectionImportDirSizer->Add(m_AutoImportCheck, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_CollectionImportDirSizer->Add(m_ImportDirLocation, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    m_CollectionImportDirSizer->Add(m_BrowseAutoImportDirButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_CollectionImportOptionsSizer->Add(m_FollowSymLinksCheck, 0, wxALL, 2);
    m_CollectionImportOptionsSizer->Add(m_RecursiveImportCheck, 0, wxALL, 2);
    m_CollectionShowExtensionSizer->Add(m_ShowFileExtensionCheck, 0, wxALL, 2);

    m_CollectionMainSizer->Add(m_CollectionImportDirSizer, 0, wxALL | wxEXPAND, 2);
    m_CollectionMainSizer->Add(m_CollectionImportOptionsSizer, 0, wxALL | wxEXPAND, 2);
    m_CollectionMainSizer->Add(m_CollectionShowExtensionSizer, 0, wxALL | wxEXPAND, 2);

    m_ButtonSizer->Add(m_OkButton, 0, wxALL | wxALIGN_BOTTOM, 2);
    m_ButtonSizer->Add(m_CancelButton, 0, wxALL | wxALIGN_BOTTOM, 2);

    m_MainSizer->Add(m_NotebookSizer, 1, wxALL | wxEXPAND, 2);
    m_MainSizer->Add(m_ButtonSizer, 0, wxALL | wxALIGN_RIGHT, 2);

    // Top panel layout
    m_Panel->SetSizer(m_MainSizer);
    m_MainSizer->Fit(m_Panel);
    m_MainSizer->SetSizeHints(m_Panel);
    m_MainSizer->Layout();

    // Display panel layout
    m_DisplaySettingPanel->SetSizer(m_DisplayTopSizer);
    m_DisplayTopSizer->Fit(m_DisplaySettingPanel);
    m_DisplayTopSizer->SetSizeHints(m_DisplaySettingPanel);
    m_DisplayTopSizer->Layout();

    // Collection panel layout
    m_CollectionSettingPanel->SetSizer(m_CollectionMainSizer);
    m_CollectionMainSizer->Fit(m_CollectionSettingPanel);
    m_CollectionMainSizer->SetSizeHints(m_CollectionSettingPanel);
    m_CollectionMainSizer->Layout();

    // Configuration panel layout
    m_ConfigurationSettingPanel->SetSizer(m_GeneralMainSizer);
    m_GeneralMainSizer->Fit(m_ConfigurationSettingPanel);
    m_GeneralMainSizer->SetSizeHints(m_ConfigurationSettingPanel);
    m_GeneralMainSizer->Layout();
}

void Settings::OnClickConfigBrowse(wxCommandEvent& event)
{
    wxString initial_dir = wxGetHomeDir();

    wxDirDialog dir_dialog(this, "Select a directory..", initial_dir,
                           wxDD_DEFAULT_STYLE |
                           wxDD_DIR_MUST_EXIST |
                           wxDD_NEW_DIR_BUTTON,
                           wxDefaultPosition, wxDefaultSize);

    switch (dir_dialog.ShowModal())
    {
        case wxID_OK:
        {
            wxString path = dir_dialog.GetPath();
            m_ConfigText->SetValue(path + "/config.yaml");
            break;
        }
        default:
            return;
    }
}

void Settings::OnClickDatabaseBrowse(wxCommandEvent& event)
{
    wxString initial_dir = wxGetHomeDir();

    wxDirDialog dir_dialog(this, "Select a directory..", initial_dir,
                           wxDD_DEFAULT_STYLE |
                           wxDD_DIR_MUST_EXIST |
                           wxDD_NEW_DIR_BUTTON,
                           wxDefaultPosition, wxDefaultSize);

    switch (dir_dialog.ShowModal())
    {
        case wxID_OK:
        {
            wxString path = dir_dialog.GetPath();
            m_DatabaseText->SetValue(path + "/config.yaml");
            break;
        }
        default:
            return;
    }
}

void Settings::OnCheckAutoImport(wxCommandEvent& event)
{
    Serializer serializer;

    if (!m_AutoImportCheck->GetValue())
    {
        bAutoImport = false;
        m_ImportDirLocation->Disable();
        m_BrowseAutoImportDirButton->Disable();
        m_FollowSymLinksCheck->Disable();
        m_RecursiveImportCheck->Disable();

        serializer.SerializeAutoImport(bAutoImport, m_ImportDirLocation->GetValue().ToStdString());
    }
    else
    {
        bAutoImport = true;
        m_ImportDirLocation->Enable();
        m_BrowseAutoImportDirButton->Enable();
        m_FollowSymLinksCheck->Enable();
        m_RecursiveImportCheck->Enable();

        serializer.SerializeAutoImport(bAutoImport, m_ImportDirLocation->GetValue().ToStdString());
    }
}

void Settings::OnCheckFollowSymLinks(wxCommandEvent& event)
{
    Serializer serializer;

    serializer.SerializeFollowSymLink(m_FollowSymLinksCheck->GetValue());
}

void Settings::OnCheckRecursiveImport(wxCommandEvent& event)
{
    Serializer serializer;

    serializer.SerializeRecursiveImport(m_RecursiveImportCheck->GetValue());
}

void Settings::OnCheckShowFileExtension(wxCommandEvent& event)
{
    Serializer serializer;

    serializer.SerializeShowFileExtension(m_ShowFileExtensionCheck->GetValue());
}

void Settings::OnClickBrowseAutoImportDir(wxCommandEvent& event)
{
    Serializer serializer;

    wxString initial_dir = wxGetHomeDir();

    wxDirDialog dir_dialog(this, "Select a directory..", initial_dir,
                           wxDD_DEFAULT_STYLE |
                           wxDD_DIR_MUST_EXIST |
                           wxDD_NEW_DIR_BUTTON,
                           wxDefaultPosition, wxDefaultSize);

    switch (dir_dialog.ShowModal())
    {
        case wxID_OK:
        {
            wxString path = dir_dialog.GetPath();
            m_ImportDirLocation->SetValue(path);

            serializer.SerializeAutoImport(bAutoImport, m_ImportDirLocation->GetValue().ToStdString());
            break;
        }
        default:
            return;
    }
}

void Settings::OnSelectFont(wxCommandEvent& event)
{
    wxFontDialog font_dialog(this);

    switch (font_dialog.ShowModal())
    {
        case wxID_OK:
        {
            wxFontData fontData = font_dialog.GetFontData();
            m_Font = fontData.GetChosenFont();

            if (m_FontType->GetCount() > 1)
            {
                m_FontType->Delete(1);
                m_FontType->AppendString(m_Font.GetFaceName());
                m_FontType->SetSelection(1);
            }
            else
            {
                m_FontType->AppendString(m_Font.GetFaceName());
                m_FontType->SetSelection(1);
            }

            SetCustomFont();
        }
        default:
            return;
    }

    PrintFont();
}

void Settings::OnChangeFontSize(wxSpinEvent& event)
{
    Serializer serializer;

    int font_size = m_FontSize->GetValue();

    if (m_FontType->GetStringSelection() == "System default")
        m_Font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);

    m_Font.SetPointSize(font_size);

    serializer.SerializeFontSettings(m_Font);

    m_Window->SetFont(m_Font);
    this->SetFont(m_Font);

    SH_LOG_DEBUG("Font size: {}", font_size);
    SH_LOG_DEBUG("Font size: {}", m_Font.GetPointSize());
}

void Settings::LoadDefaultConfig()
{
    Serializer serializer;

    wxFont sys_font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
    wxString system_font = sys_font.GetFaceName();
    int system_font_size = sys_font.GetPointSize();

    wxString font_face = serializer.DeserializeFontSettings().GetFaceName();
    int font_size = serializer.DeserializeFontSettings().GetPointSize();

    if (system_font != font_face)
    {
        if (m_FontType->GetCount() > 1)
        {
            m_FontType->Delete(1);
            m_FontType->AppendString(font_face);
            m_FontType->SetSelection(1);

            m_Font.SetFaceName(font_face);
            m_Font.SetPointSize(font_size);
        }
        else
        {
            m_FontType->AppendString(font_face);
            m_FontType->SetSelection(1);

            m_Font.SetFaceName(font_face);
            m_Font.SetPointSize(font_size);
        }
    }

    m_FontSize->SetValue(font_size);
    SetCustomFont();

    bAutoImport = serializer.DeserializeAutoImport().first;

    m_AutoImportCheck->SetValue(bAutoImport);
    m_ImportDirLocation->SetValue(serializer.DeserializeAutoImport().second);
    m_ShowFileExtensionCheck->SetValue(serializer.DeserializeShowFileExtension());
    m_FollowSymLinksCheck->SetValue(serializer.DeserializeFollowSymLink());
    m_RecursiveImportCheck->SetValue(serializer.DeserializeRecursiveImport());

    if (bAutoImport)
    {
        m_ImportDirLocation->Enable();
        m_BrowseAutoImportDirButton->Enable();
        m_FollowSymLinksCheck->Enable();
        m_RecursiveImportCheck->Enable();
    }
}

void Settings::SetShowExtension(bool value)
{
    Serializer serializer;

    m_ShowFileExtensionCheck->SetValue(value);
    serializer.SerializeShowFileExtension(value);
}

void Settings::PrintFont()
{
    SH_LOG_DEBUG("Font face: {}", m_Font.GetFaceName());
    SH_LOG_DEBUG("Font size: {}", m_Font.GetPointSize());
    SH_LOG_DEBUG("Font family: {}", m_Font.GetFamilyString());
    SH_LOG_DEBUG("Font style: {}", m_Font.GetStyleString());
    SH_LOG_DEBUG("Font weight: {}", m_Font.GetWeightString());
}

void Settings::SetCustomFont()
{
    Serializer serializer;

    wxFont sys_font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
    std::string system_font = sys_font.GetFaceName().ToStdString();
    int system_font_size = sys_font.GetPointSize();

    wxString font_face = serializer.DeserializeFontSettings().GetFaceName();
    int font_size = serializer.DeserializeFontSettings().GetPointSize();

    if (m_FontType->GetStringSelection() == "System default")
    {
        m_Window->SetFont(sys_font);
        this->SetFont(sys_font);

        serializer.SerializeFontSettings(sys_font);
    }
    else
    {
        m_Window->SetFont(m_Font);
        this->SetFont(m_Font);

        serializer.SerializeFontSettings(m_Font);
    }
}

wxString Settings::GetImportDirPath()
{
    wxString dir = wxEmptyString;

    if (m_AutoImportCheck->GetValue())
        dir = m_ImportDirLocation->GetValue();

    return dir;
}

void Settings::OnChangeWaveformColour(wxColourPickerEvent& event)
{
    Serializer serializer;
    wxColour colour = m_WaveformColourPickerCtrl->GetColour();

    wxColour wave_colour = serializer.DeserializeWaveformColour();

    if (colour != wave_colour)
    {
        SH_LOG_INFO("Waveform colour changed.");
        bWaveformColourChanged = true;

        serializer.SerializeWaveformColour(colour);
    }
    else
    {
        SH_LOG_INFO("Waveform colour not changed.");
        bWaveformColourChanged = false;

        serializer.SerializeWaveformColour(colour);
    }
}

Settings::~Settings(){}
