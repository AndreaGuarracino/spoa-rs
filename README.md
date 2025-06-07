# spoa-rs

Rust bindings for [Spoa (SIMD POA)](https://github.com/rvaser/spoa/).

Spoa is a C++ implementation of the partial order alignment (POA) algorithm (as described in 10.1093/bioinformatics/18.3.452) which is used to generate consensus sequences (as described in 10.1093/bioinformatics/btg109). It supports three alignment modes: local (Smith-Waterman), global (Needleman-Wunsch) and semi-global alignment (overlap), and three gap modes: linear, affine and convex (piecewise affine). It also supports Intel SSE4.1+ and AVX2 vectorization (marginally faster due to high latency shifts), SIMDe and dispatching.

# Install

Spoa-rs use the cargo build system. Check out the git repo, install a recent version of Rust and

```sh
rm -rf target/  # may be necessary when check-for-pregenerated-files fails
CC=gcc cargo build
```

will create a static library that can be used from other Rust projects.

## Guix

See the header of [guix.scm](./guix.scm) for instructions on creating a build shell.

# License

Spoa-rs is published under BSD-3 Clause license.
