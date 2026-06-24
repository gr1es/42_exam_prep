# vect2 — crash course

A small operator-overloading exercise: a 2D int vector. The whole difficulty is
getting every operator's *signature* (member vs. free, return type, const-ness)
right so the given `main` compiles and behaves — the actual math is trivial.

## 1. What you're building

```cpp
class vect2
{
	private:
		int _x;
		int _y;
	public:
		vect2();                       // 0, 0
		vect2(int x, int y);
		vect2(const vect2 &other);
		vect2 &operator=(const vect2 &other);
		~vect2();

		int &operator[](int i);              // v1[1] = 12; needs non-const ref
		const int &operator[](int i) const;  // v3[1] where v3 is const

		vect2 &operator+=(const vect2 &other);
		vect2 &operator-=(const vect2 &other);
		vect2 &operator*=(int scalar);

		vect2 operator+(const vect2 &other) const;
		vect2 operator-(const vect2 &other) const;
		vect2 operator*(int scalar) const;
		vect2 operator-() const;             // unary minus, -v2

		vect2 &operator++();      // prefix
		vect2 operator++(int);    // postfix
		vect2 &operator--();      // prefix
		vect2 operator--(int);    // postfix

		bool operator==(const vect2 &other) const;
		bool operator!=(const vect2 &other) const;
};

vect2 operator*(int scalar, const vect2 &v);   // 3 * v2 — must be a free function
std::ostream &operator<<(std::ostream &os, const vect2 &v);
```

No bound checking required for `operator[]` per the subject — index is
always 0 or 1 in the given main, don't add a check that isn't asked for.

## 2. Why `operator[]` needs two overloads

`main` does both:
- `v1[1] = 12;` — requires a *non-const* `operator[]` returning `int&` (you need
  a writable reference into `_x`/`_y`).
- `v3[1]` where `v3` is `const vect2` — calling `operator[]` on a const object
  only resolves to a `const`-qualified overload. If you only write the
  non-const version, this line fails to compile.

So you need both `int &operator[](int)` and `const int &operator[](int) const`,
typically implemented as `return (i == 0) ? _x : _y;` (or via a small
`if`/ternary — no bounds error path needed).

## 3. `3 * v2` is why one operator* must be a free function

A member `operator*` only works when the **left-hand side** is a `vect2`
(`v2 * 3`). For `3 * v2`, the left operand is `int`, which has no member
functions you can hook into — so you need a free function:

```cpp
vect2 operator*(int scalar, const vect2 &v) { return v * scalar; }
```

implemented by just forwarding to the member `operator*(int) const`. This is
the same "free function for the reversed-operand case" pattern as bigint's
stream `<<` — anytime the left operand isn't your class type, the operator
can't be a member.

## 4. Mutating vs. non-mutating operators — same split as bigint

- `+=`, `-=`, `*=`, prefix `++`/`--` all **mutate `*this` and return `vect2&`**
  (enables `v2 += v2 += v3;` chaining in the given main — the inner `+=`
  must return a reference usable as the right-hand operand of the outer one).
- `+`, `-`, scalar `*`, unary `-`, postfix `++`/`--` all **return a new
  `vect2` by value** and must not modify either operand. Returning a reference
  to a local in these is the same dangling-reference bug as in bigint.
- Easiest implementation: write `+=`/`-=`/`*=` first, then implement `+`/`-`/`*`
  as "copy `*this`, apply the `+=`-style op to the copy, return the copy."
  Postfix `++`/`--` follow the identical pattern from bigint: snapshot first,
  call the prefix version, return the snapshot.

## 5. Printing

`<<` must be a free function (left operand is `std::ostream`, not `vect2`),
returning `std::ostream&` to allow chaining, and must produce exactly
`{x, y}` — match the format the subject spells out
(`"{" << v[0] << ", " << v[1] << "}"`) character for character, including the
comma-space.

## 6. OCF reminders (same as every other exam class)

- Default ctor `vect2()` → `{0, 0}` (the given main relies on this: `vect2 v1;`
  then prints `v1` as `{0, 0}`).
- Copy ctor and copy-assignment are trivial here (no pointers/owned resources),
  but still required — `vect2 v4 = v2;` and `const vect2 v3(v2);` exercise them.
- `operator==`/`operator!=` compare both components; return `bool` by value.
- Mark every operator that doesn't need to mutate `*this` as `const` —
  `v1 == v3`/`v1 != v1` are called where one side may be `const`, and the
  compiler will reject a non-const member call on a const object.

## 7. Quick mental checklist while coding

1. Constructors + OCF.
2. `operator[]` both overloads (the const one is easy to forget).
3. `+=`, `-=`, `*=`.
4. `+`, `-`, member `*(int)`, free `*(int, const vect2&)`, unary `-`.
5. prefix/postfix `++`/`--` (four functions total, two pairs).
6. `==`, `!=`.
7. free `operator<<`.

If you reproduce this skeleton from memory and fill bodies with plain
component-wise arithmetic, this exercise should be fully solvable inside the
exam window even without prior practice — there's no tricky data-structure
invariant like bigint's zero/carry logic, just signature discipline.
