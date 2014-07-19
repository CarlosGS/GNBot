// This file is part of GNBot (https://github.com/carlosgs/GNBot)
// by Carlosgs (http://carlosgs.es)
// CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

LDR_angle = 45;

LDR_count = 4;

LDR_radius = 18/2;

cyl_btm_r = 15/2;

drill_resolution = 40;
drill_diameter = 3.5;

LDR_width = 6;
LDR_height = 5;
LDR_depth = 4;

piece_height = 40;
top_radius = 16;

cover_thickness = 1;
cover_margin = 1;
cover_height = piece_height-5;

screw_support_thickness = 3;
nut_d = 7.3; // M3
screw_d = 4.1; // M3

ring_h = 24;
ring_thick = 1.4;

// Linear interpolation to fit the ring to the cone shape
ring_r = 0.5+(1-ring_h/piece_height)*cyl_btm_r+(ring_h/piece_height)*(LDR_radius+LDR_depth);

module LDR_sensor_array_support() {
    intersection() {
        difference() {
            cylinder(r1=cyl_btm_r,r2=LDR_radius+LDR_depth,h=piece_height,$fn=50);
            translate([0,0,screw_support_thickness]) // nut slot
                cylinder(r=nut_d/2,h=100,$fn=6,center=false);
            cylinder(r=screw_d/2,h=100,$fn=20,center=true); // screw drill

            for (i = [0:LDR_count-1])
              rotate([0,0,i*360/LDR_count+LDR_angle]) translate([LDR_radius+LDR_depth/2,0,piece_height/2])
                cube([LDR_depth,LDR_width,2*piece_height],center=true);

            translate([0, 0, ring_h])
                rotate_extrude($fn = 50)
                    translate([ring_r, 0, 0])
                        rotate([0,0,45])
                            polygon([[-ring_thick,ring_thick],[2,2],[0,0]]);
                            //square([ring_thick,ring_thick]);
        }

        union() {
            cylinder(r=top_radius,h=piece_height-top_radius,$fn=50);
            translate([0,0,piece_height-top_radius]) sphere(r=top_radius,$fn=100);
        }

    }//intersection
}

LDR_sensor_array_support();
