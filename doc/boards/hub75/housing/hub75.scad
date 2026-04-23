// OpenSCAD code for Hub74 64x64 LED Matrix with
// Adafruit MatrixPortal S3 development board incl.
// mounting holes and cutouts for USB-C and power connectors.
//
// Dimensions: https://learn.adafruit.com/assets/123526
// Units: mm
// Print orientation:
// - frame: rear side down
// - faceplate: flat on rear face
// - spacer: flat on panel side
// - backplate: flat on inner face
//
// Author: Andreas Merkle (web@blue-andi.de)
// License: MIT License (https://opensource.org/licenses/MIT)

// General parameters
factor_inc_to_mm = 25.4; // Conversion factor from inches to millimeters
eps = 0.01; // Small overlap for boolean operations in mm
fit_tolerance = 0.2; // General assembly clearance in mm
rounded_fn = 96; // Segment count for smooth rounded contours ($fn for frame/backplate corner rounding)

// Preview / export selection
part = "assembly"; // assembly, frame, faceplate, spacer, backplate
show_panel = false;
show_spacer = false;
show_board = false;

// Parameters LED matrix Hub75 panel
panel_width = 192; // Width of the LED matrix panel in mm
panel_height = 192; // Height of the LED matrix panel in mm
panel_depth = 15; // Depth of the LED matrix panel in mm

// Parameters AdaFruit Matrix Portal S3
board_width = 1.75 * factor_inc_to_mm; // Width of the MatrixPortal S3 board in mm
board_height = 2.50 * factor_inc_to_mm; // Height of the MatrixPortal S3 board in mm
board_depth = 20; // Depth of the MatrixPortal S3 board in mm
board_mounting_holes_h_distance = 1.6 * factor_inc_to_mm; // Distance between mounting holes in mm (1.6 inches converted to mm)
board_mounting_holes_v_distance = 0.78 * factor_inc_to_mm; // Distance between mounting holes in mm (0.78 inches converted to mm)
board_mounting_holes_diameter = 3.2; // Diameter of the mounting holes in mm
board_mounting_holes_offset_x = (board_width - board_mounting_holes_h_distance) / 2; // X offset of the mounting holes from the center of the board in mm
board_mounting_holes_offset_y = (board_height - board_mounting_holes_v_distance) / 2; // Y offset of the mounting holes from the center of the board in mm

// Parameters for the LED matrix panel spacer
spacer_thickness = 3; // Thickness of the spacer in mm
spacer_width = panel_width; // Width of the spacer in mm
spacer_height = panel_height; // Height of the spacer in mm

// Parameters for frame
frame_panel_space = 0.5; // Space between the frame and the LED matrix panel in mm
frame_wall_thickness = 3; // Thickness of the frame walls in mm
frame_corner_radius = 8; // Outer corner radius of the frame when viewed from the front in mm
frame_corner_clearance = frame_corner_radius > 0 ? frame_corner_radius * (1 - 1 / sqrt(2)) + fit_tolerance : 0; // Extra half-margin required so square plates still fit inside the rounded outer contour in mm
frame_corner_side_relief = frame_corner_radius > frame_wall_thickness ? frame_corner_radius - sqrt(pow(frame_corner_radius, 2) - pow(frame_corner_radius - frame_wall_thickness, 2)) : 0; // Loss of usable wall height at each rounded corner before full wall thickness is available in mm
frame_faceplate_outer_margin = 2; // Additional outer frame margin around the rectangular faceplate in mm (per side)
frame_faceplate_glue_lip = 1.5; // Glue lip behind the faceplate opening in mm
frame_panel_stop_depth = 2; // Depth of the panel stop behind the faceplate in mm
frame_panel_stop_overlap = 2.5; // Support overlap around the panel in mm
frame_rear_clearance = 1; // Clearance behind the board in mm
frame_foot_width = 26; // Width of one stand foot in X direction in mm
frame_foot_depth = 18; // Stand foot extension behind the frame in Y direction in mm
frame_foot_height = 8; // Stand foot height from the base in Z direction in mm
frame_foot_inset = 16; // Distance from outer frame side to stand foot in mm
frame_width = max(panel_width + 2 * frame_panel_space + 2 * frame_wall_thickness, panel_width + 4 + 2 * frame_corner_clearance + 2 * frame_faceplate_outer_margin); // Total width of the frame in mm
frame_height = max(panel_height + 2 * frame_panel_space + 2 * frame_wall_thickness, panel_height + 4 + 2 * frame_corner_clearance + 2 * frame_faceplate_outer_margin); // Total height of the frame in mm

// Parameters faceplate
faceplate_width = panel_width + 4; // Width of the faceplate in mm
faceplate_height = panel_height + 4; // Height of the faceplate in mm
faceplate_thickness = 3; // Thickness of the faceplate in mm
faceplate_cutout_panel_space = 1; // Space between the faceplate cutout and the LED matrix panel in mm
faceplate_cutout_width = panel_width + 2 * faceplate_cutout_panel_space; // Width of the faceplate cutout in mm
faceplate_cutout_height = panel_height + 2 * faceplate_cutout_panel_space; // Height of the faceplate cutout in mm
faceplate_recess_width = faceplate_width + 2 * fit_tolerance; // Pocket width for the faceplate in mm
faceplate_recess_height = faceplate_height + 2 * fit_tolerance; // Pocket height for the faceplate in mm
faceplate_recess_depth = faceplate_thickness + fit_tolerance; // Pocket depth for the faceplate in mm

// Parameters for board assembly on the spacer
board_thickness = 1.6; // Approximate PCB thickness in mm
board_standoff_height = 6; // Distance between spacer and board in mm
board_component_depth = board_depth - board_thickness - board_standoff_height; // Remaining depth for components in mm
board_mounting_standoff_diameter = 7; // Diameter of the spacer standoffs in mm
board_mounting_hole_clearance = board_mounting_holes_diameter + 0.4; // Clearance through hole in the spacer in mm

// Parameters for panel to spacer fastening
panel_mount_hole_inset = 8; // Configurable inset for panel/spacer mounting holes in mm
panel_mount_hole_diameter = 3.2; // Diameter of the panel/spacer mounting holes in mm

// Parameters for backplate
backplate_thickness = frame_wall_thickness; // Thickness of the backplate in mm
backplate_corner_radius = 3; // Corner radius of the backplate in mm for better fit in the rounded frame contour
backplate_slot_depth = 1.5; // Depth of the rail groove into each side wall in mm
backplate_slot_wall = 2.0; // Thickness of the rear wall behind the backplate slot in mm
backplate_bottom_stop_height = frame_wall_thickness; // Bottom stop for the sliding backplate in mm
backplate_top_clearance = frame_corner_side_relief + fit_tolerance; // Required clearance below the rounded top corners so the side slots stay closed in mm

// Derived housing dimensions
panel_cavity_width = panel_width + 2 * frame_panel_space;
panel_cavity_height = panel_height + 2 * frame_panel_space;
panel_cavity_depth_start = faceplate_recess_depth + frame_panel_stop_depth;
frame_depth = panel_cavity_depth_start + panel_depth + spacer_thickness + board_depth + frame_rear_clearance; // Body depth of the frame in mm
panel_stop_opening_width = panel_width - 2 * frame_panel_stop_overlap;
panel_stop_opening_height = panel_height - 2 * frame_panel_stop_overlap;
frame_front_opening_width = faceplate_cutout_width - 2 * frame_faceplate_glue_lip;
frame_front_opening_height = faceplate_cutout_height - 2 * frame_faceplate_glue_lip;
backplate_width = frame_width - 2 * (frame_wall_thickness - backplate_slot_depth) - 2 * fit_tolerance;
backplate_height = frame_height - backplate_bottom_stop_height - backplate_top_clearance;
backplate_bottom_z = backplate_bottom_stop_height;

function centered_offset(outer, inner) = (outer - inner) / 2;

module rounded_visual_cube(size_xyz, color_name, alpha_value=1.0)
{
	color(color_name, alpha_value)
		cube(size_xyz);
}

module cutout_cube(offset_xyz, size_xyz)
{
	translate(offset_xyz)
		cube(size_xyz);
}

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

module rounded_cube_xz(size_xyz, radius)
{
	translate([0, size_xyz[1], 0])
		rotate([90, 0, 0])
			linear_extrude(height=size_xyz[1])
				rounded_profile_xz(size_xyz[0], size_xyz[2], radius);
}

module frame_stand_feet()
{
	left_foot_x = frame_foot_inset;
	right_foot_x = frame_width - frame_foot_inset - frame_foot_width;

	translate([left_foot_x, frame_depth, 0])
		cube([frame_foot_width, frame_foot_depth, frame_foot_height]);

	translate([right_foot_x, frame_depth, 0])
		cube([frame_foot_width, frame_foot_depth, frame_foot_height]);
}

module y_hole_from_back(x_pos, z_pos, diameter, depth)
{
	translate([x_pos, frame_depth - depth - eps, z_pos])
		rotate([-90, 0, 0])
			cylinder(d=diameter, h=depth + 2 * eps, $fn=48);
}

module local_y_hole(x_pos, y_pos, z_pos, diameter, depth)
{
	translate([x_pos, y_pos - eps, z_pos])
		rotate([-90, 0, 0])
			cylinder(d=diameter, h=depth + 2 * eps, $fn=48);
}

module board_mount_pattern(hole_diameter, depth, standoff_diameter=0)
{
	hole_positions = [
		[board_mounting_holes_offset_x, board_mounting_holes_offset_y],
		[board_width - board_mounting_holes_offset_x, board_mounting_holes_offset_y],
		[board_mounting_holes_offset_x, board_height - board_mounting_holes_offset_y],
		[board_width - board_mounting_holes_offset_x, board_height - board_mounting_holes_offset_y]
	];

	for (position = hole_positions)
	{
		translate([position[0], 0, position[1]])
		{
			if (standoff_diameter > 0)
			{
				cylinder(d=standoff_diameter, h=depth, $fn=48);
			}

			translate([0, -eps, 0])
				cylinder(d=hole_diameter, h=depth + 2 * eps, $fn=48);
		}
	}
}

module panel_mount_pattern(hole_diameter, depth)
{
	hole_positions = [
		[panel_mount_hole_inset, panel_mount_hole_inset],
		[panel_width - panel_mount_hole_inset, panel_mount_hole_inset],
		[panel_mount_hole_inset, panel_height - panel_mount_hole_inset],
		[panel_width - panel_mount_hole_inset, panel_height - panel_mount_hole_inset]
	];

	for (position = hole_positions)
	{
		translate([position[0], -eps, position[1]])
			cylinder(d=hole_diameter, h=depth + 2 * eps, $fn=48);
	}
}

module faceplate()
{
	difference()
	{
		cube([faceplate_width, faceplate_thickness, faceplate_height]);

		cutout_cube(
			[centered_offset(faceplate_width, faceplate_cutout_width), -eps, centered_offset(faceplate_height, faceplate_cutout_height)],
			[faceplate_cutout_width, faceplate_thickness + 2 * eps, faceplate_cutout_height]
		);
	}
}

module panel()
{
	difference()
	{
		cube([panel_width, panel_depth, panel_height]);
		panel_mount_pattern(panel_mount_hole_diameter, panel_depth);
	}
}

module spacer()
{
	difference()
	{
		union()
		{
			cube([spacer_width, spacer_thickness, spacer_height]);

			translate([
				centered_offset(spacer_width, board_width),
				spacer_thickness,
				centered_offset(spacer_height, board_height)
			])
				difference()
				{
					board_mount_pattern(board_mounting_hole_clearance, board_standoff_height, board_mounting_standoff_diameter);
				}
		}

		panel_mount_pattern(panel_mount_hole_diameter, spacer_thickness);

		translate([
			centered_offset(spacer_width, board_width),
			spacer_thickness,
			centered_offset(spacer_height, board_height)
		])
			board_mount_pattern(board_mounting_hole_clearance, board_standoff_height + eps);
	}
}

module board()
{
	union()
	{
		color("DarkGreen", 1.0)
			cube([board_width, board_thickness, board_height]);

		if (board_component_depth > 0)
		{
			color("DimGray", 0.9)
				translate([4, board_thickness, 4])
					cube([
						board_width - 8,
						board_component_depth,
						board_height - 8
					]);
		}
	}
}

module backplate()
{
	rounded_cube_xz(
		[backplate_width, backplate_thickness, backplate_height],
		min(backplate_corner_radius, min(backplate_width, backplate_height) / 2)
	);
}

module frame()
{
	difference()
	{
		union()
		{
			rounded_cube_xz([frame_width, frame_depth, frame_height], frame_corner_radius);
			frame_stand_feet();
		}

		// Front opening behind the glued faceplate.
		cutout_cube(
			[
				centered_offset(frame_width, frame_front_opening_width),
				-eps,
				centered_offset(frame_height, frame_front_opening_height)
			],
			[frame_front_opening_width, faceplate_recess_depth + eps, frame_front_opening_height]
		);

		// Flush front pocket for the faceplate.
		cutout_cube(
			[
				centered_offset(frame_width, faceplate_recess_width),
				-eps,
				centered_offset(frame_height, faceplate_recess_height)
			],
			[faceplate_recess_width, faceplate_recess_depth + eps, faceplate_recess_height]
		);

		// Opening in the panel stop ring. The panel is inserted from the rear and rests here.
		cutout_cube(
			[
				centered_offset(frame_width, panel_stop_opening_width),
				faceplate_recess_depth - eps,
				centered_offset(frame_height, panel_stop_opening_height)
			],
			[panel_stop_opening_width, frame_panel_stop_depth + 2 * eps, panel_stop_opening_height]
		);

		// Main rear cavity for panel, spacer, and board.
		cutout_cube(
			[
				centered_offset(frame_width, panel_cavity_width),
				panel_cavity_depth_start,
				centered_offset(frame_height, panel_cavity_height)
			],
			[panel_cavity_width, frame_depth - panel_cavity_depth_start + eps, panel_cavity_height]
		);

		// Rear guide channel (main portion above snap zone) - full width for easy slide-in.
		// Left wall rail slot - grips the left edge of the backplate in the depth (Y) direction.
		cutout_cube(
			[
				frame_wall_thickness - backplate_slot_depth,
				frame_depth - backplate_slot_wall - backplate_thickness - fit_tolerance - eps,
				backplate_bottom_z
			],
			[
				backplate_slot_depth,
				backplate_thickness + fit_tolerance + 2 * eps,
				backplate_height + eps
			]
		);

		// Right wall rail slot - grips the right edge of the backplate in the depth (Y) direction.
		cutout_cube(
			[
				frame_width - frame_wall_thickness,
				frame_depth - backplate_slot_wall - backplate_thickness - fit_tolerance - eps,
				backplate_bottom_z
			],
			[
				backplate_slot_depth,
				backplate_thickness + fit_tolerance + 2 * eps,
				backplate_height + eps
			]
		);

		// Top wall center opening - allows the backplate to be inserted from above.
		cutout_cube(
			[
				frame_wall_thickness - backplate_slot_depth,
				frame_depth - backplate_slot_wall - backplate_thickness - fit_tolerance - eps,
				frame_height - frame_wall_thickness - eps
			],
			[
				frame_width - 2 * (frame_wall_thickness - backplate_slot_depth),
				backplate_thickness + fit_tolerance + 2 * eps,
				frame_wall_thickness + 2 * eps
			]
		);
	}
}

module assembly()
{
	faceplate_position = [
		centered_offset(frame_width, faceplate_width),
		0,
		centered_offset(frame_height, faceplate_height)
	];
	panel_position = [
		centered_offset(frame_width, panel_width),
		panel_cavity_depth_start,
		centered_offset(frame_height, panel_height)
	];
	spacer_position = [
		centered_offset(frame_width, spacer_width),
		panel_cavity_depth_start + panel_depth,
		centered_offset(frame_height, spacer_height)
	];
	board_position = [
		centered_offset(frame_width, board_width),
		panel_cavity_depth_start + panel_depth + spacer_thickness + board_standoff_height,
		centered_offset(frame_height, board_height)
	];
	backplate_position = [
		frame_wall_thickness - backplate_slot_depth + fit_tolerance,
		frame_depth - backplate_slot_wall - backplate_thickness,
		backplate_bottom_z
	];

	color("SlateGray", 0.95)
		frame();

	color("Gainsboro", 1.0)
		translate(faceplate_position)
			faceplate();

	if (show_panel)
	{
		color("Black", 0.8)
			translate(panel_position)
				panel();
	}

	if (show_spacer)
	{
		color("Silver", 0.85)
			translate(spacer_position)
				spacer();
	}

	if (show_board)
	{
		translate(board_position)
			board();
	}

	color("LightSteelBlue", 0.9)
		translate(backplate_position)
			backplate();
}

if (part == "frame")
{
	frame();
}
else if (part == "faceplate")
{
	faceplate();
}
else if (part == "spacer")
{
	spacer();
}
else if (part == "backplate")
{
	backplate();
}
else
{
	assembly();
}

