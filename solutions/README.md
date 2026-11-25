# Reference solutions

One solved header per problem, at the same path as the stub it replaces:
`solutions/<set>/NN_name/<name>.hpp` solves `<set>/problems/NN_name/<name>.hpp`.
Try each problem yourself first and use these to compare approaches after your
tests pass. They favour clarity over the last bit of speed; several problem
READMEs suggest faster variants under "Going further".

To check every solution (or a subset) against the tests without touching your
working tree:

```bash
./solutions/check_solutions.sh             # all sets
./solutions/check_solutions.sh dp -- 09    # options are the same as ../run_all.sh
```

It copies the repository to a temporary folder, drops the solutions over the
stubs there and runs the scoreboard.
