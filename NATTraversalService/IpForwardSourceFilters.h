#pragma once

class CMainWorkerThread;
class CIpForwardSourceFilters
{
    struct _dist_conditions
    {
        FWPM_FILTER_CONDITION0  _conditions[2];
    };
public:
    CIpForwardSourceFilters(
        CMainWorkerThread* pOwner
        );
    ~CIpForwardSourceFilters(void);
    CMainWorkerThread* m_pOwner;
    _dist_conditions* m_pConditionsArray;
    FWPM_FILTER0* m_pFiltersArray;
    FWP_RANGE0* m_pIp4RangeArray;
    FWP_V4_ADDR_AND_MASK* m_pV4AddMaskArray;
    void CreateFilters(
        int nSingleIP,
        int nRangeIP,
        int nSubnetIP,
        UINT32 ulRouterIndex
        );
};

