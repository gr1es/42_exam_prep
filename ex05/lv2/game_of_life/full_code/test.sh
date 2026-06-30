#!/bin/bash
# compile first: gcc -Wall -Wextra -Werror -o a.out main.c

BIN="./a.out"
PASS=0
FAIL=0

t() {
	local name="$1"
	local input="$2"
	local args="$3"
	local expected="$4"
	local want_exit="${5:-0}"

	local actual actual_exit
	actual=$(printf '%s' "$input" | $BIN $args 2>/dev/null)
	actual_exit=$?

	if [ "$actual" = "$expected" ] && [ "$actual_exit" = "$want_exit" ]; then
		printf "PASS  %s\n" "$name"
		PASS=$((PASS + 1))
	else
		printf "FAIL  %s\n" "$name"
		if [ "$actual" != "$expected" ]; then
			printf "  want: %s\n" "$(printf '%s' "$expected" | cat -A)"
			printf "  got:  %s\n" "$(printf '%s' "$actual" | cat -A)"
		fi
		if [ "$actual_exit" != "$want_exit" ]; then
			printf "  exit: got=%d want=%s\n" "$actual_exit" "$want_exit"
		fi
		FAIL=$((FAIL + 1))
	fi
}

echo "=== invalid input ==="
t "no args"           ""  ""          "" 1
t "too few args"      ""  "3 3"       "" 1
t "too many args"     ""  "3 3 0 x"   "" 1
t "width 0"           ""  "0 3 1"     "" 1
t "width -1"          ""  "-1 3 1"    "" 1
t "height 0"          ""  "3 0 1"     "" 1
t "height -1"         ""  "3 -1 1"    "" 1
t "iter -1"           ""  "3 3 -1"    "" 1
t "iter non-numeric"  ""  "3 3 abc"   "" 1
t "iter '0abc'"       ""  "3 3 0abc"  "" 1

echo "=== subject examples ==="
t "subject 1 (5x5 0-iter)"   "sdxddssaaww"                  "5 5 0"  $'     \n 000 \n 0 0 \n 000 \n     '
t "subject 2 (10x6 0-iter)"  "sdxssdswdxdddxsaddawxwdxwaa"  "10 6 0" $'          \n 0   000  \n 0     0  \n 000  0   \n  0  000  \n          '
t "subject 3 (3x3 0-iter)"   "dxss"                          "3 3 0"  $' 0 \n 0 \n 0 '
t "subject 4 (3x3 1-iter)"   "dxss"                          "3 3 1"  $'   \n000\n   '
t "subject 5 (3x3 2-iter)"   "dxss"                          "3 3 2"  $' 0 \n 0 \n 0 '

echo "=== empty / no-draw input ==="
t "empty stdin"           ""       "3 3 0"  $'   \n   \n   '
t "newline only"          $'\n'    "3 3 0"  $'   \n   \n   '
t "invalid chars only"    "fghijk" "3 3 0"  $'   \n   \n   '
t "0 iterations on draw"  "dxss"   "3 3 0"  $' 0 \n 0 \n 0 '

echo "=== pen boundary behavior ==="
t "left boundary (a at x=0)"    "xa"      "3 3 0"  $'0  \n   \n   '
t "top boundary (w at y=0)"     "xw"      "3 3 0"  $'0  \n   \n   '
t "right boundary (d clamped)"  "xddddd"  "3 3 0"  $'000\n   \n   '
t "bottom boundary (s clamped)" "xssss"   "3 3 0"  $'0  \n0  \n0  '

echo "=== x toggle ==="
t "draw then lift"       "xdx"   "3 1 0"  "00 "
t "move then draw"       "dxd"   "3 1 0"  " 00"
t "lift mid-row"         "xdxd"  "3 1 0"  "00 "

echo "=== 1x1 grid ==="
t "1x1 empty"        ""  "1 1 0"  " "
t "1x1 draw 0-iter"  "x" "1 1 0"  "0"
t "1x1 draw 1-iter"  "x" "1 1 1"  " "

echo "=== stable configuration: 2x2 block ==="
# s d x d s a draws cells (1,1)(1,2)(2,2)(2,1) in a 4x4 grid
BLOCK=$'    \n 00 \n 00 \n    '
t "2x2 block 0-iter"   "sdxdsa"  "4 4 0"   "$BLOCK"
t "2x2 block 1-iter"   "sdxdsa"  "4 4 1"   "$BLOCK"
t "2x2 block 10-iter"  "sdxdsa"  "4 4 10"  "$BLOCK"

echo ""
printf "Results: %d passed, %d failed\n" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ] && exit 0 || exit 1
