#!/bin/bash

for ((n = 1000; n < 2000; n++)) do
    nc -q -1 -C localhost 8080  << END &
PASS pass
NICK a$n
USER a a a a a
END
done
wait