## 1 threads and 2 connections
### No cache
| Thread Stats   | Avg      | Stdev   | Max      | +/- Stdev |
|----------------|----------|---------|----------|-----------|
| Latency        | 17.97ms  | 23.27ms | 192.96ms | 92.44%    |
| Req/Sec        | 171.92   | 59.02   | 330.00   | 70.83%    |

1687 requests in 10.01s, 439.87KB read\
Requests/sec:    168.60\
Transfer/sec:     43.96KB

### With cache
|Thread Stats  | Avg   |   Stdev |    Max |  +/- Stdev|
|----------------|----------|---------|----------|-----------|
|    Latency    | 7.71ms |  5.67ms | 45.01ms |  81.38% |
|    Req/Sec |  286.98  |   54.34  | 470.00  |   76.00% |

  2875 requests in 10.06s, 749.63KB read\
Requests/sec:    285.88\
Transfer/sec:     74.54KB

## 2 threads and 4 connections
### No cache
| Thread Stats   | Avg      | Stdev   | Max      | +/- Stdev |
|----------------|----------|---------|----------|-----------|
|Latency|    14.84ms    |8.76ms  |48.80ms   |66.60%|
|    Req/Sec|   137.03|     23.65|   220.00|     69.85%|

2744 requests in 10.10s, 715.61KB read\
Requests/sec:    271.69\
Transfer/sec:     70.86KB

### With cache
|Thread Stats  | Avg   |   Stdev |    Max |  +/- Stdev|
|----------------|----------|---------|----------|-----------|
|Latency|    13.28ms|    9.15ms|  65.56ms|   71.89%|
|    Req/Sec|   159.13|     36.27|   306.00|     70.50%|

  3193 requests in 10.07s, 832.55KB read\
Requests/sec:    317.11\
Transfer/sec:     82.68KB

## 4 threads and 8 connections
### No cache
| Thread Stats   | Avg      | Stdev   | Max      | +/- Stdev |
|----------------|----------|---------|----------|-----------|
|Latency|    23.37ms|   13.13ms| 133.46ms|   69.01%|
|    Req/Sec|    87.20|     20.39|   150.00|     70.71%|

  3501 requests in 10.06s, 0.89MB read\
Requests/sec:    347.94\
Transfer/sec:     90.74KB

### With cache
|Thread Stats  | Avg   |   Stdev |    Max |  +/- Stdev|
|----------------|----------|---------|----------|-----------|
|Latency    |26.97ms|   17.09ms| 153.58ms|   68.36%|
|    Req/Sec|    76.53|     19.87|   150.00|     73.23%|

  3062 requests in 10.04s, 798.39KB read\
Requests/sec:    304.83\
Transfer/sec:     79.48KB