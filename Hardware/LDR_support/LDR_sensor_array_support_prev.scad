// This file is part of GNBot (https://github.com/carlosgs/GNBot)
// by Carlosgs (http://carlosgs.es)
// CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

drill_resolution = 40;
drill_diameter = 3.5;

LDR_width = 6;
LDR_height = 5;

piece_height = 12;

intersection() {
    difference() {

        //cylinder(r=25/2,h=piece_height,$fn=50);
        translate([0,0,piece_height/2])
          roundedRect2D([25,25,piece_height], 9);

        //cylinder(r=8/2,h=200,center=true,$fn=drill_resolution);
        cube([LDR_width,LDR_width,100],center=true);

        for (i = [0:2])
          rotate([0,0,i*360/3+90]) translate([8,0,0])
	        cylinder(r=drill_diameter/2,h=10,center=true,$fn=drill_resolution);

        translate([0,0,piece_height])
          cube([13.5,13.5,4*2],center=true);

        translate([0,0,2+LDR_height/2]) {
            cube([LDR_width,100,LDR_height],center=true);
            cube([100,LDR_width,LDR_height],center=true);
        }


    }

    sphere(r=15,$fn=50);

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

