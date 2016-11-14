#pragma once

class CMainWorkerThread;
class CIpForwardDistinationFilters
{
public:
    CIpForwardDistinationFilters(
        CMainWorkerThread* pOwner
        );
    ~CIpForwardDistinationFilters(void);
    CMainWorkerThread* m_pOwner;
    FWPM_FILTER_CONDITION0* m_pConditionsArray;
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

