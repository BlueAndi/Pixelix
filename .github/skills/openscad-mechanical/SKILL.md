---
name: openscad-mechanical
description: 'Create and edit OpenSCAD (.scad) files for 3D-printable mechanical parts and housings. Use when: modeling enclosures, brackets, mounting plates, standoffs, panels, or any parametric mechanical part. Covers OpenSCAD language syntax, project conventions, boolean operations, module and function design, tolerance/clearance choices, and print-orientation notes.'
argument-hint: 'Describe the part you need (e.g., "mounting bracket 40×20 mm, two M3 holes")'
---

# OpenSCAD Mechanical Part Authoring

## When to Use

Load this skill when asked to:
- Create or update a `.scad` file for any printable mechanical part (housing, frame, bracket, spacer, backplate, faceplate …)
- Add parametric models for PCB mounting, connector cutouts, or panel assemblies
- Adapt tolerances or clearances for a new printer/material
- Explain or review OpenSCAD geometry logic

## Language Reference

Full language reference: [language-reference.md](./references/language-reference.md)

Quick syntax recap:
- Variables are **single-assignment** within a scope; use `let()` for local rebinding.
- Modules define reusable 3-D geometry; functions return values (no geometry).
- Boolean: `union()`, `difference()`, `intersection()`.
- 3-D primitives: `cube([x,y,z])`, `sphere(r)`, `cylinder(h,d)`, `polyhedron()`.
- 2-D primitives + extrusion: `square()`, `circle()`, `polygon()` → `linear_extrude()` / `rotate_extrude()`.
- Transforms: `translate()`, `rotate()`, `mirror()`, `scale()`, `resize()`.
- Rounding via 2-D `offset(r)` inside `linear_extrude()`, or `minkowski()`.
- Special variables: `$fn` (facets), `$fs` (min-segment size), `$fa` (min-angle).
- Flow: `for()`, `if/else`, conditional `? :`.
- Includes: `use <file.scad>` (modules/functions only), `include <file.scad>` (full execution).

## Project Conventions (this repository)

Derived from the existing `doc/boards/hub75/housing/hub75.scad`:

### File Header

```scad
// <Short description of what this part is>
//
// Dimensions: <link or "measured" or "N/A">
// Units: mm
// Print orientation:
// - <part-name>: <orientation advice>
//
// Author: <Name> (<email>)
// License: MIT License (https://opensource.org/licenses/MIT)
```

### Parameter Block Order

1. **Global constants** — `eps`, `fit_tolerance`, `rounded_fn`
2. **Component parameters** — one group per physical object, prefixed (`frame_`, `panel_`, `board_`, …)
3. **Derived dimensions** — calculated from the above, never hard-coded
4. **Preview/export selector** — `part = "assembly";`

```scad
// Global constants
eps           = 0.01;  // Boolean overlap in mm
fit_tolerance = 0.2;   // Assembly clearance in mm
rounded_fn    = 96;    // Facet count for outer curves

// Part selector (assembly | frame | faceplate | …)
part = "assembly";
```

### Naming Conventions

| Item | Convention | Example |
|------|-----------|---------|
| Parameters | `snake_case` prefixed by component | `frame_wall_thickness` |
| Modules | `snake_case` verbs or nouns | `rounded_cube_xz()`, `board_mount_pattern()` |
| Pure functions | `snake_case` | `centered_offset(outer, inner)` |
| Booleans | descriptive flag | `show_panel = false;` |

### Inline Comment Rule

Every parameter must have a `// Description in mm` (or relevant unit) comment on the same line:

```scad
frame_wall_thickness = 3; // Thickness of the frame walls in mm
```

### Boolean Operation Patterns

Always add `eps` overlap to avoid z-fighting / non-manifold edges:

```scad
eps = 0.01;

difference() {
    cube([width, depth, height]);
    // Cutout starts eps below the surface, extends eps beyond the far side
    translate([x, y, -eps])
        cylinder(d=hole_d, h=height + 2*eps, $fn=48);
}
```

### Hole Sizing Guidelines

| Purpose | Diameter formula |
|---------|-----------------|
| Clearance (bolt passes through) | `bolt_nominal + 0.4` |
| Tapped / press-fit | `bolt_nominal - 0.2` (tune per material) |
| General assembly fit | `nominal + fit_tolerance` |

### Rounded Corners

Prefer the 2-D `offset(r)` + `linear_extrude()` pattern (avoids Minkowski overhead):

```scad
module rounded_profile_xz(width, height, radius) {
    $fn = rounded_fn;
    if (radius > 0)
        translate([radius, radius])
            offset(r=radius)
                square([width - 2*radius, height - 2*radius]);
    else
        square([width, height]);
}

module rounded_cube_xz(size_xyz, radius) {
    translate([0, size_xyz[1], 0])
        rotate([90, 0, 0])
            linear_extrude(height=size_xyz[1])
                rounded_profile_xz(size_xyz[0], size_xyz[2], radius);
}
```

### Part Selector Pattern

Use a top-level `if` chain on a `part` string so each piece can be exported independently or shown in assembly:

```scad
if      (part == "assembly") { assembly(); }
else if (part == "frame")    { frame(); }
else if (part == "faceplate"){ faceplate(); }
```

## Procedure

### 1. Gather Requirements

Collect these before writing any geometry:
- Overall envelope (width × height × depth in mm)
- List of sub-parts to model (frame, lid, spacer, …)
- External components that must fit inside (PCB, connectors, panels)
- Mounting interfaces (screw holes, snap fits, press fits)
- Print orientation constraints (overhang limits, layer strength direction)
- Clearances and tolerances (default: `fit_tolerance = 0.2 mm`)

### 2. Read Existing File (if editing)

Always read the full file before modifying. Identify:
- Existing parameter names to reuse
- Module and function signatures to extend, not replace
- The `part` selector — add new values rather than removing old ones

### 3. Structure the File

Use the template in [assets/template.scad](./assets/template.scad):
1. File header comment
2. Global constants block
3. Per-component parameter blocks (ordered by assembly depth: outermost first)
4. Derived dimensions
5. `part` selector variable
6. Pure functions
7. Reusable geometry modules (primitives → composites)
8. Top-level part modules
9. `if/else` part-selector dispatcher

### 4. Implement Geometry

**Work outside-in** (define outer shell first, then subtract holes/cutouts):

1. Build the solid body as a `union()` or single primitive.
2. Subtract connector cutouts, mounting holes, and internal cavities via `difference()`.
3. Add small features last (chamfers, lips, standoffs).

**Parametrize everything** — no magic numbers in geometry calls. Every numeric literal must trace back to a named variable.

### 5. Verify

Check all of the following before finishing:
- [ ] No hard-coded mm values in geometry calls (only in the parameter block)
- [ ] Every parameter has a `// ... in mm` (or unit) inline comment
- [ ] `eps` is applied to all `difference()` cutouts
- [ ] Holes use the correct clearance formula (see table above)
- [ ] `$fn` or `$fn=48` set on every `cylinder()` / `circle()`
- [ ] Print orientation note added to file header
- [ ] Part selector works: each `part` value renders without error
- [ ] Assembly preview (`part = "assembly"`) shows correct relative positions

### 6. Export Notes

- Render to STL per part: `File → Export → Export as STL` (or CLI: `openscad -o part.stl -D 'part="frame"' file.scad`)
- For FFF printing: slice with part-specific orientation noted in the file header
- Recommended `$fn` for final export: ≥ 96 for outer curves, 48 for holes
