On 6 different machines run 1 time using original filebench workloads configs
No grpc; No unreliable; using filesystem directly for base comparison 


Notes: 
-  createfiles, createrand, rread, & seqwrite:  resulted in a warning ```
      7.340: WARNING! Run stopped early:
      flowop finish-1 could not obtain a file. Please
      reduce runtime, increase fileset entries ($nfiles), or switch modes.
    ``` 