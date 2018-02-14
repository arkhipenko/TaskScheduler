/**
    TaskScheduler Test sketch - test of Task destructor
    Test case:
      Main task runs every 100 milliseconds 100 times and in 50% cases generates a task object
      which runs 1 to 10 times with 100 ms to 5 s interval, and then destroyed.

      This sketch uses a FreeMemory library: https://github.com/McNeight/MemoryFree
*/

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN  // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#include <TaskScheduler.h>

#include <MemoryFree.h>

Scheduler ts;

// Callback methods prototypes
void MainLoop();

// Statis task
Task tMain(100*TASK_MILLISECOND, 100, MainLoop, &ts, true);


void Iteration();
bool OnEnable();
void OnDisable();

int noOfTasks = 0;

void MainLoop() {
  Serial.print(millis()); Serial.print("\t");
  Serial.print("MainLoop run: ");
  int i = tMain.getRunCounter();
  Serial.print(i); Serial.print(F(".\t"));

  if ( random(0, 101) > 50 ) {  // generate a new task only in 50% of cases
    // Generating another task
    long p = random(100, 5001); // from 100 ms to 5 seconds
    long j = random(1, 11); // from 1 to 10 iterations)
    Task *t = new Task(p, j, Iteration, &ts, false, OnEnable, OnDisable);

    Serial.print(F("Generated a new task:\t")); Serial.print(t->getId()); Serial.print(F("\tInt, Iter = \t"));
    Serial.print(p); Serial.print(", "); Serial.print(j); Serial.print(F("\tFree mem="));
    Serial.print(freeMemory()); Serial.print(F("\tNo of tasks=")); Serial.println(++noOfTasks);
    t->enable();
  }
  else {
    Serial.println(F("Skipped generating a task"));
  }
}


void Iteration() {
  Task &t = ts.currentTask();

  Serial.print(millis()); Serial.print("\t");
  Serial.print("Task N"); Serial.print(t.getId()); Serial.print(F("\tcurrent iteration: "));
  int i = t.getRunCounter();
  Serial.println(i);
}

bool OnEnable() {
  // to-do: think of something to put in here.
  return  true;
}

void OnDisable() {
  Task *t = &ts.currentTask();
  unsigned int tid = t->getId();
    
  delete t;
  Serial.print(millis()); Serial.print("\t");
  Serial.print("Task N"); Serial.print(tid); Serial.print(F("\tfinished and destroyed.\tFree mem="));
  Serial.print(freeMemory());Serial.print(F("\tNo of tasks=")); Serial.println(--noOfTasks);
}

/**
   Standard Arduino setup and loop methods
*/
void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(0) + analogRead(5));
  noOfTasks = 0;

  Serial.println(F("Dynamic Task Creation/Destruction Example"));
  Serial.println();
  Serial.print(F("Free mem="));
  Serial.print(freeMemory()); Serial.print(F("\tNo of tasks=")); Serial.println(noOfTasks);
  Serial.println();
}

void loop() {
  ts.execute();
}


/* Output on Arduino Uno:
 *  
 *  Compile:

Sketch uses 5312 bytes (16%) of program storage space. Maximum is 32256 bytes.
Global variables use 282 bytes (13%) of dynamic memory, leaving 1766 bytes for local variables. Maximum is 2048 bytes.

 * Execution: 
  
Dynamic Task Creation/Destruction Example

Free mem=1758  No of tasks=0

1 MainLoop run: 1.  Generated a new task: 2 Int, Iter =   421, 3  Free mem=1701 No of tasks=1
8 Task N2 current iteration: 1
100 MainLoop run: 2.  Generated a new task: 3 Int, Iter =   4099, 9 Free mem=1656 No of tasks=2
102 Task N3 current iteration: 1
200 MainLoop run: 3.  Skipped generating a task
300 MainLoop run: 4.  Generated a new task: 4 Int, Iter =   1795, 1 Free mem=1611 No of tasks=3
302 Task N4 current iteration: 1
305 Task N4 finished and destroyed. Free mem=1613 No of tasks=2
400 MainLoop run: 5.  Skipped generating a task
429 Task N2 current iteration: 2
500 MainLoop run: 6.  Skipped generating a task
600 MainLoop run: 7.  Skipped generating a task
700 MainLoop run: 8.  Generated a new task: 5 Int, Iter =   4623, 7 Free mem=1611 No of tasks=3
702 Task N5 current iteration: 1
800 MainLoop run: 9.  Generated a new task: 6 Int, Iter =   4987, 4 Free mem=1566 No of tasks=4
802 Task N6 current iteration: 1
850 Task N2 current iteration: 3
850 Task N2 finished and destroyed. Free mem=1568 No of tasks=3
900 MainLoop run: 10. Generated a new task: 7 Int, Iter =   600, 4  Free mem=1566 No of tasks=4
902 Task N7 current iteration: 1
1000  MainLoop run: 11. Skipped generating a task
1100  MainLoop run: 12. Generated a new task: 8 Int, Iter =   2530, 1 Free mem=1521 No of tasks=5
1102  Task N8 current iteration: 1
1105  Task N8 finished and destroyed. Free mem=1523 No of tasks=4
1200  MainLoop run: 13. Skipped generating a task
1300  MainLoop run: 14. Generated a new task: 9 Int, Iter =   2215, 7 Free mem=1521 No of tasks=5
1302  Task N9 current iteration: 1
1400  MainLoop run: 15. Skipped generating a task
1500  MainLoop run: 16. Skipped generating a task
1502  Task N7 current iteration: 2
1600  MainLoop run: 17. Skipped generating a task
1700  MainLoop run: 18. Skipped generating a task
1800  MainLoop run: 19. Skipped generating a task
1900  MainLoop run: 20. Skipped generating a task
2000  MainLoop run: 21. Generated a new task: 10  Int, Iter =   189, 10 Free mem=1476 No of tasks=6
2002  Task N10  current iteration: 1
2100  MainLoop run: 22. Generated a new task: 11  Int, Iter =   2898, 9 Free mem=1431 No of tasks=7
2102  Task N7 current iteration: 3
2105  Task N11  current iteration: 1
2191  Task N10  current iteration: 2
2200  MainLoop run: 23. Generated a new task: 12  Int, Iter =   1691, 6 Free mem=1386 No of tasks=8
2202  Task N12  current iteration: 1
2300  MainLoop run: 24. Generated a new task: 13  Int, Iter =   1448, 7 Free mem=1341 No of tasks=9
2304  Task N13  current iteration: 1
2380  Task N10  current iteration: 3
2400  MainLoop run: 25. Generated a new task: 14  Int, Iter =   3919, 7 Free mem=1296 No of tasks=10
2403  Task N14  current iteration: 1
2500  MainLoop run: 26. Generated a new task: 15  Int, Iter =   3745, 5 Free mem=1251 No of tasks=11
2503  Task N15  current iteration: 1
2569  Task N10  current iteration: 4
2600  MainLoop run: 27. Skipped generating a task
2700  MainLoop run: 28. Skipped generating a task
2702  Task N7 current iteration: 4
2702  Task N7 finished and destroyed. Free mem=1253 No of tasks=10
2758  Task N10  current iteration: 5
2800  MainLoop run: 29. Generated a new task: 16  Int, Iter =   2144, 1 Free mem=1251 No of tasks=11
2803  Task N16  current iteration: 1
2806  Task N16  finished and destroyed. Free mem=1253 No of tasks=10
2900  MainLoop run: 30. Generated a new task: 17  Int, Iter =   4618, 10  Free mem=1251 No of tasks=11
2904  Task N17  current iteration: 1
2947  Task N10  current iteration: 6
3000  MainLoop run: 31. Skipped generating a task
3100  MainLoop run: 32. Generated a new task: 18  Int, Iter =   2885, 6 Free mem=1206 No of tasks=12
3103  Task N18  current iteration: 1
3136  Task N10  current iteration: 7
3200  MainLoop run: 33. Skipped generating a task
3300  MainLoop run: 34. Skipped generating a task
3325  Task N10  current iteration: 8
3400  MainLoop run: 35. Skipped generating a task
3500  MainLoop run: 36. Generated a new task: 19  Int, Iter =   2250, 4 Free mem=1161 No of tasks=13
3503  Task N19  current iteration: 1
3514  Task N10  current iteration: 9
3518  Task N9 current iteration: 2
3600  MainLoop run: 37. Skipped generating a task
3700  MainLoop run: 38. Generated a new task: 20  Int, Iter =   1689, 7 Free mem=1116 No of tasks=14
3703  Task N10  current iteration: 10
3706  Task N20  current iteration: 1
3709  Task N10  finished and destroyed. Free mem=1118 No of tasks=13
3750  Task N13  current iteration: 2
3800  MainLoop run: 39. Generated a new task: 21  Int, Iter =   2607, 5 Free mem=1116 No of tasks=14
3803  Task N21  current iteration: 1
3893  Task N12  current iteration: 2
3900  MainLoop run: 40. Generated a new task: 22  Int, Iter =   1390, 6 Free mem=1071 No of tasks=15
3903  Task N22  current iteration: 1
4000  MainLoop run: 41. Generated a new task: 23  Int, Iter =   3340, 8 Free mem=1026 No of tasks=16
4003  Task N23  current iteration: 1
4100  MainLoop run: 42. Skipped generating a task
4200  MainLoop run: 43. Skipped generating a task
4201  Task N3 current iteration: 2
4300  MainLoop run: 44. Skipped generating a task
4400  MainLoop run: 45. Generated a new task: 24  Int, Iter =   4083, 2 Free mem=981  No of tasks=17
4403  Task N24  current iteration: 1
4500  MainLoop run: 46. Generated a new task: 25  Int, Iter =   4510, 1 Free mem=936  No of tasks=18
4503  Task N25  current iteration: 1
4506  Task N25  finished and destroyed. Free mem=938  No of tasks=17
4600  MainLoop run: 47. Generated a new task: 26  Int, Iter =   4782, 10  Free mem=936  No of tasks=18
4603  Task N26  current iteration: 1
4700  MainLoop run: 48. Generated a new task: 27  Int, Iter =   641, 6  Free mem=891  No of tasks=19
4702  Task N27  current iteration: 1
4800  MainLoop run: 49. Generated a new task: 28  Int, Iter =   695, 5  Free mem=846  No of tasks=20
4802  Task N28  current iteration: 1
4900  MainLoop run: 50. Generated a new task: 29  Int, Iter =   3520, 3 Free mem=801  No of tasks=21
4903  Task N29  current iteration: 1
5000  MainLoop run: 51. Generated a new task: 30  Int, Iter =   3091, 9 Free mem=756  No of tasks=22
5002  Task N11  current iteration: 2
5006  Task N30  current iteration: 1
5100  MainLoop run: 52. Skipped generating a task
5198  Task N13  current iteration: 3
5200  MainLoop run: 53. Skipped generating a task
5293  Task N22  current iteration: 2
5300  MainLoop run: 54. Generated a new task: 31  Int, Iter =   4359, 9 Free mem=711  No of tasks=23
5303  Task N31  current iteration: 1
5325  Task N5 current iteration: 2
5343  Task N27  current iteration: 2
5392  Task N20  current iteration: 2
5400  MainLoop run: 55. Generated a new task: 32  Int, Iter =   837, 4  Free mem=666  No of tasks=24
5403  Task N32  current iteration: 1
5497  Task N28  current iteration: 2
5501  MainLoop run: 56. Generated a new task: 33  Int, Iter =   274, 8  Free mem=621  No of tasks=25
5505  Task N33  current iteration: 1
5584  Task N12  current iteration: 3
5600  MainLoop run: 57. Generated a new task: 34  Int, Iter =   923, 7  Free mem=576  No of tasks=26
5603  Task N34  current iteration: 1
5700  MainLoop run: 58. Generated a new task: 35  Int, Iter =   1007, 8 Free mem=531  No of tasks=27
5703  Task N35  current iteration: 1
5732  Task N9 current iteration: 3
5753  Task N19  current iteration: 2
5778  Task N33  current iteration: 2
5789  Task N6 current iteration: 2
5800  MainLoop run: 59. Skipped generating a task
5900  MainLoop run: 60. Generated a new task: 36  Int, Iter =   608, 4  Free mem=486  No of tasks=28
5903  Task N36  current iteration: 1
5984  Task N27  current iteration: 3
5988  Task N18  current iteration: 2
6000  MainLoop run: 61. Generated a new task: 37  Int, Iter =   4043, 3 Free mem=441  No of tasks=29
6003  Task N37  current iteration: 1
6052  Task N33  current iteration: 3
6100  MainLoop run: 62. Skipped generating a task
6192  Task N28  current iteration: 3
6200  MainLoop run: 63. Skipped generating a task
6239  Task N32  current iteration: 2
6248  Task N15  current iteration: 2
6300  MainLoop run: 64. Skipped generating a task
6322  Task N14  current iteration: 2
6326  Task N33  current iteration: 4
6400  MainLoop run: 65. Skipped generating a task
6410  Task N21  current iteration: 2
6500  MainLoop run: 66. Skipped generating a task
6510  Task N36  current iteration: 2
6525  Task N34  current iteration: 2
6600  MainLoop run: 67. Skipped generating a task
6600  Task N33  current iteration: 5
6625  Task N27  current iteration: 4
6646  Task N13  current iteration: 4
6683  Task N22  current iteration: 3
6700  MainLoop run: 68. Generated a new task: 38  Int, Iter =   1907, 9 Free mem=396  No of tasks=30
6703  Task N38  current iteration: 1
6709  Task N35  current iteration: 2
6800  MainLoop run: 69. Skipped generating a task
6874  Task N33  current iteration: 6
6887  Task N28  current iteration: 4
6900  MainLoop run: 70. Generated a new task: 39  Int, Iter =   2697, 1 Free mem=351  No of tasks=31
6903  Task N39  current iteration: 1
6906  Task N39  finished and destroyed. Free mem=353  No of tasks=30
7000  MainLoop run: 71. Generated a new task: 40  Int, Iter =   2849, 4 Free mem=351  No of tasks=31
7003  Task N40  current iteration: 1
7076  Task N32  current iteration: 3
7081  Task N20  current iteration: 3
7100  MainLoop run: 72. Skipped generating a task
7118  Task N36  current iteration: 3
7148  Task N33  current iteration: 7
7200  MainLoop run: 73. Skipped generating a task
7266  Task N27  current iteration: 5
7275  Task N12  current iteration: 4
7300  MainLoop run: 74. Generated a new task: 41  Int, Iter =   4466, 4 Free mem=306  No of tasks=32
7303  Task N41  current iteration: 1
7343  Task N23  current iteration: 2
7400  MainLoop run: 75. Skipped generating a task
7422  Task N33  current iteration: 8
7422  Task N33  finished and destroyed. Free mem=308  No of tasks=31
7448  Task N34  current iteration: 3
7500  MainLoop run: 76. Generated a new task: 42  Int, Iter =   2133, 3 Free mem=306  No of tasks=32
7503  Task N42  current iteration: 1
7522  Task N17  current iteration: 2
7582  Task N28  current iteration: 5
7582  Task N28  finished and destroyed. Free mem=308  No of tasks=31
7600  MainLoop run: 77. Skipped generating a task
7700  MainLoop run: 78. Skipped generating a task
7716  Task N35  current iteration: 3
7726  Task N36  current iteration: 4
7726  Task N36  finished and destroyed. Free mem=353  No of tasks=30
7800  MainLoop run: 79. Generated a new task: 43  Int, Iter =   4113, 6 Free mem=351  No of tasks=31
7803  Task N43  current iteration: 1
7898  Task N11  current iteration: 3
7900  MainLoop run: 80. Skipped generating a task
7907  Task N27  current iteration: 6
7907  Task N27  finished and destroyed. Free mem=353  No of tasks=30
7913  Task N32  current iteration: 4
7913  Task N32  finished and destroyed. Free mem=398  No of tasks=29
7947  Task N9 current iteration: 4
8000  MainLoop run: 81. Skipped generating a task
8003  Task N19  current iteration: 3
8073  Task N22  current iteration: 4
8093  Task N30  current iteration: 2
8094  Task N13  current iteration: 5
8100  MainLoop run: 82. Skipped generating a task
8200  MainLoop run: 83. Generated a new task: 44  Int, Iter =   3389, 1 Free mem=396  No of tasks=30
8203  Task N44  current iteration: 1
8206  Task N44  finished and destroyed. Free mem=398  No of tasks=29
8300  MainLoop run: 84. Generated a new task: 45  Int, Iter =   3548, 10  Free mem=396  No of tasks=30
8303  Task N3 current iteration: 3
8306  Task N45  current iteration: 1
8371  Task N34  current iteration: 4
8400  MainLoop run: 85. Skipped generating a task
8422  Task N29  current iteration: 2
8485  Task N24  current iteration: 2
8485  Task N24  finished and destroyed. Free mem=398  No of tasks=29
8500  MainLoop run: 86. Generated a new task: 46  Int, Iter =   4962, 2 Free mem=396  No of tasks=30
8503  Task N46  current iteration: 1
8600  MainLoop run: 87. Skipped generating a task
8609  Task N38  current iteration: 2
8700  MainLoop run: 88. Skipped generating a task
8723  Task N35  current iteration: 4
8770  Task N20  current iteration: 4
8800  MainLoop run: 89. Skipped generating a task
8873  Task N18  current iteration: 3
8900  MainLoop run: 90. Skipped generating a task
8966  Task N12  current iteration: 5
9000  MainLoop run: 91. Skipped generating a task
9017  Task N21  current iteration: 3
9100  MainLoop run: 92. Skipped generating a task
9200  MainLoop run: 93. Skipped generating a task
9294  Task N34  current iteration: 5
9300  MainLoop run: 94. Skipped generating a task
9385  Task N26  current iteration: 2
9400  MainLoop run: 95. Generated a new task: 47  Int, Iter =   3556, 9 Free mem=351  No of tasks=31
9403  Task N47  current iteration: 1
9463  Task N22  current iteration: 5
9500  MainLoop run: 96. Generated a new task: 48  Int, Iter =   1226, 3 Free mem=306  No of tasks=32
9503  Task N48  current iteration: 1
9542  Task N13  current iteration: 6
9600  MainLoop run: 97. Generated a new task: 49  Int, Iter =   2850, 9 Free mem=261  No of tasks=33
9603  Task N49  current iteration: 1
9635  Task N42  current iteration: 2
9661  Task N31  current iteration: 2
9700  MainLoop run: 98. Skipped generating a task
9730  Task N35  current iteration: 5
9800  MainLoop run: 99. Generated a new task: 50  Int, Iter =   2782, 10  Free mem=216  No of tasks=34
9803  Task N50  current iteration: 1
9851  Task N40  current iteration: 2
9900  MainLoop run: 100.  Skipped generating a task
9948  Task N5 current iteration: 3
9993  Task N15  current iteration: 3
10045 Task N37  current iteration: 2
10162 Task N9 current iteration: 5
10217 Task N34  current iteration: 6
10241 Task N14  current iteration: 3
10253 Task N19  current iteration: 4
10253 Task N19  finished and destroyed. Free mem=218  No of tasks=33
10459 Task N20  current iteration: 5
10516 Task N38  current iteration: 3
10657 Task N12  current iteration: 6
10657 Task N12  finished and destroyed. Free mem=263  No of tasks=32
10683 Task N23  current iteration: 3
10728 Task N48  current iteration: 2
10737 Task N35  current iteration: 6
10776 Task N6 current iteration: 3
10796 Task N11  current iteration: 4
10853 Task N22  current iteration: 6
10853 Task N22  finished and destroyed. Free mem=308  No of tasks=31
10990 Task N13  current iteration: 7
10990 Task N13  finished and destroyed. Free mem=353  No of tasks=30
11140 Task N34  current iteration: 7
11140 Task N34  finished and destroyed. Free mem=398  No of tasks=29
11184 Task N30  current iteration: 3
11624 Task N21  current iteration: 4
11744 Task N35  current iteration: 7
11758 Task N18  current iteration: 4
11768 Task N41  current iteration: 2
11768 Task N42  current iteration: 3
11769 Task N42  finished and destroyed. Free mem=443  No of tasks=28
11851 Task N45  current iteration: 2
11915 Task N43  current iteration: 2
11942 Task N29  current iteration: 3
11942 Task N29  finished and destroyed. Free mem=488  No of tasks=27
11954 Task N48  current iteration: 3
11954 Task N48  finished and destroyed. Free mem=533  No of tasks=26
12140 Task N17  current iteration: 3
12148 Task N20  current iteration: 6
12377 Task N9 current iteration: 6
12399 Task N3 current iteration: 4
12423 Task N38  current iteration: 4
12452 Task N49  current iteration: 2
12585 Task N50  current iteration: 2
12700 Task N40  current iteration: 3
12751 Task N35  current iteration: 8
12751 Task N35  finished and destroyed. Free mem=578  No of tasks=25
12958 Task N47  current iteration: 2
13464 Task N46  current iteration: 2
13464 Task N46  finished and destroyed. Free mem=623  No of tasks=24
13694 Task N11  current iteration: 5
13739 Task N15  current iteration: 4
13837 Task N20  current iteration: 7
13837 Task N20  finished and destroyed. Free mem=668  No of tasks=23
14020 Task N31  current iteration: 3
14023 Task N23  current iteration: 4
14088 Task N37  current iteration: 3
14088 Task N37  finished and destroyed. Free mem=713  No of tasks=22
14160 Task N14  current iteration: 4
14167 Task N26  current iteration: 3
14231 Task N21  current iteration: 5
14231 Task N21  finished and destroyed. Free mem=758  No of tasks=21
14275 Task N30  current iteration: 4
14330 Task N38  current iteration: 5
14571 Task N5 current iteration: 4
14592 Task N9 current iteration: 7
14592 Task N9 finished and destroyed. Free mem=803  No of tasks=20
14643 Task N18  current iteration: 5
15302 Task N49  current iteration: 3
15367 Task N50  current iteration: 3
15399 Task N45  current iteration: 3
15549 Task N40  current iteration: 4
15549 Task N40  finished and destroyed. Free mem=848  No of tasks=19
15763 Task N6 current iteration: 4
15763 Task N6 finished and destroyed. Free mem=893  No of tasks=18
16028 Task N43  current iteration: 3
16234 Task N41  current iteration: 3
16237 Task N38  current iteration: 6
16498 Task N3 current iteration: 5
16514 Task N47  current iteration: 3
16592 Task N11  current iteration: 6
16758 Task N17  current iteration: 4
17363 Task N23  current iteration: 5
17366 Task N30  current iteration: 5
17483 Task N15  current iteration: 5
17483 Task N15  finished and destroyed. Free mem=938  No of tasks=17
17528 Task N18  current iteration: 6
17528 Task N18  finished and destroyed. Free mem=983  No of tasks=16
18079 Task N14  current iteration: 5
18144 Task N38  current iteration: 7
18149 Task N50  current iteration: 4
18152 Task N49  current iteration: 4
18379 Task N31  current iteration: 4
18947 Task N45  current iteration: 4
18949 Task N26  current iteration: 4
19194 Task N5 current iteration: 5
19490 Task N11  current iteration: 7
20051 Task N38  current iteration: 8
20070 Task N47  current iteration: 4
20141 Task N43  current iteration: 4
20457 Task N30  current iteration: 6
20597 Task N3 current iteration: 6
20700 Task N41  current iteration: 4
20700 Task N41  finished and destroyed. Free mem=1028 No of tasks=15
20703 Task N23  current iteration: 6
20931 Task N50  current iteration: 5
21002 Task N49  current iteration: 5
21376 Task N17  current iteration: 5
21958 Task N38  current iteration: 9
21958 Task N38  finished and destroyed. Free mem=1073 No of tasks=14
21998 Task N14  current iteration: 6
22388 Task N11  current iteration: 8
22495 Task N45  current iteration: 5
22738 Task N31  current iteration: 5
23548 Task N30  current iteration: 7
23626 Task N47  current iteration: 5
23713 Task N50  current iteration: 6
23731 Task N26  current iteration: 5
23817 Task N5 current iteration: 6
23852 Task N49  current iteration: 6
24043 Task N23  current iteration: 7
24254 Task N43  current iteration: 5
24696 Task N3 current iteration: 7
25286 Task N11  current iteration: 9
25286 Task N11  finished and destroyed. Free mem=1118 No of tasks=13
25917 Task N14  current iteration: 7
25917 Task N14  finished and destroyed. Free mem=1163 No of tasks=12
25994 Task N17  current iteration: 6
26043 Task N45  current iteration: 6
26496 Task N50  current iteration: 7
26639 Task N30  current iteration: 8
26702 Task N49  current iteration: 7
27097 Task N31  current iteration: 6
27182 Task N47  current iteration: 6
27383 Task N23  current iteration: 8
27383 Task N23  finished and destroyed. Free mem=1208 No of tasks=11
28367 Task N43  current iteration: 6
28367 Task N43  finished and destroyed. Free mem=1253 No of tasks=10
28440 Task N5 current iteration: 7
28440 Task N5 finished and destroyed. Free mem=1298 No of tasks=9
28513 Task N26  current iteration: 6
28795 Task N3 current iteration: 8
29277 Task N50  current iteration: 8
29552 Task N49  current iteration: 8
29591 Task N45  current iteration: 7
29730 Task N30  current iteration: 9
29730 Task N30  finished and destroyed. Free mem=1343 No of tasks=8
30612 Task N17  current iteration: 7
30738 Task N47  current iteration: 7
31456 Task N31  current iteration: 7
32059 Task N50  current iteration: 9
32402 Task N49  current iteration: 9
32402 Task N49  finished and destroyed. Free mem=1388 No of tasks=7
32894 Task N3 current iteration: 9
32894 Task N3 finished and destroyed. Free mem=1433 No of tasks=6
33139 Task N45  current iteration: 8
33295 Task N26  current iteration: 7
34294 Task N47  current iteration: 8
34841 Task N50  current iteration: 10
34841 Task N50  finished and destroyed. Free mem=1478 No of tasks=5
35230 Task N17  current iteration: 8
35815 Task N31  current iteration: 8
36687 Task N45  current iteration: 9
37850 Task N47  current iteration: 9
37850 Task N47  finished and destroyed. Free mem=1523 No of tasks=4
38077 Task N26  current iteration: 8
39848 Task N17  current iteration: 9
40174 Task N31  current iteration: 9
40174 Task N31  finished and destroyed. Free mem=1568 No of tasks=3
40235 Task N45  current iteration: 10
40235 Task N45  finished and destroyed. Free mem=1613 No of tasks=2
42859 Task N26  current iteration: 9
44466 Task N17  current iteration: 10
44466 Task N17  finished and destroyed. Free mem=1658 No of tasks=1
47641 Task N26  current iteration: 10
47641 Task N26  finished and destroyed. Free mem=1703 No of tasks=0

 */
