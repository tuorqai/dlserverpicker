//------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------

#include "ServerData.h"

//------------------------------------------------------------------------------

class FirewallManager
{
public:
    static FirewallManager *Get();

    FirewallManager() = default;
    virtual ~FirewallManager() = default;

    virtual bool IsFirewallEnabled() = 0;
    virtual bool IsLocationBlocked(ServerData::Location const &location) = 0;

    virtual void BlockLocation(ServerData::Location const &location) = 0;
    virtual void UnblockLocation(ServerData::Location const &location) = 0;

    virtual void Clear() = 0;
    virtual bool IsClear() = 0;
};
