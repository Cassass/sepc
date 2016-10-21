#ifndef __JOBS_HANDLER_H
#define __JOBS_HANDLER_H

void add_pid_list(int p, char *cmd);
void delete_pid(int to_destroy);
void check_running(void);
void jobs(void);

#endif

