
| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 8x8
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------
|   100.0% |              172.25 |                5.81 |    1.0% |     31.20 | `dp::thread_pool - std::function`
|   102.7% |              167.79 |                5.96 |    0.7% |     30.55 | `dp::thread_pool - std::move_only_function`
|    93.5% |              184.15 |                5.43 |    1.2% |     32.97 | `dp::thread_pool - fu2::unique_function`
|    89.3% |              192.93 |                5.18 |    0.2% |     34.54 | `BS::thread_pool`
|    91.2% |              188.90 |                5.29 |    4.0% |     34.18 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 64x64
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |              133.91 |                7.47 |    1.0% |     24.28 | `dp::thread_pool - std::function`
|   102.6% |              130.52 |                7.66 |    0.9% |     23.34 | `dp::thread_pool - std::move_only_function`
|    98.7% |              135.72 |                7.37 |    0.8% |     24.43 | `dp::thread_pool - fu2::unique_function`
|    90.0% |              148.80 |                6.72 |    0.7% |     26.80 | `BS::thread_pool`
|    95.7% |              139.92 |                7.15 |    0.6% |     25.69 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 256x256
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               94.98 |               10.53 |    1.3% |     17.15 | `dp::thread_pool - std::function`
|   102.8% |               92.43 |               10.82 |    0.8% |     16.44 | `dp::thread_pool - std::move_only_function`
|    99.0% |               95.98 |               10.42 |    0.8% |     17.27 | `dp::thread_pool - fu2::unique_function`
|    89.8% |              105.77 |                9.45 |    0.3% |     18.94 | `BS::thread_pool`
|    96.8% |               98.07 |               10.20 |    0.5% |     17.59 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 512x512
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------------
|   100.0% |               28.10 |               35.59 |    1.0% |      5.06 | `dp::thread_pool - std::function`
|    99.0% |               28.37 |               35.24 |    3.2% |      5.30 | `dp::thread_pool - std::move_only_function`
|    97.9% |               28.70 |               34.85 |    2.8% |      5.26 | `dp::thread_pool - fu2::unique_function`
|    35.5% |               79.08 |               12.65 |    1.1% |     14.24 | `BS::thread_pool`
|    95.1% |               29.54 |               33.85 |    1.1% |      5.32 | `riften::Thiefpool`

| relative |               ms/op |                op/s |    err% |     total | matrix multiplication 1024x1024
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------------------
|   100.0% |               56.71 |               17.63 |    2.3% |     10.60 | `dp::thread_pool - std::function`
|    99.9% |               56.76 |               17.62 |    1.3% |     10.08 | `dp::thread_pool - std::move_only_function`
|    99.7% |               56.87 |               17.58 |    4.2% |     10.39 | `dp::thread_pool - fu2::unique_function`
|    80.2% |               70.71 |               14.14 |    1.1% |     12.93 | `BS::thread_pool`
|   103.0% |               55.04 |               18.17 |    2.2% |     10.12 | `riften::Thiefpool`
