// This file is part of GNBot (https://github.com/carlosgs/GNBot)
// by Carlosgs (http://carlosgs.es)
// CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

LDR_angle = 45;

LDR_count = 4;

LDR_radius = 18/2;

drill_resolution = 40;
drill_diameter = 3.5;

LDR_width = 6;
LDR_height = 5;
LDR_depth = 2;

conn_height = 1.5;
conn_width = 3;
conn_length = 14;

piece_height = 25;
top_radius = 20;

cover_thickness = 1;
cover_margin = 1;
cover_height = piece_height-5;

module LDR_sensor_array_support() {
    intersection() {
        difference() {
            cylinder(r=LDR_radius+LDR_depth,h=piece_height,$fn=50);
            cube([LDR_width,LDR_width,100],center=true);

            for (i = [0:LDR_count-1])
              rotate([0,0,i*360/LDR_count+LDR_angle]) translate([LDR_radius+LDR_depth/2,0,piece_height/2])
                cube([LDR_depth,LDR_width,2*piece_height],center=true);

            translate([0,0,conn_height/2-0.01])
              cube([conn_width,conn_length,conn_height],center=true);


        }

        union() {
            cylinder(r=top_radius,h=piece_height-top_radius,$fn=50);
            translate([0,0,piece_height-top_radius]) sphere(r=top_radius,$fn=100);
        }

    }//intersection
}


module LDR_sensor_array_support_cover() {
color([0.9,0.9,0.9,0.6])
intersection() {
    difference() {
        cylinder(r=LDR_radius+LDR_depth+cover_margin+cover_thickness,h=cover_height,$fn=50);
        translate([0,0,cover_height/2])
            cylinder(r=LDR_radius+LDR_depth+cover_margin,h=cover_height+0.1,$fn=50,center=true);
    }

}//intersection
}

LDR_sensor_array_support();
LDR_sensor_array_support_cover();
