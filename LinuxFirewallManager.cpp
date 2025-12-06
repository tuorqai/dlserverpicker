
//------------------------------------------------------------------------------

#include "FirewallManager.h"

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
};

//------------------------------------------------------------------------------

LinuxFirewallManager::LinuxFirewallManager()
{
}

LinuxFirewallManager::~LinuxFirewallManager()
{
}

bool LinuxFirewallManager::IsFirewallEnabled()
{
    return true;
}

bool LinuxFirewallManager::IsLocationBlocked(ServerData::Location const &location)
{
    return false;
}

void LinuxFirewallManager::BlockLocation(ServerData::Location const &location)
{
}

void LinuxFirewallManager::UnblockLocation(ServerData::Location const &location)
{
}

void LinuxFirewallManager::Clear()
{
}

bool LinuxFirewallManager::IsClear()
{
    return true;
}

//------------------------------------------------------------------------------

FirewallManager *FirewallManager::Get()
{
    static LinuxFirewallManager instance;
    return &instance;
}
