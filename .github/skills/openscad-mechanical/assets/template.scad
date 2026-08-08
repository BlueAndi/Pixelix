// <Short description of the part>
//
// Dimensions: <link or "measured" or "N/A">
// Units: mm
// Print orientation:
// - <part-name>: <e.g. "flat on rear face", "rear side down">
//
// Author: <Name> (<email>)
// License: MIT License (https://opensource.org/licenses/MIT)

// -----------------------------------------------------------------------
// Global constants
// -----------------------------------------------------------------------
eps           = 0.01; // Boolean overlap to avoid co-planar faces in mm
fit_tolerance = 0.2;  // General assembly clearance in mm
rounded_fn    = 96;   // Facet count for outer rounded contours

// -----------------------------------------------------------------------
// Preview / export selector
// -----------------------------------------------------------------------
// Possible values: assembly | <part1> | <part2> | …
part = "assembly";

// -----------------------------------------------------------------------
// Parameters – <ComponentA> (replace with real component name)
// -----------------------------------------------------------------------
comp_a_width  = 100; // Width of ComponentA in mm
comp_a_height = 50;  // Height of ComponentA in mm
comp_a_depth  = 30;  // Depth of ComponentA in mm

// -----------------------------------------------------------------------
// Parameters – <ComponentB>
// -----------------------------------------------------------------------
comp_b_diameter = 3.2; // Diameter of ComponentB mounting holes in mm
comp_b_count    = 4;   // Number of mounting holes

// -----------------------------------------------------------------------
// Derived dimensions (never hard-code these as literals in geometry)
// -----------------------------------------------------------------------
comp_a_half_width  = comp_a_width  / 2; // Half-width for centering in mm
comp_a_half_height = comp_a_height / 2; // Half-height for centering in mm

// -----------------------------------------------------------------------
// Pure helper functions
// -----------------------------------------------------------------------

/// Returns the offset needed to center an inner dimension within an outer one.
function centered_offset(outer, inner) = (outer - inner) / 2;

/// Returns a clearance-hole diameter for a given nominal bolt diameter.
function clearance_hole_d(nominal) = nominal + 0.4;

// -----------------------------------------------------------------------
// Reusable geometry modules
// -----------------------------------------------------------------------

/// Rounded rectangular 2-D profile in the XZ plane.
/// \param width   Extent in X in mm
/// \param height  Extent in Z in mm
/// \param radius  Outer corner radius in mm (0 = sharp)
module rounded_profile_xz(width, height, radius)
{
    $fn = rounded_fn;

    if (radius > 0)
    {
        translate([radius, radius])
            offset(r=radius)
                square([width - 2 * radius, height - 2 * radius]);
    }
    else
    {
        square([width, height]);
    }
}

/// Solid box with rounded XZ corners, extruded along Y.
/// \param size_xyz  [width, depth, height] in mm
/// \param radius    Corner radius in mm
module rounded_cube_xz(size_xyz, radius)
{
    translate([0, size_xyz[1], 0])
        rotate([90, 0, 0])
            linear_extrude(height=size_xyz[1])
                rounded_profile_xz(size_xyz[0], size_xyz[2], radius);
}

/// Vertical cylinder cutout positioned by XZ coords, drilled in +Y direction.
/// \param x_pos    Center X in mm
/// \param z_pos    Center Z in mm
/// \param diameter Hole diameter in mm
/// \param depth    Hole depth in mm
module hole_y(x_pos, z_pos, diameter, depth)
{
    translate([x_pos, -eps, z_pos])
        rotate([-90, 0, 0])
            cylinder(d=diameter, h=depth + 2 * eps, $fn=48);
}

// -----------------------------------------------------------------------
// Top-level part modules
// -----------------------------------------------------------------------

module part_a()
{
    difference()
    {
        // Outer shell
        cube([comp_a_width, comp_a_depth, comp_a_height]);

        // Example hole pattern
        for (i = [0 : comp_b_count - 1])
        {
            x = (i % 2 == 0) ? 10 : comp_a_width - 10;
            z = (i < 2)      ? 10 : comp_a_height - 10;
            hole_y(x, z, clearance_hole_d(comp_b_diameter), comp_a_depth);
        }
    }
}

module assembly()
{
    part_a();
    // translate([…]) part_b();
}

// -----------------------------------------------------------------------
// Dispatcher
// -----------------------------------------------------------------------
if      (part == "assembly") { assembly(); }
else if (part == "part_a")   { part_a();   }
else
{
    echo(str("Unknown part: ", part));
}
