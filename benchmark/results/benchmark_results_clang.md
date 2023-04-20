
| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 8x8
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------
|   100.0% |              174.67 |                5.73 |    2.8% |     30.98 | `dp::thread_pool - std::function`
|   103.4% |              168.90 |                5.92 |    1.4% |     30.16 | `dp::thread_pool - std::move_only_function`
|    98.3% |              177.71 |                5.63 |    1.2% |     31.77 | `dp::thread_pool - fu2::unique_function`
|    88.9% |              196.41 |                5.09 |    0.2% |     35.12 | `BS::thread_pool`
|    99.0% |              176.50 |                5.67 |    1.1% |     31.57 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 64x64
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |              129.37 |                7.73 |    0.5% |     23.27 | `dp::thread_pool - std::function`
|   102.2% |              126.59 |                7.90 |    0.4% |     22.70 | `dp::thread_pool - std::move_only_function`
|    98.5% |              131.36 |                7.61 |    0.7% |     23.58 | `dp::thread_pool - fu2::unique_function`
|    86.5% |              149.61 |                6.68 |    0.8% |     26.80 | `BS::thread_pool`
|    90.1% |              143.59 |                6.96 |    1.7% |     25.56 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 256x256
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               94.01 |               10.64 |    1.2% |     16.80 | `dp::thread_pool - std::function`
|   100.1% |               93.91 |               10.65 |    1.4% |     16.94 | `dp::thread_pool - std::move_only_function`
|    97.9% |               96.01 |               10.42 |    0.4% |     17.18 | `dp::thread_pool - fu2::unique_function`
|    88.4% |              106.31 |                9.41 |    0.4% |     19.00 | `BS::thread_pool`
|    98.3% |               95.66 |               10.45 |    1.0% |     17.18 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 512x512
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               72.31 |               13.83 |    1.5% |     13.01 | `dp::thread_pool - std::function`
|   105.0% |               68.89 |               14.51 |    1.4% |     12.36 | `dp::thread_pool - std::move_only_function`
|   101.1% |               71.54 |               13.98 |    1.2% |     12.86 | `dp::thread_pool - fu2::unique_function`
|    90.5% |               79.91 |               12.51 |    0.7% |     14.32 | `BS::thread_pool`
|    97.5% |               74.17 |               13.48 |    2.9% |     13.48 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 1024x1024
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------------
|   100.0% |               53.11 |               18.83 |    4.6% |      9.17 | `dp::thread_pool - std::function`
|   105.4% |               50.37 |               19.85 |    5.7% |      8.43 | :wavy_dash: `dp::thread_pool - std::move_only_function` (Unstable with ~16.3 iters. Increase `minEpochIterations` to e.g. 163)
|    99.3% |               53.50 |               18.69 |    3.6% |      8.97 | `dp::thread_pool - fu2::unique_function`
|    82.0% |               64.78 |               15.44 |    0.6% |     11.51 | `BS::thread_pool`
|    93.0% |               57.11 |               17.51 |    2.1% |      9.59 | `riften::Thiefpool`
