#pragma once

class CMainWorkerThread;
class CDriverInitialize
{
public:
    CDriverInitialize(
        CMainWorkerThread* pOwner
        );
    ~CDriverInitialize(void);
    CMainWorkerThread* m_pOwner;
    void InitFilterDriver(
        UINT32 nhRouterIPAddress,
        int nSingleIP,
        int nRangeIP,
        int nSubnetIP,
        int nSinglePrivateIP,
        int nRangePrivateIP,
        int nSubnetPrivateIP,
        int nDisabledSinglePorts,
        int nDisabledRangePorts
        );
    BYTE* m_pInitByteBuffer;
};

