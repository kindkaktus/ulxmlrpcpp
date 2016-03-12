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

function hasIpv6()
{
    if ip addr show | grep -q "inet6 ::1"
    then
        return 0
    else
        return 1
    fi
}

succeeded=0
failed=0

echo "*** Running tests:"

if hasIpv6 ; then
    runTest
    runTest "connect-ipv4"
    runTest "ssl"
    runTest "ssl connect-ipv4"
    runTest "performance"
    runTest "performance ssl"
else
    runTest "ipv4-only"
    runTest "ssl ipv4-only"
    runTest "performance ipv4-only"
    runTest "performance ssl ipv4-only"
fi


echo
if [ $failed -gt 0 ]; then
    echo $succeeded tests SUCCEEDED, $failed tests FAILED
else
    echo ALL $succeeded tests SUCCEEDED
fi
echo
exit $failed