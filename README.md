Path Summaries for Symbolic Execution of SystemC Modules
=============================================

With Path Summaries, one symbolic state can represent multiple similar / identical paths. They are an opportunity to combine experience / practices from the software verification domain with knowledge about the hardware domain to improve the established method of applying software symbolic execution tools to hardware models. Toward this goal, two concepts are combined:
1. Branch Summaries to combine multiple paths through a module process (e.g. SC_THREAD)
2. State Pruning to combine multiple paths through the inter-module communication over signals

Our implementation is based on SEFOS (https://github.com/agra-uni-bremen/sefos), which is based on KLEE (https://github.com/klee/klee).

---
# Getting started
Build this repository into a Docker image: `docker build --tag path-summaries:3.1 .`. Based on this Docker image, you can create containers in which the modified KLEE version is available.

