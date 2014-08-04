#!/usr/bin/env bash

function runTest()
{
    echo "--- Running: all_tests $@"
    if ./all_tests $@ ; then
        succeeded=$(($succeeded + 1))
    else
        failed=$(($failed + 1))
    fi
    sleep 0.5 # to make sure all processes are stopped so we do not get "Address already in use"
}

succeeded=0
failed=0

echo "*** Running tests:"
runTest
runTest "ipv4"
runTest "ssl"
runTest "ssl ipv4"
runTest "performance"
runTest "performance ssl"

echo 
if [ $failed -gt 0 ]; then
    echo $succeeded tests SUCCEEDED, $failed tests FAILED
else
    echo ALL $succeeded tests SUCCEEDED
fi
echo
exit $failed