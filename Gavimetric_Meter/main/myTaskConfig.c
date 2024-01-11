#include "myTaskConfig.h"

void taskConfigSet(taskConfig_t* pTaskConfig, uint32_t periodo, const char* tag)
{
    pTaskConfig->periodo = periodo;
    pTaskConfig->numActivaciones = 0;
    pTaskConfig->tag = tag;

    return;
}

void taskInfoSet(taskInfo_t* pTaskInfo, taskConfig_t* pTaskConfig, void* pTaskData)
{
    pTaskInfo->pConfig = pTaskConfig;
    pTaskInfo->pData   = pTaskData;

    return;
}
