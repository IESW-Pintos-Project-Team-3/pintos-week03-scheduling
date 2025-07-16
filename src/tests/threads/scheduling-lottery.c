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
#include <random.h>

static struct lock count_lock;
static int thread_counts[3]; // 각 스레드의 실행 횟수
static int thread_tickets[3] = { 250, 500, 750 };
static bool test_finished;
static int64_t start_tick;
static int cnt = 0;
static int tick_thread_counts[30][3];
static inline uint64_t rdtsc(void) {
  unsigned hi, lo;
  __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

struct thread_data {
  int id;
  int tickets;
};

static void lottery_schedule(void*);

void
test_lottery_scheduling (void) 
{
  int i;
  struct thread_data data[3];
  
  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  lock_init (&count_lock);
  
  msg ("Starting lottery scheduling test with 5 threads:");
  msg ("Thread 0: 50 tickets");
  msg ("Thread 1: 250 tickets");
  msg ("Thread 2: 500 tickets"); 
  msg ("Thread 3: 750 tickets");
  msg ("Thread 4: 1000 tickets");
 
  /* Create 3 threads with different ticket counts */
  for (i = 0; i < 3; i++) 
    {
      char name[32];
      snprintf (name, sizeof name, "stride-thread-%d", i);
      
      data[i].id = i;
      data[i].tickets = thread_tickets[i];
      
      thread_create (name, thread_tickets[i], lottery_schedule, &data[i]);
      msg ("Created thread %d with %d tickets", i, thread_tickets[i]);
    }

  /* Let threads run for a longer time */
  msg ("Running stride scheduling test for 6000 ticks...");
  start_tick = timer_ticks();
  uint64_t start_cycles = rdtsc();
  timer_sleep (3000);

  /* Signal threads to finish */
  test_finished = true;
  uint64_t end_cycles = rdtsc();
  msg("Total CPU cycles spent during scheduling: %llu\n", end_cycles - start_cycles);
  
  /* Wait a bit more for threads to finish */
  timer_sleep (100);

  /* Print results */
  lock_acquire (&count_lock);
  msg ("lottery scheduling results:");
    
  /* Show expected vs actual distribution (using integer arithmetic) */
  msg ("Expected vs Actual distribution:");
  for (i = 0; i < cnt; i++) {
    int total_runs = 0;
    int fairness = 0;
    for (int j = 0; j < 3; j++) {
      total_runs += tick_thread_counts[i][j];
      // msg ("Thread %d (tickets=%d): ran %d times", j, thread_tickets[j], tick_thread_counts[i][j]);
    }
    msg ("%dtick result", (i+1)*100);
    for(int j = 0;j< 3;j++){
      int expected_percent = (thread_tickets[j] * 100000) / 1500; // 250+500+750=1500
      int actual_percent = (tick_thread_counts[i][j] * 100000) / total_runs;
      // msg ("Thread %d: expected %d%%, actual %d%%", j, expected_percent, actual_percent);
      fairness += abs(expected_percent-actual_percent);
    }
    msg("fairness = %d%%", (100000-fairness/3));
  }
  
  /* Check if stride scheduling is working */
  // if (thread_counts[4] > thread_counts[3] &&thread_counts[3] > thread_counts[2] &&thread_counts[2] > thread_counts[1] && thread_counts[1] > thread_counts[0]) {
  //   msg ("Stride scheduling appears to be working correctly!");
  // } else {
  //   msg ("Stride scheduling may not be working as expected.");
  // }
  
  lock_release (&count_lock);
}

static void
lottery_schedule (void *aux) 
{
  struct thread_data *data = (struct thread_data *)aux;
  int my_id = data->id;
  
  while (!test_finished) 
    {  
      uint32_t *esp;
      asm ("mov %%esp, %0" : "=g" (esp));
      struct thread* t = pg_round_down (esp);
      // printf("thread%d is scheduled, thread's pass = %lld\n", my_id, t->pass);
      unsigned long old_random = t->random_num;
      lock_acquire (&count_lock);
      thread_counts[my_id]++;
      int64_t timing = timer_ticks()-start_tick;
      if (timing > 0 && timing%100 == 0 ){
        for (int i = 0; i < 3; i++) {
          tick_thread_counts[cnt][i] += thread_counts[i]*4;
          // tick_thread_counts[cnt+1][i] = tick_thread_counts[cnt][i];
        }
        cnt++;
      }
      lock_release (&count_lock);
      while(old_random == t->random_num){
        asm ("mov %%esp, %0" : "=g" (esp));
        t = pg_round_down (esp);
      }
      // thread_yield();
    }
}
