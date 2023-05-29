
| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 8x8
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------
|   100.0% |              169.93 |                5.88 |    0.5% |     30.46 | `dp::thread_pool - std::function`
|    97.4% |              174.49 |                5.73 |    0.5% |     31.26 | `dp::thread_pool - std::move_only_function`
|    93.6% |              181.49 |                5.51 |    0.2% |     32.47 | `dp::thread_pool - fu2::unique_function`
|    89.1% |              190.78 |                5.24 |    0.3% |     34.13 | `BS::thread_pool`
|   106.3% |              159.82 |                6.26 |    0.3% |     28.65 | `task_thread_pool`
|    84.7% |              200.63 |                4.98 |    1.5% |     36.11 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 64x64
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |              131.42 |                7.61 |    0.5% |     23.80 | `dp::thread_pool - std::function`
|   102.7% |              127.92 |                7.82 |    0.6% |     23.02 | `dp::thread_pool - std::move_only_function`
|    97.5% |              134.74 |                7.42 |    0.4% |     24.16 | `dp::thread_pool - fu2::unique_function`
|    90.6% |              145.05 |                6.89 |    0.5% |     25.90 | `BS::thread_pool`
|   104.9% |              125.24 |                7.98 |    0.2% |     22.41 | `task_thread_pool`
|    91.6% |              143.44 |                6.97 |    1.1% |     25.50 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 256x256
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               93.27 |               10.72 |    0.7% |     16.69 | `dp::thread_pool - std::function`
|   102.9% |               90.66 |               11.03 |    0.6% |     16.22 | `dp::thread_pool - std::move_only_function`
|    98.7% |               94.50 |               10.58 |    0.2% |     16.91 | `dp::thread_pool - fu2::unique_function`
|    93.5% |               99.73 |               10.03 |    0.4% |     17.86 | `BS::thread_pool`
|   102.2% |               91.29 |               10.95 |    0.6% |     16.39 | `task_thread_pool`
|   100.1% |               93.18 |               10.73 |    1.4% |     16.61 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 512x512
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               69.90 |               14.31 |    0.7% |     12.52 | `dp::thread_pool - std::function`
|   106.0% |               65.93 |               15.17 |    2.7% |     11.76 | `dp::thread_pool - std::move_only_function`
|    97.9% |               71.37 |               14.01 |    0.5% |     12.84 | `dp::thread_pool - fu2::unique_function`
|    95.8% |               72.96 |               13.71 |    0.3% |     13.08 | `BS::thread_pool`
|    99.5% |               70.26 |               14.23 |    0.3% |     12.56 | `task_thread_pool`
|   158.3% |               44.16 |               22.65 |   16.6% |      7.85 | :wavy_dash: `riften::Thiefpool` (Unstable with ~16.3 iters. Increase `minEpochIterations` to e.g. 163)

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 1024x1024
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------------
|   100.0% |               35.99 |               27.78 |    2.3% |      6.49 | `dp::thread_pool - std::function`
|    99.7% |               36.12 |               27.69 |    2.1% |      6.39 | `dp::thread_pool - std::move_only_function`
|    99.9% |               36.03 |               27.75 |    2.6% |      6.42 | `dp::thread_pool - fu2::unique_function`
|    61.3% |               58.75 |               17.02 |    0.6% |     10.50 | `BS::thread_pool`
|    71.6% |               50.30 |               19.88 |    0.3% |      9.02 | `task_thread_pool`
|   105.0% |               34.28 |               29.17 |    0.6% |      6.17 | `riften::Thiefpool`
