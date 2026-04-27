# OpenSCAD Language Quick Reference

Source: https://en.wikibooks.org/wiki/OpenSCAD_User_Manual/Language_Reference_OBS2015.7.30

## Data Types

| Type    | Example                                |
| ------- | -------------------------------------- |
| Number  | `42`, `3.14`, `1e-3`                   |
| Boolean | `true`, `false`                        |
| String  | `"text"`                               |
| Vector  | `[1, 2, 3]`, `[[1,0],[0,1]]`           |
| Range   | `[start : step : end]` → `[0 : 1 : 5]` |
| Undef   | `undef` (missing parameter sentinel)   |

## Variables

Single-assignment per scope (last-wins at parse time in older versions; treat as immutable):

```scad
x = 10;
y = x * 2;           // 20
```

Use `let()` for true local scope inside expressions:

```scad
result = let(a = 3, b = 4) sqrt(a*a + b*b);   // 5
```

## Special Variables

| Variable    | Effect                                                  |
| ----------- | ------------------------------------------------------- |
| `$fn`       | Fixed number of facets for circles/spheres/cylinders    |
| `$fs`       | Minimum facet size (mm) — limits tiny-polygon explosion |
| `$fa`       | Minimum facet angle (degrees)                           |
| `$t`        | Animation time parameter `[0..1)`                       |
| `$children` | Number of child objects in a module                     |

Set globally or locally: `sphere(r=5, $fn=64);`

## 3-D Primitives

```scad
cube([x, y, z]);                       // box, origin at [0,0,0]
cube([x, y, z], center=true);          // centered
sphere(r=5);                           // or sphere(d=10)
cylinder(h=10, r=3);                   // r1/r2 for cone; d/d1/d2 also valid
cylinder(h=10, d1=6, d2=0);           // cone
polyhedron(points=[…], faces=[…]);     // arbitrary solid
```

## 2-D Primitives

```scad
square([w, h]);
square([w, h], center=true);
circle(r=5);                           // or circle(d=10)
polygon(points=[…]);
polygon(points=[…], paths=[…]);        // with holes
text("Hello", size=10, font="Liberation Sans");
```

## Extrusion (2-D → 3-D)

```scad
linear_extrude(height=5)              square([10, 5]);
linear_extrude(height=10, twist=90)   circle(r=3);
rotate_extrude(angle=360)             translate([5,0]) circle(r=2);
```

## Transforms

```scad
translate([x, y, z]) { … }
rotate([ax, ay, az]) { … }             // degrees, Euler XYZ
rotate(a, v=[0,0,1]) { … }            // angle + axis
mirror([1, 0, 0]) { … }               // reflect about YZ plane
scale([sx, sy, sz]) { … }
resize([new_x, new_y, new_z]) { … }
multmatrix(m) { … }                   // 4×4 affine matrix
```

## Boolean Operations

```scad
union()        { a(); b(); }   // merge (default grouping)
difference()   { solid(); cutout(); }   // solid minus all subsequent children
intersection() { a(); b(); }   // keep only overlapping volume
```

Always add `eps` (e.g., `0.01 mm`) to cutout depth to prevent co-planar faces.

## Rounding / Blending

```scad
// 2-D offset before extrusion (fast)
offset(r=3) square([20, 10]);

// Minkowski sum (expensive but general)
minkowski() { cube([10,10,5]); sphere(r=2); }

// Convex hull
hull() { translate([0,0,0]) sphere(1); translate([10,0,0]) sphere(1); }
```

## Boolean Modifiers (syntactic operators)

| Prefix | Effect                                               |
| ------ | ---------------------------------------------------- |
| `#`    | Show in transparent red (debug highlight)            |
| `%`    | Show as transparent ghost (does not affect geometry) |
| `!`    | Render only this subtree                             |
| `*`    | Disable (ignore) subtree                             |

## Flow Control

```scad
// For loop — generates multiple objects
for (i = [0:5])          translate([i*10, 0, 0]) cube(5);
for (p = [[0,0],[10,5]]) translate(p) circle(r=2);

// Intersection for loop
intersection_for (i = [1:3]) rotate(i*60) square([20, 2], center=true);

// Conditional
if (show_holes) { holes(); }
else            { solid(); }

// Inline conditional (expression context)
value = (x > 0) ? x : 0;
```

## User-Defined Functions

Return a scalar, vector, or string — **not geometry**:

```scad
function clearance_hole(nominal) = nominal + 0.4;
function centered(outer, inner)  = (outer - inner) / 2;
```

Recursive functions are supported; use conditional `? :` as the base case.

## User-Defined Modules

Encapsulate geometry. Children are optional:

```scad
module standoff(h, d_outer, d_hole) {
    difference() {
        cylinder(h=h, d=d_outer, $fn=48);
        translate([0, 0, -eps])
            cylinder(h=h + 2*eps, d=d_hole, $fn=48);
    }
}

// Module with children
module colored_group(c) {
    color(c) children();
}
```

## Include / Use

```scad
use <lib/helpers.scad>      // import modules and functions only (no execution)
include <lib/constants.scad> // execute file (runs top-level geometry too)
```

## Import / Export

```scad
import("part.stl");          // embed existing mesh
import("outline.dxf");       // 2-D import for extrusion
```

CLI export:
```sh
openscad -o output.stl -D 'part="frame"' model.scad
openscad -o output.png --camera=0,0,0,55,0,25,200 model.scad
```

## Useful Math Functions

`abs`, `sign`, `floor`, `ceil`, `round`, `sqrt`, `pow`, `exp`, `log`, `ln`,
`sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `min`, `max`,
`norm`, `cross`, `concat`, `len`.

## Debugging

```scad
echo("value =", myVar);      // prints to console
assert(condition, "msg");    // halts with message if false (OpenSCAD ≥ 2019.05)
```
