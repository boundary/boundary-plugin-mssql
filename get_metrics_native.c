#include <windows.h>
#include <pdh.h>
#include <stdio.h>

BOOL WINAPI GetCounterValues(LPTSTR serverName);

void main(int argc, char *argv[])
{
    if (argc > 1)
    {
        // argv[1] - Server Name
        GetCounterValues(argv[1]);
    }
    else
    {
        // Local System
        GetCounterValues(NULL);
    }
}

BOOL WINAPI GetCounterValues(LPTSTR serverName)
{
    PDH_STATUS s;

    HQUERY hQuery;

    // Array to specify the performance object, counter and instance for
    // which performance data should be collected.

    // typedef struct _PDH_COUNTER_PATH_ELEMENTS {
    //   LPTSTR  szMachineName;
    //   LPTSTR  szObjectName;
    //   LPTSTR  szInstanceName;
    //   LPTSTR  szParentInstance;
    //   DWORD   dwInstanceIndex;
    //   LPTSTR  szCounterName;
    // } PDH_COUNTER_PATH_ELEMENTS, *PPDH_COUNTER_PATH_ELEMENTS;

    // Each element in the array is a PDH_COUNTER_PATH_ELEMENTS structure.

    PDH_COUNTER_PATH_ELEMENTS cpe[] =
    {
        { NULL, "MSSQL$SQLSERVER2014:locks", "_total", NULL, -1, "lock waits/sec"},
        { NULL, "MSSQL$SQLSERVER2014:general statistics", NULL, NULL, -1, "active temp tables"},
        { NULL, "MSSQL$SQLSERVER2014:locks", "_total", NULL, -1, "average wait time (ms)"},
        { NULL, "MSSQL$SQLSERVER2014:locks", "_total", NULL, -1, "lock timeouts (timeout > 0)/sec"},
        { NULL, "MSSQL$SQLSERVER2014:wait statistics", "_total", NULL, -1, "page io latch waits"},
        { NULL, "MSSQL$SQLSERVER2014:databases", "_total", NULL, -1, "repl. pending xacts"},
        { NULL, "MSSQL$SQLSERVER2014:general statistics", NULL, NULL, -1, "logical connections"},
        { NULL, "MSSQL$SQLSERVER2014:locks", "_total", NULL, -1, "lock wait time (ms)"},
        { NULL, "MSSQL$SQLSERVER2014:general statistics", NULL, NULL, -1, "transactions"},
        { NULL, "MSSQL$SQLSERVER2014:general statistics", NULL, NULL, -1, "processes blocked"},
        { NULL, "MSSQL$SQLSERVER2014:sql statistics", NULL, NULL, -1, "sql compilations/sec"},
        { NULL, "MSSQL$SQLSERVER2014:general statistics", NULL, NULL, -1, "user connections"},
        { NULL, "MSSQL$SQLSERVER2014:locks", "_total", NULL, -1, "lock timeouts/sec"},
        { NULL, "MSSQL$SQLSERVER2014:databases", "_total", NULL, -1, "percent log used"}
    };

    HCOUNTER hCounter[sizeof(cpe)/sizeof(cpe[0])];

    char szFullPath[MAX_PATH];
    DWORD cbPathSize;
    int   i, j;

    BOOL  ret = FALSE;

    PDH_FMT_COUNTERVALUE counterValue;

    // Only do this setup once.
    if ((s = PdhOpenQuery(NULL, 0, &hQuery)) != ERROR_SUCCESS)
    {
        fprintf(stderr, "POQ failed %08x\n", s);
        return ret;
    }

    for (i = 0; i < sizeof(hCounter)/sizeof(hCounter[0]); i++)
    {
        cbPathSize = sizeof(szFullPath);

        cpe[i].szMachineName = serverName;

        if ((s = PdhMakeCounterPath(&cpe[i],
            szFullPath, &cbPathSize, 0)) != ERROR_SUCCESS)
        {
            fprintf(stderr,"MCP failed %08x\n", s);
            return ret;
        }

        /*if (cpe[i].szInstanceName)
        {
            printf("Adding [%s\\%s\\%s]\n",
                    cpe[i].szObjectName,
                    cpe[i].szCounterName,
                    cpe[i].szInstanceName);
        }
        else
            printf("Adding [%s\\%s]\n",
                    cpe[i].szObjectName,
                    cpe[i].szCounterName);
        */    
        if ((s = PdhAddCounter(hQuery, szFullPath, 0, &hCounter[i]))
            != ERROR_SUCCESS)
        {
            fprintf(stderr, "PAC failed %08x for %s\n", s, cpe[i].szCounterName);
            return ret;
        }
    }

    for (i = 0; i < 2; i++)
    {
        Sleep(100);

        // Collect data as often as you need to.
        if ((s = PdhCollectQueryData(hQuery)) != ERROR_SUCCESS)
        {
            fprintf(stderr, "PCQD failed %08x\n", s);
            return ret;
        }

        if (i == 0) continue;

        // Extract the calculated performance counter value for each counter or instance.
        for (j = 0; j < sizeof(hCounter)/sizeof(hCounter[0]); j++)
        {
            if ((s = PdhGetFormattedCounterValue(hCounter[j], PDH_FMT_DOUBLE,
                NULL, &counterValue)) != ERROR_SUCCESS)
            {
                //fprintf(stderr, "PGFCV failed %08x %d\n", s, hCounter[j]);
                continue;
            }
            if (cpe[j].szInstanceName)
            {
                printf("%s(%s)\\%s:%3.3f\n",
                    cpe[j].szObjectName,
                    cpe[j].szInstanceName,
                    cpe[j].szCounterName,
                    counterValue.doubleValue);
            }
            else
                printf("%s\\%s:%3.3f\n",
                    cpe[j].szObjectName,
                    cpe[j].szCounterName,
                    counterValue.doubleValue);
        }
    }

    // Remove all the counters from the query.
    for (i = 0; i < sizeof(hCounter)/sizeof(hCounter[0]); i++)
    {
        PdhRemoveCounter(hCounter[i]);
    }

    // Only do this cleanup once.
    PdhCloseQuery(hQuery);

    return TRUE;
}
