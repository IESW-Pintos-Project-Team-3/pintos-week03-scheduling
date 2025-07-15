/* Tests lottery scheduling with 3 threads having tickets 50, 100, 150.
   Shows how many times each thread gets CPU time. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/timer.h"

static thread_func lottery_thread;
static struct lock count_lock;
static int thread_counts[5] = {0, 0, 0, 0, 0}; // 각 스레드의 실행 횟수
static int thread_tickets[5] = {50, 100, 150, 200, 250}; // 각 스레드의 티켓 수
static bool test_finished = false;

struct lottery_data {
  int id;
  int tickets;
};

void
test_lottery_scheduling (void) 
{
  int i;
  struct lottery_data data[5];
  
  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  lock_init (&count_lock);
  
  msg ("Starting lottery scheduling test with 3 threads:");
  msg ("Thread 0: 50 tickets");
  msg ("Thread 1: 100 tickets"); 
  msg ("Thread 2: 150 tickets");
  
  /* Create 3 threads with different ticket counts */
  for (i = 0; i < 5; i++) 
    {
      char name[32];
      snprintf (name, sizeof name, "lottery-thread-%d", i);
      
      data[i].id = i;
      data[i].tickets = thread_tickets[i];
      
      thread_create (name, thread_tickets[i], lottery_thread, &data[i]);
      msg ("Created thread %d with %d tickets", i, thread_tickets[i]);
    }

  /* Let threads run for a longer time */
  msg ("Running lottery scheduling test for 6000 ticks...");
  timer_sleep (6000);

  /* Signal threads to finish */
  test_finished = true;
  
  /* Wait a bit more for threads to finish */
  timer_sleep (100);

  /* Print results */
  lock_acquire (&count_lock);
  msg ("Lottery scheduling results:");
  
  int total_runs = 0;
  for (i = 0; i < 5; i++) {
    total_runs += thread_counts[i];
    msg ("Thread %d (tickets=%d): ran %d times", i, thread_tickets[i], thread_counts[i]);
  }
  
  msg ("Total runs: %d", total_runs);
  
  /* Show expected vs actual distribution (using integer arithmetic) */
  msg ("Expected vs Actual distribution:");
  for (i = 0; i < 5; i++) {
    int expected_percent = (thread_tickets[i] * 100) / 750;
    int actual_percent = (thread_counts[i] * 100) / total_runs;
    msg ("Thread %d: expected %d%%, actual %d%%", i, expected_percent, actual_percent);
  }
  
  /* Check if lottery scheduling is working */
  if (thread_counts[2] > thread_counts[1] && thread_counts[1] > thread_counts[0]) {
    msg ("Lottery scheduling appears to be working correctly!");
  } else {
    msg ("Lottery scheduling may not be working as expected.");
  }
  
  lock_release (&count_lock);
}

static void
lottery_thread (void *aux) 
{
  struct lottery_data *data = (struct lottery_data *)aux;
  int my_id = data->id;
  
  while (!test_finished) 
    {
      uint32_t *esp;
      asm ("mov %%esp, %0" : "=g" (esp));
      struct thread* t = pg_round_down (esp);
      msg("thread %d is scheduled, random_number: %lu",my_id,t->random_num);
      unsigned long old_random_num = t->random_num;
      lock_acquire (&count_lock);
      thread_counts[my_id]++;
      lock_release (&count_lock);
      // while(old_random_num == t->random_num){
      //   asm ("mov %%esp, %0" : "=g" (esp));
      //   t = pg_round_down (esp);
      // }
      thread_yield();
    }
}
