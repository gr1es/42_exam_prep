# game_of_life — exam notes and pitfalls

A guide to the traps in this exercise, based on what actually broke (and got fixed) while building this implementation. Organized by the area of the program each pitfall hits, with the reasoning behind each one — that reasoning is what transfers to a from-scratch attempt under exam conditions.

## 1. The pointer-indirection trap — the single biggest source of bugs in this exercise

This exercise has an unusually high density of "does this function need to *change what the caller's pointer points to*, or just *mutate data through it*" decisions, and getting it wrong is what caused most of the real bugs encountered.

**The core distinction:** passing a pointer by value lets a function read and write the data *at* the address it holds (mutate shared memory — every copy of that pointer leads to the same place), but it does **not** let the function change *which address* the caller's own variable holds. To change that, the function needs the *address of the caller's pointer variable* — one more level of indirection than whatever you're trying to redirect.

Concretely, in this program:
- `process_input` takes `int *y, int *x` and correctly mutates the pointed-to *value* via `(*y)--`, `(*x)++`, etc. — this works because the goal is "change the number this pointer points to," which only needs one level of indirection on top of `int`.
- `process_states` needs to **replace which grid the caller's `grid` variable refers to** (swap in a freshly computed generation) — or, as eventually implemented, mutate the existing grid's *contents* in place rather than swap pointers at all. Either approach is valid; what doesn't work is taking `char **grid` (no extra indirection) and trying to do `grid = some_new_grid;` inside the function — that only reassigns the function's own local copy of the pointer, invisible to the caller. The fix used here was bumping the signature to `char ***grid`, calling it as `process_states(&grid, ...)`, and writing through `*grid` inside.

**Once you're at `char ***grid`, every subscript needs an explicit dereference-first parenthesization.** `[]` binds tighter than unary `*` in C, so `*grid[i]` parses as `*(grid[i])`, not `(*grid)[i]` — and since `grid` only validly points to *one* `char**` object (the address of the caller's variable), indexing `grid[i]` for `i != 0` reads unrelated memory. The correct form is always `(*grid)[i]` (or `(*grid)[y][x]` for two subscripts) — dereference once to get the real row-pointer array, *then* index into it. This bug is sneaky because `i == 0` (or `y == 0`) often happens to "work" by coincidence of how array indexing and dereferencing are defined (`grid[0] ≡ *grid` always, for any pointer), masking the bug until a later index is reached and it reads garbage or crashes.

**A pointer being passed by value also means a function reassigning it locally just loses the new value when it returns.** This came up with an early `inputs`-buffer attempt inside `process_input`: the parameter was `char *inputs`, and the function tried `inputs = malloc(...)` / `inputs = realloc(...)` inside — neither ever reached the caller, because both only updated the local copy of the pointer. The fix (or, as it turned out, the better fix) was realizing the buffer wasn't needed there at all — see section 3.

## 2. `read()` and the rules around it

**`read()`'s return value needs careful parenthesization in a `while` condition.** `=` has lower precedence than `>`, so `while (bytes_read = read(0, &c, 1) > 0)` parses as `bytes_read = (read(...) > 0)` — assigning `bytes_read` the *boolean result of the comparison* (`0` or `1`), not the actual byte count or error code `read()` returned. This silently destroys your ability to distinguish a clean EOF (`read()` returns `0`) from a genuine I/O error (`read()` returns `-1`), since both end up making the (mis-assigned) `bytes_read` equal to `0`. Correct form: `while ((bytes_read = read(0, &c, 1)) > 0)` — parenthesize the assignment so it captures the real return value first.

**The variable holding `read()`'s return value must be a signed type.** `read()` returns `ssize_t` (or here, `int` is fine too since you're reading 1 byte at a time and won't overflow it) — specifically a *signed* type, because `-1` is a meaningful, distinct return value (error) that must remain distinguishable from `0` (EOF) and any positive byte count. Declaring this variable as `size_t` (unsigned) makes any error-comparison like `bytes_read < 0` **always false** — the compiler will warn about this directly (`comparison of unsigned expression in '< 0' is always false`) if you have warnings on, which is exactly how this bug was caught here.

**Don't conflate "real error" with "legitimately empty input."** `read()` returning `0` on the very first call (clean EOF immediately) is a *valid* scenario — it means stdin was empty, not that something went wrong. If your input-buffer variable is only ever allocated *inside* the read loop, and the loop body never runs because EOF arrives instantly, that variable stays uninitialized/`NULL` — and any code afterward that assumes "the loop ran at least once" (e.g., writing a null terminator into it unconditionally) will dereference a pointer that was never allocated. Decide explicitly what an empty command sequence should produce (a valid empty string, most likely) and make sure the allocation path accounts for zero-iterations of the read loop, not just "one or more."

## 3. Buffer the input, or process it inline? — a design decision worth making deliberately

The allowed-functions list for this exercise (`atoi, read, putchar, malloc, calloc, realloc, free`) deliberately excludes `string.h` — no `strlen`, no `memcpy`. That makes "read the whole command sequence into a buffer first, then parse it in a second pass" meaningfully more work than it looks: you have to manually grow a buffer byte-by-byte with `realloc`, track length yourself, and null-terminate by hand.

Given that every command (`w a s d x`, or "anything else, do nothing") can be acted on the instant it's read — no command needs to look ahead or look back — there's a real simplification available: read and interpret one character at a time in the *same* loop, updating the pen's position/draw-state directly, without ever storing the full command string. This sidesteps the whole manual-buffer-growth problem entirely for the *drawing phase*.

This implementation ended up doing both for different reasons: `get_inputs()` buffers the whole command string (so it's available as a known-length, indexable sequence — useful for `play_game`'s `for` loop, and means the drawing phase and the read-from-stdin phase are decoupled, which can simplify testing). Neither approach is "more correct" than the other — but pick deliberately, since starting with one and bolting on the other (as happened during development, when an `inputs`-recording mechanism was half-built directly inside `process_input` before being abandoned for a cleaner separate `get_inputs` function) wastes time during an exam.

If you do buffer the input, the growable-buffer pattern (no `string.h`):
1. Start with `char *buf = NULL; size_t len = 0;`.
2. Each successful `read()`: if `buf` is `NULL`, `malloc` enough for this byte *plus* a future null terminator (i.e., at least 2 bytes on the first character, not 1 — undersizing the very first allocation by forgetting to leave room for the eventual terminator is an easy off-by-one); otherwise `realloc(buf, len + 2)` (room for the new byte plus the terminator).
3. Write the new byte at `buf[len]`, increment `len`.
4. On EOF (`read() == 0`): `buf[len] = '\0'`.
5. On error (`read() < 0`): free what you have and propagate failure distinctly from the EOF case (see section 2).

## 4. The pen model and grid coordinates

**The pen starts at `(0, 0)` and there's no requirement it ever needs bounds-checking against a board that might not exist.** This sounds obvious, but it's exactly what broke when `width` or `height` of `0` was allowed through argument validation (see section 6) — the pen unconditionally starts at row `0`, column `0`, and the `'x'` command unconditionally writes there the moment it's read, with no check that row `0`/column `0` actually exists on a board with zero rows or zero columns. The fix that actually mattered was rejecting degenerate board dimensions at the validation stage, not adding defensive checks deeper in the pen logic — it's much simpler to guarantee "the board always has at least one valid cell" once, up front, than to litter every grid-access site with existence checks.

**Boundary clamping for `w`/`a`/`s`/`d` needs to check the *value* the pointer points to, not the pointer itself.** `y != 0` (comparing the pointer `y` to a null check) is always true for a valid, non-null pointer — it does not tell you whether the row index *stored at* that address is `0`. You need `*y != 0`. This is an easy slip specifically because `!= 0` "looks like" a value comparison at a glance, when actually comparing a pointer.

**Upper-bound clamps are off-by-one if you don't subtract 1.** Valid row indices for a board of `height` rows are `0` to `height - 1`; valid column indices for `width` columns are `0` to `width - 1`. A guard of `*y < height` (rather than `*y < height - 1`) before incrementing lets the pen move one row past the last valid row — and depending on your grid's internal layout, that can mean indexing into a sentinel `NULL` row-pointer (`create_grid` here sets `grid[height] = NULL` as an end-of-array marker) and crashing on the next access through it.

## 5. `count_neighbors` — the Moore-neighborhood boundary checks are easy to over- or under-guard

**Every one of the 8 neighbor checks needs its *own independent* boundary guard — don't nest two checks that need different guards inside one shared `if`.** A real bug here: the left-neighbor check (`grid[y][x-1]`, needs `x != 0`) and the right-neighbor check (`grid[y][x+1]`, needs `x + 1 != width`) were both nested inside a single outer `if (x != 0)`. The right-neighbor check already had its own correct `x + 1 != width` guard — but being *also* wrapped in the outer `x != 0` meant it got skipped entirely whenever `x == 0`, even though a perfectly valid right neighbor exists at column `1` in that case. Each of the 8 directions needs to stand on its own guard condition; don't share one `if` across two checks unless they genuinely need the exact same condition.

**Check for "alive," don't check for "exists."** A neighbor check like `if (x != width && grid[y-1][x+1]) c++;` tests whether the character is non-`'\0'` (truthy), not whether it specifically equals `'0'` (alive). Since a dead cell is represented by `' '` (space, which is also non-zero/truthy), this kind of check silently counts *dead* neighbors as alive too. Every neighbor check needs an explicit `== '0'` comparison, not a bare truthiness check.

**Initialize the counter.** `size_t c;` with no initializer, followed by conditional `c++` calls, accumulates onto garbage from the very first increment. Always `size_t c = 0;`.

**A boundary guard that's *always true* gives a false sense of safety.** `if (x != width && ...)` inside a function only ever called with `x` already constrained to `0 <= x < width` by the caller's loop is a guard that can never actually fire — `x` can never equal `width` in valid use, so the condition is vacuously always true and never protects anything. The intended guard is almost always `x + 1 != width` (checking the *neighbor's* position, not the current cell's), which is what every other direction in a correct version of this function uses. If a guard never seems to trigger during testing, check whether it's testing the right value.

## 6. The aliased-read/write bug — the subtlest one in this exercise

This is the bug most likely to produce *plausible-looking but wrong* output rather than an obvious crash, which makes it the most dangerous one to carry into an exam.

**Never compute neighbor counts for the new generation from the same array you're simultaneously writing the new generation into, mid-pass.** If a single loop walks every cell, computes `count_neighbors` from array `A`, and also writes that cell's new state into `A` (the *same* array) before moving to the next cell, then every cell processed later in iteration order sees a *mix* of old-generation and already-updated-this-pass values for its neighbors — not a consistent snapshot of "the previous generation." The corruption is order-dependent: which neighbors get corrupted depends on which earlier cells in the loop happened to be adjacent and already got updated.

The symptom in this exercise was dramatic: an oscillator pattern that should evolve into a clearly different (but populated) shape after one generation instead came out *entirely dead*, because cells that should have survived or been born got starved of neighbor counts that had already been zeroed out by earlier same-pass writes to adjacent cells.

**The fix is to keep three roles conceptually distinct, even if you only have two physical arrays:** (1) the stable, read-only snapshot of the *previous* generation — used for both the "is this cell currently alive" check and for `count_neighbors` — (2) the array you're actively writing the *new* generation's results into, and (3) at the very end of the pass, sync the new results back into whatever the caller treats as "the grid." In this implementation, the original (caller's) grid naturally serves role (1) correctly, since nothing writes to it during the simulation loop — the bug was that `count_neighbors` was pointed at the *wrong* array (the in-progress write target) instead of the stable original. The fix was a one-line change: read neighbor counts from the untouched original grid, write results into the separate `copy`, then copy `copy`'s contents back into the original at the end.

If you ever restructure this differently (e.g., genuinely swapping which grid the caller's pointer refers to, rather than copying contents back), the same principle still applies: whatever you read neighbor counts from during a pass must not be the same memory you're mutating during that same pass.

## 7. Argument validation — `0` is a tempting but wrong "is this numeric" check

**`atoi` returns `0` for both a genuinely-typed `"0"` and for any string that fails to parse as a number at all** (e.g., `"abc"`, `""`). A validation check of `atoi(argv[n]) == 0` can't tell these apart on its own — which is why you'll often see an extra clause checking whether the raw string is literally `"0"` to disambiguate "the user meant zero" from "this isn't a number." But that disambiguation is only useful for arguments where `0` is actually a *valid* value (this exercise's `iterations` argument — `0` iterations is explicitly demonstrated in the subject's own examples). For `width`/`height`, `0` is never a sensible board dimension regardless of whether it was "intentional" — a board with zero rows or columns has no valid cell positions at all, and nothing downstream is expected to handle that gracefully. The bug here was applying the same "let literal `0` through" exemption to `width`/`height` as well, which let a degenerate board slip past validation and crash deeper in the program (see section 4). The fix: reject `< 1` outright for `width`/`height` (no exemption), and reserve the `0`-disambiguation logic specifically for `iterations`.

**A negative number for `iterations` is also worth explicitly rejecting**, not just relying on a `for (i = 0; i < iterations; i++)` loop to naturally not execute — `atoi("-5")` returns `-5`, which *would* make such a loop simply not run (since `0 < -5` is false), silently treating a malformed argument as "zero iterations" rather than flagging it as invalid input. Whether that's acceptable depends on how strict you want argument validation to be; this implementation chose to reject negative iteration counts explicitly rather than let them silently degrade to a no-op.

## 8. General testing strategy for this exercise

- **Compile with `-Wall -Wextra`, always.** The unsigned/signed comparison bug in section 2 (`bytes_read < 0` always false because `bytes_read` was `size_t`) was caught directly by a compiler warning before ever needing a test case to expose it.
- **`valgrind --leak-check=full --show-leak-kinds=all` catches more than leaks** — both the heap-buffer-overflow (width `0`) and the use-after-free-adjacent bugs in this exercise's development showed up as `Invalid read`/`Invalid write` reports with exact file/line attribution, which is far faster than guessing from a segfault alone.
- **Test degenerate board sizes specifically: `1×1`, `width 0`, `height 0`.** A `1×1` board with a single live cell is a legitimate, useful test (it should die after one generation — zero possible neighbors, underpopulation) and is cheap to hand-verify. `width`/`height` of `0` exposed the most serious bug in this whole exercise.
- **Test invalid/garbage command characters mixed into otherwise-valid input** — confirms the "pen does nothing in case of invalid command" requirement without needing a dedicated separate test harness.
- **Test pen movement driven far past every boundary in one long command string** (many more `w`/`a`/`s`/`d` than the board is wide/tall) — confirms clamping holds under sustained pressure, not just a single out-of-bounds attempt.
- **Test empty stdin specifically** (`printf '' | ./life ...`) — this is the scenario most likely to be skipped in casual testing (everyone tests with *some* input) but is exactly the kind of edge case validation-focused grading tends to include.
- **Verify oscillating/periodic patterns across multiple iteration counts**, not just one. A vertical-line oscillator (the subject's own `dxss` example) returning to its original shape after exactly 2 iterations is a strong correctness signal — both because it's a true property of that pattern under the real rules, and because subtle bugs (like the aliased-array issue in section 6) tend to silently break exactly this kind of multi-generation consistency while still looking plausible after only one generation.
- **Diff your output against the subject's literal examples byte-for-byte** (`cat -e` to make trailing spaces and line endings visible) rather than eyeballing it — this exercise's expected output is whitespace-sensitive (`' '` vs `'0'`), and visual inspection alone misses trailing-space discrepancies easily.

## 9. Reference: the working function set (signatures, as of this implementation)

```c
static int      verify_input(int argc, char *argv[]);
static char   **create_grid(int width, int height);
static void     copy_grid(char **copy, char **grid);
static void     print_grid(char **grid, int height, int width);
static void     free_grid(char **grid, int height);
static void     process_input(char **grid, int *y, int *x, int *writing_flag,
                                int height, int width, char input);
static void     play_game(char **grid, int height, int width, char *inputs);
static size_t   count_neighbors(char **grid, int y, int x, int height, int width);
static void     process_states(char ***grid, int height, int width);
static char    *get_inputs(void);
```

Notes on the shape of this set, to reconstruct from memory:

- `process_states` is the one function that needs the extra pointer indirection (`char ***`), because it's the one place a function needs to make the *caller's* grid reflect a newly computed generation, rather than just mutate existing cell contents. Every other grid-taking function is plain `char **` because they only ever read or write through an existing, unchanged pointer.
- `count_neighbors` returns `size_t` (never negative, naturally bounded `0`–`8`) and takes the grid to read from as a parameter — critically, the *caller* (`process_states`) is responsible for passing the correct (stable, previous-generation) array, not `count_neighbors` itself; the function has no way to know on its own which of two grids is "the right one to read from."
- `get_inputs` takes no parameters and returns the heap-allocated, null-terminated command string (or `NULL` specifically for a genuine `read()` error) — the caller (`main`) is responsible for deciding what a `NULL` return versus a valid-but-empty string should each mean for program behavior.
