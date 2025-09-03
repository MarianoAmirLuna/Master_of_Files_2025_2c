#ifndef TEMPORAL_EXT_H
#define TEMPORAL_EXT_H

#include "commons/temporal.h"
#include "stdio.h"

int temporal_is_empty(t_temporal* temp){
    if(temp == NULL)
        return 1;
    return temp->elapsed_ms=0;
}
void temporal_restart(t_temporal* temp){
    temp->status = TEMPORAL_STATUS_RUNNING;
    temp->elapsed_ms = 0;
    if(clock_gettime(CLOCK_MONOTONIC_RAW, &temp->current) == -1){
        printf("Error no se pudo obtener el tiempo exit(1) invoked");
        exit(1);
    }
}

#endif