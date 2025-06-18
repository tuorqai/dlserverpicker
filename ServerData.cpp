
#include "ServerData.h"
#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>

namespace {
    void SaveToCache(wxString const &rawJSON)
    {
        if (rawJSON.IsEmpty()) {
            return;
        }

        wxString userDataDir = wxStandardPaths::Get().GetUserDataDir();

        if (!wxDirExists(userDataDir)) {
            return;
        }

        wxFileName responseCache(userDataDir, "response-cache.json");
        wxFileOutputStream fstream(responseCache.GetFullPath());

        if (!fstream.IsOk()) {
            return;
        }

        wxStringInputStream input(rawJSON);
        fstream.Write(input);
    }
}

bool ServerData::RestoreFromCache()
{
    wxString userDataDir = wxStandardPaths::Get().GetUserDataDir();

    if (!wxDirExists(userDataDir)) {
        if (!wxMkDir(userDataDir)) {
            return false;
        }
    }

    wxFileName responseCache(userDataDir, "response-cache.json");

    if (!responseCache.FileExists()) {
        return false;
    }

    wxFileInputStream fstream(responseCache.GetFullPath());

    if (!fstream.IsOk()) {
        return false;
    }

    wxStringOutputStream sstream;
    fstream.Read(sstream);

    if (!UpdateFromJSON(sstream.GetString(), true)) {
        wxRemoveFile(responseCache.GetFullPath());
        return false;
    }

    return true;
}

bool ServerData::UpdateFromJSON(wxString const &rawJSON, bool isAlreadyCached)
{
    m_locations.clear();

    try {
        JSON::parse(rawJSON).get_to(*this);
    } catch (std::exception const &exception) {
        return false;
    }

    if (!isAlreadyCached) {
        SaveToCache(rawJSON);
    }

    return true;
}

void from_json(JSON const &json, ServerData &data)
{
    data.m_revision = json["revision"].get<std::uint64_t>();

    JSON pops = json["pops"];

    for (auto it = pops.begin(); it != pops.end(); it++) {
        data.m_locations.push_back({ it.key() });
        it.value().get_to(data.m_locations.back());
    }
}

void from_json(JSON const &json, ServerData::Location &location)
{
    location.description = json["desc"].get<std::string>();

    if (!json.contains("relays")) {
        return;
    }

    for (JSON const &relay : json["relays"]) {
        location.relays.push_back({});
        relay.get_to(location.relays.back());
    }
}

void from_json(JSON const &json, ServerData::Relay &relay)
{
    relay.ipv4 = json["ipv4"].get<std::string>();
    relay.portRangeMin = json["port_range"][0].get<unsigned short>();
    relay.portRangeMax = json["port_range"][1].get<unsigned short>();
}
