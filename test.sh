#!/bin/bash

for ((n = 100; n < 200; n++)) do
    nc -q -1 -C localhost 8080  << END &
PASS pass
NICK a$n
USER a a a a a
END
done
wait