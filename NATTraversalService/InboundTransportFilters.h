#pragma once

class CMainWorkerThread;
class CInboundTransportFilters
{
public:
    CInboundTransportFilters(
        CMainWorkerThread* pOwner
        );
    ~CInboundTransportFilters(void);
    CMainWorkerThread* m_pOwner;
    FWPM_FILTER_CONDITION0* m_pConditionsArray;
    FWPM_FILTER0* m_pFiltersArray;
    void CreateFilters(
        UINT32 nhRouterIPAddress
        );
};

