
| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 8x8
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------
|   100.0% |              166.43 |                6.01 |    0.2% |     29.75 | `dp::thread_pool - std::function`
|   102.8% |              161.86 |                6.18 |    0.6% |     28.98 | `dp::thread_pool - std::move_only_function`
|   100.1% |              166.31 |                6.01 |    0.4% |     29.74 | `dp::thread_pool - fu2::unique_function`
|    87.6% |              189.90 |                5.27 |    0.3% |     33.99 | `BS::thread_pool`
|   107.5% |              154.85 |                6.46 |    0.7% |     27.62 | `task_thread_pool`
|    89.6% |              185.73 |                5.38 |    1.9% |     33.74 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 64x64
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |              128.74 |                7.77 |    0.1% |     23.08 | `dp::thread_pool - std::function`
|   102.6% |              125.52 |                7.97 |    0.3% |     22.48 | `dp::thread_pool - std::move_only_function`
|   100.3% |              128.37 |                7.79 |    0.3% |     22.98 | `dp::thread_pool - fu2::unique_function`
|    80.5% |              159.85 |                6.26 |    0.8% |     28.59 | `BS::thread_pool`
|    96.5% |              133.37 |                7.50 |    0.9% |     23.90 | `task_thread_pool`
|    94.4% |              136.32 |                7.34 |    0.9% |     24.47 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 256x256
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               91.76 |               10.90 |    0.3% |     16.45 | `dp::thread_pool - std::function`
|   103.1% |               88.99 |               11.24 |    0.5% |     15.92 | `dp::thread_pool - std::move_only_function`
|   100.2% |               91.58 |               10.92 |    0.2% |     16.40 | `dp::thread_pool - fu2::unique_function`
|    87.6% |              104.81 |                9.54 |    0.3% |     18.82 | `BS::thread_pool`
|   104.4% |               87.92 |               11.37 |    0.4% |     15.75 | `task_thread_pool`
|    99.9% |               91.85 |               10.89 |    0.3% |     16.43 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 512x512
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               68.26 |               14.65 |    0.5% |     12.23 | `dp::thread_pool - std::function`
|   102.7% |               66.49 |               15.04 |    0.5% |     11.90 | `dp::thread_pool - std::move_only_function`
|   100.1% |               68.21 |               14.66 |    0.8% |     12.14 | `dp::thread_pool - fu2::unique_function`
|    90.8% |               75.21 |               13.30 |    0.6% |     13.48 | `BS::thread_pool`
|    97.9% |               69.70 |               14.35 |    0.2% |     12.45 | `task_thread_pool`
|   100.8% |               67.69 |               14.77 |    0.6% |     12.07 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 1024x1024
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------------
|   100.0% |               53.61 |               18.65 |    2.0% |      9.50 | `dp::thread_pool - std::function`
|   105.1% |               50.99 |               19.61 |    3.8% |      8.68 | `dp::thread_pool - std::move_only_function`
|    99.2% |               54.05 |               18.50 |    1.5% |      9.58 | `dp::thread_pool - fu2::unique_function`
|    86.9% |               61.67 |               16.22 |    0.6% |     11.02 | `BS::thread_pool`
|    98.7% |               54.34 |               18.40 |    1.2% |      9.75 | `task_thread_pool`
|   102.0% |               52.54 |               19.03 |    1.2% |      9.25 | `riften::Thiefpool`
