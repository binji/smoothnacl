#ifndef TASK_QUEUE_H_
#define TASK_QUEUE_H_

#include <deque>

class Task;
typedef std::deque<Task*> TaskQueue;

#endif  // TASK_QUEUE_H_
