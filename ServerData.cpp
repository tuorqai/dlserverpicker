
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
        if (!wxMkDir(userDataDir, wxS_DIR_DEFAULT)) {
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

enum ServerRegion
{
    SERVER_REGION_EUROPE,
    SERVER_REGION_ASIA,
    SERVER_REGION_AFRICA,
    SERVER_REGION_N_AMERICA,
    SERVER_REGION_S_AMERICA,
    SERVER_REGION_OCEANIA,
};

wxString MapLocationToRegion(ServerData::Location const &location)
{
    std::unordered_map<wxString, int> const regionIdMap = {
        { "ams", SERVER_REGION_EUROPE },
        { "ams4", SERVER_REGION_EUROPE },
        { "atl", SERVER_REGION_N_AMERICA },
        { "bom2", SERVER_REGION_ASIA },
        { "dfw", SERVER_REGION_N_AMERICA },
        { "dxb", SERVER_REGION_ASIA },
        { "eat", SERVER_REGION_N_AMERICA },
        { "eze", SERVER_REGION_S_AMERICA },
        { "fra", SERVER_REGION_EUROPE },
        { "fsn", SERVER_REGION_EUROPE },
        { "gru", SERVER_REGION_S_AMERICA },
        { "hel", SERVER_REGION_EUROPE },
        { "hkg", SERVER_REGION_ASIA },
        { "hkg4", SERVER_REGION_ASIA },
        { "iad", SERVER_REGION_N_AMERICA },
        { "jnb", SERVER_REGION_AFRICA },
        { "lax", SERVER_REGION_N_AMERICA },
        { "lhr", SERVER_REGION_EUROPE },
        { "lim", SERVER_REGION_S_AMERICA },
        { "maa2", SERVER_REGION_ASIA },
        { "mad", SERVER_REGION_EUROPE },
        { "ord", SERVER_REGION_N_AMERICA },
        { "par", SERVER_REGION_EUROPE },
        { "scl", SERVER_REGION_S_AMERICA },
        { "sea", SERVER_REGION_N_AMERICA },
        { "seo", SERVER_REGION_ASIA },
        { "sgp", SERVER_REGION_ASIA },
        { "sto", SERVER_REGION_EUROPE },
        { "sto2", SERVER_REGION_EUROPE },
        { "syd", SERVER_REGION_OCEANIA },
        { "tyo", SERVER_REGION_ASIA },
        { "vie", SERVER_REGION_EUROPE },
        { "waw", SERVER_REGION_EUROPE },
    };

    std::unordered_map<int, wxString> const regionNameMap = {
        { SERVER_REGION_EUROPE, "Europe" },
        { SERVER_REGION_ASIA, "Asia" },
        { SERVER_REGION_AFRICA, "Africa" },
        { SERVER_REGION_N_AMERICA, "N America" },
        { SERVER_REGION_S_AMERICA, "S America" },
        { SERVER_REGION_OCEANIA, "Oceania" },
    };

    if (regionIdMap.count(location.identifier) == 0) {
        return "!UNMAPPED!";
    }

    return regionNameMap.at(regionIdMap.at(location.identifier));
}
