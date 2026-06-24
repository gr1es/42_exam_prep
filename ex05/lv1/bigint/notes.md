# bigint — exam notes and pitfalls

A guide to the traps in this exercise, organized by the area of the class they hit. Each section explains *why* the pitfall exists, not just what to avoid — the reasoning is what transfers to other exam exercises.

## 1. Storage design

**The member variable cannot be a fixed-width integer type.** The entire point of the exercise is arbitrary precision; storing the value in an `unsigned int` caps you at ~4 billion and defeats the assignment before you've written a single operator. You need a *sequence container* of digits: `std::string` (digit characters `'0'`–`'9'`) or `std::vector<int>`/`std::deque<int>` (digit values `0`–`9`).

**Digit order matters more than container choice.** Decide up front whether index 0 holds the most-significant digit (MSDF, "as you'd write it on paper") or the least-significant digit (LSDF, reversed). This decision ripples through every operator:

- **LSDF** (e.g. `1337` stored as `[7,3,3,1]`): addition is a clean forward loop with carry, matching how you do long addition by hand but with increasing indices. Digitshift-left (×10ⁿ) is inserting `n` zeros at the *front* (low end); digitshift-right is dropping `n` elements from the *front*. The cost: printing requires iterating in *reverse*, and comparisons need most-significant-first traversal, which is also reverse iteration on this layout.
- **MSDF** (`1337` as `[1,3,3,7]`): printing is trivial (forward iteration), but addition and digitshift become awkward because carry propagation naturally happens from the *back*, and operand-length mismatches force you to index with offsets from the back.

LSDF is usually the less error-prone choice, since the two graded operations (addition, digitshift) are easier in that orientation and printing is a one-time, low-stakes reversal.

**Decide the zero invariant before writing any constructor.** Two real options: an empty container, or a container holding a single `0` element. Whichever you pick, *every* code path that can produce a zero value must agree — including the constructor from `unsigned int` when given `0` (a naive digit-peeling loop like `while (n != 0) { ... n /= 10; }` produces an **empty** container for `n == 0`, while a separately-written default constructor might use `{0}` — two different representations of the same value will silently break comparisons, printing, and addition simultaneously). This was a real bug encountered during development: `bigint(0)` printed as an empty string while `bigint()` printed `"0"`.

**Leading zeros live at the unintuitive end, depending on your digit order.** In LSDF storage, "no leading zeros" (a print-time/normalization requirement) means no *trailing* zero elements at the back (high-index, most-significant end) of the container — not the front. Get this backwards and your trim logic silently does nothing or trims the wrong digits.

**Operations can produce a "dirty" zero that needs explicit collapsing.** Even with a correct zero invariant at construction time, downstream operations can produce an all-zero result that isn't in canonical form. Concretely: `0 << 5` — inserting 5 zero-digits in front of a correctly-represented `{0}` gives `{0,0,0,0,0,0}`, which is mathematically zero but will print as `"000000"` unless you explicitly detect "is this entire result zero?" and collapse it back to the single-element canonical form. This is easy to miss because it only shows up when the *input* is already zero — testing only with non-zero inputs hides it completely.

## 2. Orthodox Canonical Form (OCF) and const-correctness

You need: default constructor, constructor from `unsigned int`, copy constructor, copy-assignment operator, destructor. The given `main` exercises all of them (`bigint c;`, `bigint d(1337)`, `bigint e(d)`).

**Self-assignment guard.** `operator=` should check `if (this != &other)` before copying, to handle `a = a` safely — particularly important if your assignment ever does anything more complex than a single container copy (e.g., frees/reallocates before copying).

**Const-correctness is not optional here — the given main forces it.** `a` is declared `const bigint a(42);` and is then used in `a + b`, `(d < a)`, `(d == a)`, and is printed. Every member function callable on a const object — every comparison, `operator+`, and the non-mutating digitshift operators — must be marked `const`. If you forget even one, code that compiles fine in isolation will fail the moment it's exercised against the `const` object in `main`.

A function being `const` also constrains *what iterator types you may use inside it*: a `const` member function sees its own members (and any `const&` parameter's members) as `const`, so calling `.begin()`/`.rbegin()` etc. on them yields `const_iterator`/`const_reverse_iterator`, not the mutable iterator types. Declaring the wrong iterator type is a compile error, not a logic bug — but it's an extremely common one to trip on early.

## 3. Return types — the single most common mistake category

This exercise has an unusually high density of "what should this operator return, and how" decisions, and getting them wrong tends to *compile* and then fail in ways that aren't always obvious from the test main alone.

| Operator | Returns | Why |
|---|---|---|
| `operator+(const bigint&) const` | `bigint` (by value) | Must not mutate either operand; the result is a brand-new value. Returning a reference here is returning a reference to a local stack object that's destroyed when the function returns — undefined behavior. |
| `operator+=(const bigint&)` | `bigint&` | Mutates `*this` and returns a reference to itself, enabling chaining (`a += b += c`) and avoiding an unnecessary copy. |
| `operator<<`, `operator>>` (digitshift, non-assigning) | `bigint` (by value) | Same logic as `+`: must not mutate the operand. Look at the given main — `b << 10` is used inline without `b` itself changing afterward (`b++` still operates on the un-shifted `b`). |
| `operator<<=`, `operator>>=` | `bigint&` | Mutates `*this`, returns reference to self. |
| `operator++()` (prefix) | `bigint&` | Mutates `*this`, returns reference to the *already-updated* self. |
| `operator++(int)` (postfix) | `bigint` (by value) | Must return the value *before* increment, as a separate object — the convention from built-in types. The `int` parameter is a dummy, never used, purely there to distinguish this overload from prefix at the declaration level. |
| `operator==`/`!=`/`<`/`<=`/`>`/`>=` | `bool` (by value) | Returning `bool&` is a guaranteed bug — there's no meaningful object for that reference to point to without it being a dangling reference to a temporary. |
| free `operator<<(std::ostream&, const bigint&)` | `std::ostream&` | This is what enables chaining (`std::cout << a << b`) — you must return the *same stream* you were given, not some other type. |

**The prefix/postfix distinction is purely syntactic, not semantic, at the declaration level** — this is the "not really done intuitively" pitfall: `operator++()` and `operator++(int)` differ only by an unused `int` parameter that exists *solely* to give the compiler two distinct overloads to choose between for `++x` vs `x++`. There is nothing about the parameter that's ever read; you'll see `(void)n;` or similar to silence an unused-parameter warning. If you forget the `int` parameter on the postfix version, you've just redeclared prefix twice and the program won't compile (or worse, picks the wrong overload depending on how strict your compiler is).

A clean way to implement both without duplicating the carry logic: have `operator++()` delegate to `*this += 1` (relying on the implicit `int → bigint` conversion described in the next section) and return `*this`; have `operator++(int)` copy-construct a snapshot of the current value first, then call `++(*this)` (or `*this += 1`), then return the snapshot.

## 4. The implicit-conversion trap (and why digitshift's parameter type matters)

The given main contains this line:

```cpp
std::cout << "(d >>= 2) = " << (d >>= (const bigint)2) << std::endl;
```

`(const bigint)2` is a C-style cast that constructs a temporary `bigint` from the `int` literal `2`, using your `bigint(unsigned int)` constructor. This means **your digitshift operators must accept a `const bigint&` parameter, not `unsigned int`** — a `bigint`-typed argument cannot implicitly become `unsigned int` (there's no conversion operator defined for that direction unless you add one, and you shouldn't need to).

The flip side: as long as `bigint(unsigned int)` is **not** marked `explicit`, the compiler is also allowed to use it to silently convert a plain `int` literal (like `b << 10` elsewhere in main) into a `bigint` when a `bigint`/`const bigint&` parameter is expected. This single non-`explicit` constructor is what makes *both* call shapes (`<< 10` and `>>= (const bigint)2`) compile against one `const bigint&`-parameter overload — you don't need separate `unsigned int` and `bigint` overloads.

**Extracting a plain count from a `bigint` parameter does not violate the spirit of the class.** It's tempting to feel like converting a `bigint` shift-amount back into `unsigned int` undermines "arbitrary precision," but the *value being stored* (`_n`) and the *shift amount* are conceptually different things — a shift amount is bounded by how many elements you're willing to insert/erase in a container, which is itself bounded by memory regardless of what integer type counts it. There's no real scenario where the shift count needs more digits than `unsigned int` can hold without the underlying insert/erase operation already being impractical for other reasons.

To extract the count: walk the shift-amount `bigint`'s digit container from *most significant to least* (in LSDF storage, that's `crbegin()`→`crend()`) and accumulate `count = count * 10 + digit` at each step — the mirror image of the digit-decomposition helper used in the `unsigned int` constructor.

## 5. Comparison operators

**You cannot compare two digit-sequences purely lexicographically without first checking length.** A 3-digit number is never less than a 2-digit number regardless of what the first digits happen to be — so always compare `.size()` first, and only fall through to a digit-by-digit comparison when lengths are equal.

**Digit-by-digit comparison must go from most-significant to least.** In LSDF storage that means reverse iteration (`crbegin()`/`crend()`), comparing pairs and returning as soon as a differing pair is found; if no differing pair is found, the values are equal (and the `<`/`>` result for that case is `false`, not "the loop should continue forever").

**Don't forget to advance both iterators in the equal-digit branch.** A very easy mistake: handle the "digits differ → return" branch, then accidentally never `++` either iterator in the "digits are equal, keep going" path — this produces an infinite loop, since the loop condition never changes.

**`const_reverse_iterator` can absolutely still be incremented inside a `const` function.** A common misconception: `const` on the function means the *elements* can't be modified through that iterator (`*it = 5` would be illegal), but advancing the iterator's position (`++it`) is completely fine — incrementing changes where the iterator points, it doesn't write to the container.

**Don't compare iterators from two different containers.** `it_o != _n.rend()` (comparing an iterator into `other._n` against `_n`'s `rend()`) is undefined behavior, even though both sides have the same type. Always pair an iterator with the `begin()`/`end()`/`rbegin()`/`rend()` of the *same* container it came from.

**`std::vector` already provides element-wise `==`/`!=`.** You don't need a manual loop to compare two digit-vectors for exact equality — `_n == other._n` already checks size and every element. (Its `<`/`<=`/`>`/`>=` are also defined, lexicographically — but don't use those directly for bigint's relational operators, since lexicographic vector comparison doesn't account for differing lengths the way numeric comparison needs to.)

## 6. Addition — where most of the subtle bugs live

**Carry must be a variable that survives across loop iterations**, separate from the per-iteration digit sum. A bug encountered during development: declaring the sum variable *outside* the loop and using `+=` to compute it meant each iteration's sum accumulated on top of the *previous* iteration's already-consumed sum, corrupting every digit after the first. The fix was either resetting the sum to `0` at the start of every iteration (then `+=` becomes harmless, though `=` is clearer) or just assigning (`sum = ...`) instead of accumulating — but the *carry* variable itself (distinct from the per-digit sum) must NOT be reset each iteration; it has to persist and feed into the next digit's calculation.

**Mismatched-length operands need carry to keep propagating through the "tail."** Once one operand's digits are exhausted, you're copying over the remaining digits of the longer operand — but if there's a pending carry at that point, it still has to be added into each of those remaining digits (and can itself keep propagating further), not just copied verbatim. Forgetting this breaks any addition where a carry survives past the shorter operand's length (e.g., `995 + 5`, `99999 + 1`).

**Don't push a raw two-digit sum.** In any branch, the value being pushed into the result container must be `sum % 10`, never the raw `sum` — a tail branch like `sum = *it + carry` can total up to `9 + 1 = 10`, which is not a valid single digit. The carry for the *next* digit becomes `sum / 10`.

**A leftover carry after both operands are exhausted must become a new digit.** `99 + 1 = 100` needs the final carry (`1`) appended as an entirely new most-significant digit after the main loop ends — easy to omit, and the resulting number will silently be a `% 10` of the correct answer (i.e., `0` instead of `100`) if you skip it.

## 7. Digitshift

**`std::vector` has no `push_front`.** Only `push_back`/`pop_back` are O(1). For inserting `n` zeros at the front (digitshift-left in LSDF storage), use `vector::insert(begin(), n, 0)` — a single call that inserts `n` copies at once, more efficient than `n` separate front-insertions even though it's still O(size) overall. (If you anticipate doing a lot of front manipulation, switching the container to `std::deque<int>` gives genuine O(1) `push_front`/`pop_front`, at the cost of slightly worse cache locality versus `vector`.)

**`insert()` shifts existing elements; it never overwrites.** Inserting at a position grows the container and pushes everything from that position onward to higher indices — it's not the same operation as assigning through an iterator/index.

**Digitshift-right needs a bounds check before erasing/advancing.** If the shift amount is greater than or equal to the number of digits, the entire number shifts away to zero — but advancing an iterator past `end()` (or erasing a range that runs past it) to try to express that is undefined behavior. Check `shiftAmount >= _n.size()` explicitly and short-circuit to the canonical zero value, rather than letting the erase/iterator-advance logic run unguarded. (A real crash was hit during development this way: a shift amount larger than the digit count caused a `std::length_error` from an out-of-bounds erase.)

**Re-normalize after digitshift, the same as after addition.** As covered in section 1, shifting a stored zero still needs to collapse back to the single canonical zero representation, not a multi-zero artifact of the insertion.

## 8. Stream output (`operator<<`)

**Must be a free function, not a member.** The left-hand operand is `std::ostream`, not `bigint` — member operator overloading binds the left operand to `*this`, which doesn't fit here. Signature: `std::ostream &operator<<(std::ostream &os, const bigint &b);`, declared outside the class (but still needing access to the private digit container via either a `const` accessor method or a `friend` declaration inside the class).

**Must print in the opposite order from storage, if you chose LSDF.** Printing has to go most-significant-digit-first for a human-readable result, which is the *reverse* of LSDF storage order — iterate with `crbegin()`/`crend()`.

**Digit values need conversion to digit characters before being appended to a string/stream.** Appending a raw `int` (value `0`–`9`) to a `std::string` via `+=` appends it as a `char` with *that numeric value* — i.e., unprintable control characters, not the visible glyphs `'0'`–`'9'`. You must add the character `'0'` to the digit value first (`'0' + digit`) to land on the correct ASCII digit character, mirroring the `c - '0'` conversion you'd use in the opposite direction.

**Return `std::ostream&`, never `std::string&`.** The return value is what permits chaining multiple `<<` calls in one statement; returning anything other than the stream itself breaks that chain at compile time or worse, silently changes what subsequent `<<` calls in the chain operate on.

## 9. General C++ traps that bit this exercise specifically

- **Iterating past `end()`/`rend()` is undefined behavior**, not a guaranteed crash — it might appear to work by accident on a given compiler/optimization level and then fail elsewhere. Always check the bound *before* dereferencing or incrementing, never after.
- **Using an uninitialized variable inside an accumulation (`+=`) is undefined behavior**, and the resulting garbage can look plausible enough to pass a casual visual check before failing on different inputs or compilers.
- **An overload resolution mismatch between the subject's `main` and your declared parameter types will not compile at all** — always check what the given `main` actually passes to each operator (literal `int`? a `bigint`-cast value? a `const` object?) before finalizing parameter types, rather than assuming the "obvious" type.
- **Compile with `-Wall -Wextra`** during development regardless of whether the exam's grading scripts require it — several of the real bugs hit while building this exercise (uninitialized variable, accumulation-instead-of-assignment) are exactly the class of issue these flags are designed to surface before runtime.

## 10. A test checklist worth running against your own implementation

Beyond whatever the given `main` shows you, deliberately exercise:

- Zero via every construction path (default constructor, `bigint(0)`) — confirm they're considered equal and print identically.
- Zero as an operand to every operator: `0 + x`, `0 << n`, `0 >> n`, `0 <<= n`, `0 >>= n` for various `n` including `0` itself.
- Shift amount exactly equal to the digit count (the boundary, not just "clearly less" or "clearly more").
- Operands of mismatched length on both sides of `+` (shorter-plus-longer and vice versa).
- A carry chain spanning multiple digits (`999 + 1`, `99999 + 1`, `9999` incremented).
- Self-referencing operations: `a += a`, `a <<= a`, `a >>= a`, `a = a` — confirm no aliasing bugs from reading and mutating the same underlying container in one operation.
- Both `==`/`!=` and the four ordering operators across equal values, and values that differ only in length.
- Full prefix/postfix increment behavior: confirm postfix returns the value *before* the increment while the object itself ends up incremented, and that a postfix call crossing a carry boundary (e.g. `9` or `99` incrementing) produces the correct carried result.
