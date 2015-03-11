#!/bin/bash
for myrun in `ls | grep batch`;
        do qsub $myrun
done