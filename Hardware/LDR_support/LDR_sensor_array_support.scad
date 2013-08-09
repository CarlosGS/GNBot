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

intersection() {
    difference() {
        cylinder(r=LDR_radius+LDR_depth,h=piece_height,$fn=50);
        //cylinder(r=8/2,h=200,center=true,$fn=drill_resolution);
        cube([LDR_width,LDR_width,100],center=true);

        for (i = [0:LDR_count-1])
          rotate([0,0,i*360/LDR_count+LDR_angle]) translate([LDR_radius+LDR_depth/2,0,piece_height/2])
            cube([LDR_depth,LDR_width,2*piece_height],center=true);

        //for (i = [0:2])
        //  rotate([0,0,i*360/3+90]) translate([8,0,0])
	     //   cylinder(r=drill_diameter/2,h=10,center=true,$fn=drill_resolution);

        //translate([0,0,piece_height])
        //  cube([13.5,13.5,4*2],center=true);

        translate([0,0,conn_height/2-0.01])
          cube([conn_width,conn_length,conn_height],center=true);


    }

    union() {
        cylinder(r=top_radius,h=piece_height-top_radius,$fn=50);
        translate([0,0,piece_height-top_radius]) sphere(r=top_radius,$fn=100);
    }

}//intersection


// http://www.thingiverse.com/thing:9347
module roundedRect2D(size, radius)
{
    $fn=50;
    x = size[0]-radius*2;
    y = size[1]-radius*2;
    z = size[2];

    minkowski()
    {
        cube(size=[x,y,z],center=true);
        cylinder(r=radius,h=0.00001);
    }
}

