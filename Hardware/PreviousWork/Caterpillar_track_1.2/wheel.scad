// stretchy bracelet

r2=40/2;// major radius
h=6;// thickness (difference in radius between inside and outside
w=15;// width
t=1;// layer width and gap width
n=20;// number of twisty shapes around the circle
m=21;// number of bendy parts around the circle

internal_radius = 6*0.95;	// 0.95 factor is to make the contact point between the
							// track and the wheel in the center rather than the sides

pi=3.14159;
rr=pi*r2/n;
r1=rr*1.5;
ro=r2+(r1+rr)*0.5;
ri=ro-h;
a=pi*2*ri/m-t;
echo("Inside Diameter = ",2*ri);

tooth_width = 0.7/2;
tooth_depth = 3;
servo_head_radius = 21.3/2;
servo_head_depth = 2;
axis_radius = 4/2;

tooth_lenght_correction = 0;

union() {
difference() {
intersection() {
cylinder(r=r2+10,h=w,$fn=m);
union(){
cylinder(r=r2-1,h=w,$fn=50);
translate([0,0,w/2])scale([1,1,2])rotate_extrude($fn=100)
	translate([ro-sqrt(internal_radius)-internal_radius-h/5,0,0])circle(r=internal_radius,$fn=50);
}
}
cylinder(r=axis_radius,h=w*4,$fn=50,center=true);
translate([0,0,w-servo_head_depth]) cylinder(r=servo_head_radius,h=servo_head_depth*2,$fn=50);
}

intersection() {
	for(i=[1:m])rotate([0,0,i*360/m])
		translate([0,0,-0.03+tooth_lenght_correction/2])linear_extrude(height=w+0.06-tooth_lenght_correction)
			polygon(points=[[ri+t,a/2-t-tooth_width],[ri+t,t+tooth_width-a/2],[ro-tooth_depth+t*h/a,0]],paths=[[0,1,2]]);

translate([0,0,w/2])scale([1,1,2])rotate_extrude($fn=100)
	translate([ro-sqrt(internal_radius*0.3)-internal_radius*0.9-h/5,0,0])circle(r=internal_radius*0.9,$fn=50);
}

}

//solid();
//hollow();

module solid(){
difference(){
	// Uncomment one of these three lines for different styles
	//base(r1=r1,w=w);// original angled cording
	//base1(r1=r1,w=w);// horizontal cording
	//cylinder(r=ro,h=w,$fn=m);// flat
	base2(r1=r1,w=w);
	
	difference() {
	for(i=[1:m])rotate([0,0,i*360/m])
		translate([0,0,-0.03])linear_extrude(height=w+0.06)
			polygon(points=[[ri+t,a/2-t],[ri+t,t-a/2],[ro+t*h/a,0]],paths=[[0,1,2]]);

	translate([0,0,w/2])scale([1,1,2])rotate_extrude($fn=100)
	translate([ro-sqrt(internal_radius)-internal_radius-h/5,0,0])circle(r=internal_radius,$fn=20);
	}
}}

module hollow(){
difference(){
	base(r1=r1,w=w);
	difference(){
		translate([0,0,-0.01])base(r1=r1-t,w=w+0.02);
		for(i=[1:m])rotate([0,0,i*360/m])
			translate([0,0,-0.02])linear_extrude(height=w+0.04)
				polygon(points=[[ri,a/2],[ri,-a/2],[ro+3*t*h/a,0]],paths=[[0,1,2]]);
	}
	for(i=[1:m])rotate([0,0,i*360/m])
		translate([0,0,-0.03])linear_extrude(height=w+0.06)
			polygon(points=[[ri+t,a/2-t],[ri+t,t-a/2],[ro+t*h/a,0]],paths=[[0,1,2]]);
}}

module base(r1,w){
render()
union(){	
	cylinder(r=r2+rr*0.5,h=w);
	for(i=[1:n]){
		rotate([0,0,i*360/n])translate([0,-r2,0])
		scale([1,0.5,1])linear_extrude(height=w,twist=180,slices=10)
			translate([rr,0,0])circle(r=r1,$fn=20);
	}
}}

module base1(r1,w){
union(){
	cylinder(r=ro-1.5,h=w,$fn=100);
	for(i=[1:3]){
		translate([0,0,(i-1)*4.5+3])scale([1,1,2])rotate_extrude($fn=100)
			translate([ro-1.5,0,0])circle(r=1.5,$fn=20);
	}
}}

module base2(r1,w){
union(){
	cylinder(r=ro-1.5,h=w,$fn=100);
	for(i=[1:3]){
		translate([0,0,(i-1)*4.5+3])scale([1,1,2])rotate_extrude($fn=100)
			translate([ro-2,0,0])circle(r=1,$fn=20);
	}
}}