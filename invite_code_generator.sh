#!/bin/bash
# this is a invite code generator for pika user register

echo "generating the invite code..."
cat /dev/urandom | sed 's/[^a-zA-Z0-9]//g' | strings -n 10 | head -n 1 >> ../invite_code
echo "done."