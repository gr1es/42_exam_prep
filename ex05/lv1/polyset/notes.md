# polyset — crash course

**Caveat first:** the actual exam gives you working source for `bag`,
`searchable_bag`, `array_bag`, and `tree_bag` (header + implementation) plus a
`main`. None of those files are present in this folder — only `subject.txt`.
The exact method names/signatures you'll see on exam day may differ in detail
from what's below. Treat this as the *shape* of the problem (inheritance
structure, the diamond, what `search` and `set` need to do) so you can adapt
fast once you actually read the given headers, rather than as an exact API to
memorize.

## 1. What's given vs. what you write

Given (read these headers carefully first, they define your exact base-class
contracts):
- `bag` — abstract base. Something like `add(value)`, `remove(value)`,
  `size()`, maybe iteration helpers. Pure virtual, not instantiable.
- `searchable_bag` — abstract, **also derives from `bag`** (that's the trap,
  see §2), adds one pure virtual method, almost certainly something like
  `bool search(int value) const = 0;`.
- `array_bag` — concrete, implements `bag` using a raw array or
  `std::vector` as storage.
- `tree_bag` — concrete, implements `bag` using a binary search tree.

You write:
- `searchable_array_bag` — inherits **both** `array_bag` and `searchable_bag`,
  implements `search()` by linear-scanning the array storage.
- `searchable_tree_bag` — inherits **both** `tree_bag` and `searchable_bag`,
  implements `search()` by walking the BST (go left/right by comparison,
  found if you hit a matching node, not-found if you hit a null leaf).
- `set` — wraps *a* `searchable_bag` (composition, not inheritance) and
  exposes set semantics (no duplicates) on top of it.

## 2. The diamond problem — the actual conceptual core of part 1

```
        bag
       /   \
 array_bag  searchable_bag
       \   /
  searchable_array_bag
```

Both `array_bag` and `searchable_bag` derive from `bag`. If `searchable_array_bag`
inherits from both, and `bag` was inherited *non-virtually* by each, you end up
with **two separate `bag` sub-objects** inside one `searchable_array_bag` —
ambiguous member access, double storage, and ambiguous-base compile errors
the moment you try to call an inherited `bag` method.

- If the given `bag`/`array_bag`/`searchable_bag` headers already declare
  `class array_bag : public virtual bag` and
  `class searchable_bag : public virtual bag`, the diamond is already solved
  for you — you don't need to do anything except inherit normally:
  `class searchable_array_bag : public array_bag, public searchable_bag`.
- If they're **not** declared virtual in the given files, you cannot fix it
  retroactively from the derived class alone — virtual inheritance has to be
  declared at the point where the *common ancestor* is named, i.e. in
  `array_bag`'s and `searchable_bag`'s own inheritance lists. If you hit
  ambiguous-base errors on exam day, this diamond is almost certainly why —
  check whether you're even allowed to edit the given base headers (you
  normally aren't, which means the subject's authors will have already made
  the inheritance virtual; if they haven't, re-read the subject, you're
  probably missing something about which classes inherit from which).

**Practical test:** if `class searchable_array_bag` compiles and a single
call like `obj.size()` (inherited from `bag` through both parents) is *not*
ambiguous, the diamond is already handled. If you get an "ambiguous access
of 'size'" or "request is ambiguous" error, that's the diamond biting you.

## 3. Implementing `search()`

- `searchable_array_bag::search(value)` — linear scan over whatever container
  `array_bag` stores internally. If `array_bag`'s storage is private (likely),
  you need either a `protected` accessor already exposed by `array_bag`, or
  you re-walk via whatever public iteration/indexing API `array_bag` exposes
  (e.g. `operator[]` + `size()`, or `begin()`/`end()` if it's vector-backed).
  You cannot reach into private members of a base class from a derived class
  — only `protected` or `public` members are visible to you.
- `searchable_tree_bag::search(value)` — exploit the BST invariant: at each
  node, if `value == node.value` found; if `value < node.value` recurse/iterate
  left; else right; null/leaf without a match means not found. Don't write a
  linear scan here even though it would "work" — the point of the exercise is
  using the right algorithm for the right structure, and a BST search should
  be O(log n) on a balanced tree, not O(n).
- Both must be `const` (the subject explicitly says "don't forget the const,"
  and `search` is a read-only query — must be callable on a `const
  searchable_array_bag&`/`const searchable_tree_bag&`).

## 4. The `set` class — composition over inheritance

`set` "wraps a `searchable_bag`" — meaning a `set` *has* one, it does not
inherit one. Concretely: a member that is a reference or pointer to a
`searchable_bag` (you cannot have a member of type `searchable_bag` directly,
since it's abstract and can't be instantiated).

Two common shapes for that member, both legitimate depending on what the
given `main` does:
- `searchable_bag &_bag;` — `set` is constructed with an already-existing
  bag (e.g. `set s(some_searchable_array_bag);`) and never owns or destroys
  it. Simpler, but a reference member means `set` has no usable default
  constructor and no copy-assignment operator (references can't be
  reseated) — only acceptable if the given `main` never needs those.
- `searchable_bag *_bag;` — `set` owns a heap-allocated bag (passed in or
  newly created), deletes it in the destructor. Lets you support
  default-construction/assignment more normally, but then OCF requires you
  to think about *deep* copy (cloning the underlying bag) for the copy
  constructor/assignment — which is awkward unless `bag` exposes a `clone()`
  method. Only go this route if the given `main` actually exercises copying
  a `set`.

**Read the given `main` before picking** — it will show you exactly how a
`set` gets constructed and whether it's ever copied/assigned, which settles
the reference-vs-pointer question for you instantly instead of guessing.

`set`'s own interface should turn "a bag" into "a set" by enforcing
uniqueness on insertion — the actual logic is:

```cpp
void set::add(int value)
{
	if (!_bag->search(value))   // or _bag.search(value) for the reference version
		_bag->add(value);
	// else: already present, a set ignores duplicate inserts
}
```

This is the entire point of wrapping a *searchable* bag specifically — a
plain `bag` has no way to check "is this value already in here?" before
adding, so it could only ever be a multiset. `searchable_bag::search()` is
what makes the dedup check possible.

## 5. OCF, again

Every class — `searchable_array_bag`, `searchable_tree_bag`, and `set` — needs
the canonical four/five: default ctor (if meaningful — abstract bases don't
get one, but your concrete classes do), copy ctor, copy-assignment, destructor.
For the multiply-inherited classes, remember that constructors must initialize
*both* base classes explicitly if either takes constructor arguments:

```cpp
searchable_array_bag::searchable_array_bag() : array_bag(), searchable_bag() {}
```

For `array_bag`/`tree_bag` that own dynamically-allocated storage (a raw
array, or BST nodes), check whether *they* already implement proper deep-copy
OCF — if so, your derived classes' copy ctor/assignment can just delegate to
the base's via the member-initializer list / explicit base-class assignment,
no need to re-implement the deep copy yourselves.

## 6. Order of attack under exam time pressure

1. Read the given `bag.hpp`, `searchable_bag.hpp`, `array_bag.hpp`,
   `tree_bag.hpp` fully before writing anything — confirm whether virtual
   inheritance is already in place, and what the exact method names/signatures
   are (this whole file used best-guess names since the real ones weren't
   available while writing these notes).
2. Write `searchable_array_bag` first — it's the easier of the two `search()`
   implementations (linear scan vs. BST walk).
3. Write `searchable_tree_bag` — same OCF/inheritance pattern, swap in the
   BST search algorithm.
4. Read the given `main` to see exactly how `set` is constructed/used, then
   pick reference-vs-pointer member accordingly and implement `set` last —
   it depends on both of the previous classes existing and working.
5. Compile early and often — diamond-inheritance ambiguity errors are exactly
   the kind of thing you want to discover in minute 10, not minute 170.
