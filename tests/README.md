# Google Test for ToyC Preprocessor

This directory contains unit tests for the ToyC preprocessor using Google Test framework.

## Building and Running Tests

```bash
# Install Google Test (Ubuntu/Debian)
sudo apt-get install libgtest-dev libgmock-dev

# Build tests
make tests

# Run tests
./tests/test_preprocessor
```

## Test Structure

- `test_preprocessor.cpp`: Main test file for preprocessor functionality
- `test_fixtures/`: Test input files for complex scenarios
