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
#include "threads/interrupt.h"

static struct lock count_lock;
static int thread_counts[30]; // 각 스레드의 실행 횟수
static int thread_tickets[30] = {300, 400, 500, 600, 700, 300, 400, 500, 600, 700, 300, 400, 500, 600, 700, 300, 400, 500, 600, 700, 300, 400, 500, 600, 700, 300, 400, 500, 600, 700};
// static int thread_tickets[10] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500}; // 각 스레드의 티켓 수
static bool test_finished = false;
static int64_t start_tick;
static int cnt = 0;
static int tick_thread_counts[60][10];
static int prev_thread_counts[30];

static inline uint64_t rdtsc(void) {
  unsigned hi, lo;
  __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

struct thread_data {
  int id;
  int tickets;
};

static void stride_thread(void*);

void
test_stride_scheduling (void) 
{
  int i;
  struct thread_data data[30];
  
  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  lock_init (&count_lock);
  enum intr_level old_level = intr_disable ();
  msg ("Starting stride scheduling test with 3 threads:");
  msg ("Thread 0: 50 tickets");
  msg ("Thread 1: 250 tickets");
  msg ("Thread 2: 500 tickets"); 
  msg ("Thread 3: 750 tickets");
  msg ("Thread 4: 1000 tickets");
  
  /* Create 3 threads with different ticket counts */
  for (i = 0; i < 30; i++) 
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
  start_tick = timer_ticks();
  intr_set_level (old_level);
  uint64_t start_cycles = rdtsc();
  timer_sleep (6000);

  /* Signal threads to finish */
  test_finished = true;
  uint64_t end_cycles = rdtsc();
  msg("Total CPU cycles spent during scheduling: %llu\n", end_cycles - start_cycles);
  
  /* Wait a bit more for threads to finish */
  timer_sleep (100);

  /* Print results */
  // lock_acquire (&count_lock);
  // msg ("Stride scheduling results:");
    
  // /* Show expected vs actual distribution (using integer arithmetic) */
  // msg ("Expected vs Actual distribution:");
  // for (i = 0; i < cnt; i++) {
  //   int fairness = 0;
  //   int total_runs = 0;
  //   bool is_sleep = false;
  //   int total_tickets = 5000;
  //   for (int j = 0; j < 10; j++) {
  //     total_runs += tick_thread_counts[i][j];
  //     // if (tick_thread_counts[i][j] == 0){
  //     //     is_sleep = true;
  //     //     total_tickets -= thread_tickets[j];
  //     // }
  //     // msg ("Thread %d (tickets=%d): ran %d times", j, thread_tickets[j], tick_thread_counts[i][j]);
  //   }
  //   // msg ("Interval %d (%d-%d ticks) results:", i + 1, i * 100, (i + 1) * 100);
  //   // if (is_sleep){
  //   //   for(int j = 0;j< 3;j++){
  //   //   int expected_percent;
  //   //   if (tick_thread_counts[i][j])
  //   //    expected_percent = (thread_tickets[j] * 100*1000) / total_tickets;
  //   //   else
  //   //    expected_percent = 0; // 250+500+750=1500
  //   //   int actual_percent = (tick_thread_counts[i][j] * 100*1000) / total_runs;
  //   // //  msg ("  Thread %d (tickets=%d): ran %d times, expected %d.%03d%%, actual %d.%03d%%",
  //   // //        j, thread_tickets[j], tick_thread_counts[i][j],
  //   // //        expected_percent / 1000, expected_percent % 1000, // 소수점 3자리까지 출력
  //   // //        actual_percent / 1000, actual_percent % 1000);
  //   // fairness += abs(expected_percent-actual_percent);
  //   // }
  //   // }else{
  //   // msg ("Interval %d (%d-%d ticks) results:", i + 1, i * 100, (i + 1) * 100);
  //   for(int j = 0;j< 10;j++){
  //     int expected_percent = (thread_tickets[j] * 100*1000) / total_tickets; // 250+500+750=1500
  //     int actual_percent = (tick_thread_counts[i][j] * 100*1000) / total_runs;
  //     msg ("  Thread %d (tickets=%d): ran %d times, expected %d.%03d%%, actual %d.%03d%%",
  //          j, thread_tickets[j], tick_thread_counts[i][j],
  //          expected_percent / 1000, expected_percent % 1000, // 소수점 3자리까지 출력
  //          actual_percent / 1000, actual_percent % 1000);
  //      fairness += abs(expected_percent-actual_percent);
  //    }
  //   // }
  //   msg("  Fairness for this interval (100.000%% - avg_deviation): %d.%03d%%",
  //       (100000 - fairness/10) / 1000, (100000 - fairness/10) % 1000);

  // }
  
  /* Check if stride scheduling is working */
  // if (thread_counts[4] > thread_counts[3] &&thread_counts[3] > thread_counts[2] &&thread_counts[2] > thread_counts[1] && thread_counts[1] > thread_counts[0]) {
  //   msg ("Stride scheduling appears to be working correctly!");
  // } else {
  //   msg ("Stride scheduling may not be working as expected.");
  // }
  
  // lock_release (&count_lock);
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
      // printf("thread%d is scheduled, thread's pass = %lld\n", my_id, t->pass);
      // lock_acquire (&count_lock);
      // thread_counts[my_id]++;
      // int64_t current_tick = timer_ticks(); // 현재 타이머 틱
      // int64_t elapsed_since_start = current_tick - start_tick;
      // if (elapsed_since_start >= 200){
      //   for (int i = 0; i < 10; i++) {
      //     tick_thread_counts[cnt][i] = thread_counts[i] - prev_thread_counts[i];
      //     prev_thread_counts[i] = thread_counts[i]; 
      //     // tick_thread_counts[cnt+1][i] = tick_thread_counts[cnt][i];
      //   }
      //   start_tick = current_tick;
      //   cnt++;
      //   // if(cnt == 1 || cnt == 10 || cnt == 20){
      //   //     lock_release (&count_lock);
      //   //     timer_sleep(300);
      //   //     lock_acquire (&count_lock);
      //   // }
      // }
      // lock_release (&count_lock);
      int64_t old_pass = t->pass;
      while(old_pass == t->pass){
        asm ("mov %%esp, %0" : "=g" (esp));
        t = pg_round_down (esp);
      }
    }
}
