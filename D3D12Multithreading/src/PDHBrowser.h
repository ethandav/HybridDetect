#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <tchar.h>
#include <iostream>
#include <list>
#include <vector>
#include <string>
#pragma comment(lib, "pdh.lib")


struct PDHCounter
{
	LPWSTR name;
	bool isSelected;
	std::vector<wchar_t> path;

	void Query();
};

struct PDHInstance
{
	LPWSTR name;
	std::list<PDHCounter*> counterList;
};

struct PDHNode
{
	TCHAR* objectName;
	std::list<PDHInstance*> instanceList;

	void EnumCounters();
};

struct ActiveQueryList
{
	std::vector<PDHCounter*> queryList;

	void RunQueries();
};

void PDHCounter::Query()
{

}

void PDHNode::EnumCounters()
{
    PDH_STATUS status = ERROR_SUCCESS;
    LPWSTR pwsCounterListBuffer = NULL;
    DWORD dwCounterListSize = 0;
    LPWSTR pwsInstanceListBuffer = NULL;
    DWORD dwInstanceListSize = 0;
    LPWSTR pTemp = NULL;
    LPWSTR cTemp = NULL;

    status = PdhEnumObjectItems(
        NULL,
        NULL,
        objectName,
        pwsCounterListBuffer,
        &dwCounterListSize,
        pwsInstanceListBuffer,
        &dwInstanceListSize,
        PERF_DETAIL_WIZARD,
        0
	);

    if (status == PDH_MORE_DATA)
    {
        pwsCounterListBuffer = (LPWSTR)malloc(dwCounterListSize * sizeof(WCHAR));
        pwsInstanceListBuffer = (LPWSTR)malloc(dwInstanceListSize * sizeof(WCHAR));

        if (NULL != pwsCounterListBuffer && NULL != pwsInstanceListBuffer)
        {
            status = PdhEnumObjectItems(
                NULL,
                NULL,
                objectName,
                pwsCounterListBuffer,
                &dwCounterListSize,
                pwsInstanceListBuffer,
                &dwInstanceListSize,
                PERF_DETAIL_WIZARD,
                0
			);

            if (status == ERROR_SUCCESS)
            {

                for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1)
                {
					PDHInstance* instance = new PDHInstance;
					instance->name = pTemp;

					for (cTemp = pwsCounterListBuffer; *cTemp != 0; cTemp += wcslen(cTemp) + 1)
					{
						PDHCounter* counter = new PDHCounter;
						counter->name = cTemp;
						counter->isSelected = FALSE;

						PDH_COUNTER_PATH_ELEMENTS pathElements;
						pathElements.szObjectName = objectName;
						pathElements.szInstanceName = instance->name;
						pathElements.szCounterName = counter->name;
						pathElements.dwInstanceIndex = 0;
						pathElements.szMachineName = nullptr;
						pathElements.szParentInstance = nullptr;

						DWORD bufferSize = 0;

						PDH_STATUS status = PdhMakeCounterPath(
							&pathElements,
							nullptr,
							&bufferSize,
							0
						);

						if (status == PDH_MORE_DATA)
						{
							std::vector<wchar_t> counterPath(bufferSize);

							status = PdhMakeCounterPath(
								&pathElements,
								counterPath.data(),
								&bufferSize,
								0
							);

							counter->path = counterPath;
						}

						instance->counterList.push_back(counter);
					}

					instanceList.push_back(instance);
                }
            }
            else
            {
                wprintf(L"Second PdhEnumObjectItems failed with %0x%x.\n", status);
            }
        }
        else
        {
            //printf("Unable to allocate buffers.\n");
            status = ERROR_OUTOFMEMORY;
        }
    }
}

void ActiveQueryList::RunQueries()
{
}

std::list<PDHNode*> QueryPDHObjects()
{
	std::list<PDHNode*> pdhObjects;
	PDH_STATUS status;
	PDH_HQUERY query;
	PDH_HCOUNTER counter;
	DWORD bufferSize = 0;
	PDH_COUNTER_INFO counterInfo;

	// Initialize PDH Library
	status = PdhOpenQuery(nullptr, 0, &query);

	if (status != ERROR_SUCCESS)
	{
		std::cerr << "PdhOpenQuery failed with error code " << status << std::endl;
	}

	status = PdhEnumObjects(
		nullptr,
		nullptr,
		nullptr,
		&bufferSize,
		PERF_DETAIL_WIZARD,
		FALSE
	);

	if (status == PDH_MORE_DATA)
	{
		std::vector<wchar_t> objectList(bufferSize);
		status = PdhEnumObjects(
			nullptr,
			nullptr,
			objectList.data(),
			&bufferSize,
			PERF_DETAIL_WIZARD,
			FALSE
		);

		if (status == ERROR_SUCCESS)
		{
			wchar_t* objectName = objectList.data();
			while (*objectName)
			{
				DWORD counterListSize = 0;
				PDHNode* node = new PDHNode;
				node->objectName = _tcsdup(objectName);
				node->EnumCounters();

				pdhObjects.push_back(node);

					//std::vector <TCHAR> counterNames(counterListSize);

					//status = PdhEnumObjectItems(
					//	nullptr,
					//	nullptr,
					//	objectName,
					//	nullptr,
					//	&counterListSize,
					//	nullptr,
					//	&counterListSize,
					//	PERF_DETAIL_WIZARD,
					//	0
					//);

					//PDH_COUNTER_PATH_ELEMENTS pathElements;
					//pathElements.szObjectName = objectName;

					//for (DWORD i = 0; i < counterListSize; i++)
					//{
					//	pathElements.szCounterName = nullptr;
					//	pathElements.szInstanceName = nullptr;
					//	pathElements.szParentInstance = nullptr;

					//	status = PdhEnumObjectItems(
					//		nullptr,
					//		nullptr,
					//		objectName,
					//		nullptr,
					//		&counterListSize,
					//		nullptr,
					//		&counterListSize,
					//		PERF_DETAIL_WIZARD,
					//		0
					//	);

					//	status = PdhMakeCounterPath(
					//		&pathElements,
					//		nullptr,
					//		&counterListSize,
					//		0
					//	);

					//	if (status == PDH_MORE_DATA)
					//	{
					//		PDH_HCOUNTER hCounter;

					//		status = PdhAddCounter(
					//			query,
					//			pathElements.szCounterName,
					//			0,
					//			&hCounter
					//		);

					//		if (status == ERROR_SUCCESS)
					//		{
					//			status = PdhGetCounterInfo(
					//				hCounter,
					//				FALSE,
					//				&bufferSize,
					//				&counterInfo
					//			);
					//			
					//			if (status == ERROR_SUCCESS)
					//			{
					//				std::wcout << L" Counter: " << counterInfo.szObjectName << std::endl;
					//			}

					//			PdhRemoveCounter(hCounter);
					//		}

					//	}
					//}
				objectName += _tcslen(objectName) + 1;
			}
		}
	}
	return pdhObjects;
}
