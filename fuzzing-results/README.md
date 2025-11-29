# Fuzzing Results

This directory contains markdown reports summarizing fuzzing campaign results.

## Files

Each fuzzing campaign generates a markdown file with the naming pattern:
`fuzzing_results_YYYYMMDD_HHMMSS.md`

Where `YYYYMMDD_HHMMSS` is the timestamp when the campaign started.

## Report Contents

Each report includes:

- Campaign configuration and timing information
- Summary table of all fuzzers with key metrics
- Detailed statistics for each fuzzer
- Coverage progression information
- Corpus file counts and sizes
- Process status

## Generating Reports

Reports are automatically created when:
1. Starting a fuzzing campaign with `run_fuzzers_6h.sh` (initial report)
2. Running `summarize_fuzzing.sh` after campaign completion (final report)

To manually generate or update a report:

```bash
cd fuzzing
./summarize_fuzzing.sh results_YYYYMMDD_HHMMSS fuzzing-results/fuzzing_results_YYYYMMDD_HHMMSS.md
```

## Metrics Explained

- **Executions**: Total number of test cases executed
- **Exec/sec**: Average executions per second (target: 100+ minimum)
- **New Units**: Number of new interesting test cases added to corpus
- **Peak RSS**: Maximum memory usage in megabytes
- **Coverage**: Code coverage metrics (edges and features discovered)

