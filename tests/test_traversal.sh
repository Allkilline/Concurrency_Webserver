#!/bin/bash
#

SERVER='localhost'
PORT=8000

printf "\n === Testing Security Traversal === \n";

test_case() {
	URI="$1"
	EXPECT="$2"

	printf "\nTesting URI: $URI"

	OUTPUT=$(../wclient $SERVER $PORT "$URI" 2>/dev/null)

	if echo "$OUTPUT" | grep -q "$EXPECT"; then
		echo " Pass ($EXPECT found)"
	else
		echo " Fail (expected $EXPECT)"
	fi

	echo
}

# Normal file should succeed
test_case "/small.html" "200 OK"

# These should all be blocked
test_case "/../etc/passwd" "403"
test_case "/a/b/../../etc/passwd" "403"
test_case "/%2e%2e/%2e%2e/etc/passwd" "403"
test_case "/secret/../file.txt" "403"
test_case "/..hidden'file.txt" "403"

printf "Done\n\n"
