
//------------------------------------------------------------------------------

#include "FirewallManager.h"

#include <nftables/libnftables.h>
#include <nlohmann/json.hpp>

//------------------------------------------------------------------------------

class LinuxFirewallManager : public FirewallManager
{
public:
    LinuxFirewallManager();
    ~LinuxFirewallManager();

    bool IsFirewallEnabled() override;
    bool IsLocationBlocked(ServerData::Location const &location) override;

    void BlockLocation(ServerData::Location const &location);
    void UnblockLocation(ServerData::Location const &location);

    void Clear() override;
    bool IsClear() override;

private:
    void CheckTables();

    nft_ctx *m_nftCtx;
    std::vector<wxString> m_definedRules;
};

//------------------------------------------------------------------------------

LinuxFirewallManager::LinuxFirewallManager()
{
    m_nftCtx = nft_ctx_new(NFT_CTX_DEFAULT);

    nft_ctx_buffer_output(m_nftCtx);
    nft_ctx_output_set_flags(m_nftCtx, NFT_CTX_OUTPUT_JSON);

    CheckTables();
}

LinuxFirewallManager::~LinuxFirewallManager()
{
    nft_ctx_free(m_nftCtx);
}

bool LinuxFirewallManager::IsFirewallEnabled()
{
    return true;
}

bool LinuxFirewallManager::IsLocationBlocked(ServerData::Location const &location)
{
    wxString rule = wxString::Format("deadlock_%s", location.identifier);

    return std::find(m_definedRules.begin(), m_definedRules.end(), rule) != m_definedRules.end();
}

void LinuxFirewallManager::BlockLocation(ServerData::Location const &location)
{
    wxString rule = wxString::Format("deadlock_%s", location.identifier);

    nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
        "add table inet %s", rule
    ));

    nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
        "add chain inet %s output { type filter hook output priority 0; }",
        rule
    ));

    for (size_t i = 0; i < location.relays.size(); i++) {
        nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
            "add rule inet %s output ip daddr %s drop",
            rule, location.relays[i].ipv4
        ));
    }

    m_definedRules.emplace_back(rule);
}

void LinuxFirewallManager::UnblockLocation(ServerData::Location const &location)
{
    wxString rule = wxString::Format("deadlock_%s", location.identifier);

    int rc = nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
        "destroy table inet %s", rule
    ));

    if (rc == 0) {
        m_definedRules.erase(std::find(m_definedRules.begin(), m_definedRules.end(), rule));
    }
}

void LinuxFirewallManager::Clear()
{
    CheckTables(); // just in case(?)

    for (wxString const &rule : m_definedRules) {
        nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
            "destroy table inet %s", rule
        ));
    }

    m_definedRules.clear();
}

bool LinuxFirewallManager::IsClear()
{
    return m_definedRules.empty();
}

void LinuxFirewallManager::CheckTables()
{
    m_definedRules.clear();

    nft_run_cmd_from_buffer(m_nftCtx, "list tables inet");

    auto output = nlohmann::json::parse(nft_ctx_get_output_buffer(m_nftCtx));
    auto nftables = output["nftables"];

    if (nftables.is_null()) {
        return;
    }

    for (auto item : nftables) {
        auto table = item["table"];

        if (table.is_null()) {
            continue;
        }

        wxString name = table["name"].get<std::string>();

        if (name.StartsWith("deadlock_")) {
            m_definedRules.emplace_back(name);
        }
    }
}

//------------------------------------------------------------------------------

FirewallManager *FirewallManager::Get()
{
    static LinuxFirewallManager instance;
    return &instance;
}
