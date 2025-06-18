
#pragma once

#include <wx/string.h>
#include <wx/vector.h>
#include <nlohmann/json.hpp>

using JSON = nlohmann::json;

class ServerData final
{
public:
    struct Relay
    {
        wxString ipv4;
        wxUint16 portRangeMin;
        wxUint16 portRangeMax;
    };

    struct Location
    {
        wxString identifier;
        wxString description;
        wxVector<Relay> relays;
        long ping{-1}; // FIXME: this is r/w from diff threads
    };

    bool RestoreFromCache();
    bool UpdateFromJSON(wxString const &rawJSON, bool isAlreadyCached = false);

    wxUint64 GetRevision() const { return m_revision; }

    std::size_t NumLocations() const { return m_locations.size(); }
    Location const &GetLocation(std::size_t idx) const { return m_locations[idx]; }
    Location &GetLocation(std::size_t idx) { return m_locations[idx]; }

    friend void from_json(JSON const &json, ServerData &data);
    friend void from_json(JSON const &json, Location &location);
    friend void from_json(JSON const &json, Relay &relay);

private:
    wxUint64 m_revision;
    wxVector<Location> m_locations;
};
