# General

This repository assumes you have read what is available on [Slurm tutorial](https://gitlab.science.ru.nl/parallel-computing2/slurm-tutorial).

Read `Lesson1-build-system.md` for information on how to build and run the
example programs. 

When you start the assignments, `Lesson2-benchmarking.md` is of help.

# Mandelbrot

For the sequential Mandelbrot example, first build with 

`BUILD=RELEASE make bin/mandelbrot_seq`

You can then run the program and save the image to a file with

`bin/mandelbrot_seq 1000 1000 100 -2 2 -2 2 2> mandelbrot.pgm`

and open the image with for example `mupdf`.

In order to do this on the cluster, see the Slurm tutorial, and `scp` the
image over to your local machine.
