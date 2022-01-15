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

#include "App.hpp"
#include "Utility/Paths.hpp"
#include "Utility/Log.hpp"

#include <wx/bitmap.h>
#include <wx/defs.h>
#include <wx/filefn.h>
#include <wx/fswatcher.h>
#include <wx/gdicmn.h>
#include <wx/splash.h>

wxIMPLEMENT_APP(App);

App::App()
{
    // Initialize the logger
    SampleHive::Log::InitLogger("SampleHive");
}

App::~App()
{

}

bool App::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    wxLog::AddTraceMask("EventSource");
    wxLog::AddTraceMask(wxTRACE_FSWATCHER);

    m_Frame = new MainFrame();

    wxBitmap bitmap;

    wxYield();

    m_Frame->Show(true);
    return true;
}

void App::OnInitCmdLine(wxCmdLineParser& parser)
{
    wxApp::OnInitCmdLine(parser);

    parser.AddSwitch("v", "version", "Shows the application version", 0);
    parser.AddSwitch("r", "reset", "Reset app data", 0);
    parser.Parse(true);
}

bool App::OnCmdLineParsed(wxCmdLineParser& parser)
{
    if (!wxApp::OnCmdLineParsed(parser))
        return false;

    if (parser.Found("version"))
    {
        std::cout << PROJECT_NAME << ' ' << PROJECT_VERSION << std::endl;
        return false;
    }
    else if (parser.Found("reset"))
    {
        char ans;

        std::cout << "Are you sure you want reset app data? [y/N] ";
        std::cin >> ans;

        if (ans == 'y' || ans == 'Y')
        {
            if (!wxFileExists(CONFIG_FILEPATH))
            {
                SH_LOG_ERROR("Error! File {} doesn't exist.", CONFIG_FILEPATH);
                return false;
            }

            bool config_is_deleted = wxRemoveFile(CONFIG_FILEPATH);

            if (config_is_deleted)
                SH_LOG_INFO("Deleted {}", CONFIG_FILEPATH);
            else
                SH_LOG_ERROR("Could not delete {}", CONFIG_FILEPATH);

            if (!wxFileExists(DATABASE_FILEPATH))
            {
                SH_LOG_ERROR("Error! File {} doesn't exist.", DATABASE_FILEPATH);
                return false;
            }

            bool db_is_deleted = wxRemoveFile(DATABASE_FILEPATH);

            if (db_is_deleted)
                SH_LOG_INFO("Deleted {}", DATABASE_FILEPATH);
            else
                SH_LOG_ERROR("Could not delete {}", DATABASE_FILEPATH);

            if (config_is_deleted && db_is_deleted)
                SH_LOG_INFO("Successfully cleared app data");
            else
                SH_LOG_ERROR("Error! Could not clear app data");

            return false;
        }
        else if (ans == 'n' || ans == 'N')
            return false;
        else
        {
            SH_LOG_ERROR("Unknown option '{}' please select a correct option", ans);
            return false;
        }
    }

    return true;
}

void App::OnEventLoopEnter(wxEventLoopBase* event)
{
    if (m_Frame->CreateWatcherIfNecessary())
        SH_LOG_INFO("Filesystem watcher created sucessfully");
}
