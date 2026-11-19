#include "fw_status.h"
#include "config.h"

void fw_finish(int success)
{
    REG_FW_STATUS = success ? 0xFFu : 0xBADu;

    while (1) {
    }
}