
| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 8x8
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------
|   100.0% |              169.71 |                5.89 |    0.7% |     30.65 | `dp::thread_pool - std::function`
|   103.0% |              164.76 |                6.07 |    0.6% |     29.70 | `dp::thread_pool - std::move_only_function`
|    99.3% |              170.86 |                5.85 |    1.7% |     30.77 | `dp::thread_pool - fu2::unique_function`
|    90.2% |              188.16 |                5.31 |    0.3% |     33.66 | `BS::thread_pool`
|   109.7% |              154.74 |                6.46 |    0.9% |     27.32 | `task_thread_pool`
|    98.5% |              172.33 |                5.80 |    0.3% |     30.86 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 64x64
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |              129.05 |                7.75 |    0.2% |     23.15 | `dp::thread_pool - std::function`
|   103.1% |              125.14 |                7.99 |    0.6% |     22.40 | `dp::thread_pool - std::move_only_function`
|   100.1% |              128.93 |                7.76 |    0.4% |     23.22 | `dp::thread_pool - fu2::unique_function`
|    90.4% |              142.73 |                7.01 |    0.5% |     25.60 | `BS::thread_pool`
|   105.6% |              122.19 |                8.18 |    0.3% |     21.94 | `task_thread_pool`
|    98.5% |              130.98 |                7.63 |    0.5% |     23.50 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 256x256
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               91.43 |               10.94 |    0.5% |     16.42 | `dp::thread_pool - std::function`
|   102.0% |               89.62 |               11.16 |    0.5% |     15.99 | `dp::thread_pool - std::move_only_function`
|    99.1% |               92.24 |               10.84 |    0.4% |     16.49 | `dp::thread_pool - fu2::unique_function`
|    93.0% |               98.36 |               10.17 |    0.5% |     17.60 | `BS::thread_pool`
|   103.9% |               87.99 |               11.37 |    0.3% |     15.78 | `task_thread_pool`
|    99.0% |               92.34 |               10.83 |    0.2% |     16.59 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 512x512
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               67.97 |               14.71 |    0.5% |     12.14 | `dp::thread_pool - std::function`
|   103.0% |               65.97 |               15.16 |    0.5% |     11.80 | `dp::thread_pool - std::move_only_function`
|    99.2% |               68.49 |               14.60 |    0.5% |     12.25 | `dp::thread_pool - fu2::unique_function`
|    94.5% |               71.93 |               13.90 |    0.1% |     12.87 | `BS::thread_pool`
|    98.3% |               69.12 |               14.47 |    0.2% |     12.37 | `task_thread_pool`
|    98.3% |               69.12 |               14.47 |    0.5% |     12.34 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 1024x1024
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------------
|   100.0% |               52.12 |               19.19 |    4.0% |      8.80 | `dp::thread_pool - std::function`
|   131.1% |               39.74 |               25.16 |   25.7% |      6.49 | :wavy_dash: `dp::thread_pool - std::move_only_function` (Unstable with ~16.3 iters. Increase `minEpochIterations` to e.g. 163)
|   100.0% |               52.09 |               19.20 |    4.2% |      8.78 | `dp::thread_pool - fu2::unique_function`
|    91.2% |               57.14 |               17.50 |    0.6% |     10.23 | `BS::thread_pool`
|    99.6% |               52.32 |               19.11 |    0.6% |      9.38 | `task_thread_pool`
|    98.7% |               52.80 |               18.94 |    1.1% |      9.43 | `riften::Thiefpool`
