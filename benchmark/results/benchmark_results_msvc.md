
| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 8x8
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------
|   100.0% |              170.10 |                5.88 |    0.9% |     30.73 | `dp::thread_pool - std::function`
|   103.0% |              165.14 |                6.06 |    0.7% |     29.57 | `dp::thread_pool - std::move_only_function`
|    99.3% |              171.27 |                5.84 |    0.4% |     30.69 | `dp::thread_pool - fu2::unique_function`
|    88.4% |              192.44 |                5.20 |    0.2% |     34.46 | `BS::thread_pool`
|   109.6% |              155.26 |                6.44 |    0.3% |     27.79 | `task_thread_pool`
|    95.3% |              178.52 |                5.60 |    0.4% |     31.94 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 64x64
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |              129.75 |                7.71 |    0.4% |     23.23 | `dp::thread_pool - std::function`
|   101.8% |              127.50 |                7.84 |    0.3% |     22.86 | `dp::thread_pool - std::move_only_function`
|    97.8% |              132.61 |                7.54 |    0.1% |     23.70 | `dp::thread_pool - fu2::unique_function`
|    85.4% |              151.98 |                6.58 |    0.6% |     27.26 | `BS::thread_pool`
|   100.9% |              128.65 |                7.77 |    0.5% |     23.07 | `task_thread_pool`
|    92.8% |              139.83 |                7.15 |    0.6% |     25.07 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 256x256
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               93.47 |               10.70 |    0.5% |     16.73 | `dp::thread_pool - std::function`
|   102.6% |               91.06 |               10.98 |    0.6% |     16.26 | `dp::thread_pool - std::move_only_function`
|    99.3% |               94.17 |               10.62 |    0.4% |     17.03 | `dp::thread_pool - fu2::unique_function`
|    89.6% |              104.35 |                9.58 |    0.2% |     18.67 | `BS::thread_pool`
|   100.7% |               92.82 |               10.77 |    0.2% |     16.63 | `task_thread_pool`
|    96.1% |               97.26 |               10.28 |    0.4% |     17.42 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 512x512
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               27.65 |               36.17 |    1.8% |      4.97 | `dp::thread_pool - std::function`
|   102.7% |               26.91 |               37.16 |    2.8% |      4.86 | `dp::thread_pool - std::move_only_function`
|    99.8% |               27.70 |               36.10 |    2.4% |      4.94 | `dp::thread_pool - fu2::unique_function`
|    34.8% |               79.40 |               12.59 |    0.5% |     14.22 | `BS::thread_pool`
|    50.4% |               54.83 |               18.24 |    0.8% |      9.85 | `task_thread_pool`
|   101.0% |               27.37 |               36.54 |    0.7% |      4.91 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 1024x1024
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------------
|   100.0% |               51.97 |               19.24 |    2.1% |      9.16 | `dp::thread_pool - std::function`
|    99.8% |               52.05 |               19.21 |    2.2% |      9.29 | `dp::thread_pool - std::move_only_function`
|    98.8% |               52.61 |               19.01 |    0.9% |      9.42 | `dp::thread_pool - fu2::unique_function`
|    77.7% |               66.88 |               14.95 |    0.5% |     12.01 | `BS::thread_pool`
|    73.8% |               70.38 |               14.21 |    0.3% |     12.60 | `task_thread_pool`
|    97.0% |               53.58 |               18.66 |    1.6% |      9.47 | `riften::Thiefpool`
