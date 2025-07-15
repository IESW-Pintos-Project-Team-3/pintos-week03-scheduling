/* Tests stride scheduling with 3 threads having tickets 50, 100, 150.
   Shows how many times each thread gets CPU time. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/timer.h"

static struct lock count_lock;
static int thread_counts[5] = {0, 0, 0, 0, 0}; // 각 스레드의 실행 횟수
static int thread_tickets[5] = {50 ,250, 500, 750, 1000}; // 각 스레드의 티켓 수
static bool test_finished = false;

struct thread_data {
  int id;
  int tickets;
};

static void stride_thread(void*);

void
test_stride_scheduling (void) 
{
  int i;
  struct thread_data data[5];
  
  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  lock_init (&count_lock);
  
  msg ("Starting stride scheduling test with 3 threads:");
  msg ("Thread 0: 50 tickets");
  msg ("Thread 1: 250 tickets");
  msg ("Thread 2: 500 tickets"); 
  msg ("Thread 3: 750 tickets");
  msg ("Thread 4: 1000 tickets");
  
  /* Create 3 threads with different ticket counts */
  for (i = 0; i < 5; i++) 
    {
      char name[32];
      snprintf (name, sizeof name, "stride-thread-%d", i);
      
      data[i].id = i;
      data[i].tickets = thread_tickets[i];
      
      thread_create (name, thread_tickets[i], stride_thread, &data[i]);
      msg ("Created thread %d with %d tickets", i, thread_tickets[i]);
    }

  /* Let threads run for a longer time */
  msg ("Running stride scheduling test for 6000 ticks...");
  timer_sleep (6000);

  /* Signal threads to finish */
  test_finished = true;
  
  /* Wait a bit more for threads to finish */
  timer_sleep (100);

  /* Print results */
  lock_acquire (&count_lock);
  msg ("Stride scheduling results:");
  
  int total_runs = 0;
  for (i = 0; i < 3; i++) {
    total_runs += thread_counts[i];
    msg ("Thread %d (tickets=%d): ran %d times", i, thread_tickets[i], thread_counts[i]);
  }
  
  msg ("Total runs: %d", total_runs);
  
  /* Show expected vs actual distribution (using integer arithmetic) */
  msg ("Expected vs Actual distribution:");
  for (i = 0; i < 3; i++) {
    int expected_percent = (thread_tickets[i] * 100) / 2550; // 250+500+750=1500
    int actual_percent = (thread_counts[i] * 100) / total_runs;
    msg ("Thread %d: expected %d%%, actual %d%%", i, expected_percent, actual_percent);
  }
  
  /* Check if stride scheduling is working */
  if (thread_counts[2] > thread_counts[1] && thread_counts[1] > thread_counts[0]) {
    msg ("Stride scheduling appears to be working correctly!");
  } else {
    msg ("Stride scheduling may not be working as expected.");
  }
  
  lock_release (&count_lock);
}

static void
stride_thread (void *aux) 
{
  struct thread_data *data = (struct thread_data *)aux;
  int my_id = data->id;
  
  while (!test_finished) 
    {  
      uint32_t *esp;
      asm ("mov %%esp, %0" : "=g" (esp));
      struct thread* t = pg_round_down (esp);
      int64_t old_pass = t->pass;
      printf("thread%d is scheduled, thread's pass = %lld\n", my_id, t->pass);
      lock_acquire (&count_lock);
      thread_counts[my_id]++;
      lock_release (&count_lock);
      while(old_pass == t->pass){
        asm ("mov %%esp, %0" : "=g" (esp));
        t = pg_round_down (esp);
      }
    }
}
