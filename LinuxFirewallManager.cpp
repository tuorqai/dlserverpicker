
//------------------------------------------------------------------------------

#include "FirewallManager.h"

#include <nftables/libnftables.h>
#include <nlohmann/json.hpp>
#include <sys/capability.h>

//------------------------------------------------------------------------------

class LinuxFirewallManager : public FirewallManager
{
public:
    LinuxFirewallManager();
    ~LinuxFirewallManager();

    bool CheckPermissions() override;
    bool IsFirewallEnabled() override;
    bool IsLocationBlocked(ServerData::Location const &location) override;

    void BlockLocation(ServerData::Location const &location);
    void UnblockLocation(ServerData::Location const &location);

    void Clear() override;
    bool IsClear() override;

private:
    nft_ctx *m_nftCtx;
};

//------------------------------------------------------------------------------

LinuxFirewallManager::LinuxFirewallManager()
{
    m_nftCtx = nft_ctx_new(NFT_CTX_DEFAULT);

    if (!m_nftCtx) {
        throw std::runtime_error("nftables: failed to get context");
    }

    nft_ctx_output_set_flags(m_nftCtx, NFT_CTX_OUTPUT_JSON);
}

LinuxFirewallManager::~LinuxFirewallManager()
{
    nft_ctx_free(m_nftCtx);
}

bool LinuxFirewallManager::CheckPermissions()
{
    bool isAdminCapSet = false;

    cap_t caps = cap_get_proc();

    if (caps) {
        cap_flag_value_t admin;
        cap_get_flag(caps, CAP_NET_ADMIN, CAP_EFFECTIVE, &admin);

        if (admin == CAP_SET) {
            isAdminCapSet = true;
        }

        cap_free(caps);
    }

    return isAdminCapSet;
}

bool LinuxFirewallManager::IsFirewallEnabled()
{
    return true;
}

bool LinuxFirewallManager::IsLocationBlocked(ServerData::Location const &location)
{
    nft_ctx_buffer_output(m_nftCtx);
    nft_ctx_buffer_error(m_nftCtx);

    int rc = nft_run_cmd_from_buffer(m_nftCtx, "list table inet dlserverpicker");

    if (rc != 0) {
        return false;
    }

    auto output = nlohmann::json::parse(nft_ctx_get_output_buffer(m_nftCtx));

    nft_ctx_unbuffer_error(m_nftCtx);
    nft_ctx_unbuffer_output(m_nftCtx);
    
    for (auto item : output["nftables"]) {
        if (!item["chain"].is_null()) {
            if (item["chain"]["table"] == "dlserverpicker") {
                if (item["chain"]["name"] == location.identifier.c_str()) {
                    return true;
                }
            }
        }
    }

    return false;
}

void LinuxFirewallManager::BlockLocation(ServerData::Location const &location)
{
    int rc = nft_run_cmd_from_buffer(m_nftCtx, "add table inet dlserverpicker");

    if (rc != 0) {
        throw std::runtime_error("nftables: failed to create table!");
    }

    rc = nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
        "add chain inet dlserverpicker %s { type filter hook output priority 0; }",
        location.identifier
    ));

    if (rc != 0) {
        throw std::runtime_error("nftables: failed to add chain!");
    }

    for (size_t i = 0; i < location.relays.size(); i++) {
        rc = nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
            "add rule inet dlserverpicker %s ip daddr %s drop",
            location.identifier, location.relays[i].ipv4));

        if (rc != 0) {
            nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
                "delete chain inet dlserverpicker %s",
                location.identifier));

            throw std::runtime_error("nftables: failed to add rule!");
        }
    }
}

void LinuxFirewallManager::UnblockLocation(ServerData::Location const &location)
{
    nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
        "flush chain inet dlserverpicker %s",
        location.identifier));

    nft_run_cmd_from_buffer(m_nftCtx, wxString::Format(
        "delete chain inet dlserverpicker %s",
        location.identifier));
}

void LinuxFirewallManager::Clear()
{
    nft_run_cmd_from_buffer(m_nftCtx, "destroy table inet dlserverpicker");
}

bool LinuxFirewallManager::IsClear()
{
    nft_ctx_buffer_output(m_nftCtx);
    nft_ctx_buffer_error(m_nftCtx);

    int rc = nft_run_cmd_from_buffer(m_nftCtx, "list table inet dlserverpicker");

    nft_ctx_unbuffer_error(m_nftCtx);
    nft_ctx_unbuffer_output(m_nftCtx);

    if (rc != 0) {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

FirewallManager *FirewallManager::Get()
{
    static LinuxFirewallManager instance;
    return &instance;
}
