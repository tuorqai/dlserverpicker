//------------------------------------------------------------------------------
// WindowsFirewallManager.cpp: implementation of FirewallManager for Windows
//------------------------------------------------------------------------------

#include "FirewallManager.h"
#include <stdexcept>
#include <windows.h>
#include <atlcomcli.h>
#include <netfw.h>
#include "ServerData.h"

//------------------------------------------------------------------------------

class WindowsError : public std::exception
{
public:
    WindowsError(char const *description, HRESULT hr)
        : m_message(wxString::Format("%s [0x%08lx]", description, hr))
    {
    }

    char const *what() const override
    {
        return m_message;
    }

private:
    wxString m_message;
};

//------------------------------------------------------------------------------

class FirewallManagerWindows : public FirewallManager
{
public:
    FirewallManagerWindows();
    ~FirewallManagerWindows();

    bool IsFirewallEnabled() override;
    bool IsLocationBlocked(ServerData::Location const &location) override;

    void BlockLocation(ServerData::Location const &location) override;
    void UnblockLocation(ServerData::Location const &location) override;

    void Clear() override;
    bool IsClear() override;

private:
    void CollectSDRRules();
    void CollectIfSDRRule(INetFwRule *pNetFwRule);
    void RemoveRule(INetFwRule *pNetFwRule);

    HRESULT m_hrCoInitEx;
    INetFwPolicy2 *m_pNetFwPolicy2;
    INetFwRules *m_pNetFwRules;
    std::unordered_map<wxString, INetFwRule *> m_sdrRuleMap;
};

//------------------------------------------------------------------------------

FirewallManagerWindows::FirewallManagerWindows()
{
    m_hrCoInitEx = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    if (FAILED(m_hrCoInitEx)) {
        throw WindowsError("Failed to initialize COM", m_hrCoInitEx);
    }

    HRESULT hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        reinterpret_cast<void **>(&m_pNetFwPolicy2)
    );

    if (FAILED(hr)) {
        throw WindowsError("Unable to get firewall policy, CoCreateInstance() failed", hr);
    }

    hr = m_pNetFwPolicy2->get_Rules(&m_pNetFwRules);

    if (FAILED(hr)) {
        throw WindowsError("Unable to fetch firewall rules, INetFwPolicy2::get_Rules() failed", hr);
    }

    CollectSDRRules();
}

FirewallManagerWindows::~FirewallManagerWindows()
{
    for (auto const &rule : m_sdrRuleMap) {
        rule.second->Release();
    }

    if (m_pNetFwRules != nullptr) {
        m_pNetFwRules->Release();
    }

    if (m_pNetFwPolicy2 != nullptr) {
        m_pNetFwPolicy2->Release();
    }

    if (SUCCEEDED(m_hrCoInitEx)) {
        CoUninitialize();
    }
}

bool FirewallManagerWindows::IsFirewallEnabled()
{
    // I'm really not sure if this is the correct way to check this.

    long currentProfileBits;
    HRESULT hr = m_pNetFwPolicy2->get_CurrentProfileTypes(&currentProfileBits);

    if (FAILED(hr)) {
        throw WindowsError("Unable to determine current firewall profile mask, INetFwPolicy2::get_CurrentProfileTypes() failed", hr);
    }

    if (currentProfileBits == 0) {
        return false;
    }

    NET_FW_PROFILE_TYPE2 profiles[3] = {
        NET_FW_PROFILE2_DOMAIN,
        NET_FW_PROFILE2_PRIVATE,
        NET_FW_PROFILE2_PUBLIC
    };

    long enabledMask = 0;

    for (int i = 0; i < 3; i++) {
        if (!(currentProfileBits & profiles[i])) {
            continue;
        }

        VARIANT_BOOL bEnabled = 0;
        hr = m_pNetFwPolicy2->get_FirewallEnabled(profiles[i], &bEnabled);

        if (FAILED(hr)) {
            throw WindowsError("get_FirewallEnabled() failed", hr);
        }

        if (bEnabled) {
            enabledMask |= (1 << i);
        }
    }

    // Seems to be a good plan, eh?
    return enabledMask != 0;
}

bool FirewallManagerWindows::IsLocationBlocked(ServerData::Location const &location)
{
    if (m_sdrRuleMap.count(location.identifier) == 1) {
        return true;
    }

    return false;
}

void FirewallManagerWindows::BlockLocation(ServerData::Location const &location)
{
    if (m_sdrRuleMap.count(location.identifier) == 1) {
        return;
    }

    wxString addrStr;

    for (ServerData::Relay const &relay : location.relays) {
        addrStr.Append(relay.ipv4);
        addrStr.Append(",");
    }

    addrStr.RemoveLast();

    BSTR bStrName = SysAllocString(wxString::Format("valvesdr_%s", location.identifier).wc_str());
    BSTR bStrDesc = SysAllocString(location.description.wc_str());
    BSTR bStrAddr = SysAllocString(addrStr);

    INetFwRule *pNetFwRule = nullptr;

    HRESULT hr = CoCreateInstance(
        __uuidof(NetFwRule),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwRule),
        reinterpret_cast<void **>(&pNetFwRule)
    );

    if (FAILED(hr)) {
        throw WindowsError("Unable to create INetFwRule, CoCreateInstance() failed", hr);
    }

    pNetFwRule->put_Name(bStrName);
    pNetFwRule->put_Description(bStrDesc);
    pNetFwRule->put_Profiles(NET_FW_PROFILE2_ALL);
    pNetFwRule->put_Action(NET_FW_ACTION_BLOCK);
    pNetFwRule->put_Direction(NET_FW_RULE_DIR_OUT);
    pNetFwRule->put_RemoteAddresses(bStrAddr);
    pNetFwRule->put_Enabled(VARIANT_TRUE);

    hr = m_pNetFwRules->Add(pNetFwRule);

    if (FAILED(hr)) {
        throw WindowsError("Unable to add firewall rule, INetFwRules::Add() failed", hr);
    }

    m_sdrRuleMap[location.identifier] = pNetFwRule;

    SysFreeString(bStrName);
    SysFreeString(bStrDesc);
    SysFreeString(bStrAddr);
}

void FirewallManagerWindows::UnblockLocation(ServerData::Location const &location)
{
    if (m_sdrRuleMap.count(location.identifier) == 0) {
        return;
    }

    RemoveRule(m_sdrRuleMap[location.identifier]);
    m_sdrRuleMap.erase(location.identifier);
}

void FirewallManagerWindows::Clear()
{
    for (auto const &rule : m_sdrRuleMap) {
        RemoveRule(rule.second);
    }

    m_sdrRuleMap.clear();
}

bool FirewallManagerWindows::IsClear()
{
    return m_sdrRuleMap.empty();
}

void FirewallManagerWindows::CollectSDRRules()
{
    long ruleCount;
    IUnknown *pRuleEnum;

    HRESULT hr = m_pNetFwRules->get_Count(&ruleCount);

    if (FAILED(hr)) {
        throw WindowsError("Unable to get firewall rule count, INetFwRules::get_Count() failed", hr);
    }

    m_pNetFwRules->get__NewEnum(&pRuleEnum);

    if (pRuleEnum != nullptr) {
        IEnumVARIANT *pVariant = nullptr;

        HRESULT hr = pRuleEnum->QueryInterface(
            __uuidof(IEnumVARIANT),
            reinterpret_cast<void **>(&pVariant)
        );

        CComVariant var;
        ULONG cf;

        while (SUCCEEDED(hr) && hr != S_FALSE) {
            INetFwRule *pFwRule = nullptr;
            hr = pVariant->Next(1, &var, &cf);

            if (hr != S_FALSE) {
                if (SUCCEEDED(hr)) {
                    hr = var.ChangeType(VT_DISPATCH);
                }

                if (SUCCEEDED(hr)) {
                    hr = V_DISPATCH(&var)->QueryInterface(
                        __uuidof(INetFwRule),
                        reinterpret_cast<void **>(&pFwRule)
                    );
                }

                if (SUCCEEDED(hr)) {
                    CollectIfSDRRule(pFwRule);
                }
            }
        }
    }
}

void FirewallManagerWindows::CollectIfSDRRule(INetFwRule *pNetFwRule)
{
    BSTR bStrName;

    if (SUCCEEDED(pNetFwRule->get_Name(&bStrName))) {
        wxString identifier;

        if (wxString(bStrName).StartsWith("valvesdr_", &identifier)) {
            m_sdrRuleMap[identifier] = pNetFwRule;
        } else {
            pNetFwRule->Release();
        }
    }
}

void FirewallManagerWindows::RemoveRule(INetFwRule *pNetFwRule)
{
    BSTR bStrName;
    HRESULT hr = pNetFwRule->get_Name(&bStrName);

    if (FAILED(hr)) {
        throw WindowsError("Unable to remove firewall rule, INetFwRules::get_Name() failed", hr);
    }

    hr = m_pNetFwRules->Remove(bStrName);

    if (FAILED(hr)) {
        throw WindowsError("Unable to remove firewall rule, INetFwRules::Remove() failed", hr);
    }

    pNetFwRule->Release();
}

//------------------------------------------------------------------------------

FirewallManager *FirewallManager::Get()
{
    static FirewallManagerWindows instance;
    return &instance;
}
